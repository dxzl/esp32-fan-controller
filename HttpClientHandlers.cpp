// this file HttpClientHandlers.cpp
#include "FanController.h"

// HTTP Client: https://github.com/khoih-prog/AsyncHTTPRequest_Generic
#include <AsyncHTTPRequest_Generic.h>
#include <AsyncHTTPRequest_Impl_Generic.h>

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

void ClearLinkOk(int code){
  if (g_httpTxIP.toString() != "0.0.0.0"){
    int idx = IML.FindMdnsIp(g_httpTxIP);
    if (idx >= 0){
      IML.SetLinkOkFlag(idx, false); // Link broken! (mDNS search will set flag when link back...)
      IML.SetSendOkFlag(idx, false);
      IML.SetSendCount(idx, 0);
      prtln("Error for async http request, clearing mDNS bLinkOk flag. code=" + String(code) + ", IP=" + String(g_httpTxIP.toString()));
    }
  }
  else{
    prtln("ip is 0.0.0.0 in HttpClientCallback!");
    prtln("Error for async http request. code=" + String(code));
  }
}

//int SendHttpText(String sIP, String sText){
//  if (sIP.isEmpty())
//    return -2;  
//  IPAddress ip;
//  ip.fromString(sIP);
//  int idx = IML.FindMdnsIp(ip);
//  return SendHttpText(idx, sText);
//}
//int SendHttpText(int idx, String sText){
//  if (!g_bWiFiConnected)
//    return -2;  
//
//  if (asyncHttpreq.readyState() != readyStateUnsent && asyncHttpreq.readyState() != readyStateDone)
//    return -3;
//
//  if (sText.isEmpty() || sText.length() > HTTP_TEXT_MAXCHARS)
//    return -4;  
//
//  int count = IML.GetCount();
//  
//  if (!count || idx < 0 || idx >= count)
//    return -5;
//
//  int txToken = IML.GetTxToken(idx);
//  sText = MyEncodeStr(sText, HTTP_TABLE3, txToken, CIPH_CONTEXT_FOREGROUND);
//  if (sText.isEmpty())
//    return -7;
//
//  IPAddress ip = IML.GetIP(idx);
//  
//  sText = "http://" + ip.toString() + HTTP_ASYNCTEXTREQ + '?' +
//                              HTTP_ASYNCTEXTREQ_PARAM_TEXT + '=' + sText + '&';
//
//  if (!asyncHttpreq.open("GET", sText.c_str()))
//    return -8;
//
//  g_httpTxIP = ip; // save it for use in callback!
//  asyncHttpreq.send();
//  return 0;
//}

bool SendHttpCanRxReq(int idx){
  if (!g_bWiFiConnected)
    return false;  

  if (asyncHttpreq.readyState() != readyStateUnsent && asyncHttpreq.readyState() != readyStateDone)
    return false;

  if (IML.GetSaveToken(idx) != NO_TOKEN){ // save pending high bits + 1
    prtln("SendHttpCanRxReq(): WARNING! Callback from previous SendHttpCanRxReq() not processed yet!");
  }
  
  IPAddress ip = IML.GetIP(idx);
  String sEnc = MyEncodeStr(HTTP_COMMAND_CANRX, HTTP_TABLE2, g_defToken, CIPH_CONTEXT_FOREGROUND);
  int iTokHigh = random(1,8+1); // 3 high bits (of 6 bits total... 0-63) (note: +1 to avoid sending 0!)
  int tokShifted = iTokHigh<<CANRX_TOKEN_SHIFT;
  String sTokHigh = MyEncodeNum(tokShifted, HTTP_TABLE1, g_defToken, CIPH_CONTEXT_FOREGROUND);
  if (sTokHigh.isEmpty()){
    prtln("SendHttpCanRxReq(): Can't encode sTokHigh for tokShifted=" + String(tokShifted));
    return true; // move on to next IP
  }
  String sReq = "http://" + ip.toString() + HTTP_ASYNCCANRXREQ + '?' + HTTP_PARAM_COMMAND + '=' + sEnc + '&' +
    HTTP_ASYNCCANRXREQ_PARAM_TOK3BITS + '=' + sTokHigh + '&'; // transmit high bits << 4

  if (asyncHttpreq.open("GET", sReq.c_str())){
    g_httpTxIP = ip; // save it for use in callback!
    IML.SetSaveToken(idx, iTokHigh); // save pending high bits + 1
    IML.SetTxToken(idx, NO_TOKEN); // (should already be NO_TOKEN - reason for calling this!)
    IML.SetUtilStr(idx, sTokHigh); // save for use as a key to decrypt g_defToken for HTTPCODE_CANRX_DECFAIL response in TASK_HTTPCALLBACK
    String sMac = '&' + MyEncodeNum(IML.GetOurDeviceMacLastTwoOctets(), HTTP_TABLE2, FAILSAFE_TOKEN_3, CIPH_CONTEXT_FOREGROUND);
    asyncHttpreq.setReqHeader(HTTP_CLIENT_MAC_HEADER_NAME, sMac.c_str());
    asyncHttpreq.send();
    prtln("SendHttpCanRxReq() sent HTTP_ASYNCCANRXREQ: \"" + sReq + "\"");
  }
  else
    prtln("SendHttpCanRxReq() Can't send request");

  return true;
}

// send the sSend string in the t_indexMdns struct in IndexMdnsList.h for idx in IML.arr[]
bool SendHttpReq(int idx){
  if (!g_bWiFiConnected || (asyncHttpreq.readyState() != readyStateUnsent && asyncHttpreq.readyState() != readyStateDone))
    return false; // don't advance index

  // don't send if link is broken, but advance to next mDNS entry
  if (!g_bSyncTx || !IML.GetLinkOkFlag(idx)){
    prtln("DEBUG: SendHttpReq() Can't send because either the linkOK flag is false or g_bSyncTx is false!");
    return true; // advance index to next mDNS entry
  }
  
  // if we've exceeded MAX_HTTP_CLIENT_SENDS, delete the mDNS entry for this IP
  int sendCount = IML.GetSendCount(idx);
  if (sendCount > MAX_HTTP_CLIENT_SENDS){
    IML.DelMdnsIp(idx);
    prtln("DEBUG: SendHttpReq() sendCount exceeded, called IML.DelMdnsIp(" + String(idx) + ")");
    return false; // don't advance index
  }

  IPAddress ip = IML.GetIP(idx);
  int txToken = IML.GetTxToken(idx);  

  // if our last transmit for this IP failed, the HTTP callback task (tasks.cpp) will set txNextToken and txToken to NO_TOKEN
  // - we must query the remote IP for a new token which, when received, will be placed in txToken...
  if (txToken == NO_TOKEN){
    QueueTask(TASK_SEND_CANRX_QUERY, ip.toString()); // this will get Rx/Tx tokens!
//prtln("DEBUG: SendHttpReq() Queuing TASK_SEND_CANRX_QUERY, txToken == NO_TOKEN for: " + ip.toString());
    return true; // move on to next IP
  }

  // generate and add txNextToken plus checksum to outgoing sSend string
  String sReq = HMC.EncodeTxTokenAndChecksum(idx);
  
//prtln("DEBUG: SendHttpReq() txNextToken (being sent to " + ip.toString() + " ) is: " + String(IML.GetTxNextToken(idx)));
//prtln("DEBUG: SendHttpReq() after HMC.EncodeTxTokenAndChecksum(): " + sReq); 

  String sDateTime;
  if (IML.GetSendTimeFlag(idx)){
    sDateTime = TimeToString(); // time and date as: 2020-11-31T04:32:00pm
    if (!sDateTime.isEmpty())
      sDateTime = MyEncodeStr(sDateTime, HTTP_TABLE1, txToken, CIPH_CONTEXT_FOREGROUND);
//prtln("DEBUG: SendHttpReq() sDateTime: " + sDateTime); 
  }
  
  if (!sReq.isEmpty()){
//prtln("DEBUG: SendHttpReq(): sReq before MyEncodeStr: \"" + sReq + "\", txToken=" + String(txToken)); 
    sReq = MyEncodeStr(sReq, HTTP_TABLE1, txToken, CIPH_CONTEXT_FOREGROUND);
//prtln("DEBUG: SendHttpReq(): sReq after MyEncode: " + sReq); 
    sReq = "http://" + ip.toString() + HTTP_ASYNCREQ + '?' + HTTP_PARAM_COMMAND + '=' + sReq + '&'; // works with the & added...

    if (!sDateTime.isEmpty())
      sReq += HTTP_ASYNCREQ_PARAM_TIMESET + '=' + sDateTime + '&';
  }
  else if (!sDateTime.isEmpty()){
    sReq = "http://" + ip.toString() + HTTP_ASYNCREQ + '?' + HTTP_ASYNCREQ_PARAM_TIMESET + '=' + sDateTime + '&';
    prtln("SendHttpReq() sending time only! target ip =" + ip.toString());
  }

  if (!sReq.isEmpty()){
    if (asyncHttpreq.open("GET", sReq.c_str())){
      g_httpTxIP = ip; // save it for use in callback!
      asyncHttpreq.send();
      IML.SetSendCount(idx, ++sendCount);
//prtln("SendHttpReq() sent HTTP_ASYNCREQ: \"" + sReq + "\""); //!!!!!!!!!!!!
//prtln("SendHttpReq() sendCount: " + String(sendCount)); //!!!!!!!!!!!!
    }
    else
      prtln("SendHttpReq() Can't send request");
  }
  else
    prtln("SendHttpReq() nothing to send! target ip =" + ip.toString());

  return true;
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
// NOTE 6/26/2023 getting -13 as timeout...
// our custom codes...
// pertaining to HandleHttpAsyncReq() in WsHandlers.cpp
// HTTPCODE_OK             200
// HTTPCODE_PARAM_OK       211
// HTTPCODE_TOK_OK         222
// HTTPCODE_CANRX_OK       244
// HTTPCODE_FAIL           400
// HTTPCODE_PARAM_FAIL     501
// HTTPCODE_DECODE_FAIL    502
// HTTPCODE_DECPREV_FAIL   503
// HTTPCODE_ADDIP_FAIL     504
// HTTPCODE_TOK_FAIL       505
// HTTPCODE_CANRX_FAIL     507
// HTTPCODE_CANRX_DECODE_FAIL 508
// HTTPCODE_CANRX_NOMAC_FAIL 509
// HTTPCODE_RXDISABLED     510
// HTTPCODE_TIMESET_FAIL   511
// HTTPCODE_RXTOKEN_FAIL   512
void HttpClientCallback(void* optParm, AsyncHTTPRequest* request, int readyState){
  (void) optParm;
  if (readyState != readyStateDone) // request->available() 
    return;
  
  int httpCode = request->responseHTTPcode();

  if (httpCode < 0){
    // HTTPCODE_NOT_CONNECTED, happens a lot
    if (httpCode != HTTPCODE_NOT_CONNECTED)
      ClearLinkOk(httpCode);
  }else{
    String sCode = GetHttpCodeString(httpCode);
    if (sCode == "UNKNOWN")
      prtln("HttpClientCallback(): unknown code=" + String(httpCode) + ": \"" + request->responseHTTPString() + "\"");
    else
      prtln("HttpClientCallback(): code=HTTPCODE_" + sCode);
      
    String sRemIP;
    if (request->respHeaderExists(HTTP_SERVER_IP_HEADER_NAME)){
      String sIp = request->respHeaderValue(HTTP_SERVER_IP_HEADER_NAME);
      if (!sIp.isEmpty()){
        sRemIP = sIp;
      }
    }

    String sMac;
    
    if (request->respHeaderExists(HTTP_SERVER_MAC_HEADER_NAME))
      sMac = request->respHeaderValue(HTTP_SERVER_MAC_HEADER_NAME);
      
    if (!sRemIP.isEmpty() && httpCode != HTTPCODE_RXDISABLED)
      QueueTask(TASK_HTTPCALLBACK, 0, httpCode, request->responseText(), sMac, sRemIP); // TaskHttpCallback(), one unused int field...
      
    // don't clear LinkOk yet - let it retry via SendHttpReq()/IML.GetSendCount()
    //else
    //  ClearLinkOk();
  }
  g_httpTxIP.fromString("0.0.0.0");
  request->setDebug(false);
}

String GetHttpCodeString(int httpCode){
  String sPrt;
  if (httpCode == HTTPCODE_OK)
    sPrt = "OK";
  else if (httpCode == HTTPCODE_PARAM_OK)
    sPrt = "PARAM_OK";
  else if (httpCode == HTTPCODE_TOK_OK)
    sPrt = "TOK_OK";
  else if (httpCode == HTTPCODE_CANRX_OK)
    sPrt = "CANRX_OK";
  else if (httpCode == HTTPCODE_FAIL)
    sPrt = "FAIL";
  else if (httpCode == HTTPCODE_PARAM_FAIL)
    sPrt = "PARAM_FAIL";
  else if (httpCode == HTTPCODE_DECODE_FAIL)
    sPrt = "DECODE_FAIL";
  else if (httpCode == HTTPCODE_DECPREV_FAIL)
    sPrt = "DECPREV_FAIL";
  else if (httpCode == HTTPCODE_ADDIP_FAIL)
    sPrt = "ADDIP_FAIL";
  else if (httpCode == HTTPCODE_TOK_FAIL)
    sPrt = "TOK_FAIL";
  else if (httpCode == HTTPCODE_CANRX_FAIL)
    sPrt = "CANRX_FAIL";
  else if (httpCode == HTTPCODE_CANRX_DECODE_FAIL)
    sPrt = "CANRX_DECODE_FAIL";
  else if (httpCode == HTTPCODE_CANRX_NOMAC_FAIL)
    sPrt = "CANRX_NOMAC_FAIL";
  else if (httpCode == HTTPCODE_RXDISABLED)
    sPrt = "RXDISABLED";
  else if (httpCode == HTTPCODE_TIMESET_FAIL)
    sPrt = "TIMESET_FAIL";
  else if (httpCode == HTTPCODE_RXTOKEN_FAIL)
    sPrt = "RXTOKEN_FAIL";
  else
    sPrt = "UNKNOWN";
  return sPrt;
}
