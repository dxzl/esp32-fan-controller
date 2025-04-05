// this file HttpClientHandlers.cpp
#include "Gpc.h"

// HTTP Client: https://github.com/khoih-prog/AsyncHTTPRequest_Generic
// needed by AsyncHTTPRequest_Generic.h library (having to do with mutex locks
// NOTE: we don't want any mutex locks in that library (they will hang the system!)
#define ESP32 true
#include "AsyncHTTPRequest_Generic.h"

// have to put prototype here instead of in HttpClientHandlers.h because includes above won't work if put in the .h file!
void HttpClientCallback(void* optParm, AsyncHTTPRequest* request, int readyState);

AsyncHTTPRequest asyncHttpreq;

void InitHttpClientHandlers(){
  Serial.println(ASYNC_HTTP_REQUEST_GENERIC_VERSION);
#if defined(ASYNC_HTTP_REQUEST_GENERIC_VERSION_MIN)
  if (ASYNC_HTTP_REQUEST_GENERIC_VERSION_INT < ASYNC_HTTP_REQUEST_GENERIC_VERSION_MIN){
    Serial.print(F("Warning. Must use this example on Version equal or later than : "));
    Serial.println(ASYNC_HTTP_REQUEST_GENERIC_VERSION_MIN_TARGET);
  }
#endif
  asyncHttpreq.setDebug(false);
  asyncHttpreq.setTimeout(DEF_HTTP_TIMEOUT);
  asyncHttpreq.onReadyStateChange(HttpClientCallback);
}

// HTTPCODE_CONNECTION_REFUSED  (-1)
// HTTPCODE_SEND_HEADER_FAILED  (-2)
// HTTPCODE_SEND_PAYLOAD_FAILED (-3)
// HTTPCODE_NOT_CONNECTED       (-4) // this happens a lot... ignore
// HTTPCODE_CONNECTION_LOST     (-5)
// HTTPCODE_NO_STREAM           (-6)
// HTTPCODE_NO_HTTP_SERVER      (-7)
// HTTPCODE_TOO_LESS_RAM        (-8)
// HTTPCODE_ENCODING            (-9)
// HTTPCODE_STREAM_WRITE        (-10)
// HTTPCODE_TIMEOUT             (-11)
void ClearLinkOk(String sIp, int code){
  int idx = IML.FindMdnsIp(sIp);
  if (idx >= 0){
    IML.SetFlag(idx, MDNS_FLAG_LINK_OK, false); // Link broken! (mDNS search will set flag when link back...)
    prtln("ClearLinkOk(): clearing MDNS_FLAG_LINK_OK flag. code=" + String(code) + ", IP=" + sIp);
  }
  else
    prtln("ClearLinkOk(): ip not found: " + sIp);
}

// return false if not sent, true if sent
bool SendHttpCanRxReq(String sIp){
  if (!g_bWiFiConnected || sIp.isEmpty())
    return false;

  if (asyncHttpreq.readyState() != readyStateUnsent && asyncHttpreq.readyState() != readyStateDone)
    return false;
  
  int ipIdx = IML.FindMdnsIp(sIp);
  if (ipIdx < 0)
    return false;
    
  prtln("SendHttpCanRxReq() sending to ip: " + sIp);

  if (!IML.GetFlag(ipIdx, MDNS_FLAG_LINK_OK)){
    prtln("SendHttpCanRxReq() Can't send because linkOK flag is false");
    return false;
  }
  
  int iTokCanRxHigh = random(0,7+1); // 0-7
  int iTokCmdStrHigh = random(0,7+1); // 3 high bits (of 6 bits total... 0-63)
  int iTokShifted = (iTokCmdStrHigh<<4)|(iTokCanRxHigh<<1)|1; // add shift-marker-bit
  iTokShifted <<= random(0,MAX_CANRX_TOKEN_SHIFT+1);

  int iTokEncode = IML.GetToken(ipIdx, MDNS_TOKEN_CANRX_TX);
  
  if (iTokEncode == NO_TOKEN)
    iTokEncode = GetRandToken(); // this random token is designed to fail... use of g_defToken would be a security problem.
    
  String sTokShifted = MyEncodeNum(iTokShifted, HTTP_TABLE3, iTokEncode, CIPH_CONTEXT_FOREGROUND);
  String sEnc = String(HTTP_COMMAND_CANRX);
  int iErr = MyEncodeStr(sEnc, HTTP_TABLE1, iTokEncode, CIPH_CONTEXT_FOREGROUND);
  if (iErr) return false;
  String sParam1 = String(HTTP_ASYNCREQ_CANRX_PARAM_COMMAND);
  iErr = MyEncodeStr(sParam1, HTTP_TABLE2, iTokEncode, CIPH_CONTEXT_FOREGROUND);
  if (iErr) return false;
  String sParam2 = String(HTTP_ASYNCREQ_CANRX_PARAM_TOK3BITS);
  iErr = MyEncodeStr(sParam2, HTTP_TABLE2, iTokEncode, CIPH_CONTEXT_FOREGROUND);
  if (iErr) return false;
  String sReq = "http://" + sIp + String(HTTP_ASYNCREQ_CANRX) + '?' + sParam1 + '=' + sEnc + '&' +
                                              sParam2 + '=' + sTokShifted + '&'; // transmit high bits << 4

  if (asyncHttpreq.open("GET", sReq.c_str())){
    IML.SetToken(ipIdx, MDNS_TOKEN_SAVE, iTokShifted); // save pending high bits for both tokens
    IML.SetToken(ipIdx, MDNS_TOKEN_TX, NO_TOKEN); // (should already be NO_TOKEN - reason for calling this!)
    IML.SetStr(ipIdx, MDNS_STRING_KEY, sTokShifted); // save for use as a key to decrypt g_defToken for HTTPCODE_CANRX_DECFAIL response in TASK_HTTPCALLBACK
    String sMac = '&' + MyEncodeNum(GetOurDeviceMacLastTwoOctets(), HTTP_TABLE2, FAILSAFE_TOKEN_3, CIPH_CONTEXT_FOREGROUND);
    asyncHttpreq.setReqHeader(HTTP_CLIENT_MAC_HEADER_NAME, sMac.c_str());
    asyncHttpreq.onReadyStateChange(HttpClientCallback, (void*)IpToUInt(sIp));
    asyncHttpreq.send();
    prtln("SendHttpCanRxReq() sent HTTP_ASYNCREQ_CANRX: \"" + sReq + "\"");
    return true;
  }
  
  prtln("SendHttpCanRxReq() Can't send request");
  return false;
}

// send the sSendAll string in the t_indexMdns struct in IndexMdnsList.h for idx in IML.arr[]
// return false if not sent, true if sent
//
// Non-masters don't normally talk to one another (through the router) in order to prevent exponential
// traffic-growth on the network. Instead the master forwards or "echos" commands from non-masters to other non-
// masters by packaging (or repackaging) them into CMfrom commands. Specifically addressed command bundles
// are sent to/through the master from a non-master using the CMto command. CMto commands have the "to" IP
// address prepended as a uint32_t string with a CM_SEP. The CMto is repackaged at the master as a CMfrom
// with the sender's IP address prepended in place of the destinaltion IP. Non-masters only SendHttpReq() to
// the master unit in response to the master's SendHttpReq(). We now use the original sSendAll in the mDNS
// struct for "global commands" - commands intended for all units. A new string has been added sSendSpecific
// that will keep commands addressed to a specific unit. sSendSpecific commands are gathered and packaged by
// the non-master as seperate "CMto(ip_address):(base64 encoded string of commands)" commands for each mDNS
// entry at the time of SendHttpReq() to the master.
bool SendHttpReq(String sIp){
  if (!g_bWiFiConnected || sIp.isEmpty())
    return false;

  if (asyncHttpreq.readyState() != readyStateUnsent && asyncHttpreq.readyState() != readyStateDone)
    return false;
  
  int ipIdx = IML.FindMdnsIp(sIp);
  if (ipIdx < 0)
    return false;

  prtln("SendHttpReq() sending to ip: " + sIp);
  
  if (!IML.GetFlag(ipIdx, MDNS_FLAG_LINK_OK)){
    prtln("SendHttpReq() Can't send because linkOK flag is false");
    return false;
  }
  
  // if we've exceeded MAX_HTTP_CLIENT_SENDS, delete the mDNS entry for this IP
  int sendCount = IML.GetSendCount(ipIdx);
  if (sendCount > MAX_HTTP_CLIENT_SENDS){
    IML.DelMdnsIp(ipIdx);
    prtln("SendHttpReq() sendCount exceeded, deleted mDNS ip");
    return false;
  }

  int txToken = IML.GetToken(ipIdx, MDNS_TOKEN_TX);  

  // if our last transmit for this IP failed, the HTTP callback task (tasks.cpp) will set txNextToken and txToken to NO_TOKEN
  // - we must query the remote IP for a new token which, when received, will be placed in txToken...
  if (txToken == NO_TOKEN){
    prtln("SendHttpReq() unable to send. token == NO_TOKEN");
    return false;
  }

  // check conditions for abort of send...
  if (!g_bSyncTx){
    prtln("SendHttpReq() Can't send because g_bSyncTx is false!");
    return false;
  }

  if ((uint32_t)g_IpMaster == 0){
    prtln("SendHttpReq() Can't send because a master unit has not been designated yet, g_IpMaster is 0!");
    return false;
  }

  // check for special case of "we are not a master" and the master in the mDNS array is NOT ipIdx
  //
  // if g_bMaster is false and we HAVE a master in the mDNS array and that index is NOT ipIdx... that's a
  // special case - we should send to ipIdx its sSendSpecific ONLY. But that would not be normal operation.
  if (!g_bMaster && g_IpMaster != IML.GetIP(ipIdx)){
    prtln("SendHttpReq() Sending slave-to-slave not allowed (you must route commands through the master)!");
    return false;
  }
  
  // if g_bMaster is false then we are a slave and one of the mDNS entries is the master.
  // We need to combine sSendSpecific for each mDNS entry into CMto bundles and send them, together
  // with the master's sSendAll, to the master. ipIdx should be the master's index! Should we merge
  // sSendAll strings before sending? I don't think we need to because they should all be identical!
  //
  // if g_bMaster is true then none of the mDNS entries is master, we are. We expect that all incomming
  // CMto bundles have been repackaged as CMfrom commands. The master just needs to send sSendAll + sSendSpecific
  // to ipIdx.
  //
  // NOTE: The master needs to clear sSendAll and sSendSpecific for ipIdx upon callback response. The slave needs to clear
  // these strings for ALL mDNS entries on callback response!
  String sCmd;
  if (g_bMaster)
    sCmd = IML.GetStr(ipIdx, MDNS_STRING_SEND_SPECIFIC) + IML.GetStr(ipIdx, MDNS_STRING_SEND_ALL);
  else{
    // add "CMto(ipaddress as uint32_t)(base64 encoded data)" to sCmd
    int iCount = IML.GetCount();
    for (int ii=0; ii < iCount; ii++){
      String sSendSpecific = IML.GetStr(ii, MDNS_STRING_SEND_SPECIFIC);
      if (!sSendSpecific.isEmpty())
        HMC.AddCommand(CMto, B64C.hnEncNumOnly((uint32_t)IML.GetIP(ii)) + CM_SEP + sSendSpecific, sCmd);
    }
    String sSendAll = IML.GetStr(ipIdx, MDNS_STRING_SEND_ALL);
    if (!sSendAll.isEmpty())
      HMC.AddCommand(CMto, B64C.hnEncNumOnly(0) + CM_SEP + sSendAll, sCmd); // add commands intended for all units...
  }
  
//prtln("DEBUG: sCmd after encode: \"" + sCmd + "\"");
  
  int iErr = 0;  
  String sPrevProcCode = IML.GetStr(ipIdx, MDNS_STRING_RXPROCCODE); // saved in TaskProcessReceiveString()
  prtln("SendHttpReq() previous send's result-code, sPrevProcCode: \"" + sPrevProcCode + "\"");
  if (!sPrevProcCode.isEmpty()){
    if (!alldigits(sPrevProcCode)){
      prtln("SendHttpReq() bad sPrevProcCode (should be all digits!): \"" + sPrevProcCode + "");
      sPrevProcCode = "";
    }
    else{
      int iProcCode = sPrevProcCode.toInt();
      sPrevProcCode = ""; // now sPrevProcCode will be encoded to send to remote...
      if (iProcCode != 0){
        iProcCode <<= 4;
        iProcCode |= random(0, 16);
        sPrevProcCode = String(iProcCode);
        iErr = MyEncodeStr(sPrevProcCode, HTTP_TABLE2, txToken, CIPH_CONTEXT_FOREGROUND);
        if (iErr){
          prtln("SendHttpReq() bad sPrevProcCode encode, iErr=" + String(iErr));
          return false;
        }
      }
    }
    IML.SetStr(ipIdx, MDNS_STRING_RXPROCCODE, "");
  }
    
  if (!sCmd.isEmpty()){
//    prtln("DEBUG: sCmd before encode: \"" + sCmd + "\"");

    // generate and add txNextToken plus checksum to sCmd
    iErr = HMC.EncodeTxTokenAndChecksum(ipIdx, sCmd, false);
    if (iErr){
      prtln("SendHttpReq() EncodeTxTokenAndChecksum(), iErr=" + String(iErr));
      return false;
    }

    iErr = MyEncodeStr(sCmd, HTTP_TABLE2, txToken, CIPH_CONTEXT_FOREGROUND);
    if (iErr){
      prtln("SendHttpReq() bad sCmd encode, iErr=" + String(iErr));
      return false;
    }
  }
  
  if (sCmd.isEmpty() && sPrevProcCode.isEmpty()){
    prtln("SendHttpReq() nothing to send!");
    return false;
  }
  
  String sReq = "http://" + sIp + HTTP_ASYNCREQ + '?';

  if (!sCmd.isEmpty()){
    String sEnc = String(HTTP_ASYNCREQ_PARAM_COMMAND);
    iErr = MyEncodeStr(sEnc, HTTP_TABLE1, txToken, CIPH_CONTEXT_FOREGROUND);
    if (iErr){
      prtln("SendHttpReq() bad HTTP_ASYNCREQ_PARAM_COMMAND encode, iErr=" + String(iErr));
      return false;
    }
    sReq += sEnc + '=' + sCmd + '&';
    prtln("SendHttpReq() sending command-string \"" + sCmd + "\"");
  }
  
  if (!sPrevProcCode.isEmpty()){
    String sEnc = String(HTTP_ASYNCREQ_PARAM_PROCCODE);
    iErr = MyEncodeStr(sEnc, HTTP_TABLE1, txToken, CIPH_CONTEXT_FOREGROUND);
    if (iErr){
      prtln("SendHttpReq() bad HTTP_ASYNCREQ_PARAM_PROCCODE encode, iErr=" + String(iErr));
      return false;
    }
    sReq += sEnc + '=' + sPrevProcCode + '&';
    prtln("SendHttpReq() sending random encoded sPrevProcCode of: \"" + sPrevProcCode + "");
  }
  
  if (asyncHttpreq.open("GET", sReq.c_str())){
    asyncHttpreq.onReadyStateChange(HttpClientCallback, (void*)IpToUInt(sIp));
    asyncHttpreq.send();
    IML.SetSendCount(ipIdx, ++sendCount);
//prtln("DEBUG: sending \"" + sReq + "\"");
    return true;
  }
  
  prtln("SendHttpReq() Can't send request");

  return false;
}

// http async request callback
// AsyncHTTPRequest* request:
//  size_t responseLength(); // indicated response length or sum of chunks to date
//  int responseHTTPcode(); // HTTP response code or (negative) error code
//  String responseText(); // response (whole* or partial* as string)
//  char* responseLongText(); // response long (whole* or partial* as string)
//  size_t responseRead(uint8_t* buffer, size_t len); // Read response into buffer
// readyStateUnsent      = 0, // Client created, open not yet called
// readyStateOpened      = 1, // open() has been called, connected
// readyStateHdrsRecvd   = 2, // send() called, response headers available
// readyStateLoading     = 3, // receiving, partial data available
// readyStateDone        = 4  // Request complete, all data available.
//
// HTTPCODE_CONNECTION_REFUSED  (-1)
// HTTPCODE_SEND_HEADER_FAILED  (-2)
// HTTPCODE_SEND_PAYLOAD_FAILED (-3)
// HTTPCODE_NOT_CONNECTED       (-4) (get this after unplugging a unit...)
// HTTPCODE_CONNECTION_LOST     (-5)
// HTTPCODE_NO_STREAM           (-6)
// HTTPCODE_NO_HTTP_SERVER      (-7)
// HTTPCODE_TOO_LESS_RAM        (-8)
// HTTPCODE_ENCODING            (-9)
// HTTPCODE_STREAM_WRITE        (-10)
// HTTPCODE_TIMEOUT             (-11)
// HTTPCODE_                    (-13) // seem to get this for timeout...
//HTTPCODE_OK           200      // standard HTTP success code, don't change!
//HTTPCODE_CANRX_OK     244      // response data has other 3-bits of new Rx/Tx token pair shifted left 5
//HTTPCODE_PROCESSING_OK 255
//HTTPCODE_FAIL           400         // standard HTTP fail code, don't change!
//HTTPCODE_NOIP_FAIL      501
//HTTPCODE_RXTOKEN_FAIL   502
//HTTPCODE_RXDISABLED     503 // sent to HttpClientCallback if "c sync rx off" command (g_bSyncRx = false)
//HTTPCODE_PARAM_TOOSHORT 504
//HTTPCODE_PARAM_TOOLONG  505
//HTTPCODE_ADDIP_FAIL     510
//HTTPCODE_CANRX_FAIL     511
//HTTPCODE_CANRX_DECODE_FAIL 512 // response data has new default token to set (can be 255 = NO_TOKEN)
void HttpClientCallback(void* optParm, AsyncHTTPRequest* request, int readyState){
//  (void) optParm;
  if (!optParm || readyState != readyStateDone) // request->available() 
    return;
    
  String sRemIp = UIntToIp((uint32_t)optParm);
  int httpCode = request->responseHTTPcode();

  if (httpCode < 0){
    // HTTPCODE_NOT_CONNECTED, happens a lot
    if (httpCode != HTTPCODE_NOT_CONNECTED)
      ClearLinkOk(sRemIp, httpCode);
  }
  else{
    String sCode = GetHttpCodeString(httpCode);
    bool bUnknown = (sCode == "UNKNOWN");
    if (bUnknown)
      prtln("HttpClientCallback(): unknown code=" + String(httpCode) + ": \"" + request->responseHTTPString() + "\"");
    else
      prtln("HttpClientCallback(): code=HTTPCODE_" + sCode);

    String sMac;
    
    if (request->respHeaderExists(HTTP_SERVER_MAC_HEADER_NAME))
      sMac = request->respHeaderValue(HTTP_SERVER_MAC_HEADER_NAME);
      
    if (!bUnknown && httpCode != HTTPCODE_RXDISABLED){
      String sRsp = request->responseText();
      // TaskHttpCallback(), one unused int field...
      TSK.QueueTask(TASK_HTTPCALLBACK, 0, httpCode, sRsp, sMac, sRemIp);
    }
      
    // don't clear LinkOk yet - let it retry via SendHttpReq()/IML.GetSendCount()
    //else
    //  ClearLinkOk();
  }
  request->setDebug(false);
}

// HTTPCODE_OK             200      // standard HTTP success code, don't change!
// HTTPCODE_CANRX_OK       244      // response data has other 3-bits of new Rx/Tx token pair shifted left 5
// HTTPCODE_PROCESSING_OK  255
// HTTPCODE_FAIL           400      // standard HTTP fail code, don't change!
// HTTPCODE_NOIP_FAIL      501
// HTTPCODE_RXTOKEN_FAIL   502
// HTTPCODE_RXDISABLED     503      // sent to HttpClientCallback if "c sync rx off" command (g_bSyncRx = false)
// HTTPCODE_PARAM_TOOSHORT 504
// HTTPCODE_PARAM_TOOLONG  505
// HTTPCODE_ADDIP_FAIL     510
// HTTPCODE_CANRX_FAIL     511
// HTTPCODE_CANRX_DECODE_FAIL  513 // response data has new default token to set (can be 255 = NO_TOKEN)
String GetHttpCodeString(int httpCode){
  String sPrt;
  if (httpCode == HTTPCODE_OK)
    sPrt = "OK";
  else if (httpCode == HTTPCODE_CANRX_OK)
    sPrt = "CANRX_OK";
  else if (httpCode == HTTPCODE_PROCESSING_OK)
    sPrt = "PROCESSING_OK";
  else if (httpCode == HTTPCODE_FAIL)
    sPrt = "FAIL";
  else if (httpCode == HTTPCODE_NOIP_FAIL)
    sPrt = "NOIP_FAIL";
  else if (httpCode == HTTPCODE_RXTOKEN_FAIL)
    sPrt = "RXTOKEN_FAIL";
  else if (httpCode == HTTPCODE_RXDISABLED)
    sPrt = "RXDISABLED";
  else if (httpCode == HTTPCODE_PARAM_TOOSHORT)
    sPrt = "PARAM_TOOSHORT";
  else if (httpCode == HTTPCODE_PARAM_TOOLONG)
    sPrt = "PARAM_TOOLONG";
  else if (httpCode == HTTPCODE_ADDIP_FAIL)
    sPrt = "ADDIP_FAIL";
  else if (httpCode == HTTPCODE_CANRX_FAIL)
    sPrt = "CANRX_FAIL";
  else if (httpCode == HTTPCODE_CANRX_DECODE_FAIL)
    sPrt = "CANRX_DECODE_FAIL";
  else
    sPrt = "UNKNOWN";
  return sPrt;
}
