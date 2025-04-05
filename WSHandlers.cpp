// this file WSHandlers.cpp
#include "Gpc.h"

void HandleHttpAsyncCanRxReq(AsyncWebServerRequest *request){

  IPAddress rip = request->client()->remoteIP();
  String sIp = rip.toString();
  prtln("HandleHttpAsyncCanRxReq(): remoteIP: " + sIp);
  
  if (!g_bSyncRx){
    SendHttpClientResponse(request, HTTPRESP_RXDISABLED, HTTPCODE_RXDISABLED);
    prtln("HandleHttpAsyncCanRxReq(): Local Rx is disabled!");
    return;
  }

  int rxIdx = IML.FindMdnsIp(rip);
  if (rxIdx < 0){
    rxIdx = IML.AddMdnsIp(rip);
    if (rxIdx < 0){ // add incomming IP to mDNS table
      prtln("HandleHttpAsyncCanRxReq(): UNABLE TO ADD ip to mDNS table");
      SendHttpClientResponse(request, HTTPRESP_ADDIP_FAIL, HTTPCODE_ADDIP_FAIL);
      return;
    }
    prtln("HandleHttpAsyncCanRxReq(): ip added to mDNS table");
  }

  // get MAC header
  int weAreMoreMaster = -1;
  if (request->hasHeader(HTTP_CLIENT_MAC_HEADER_NAME)){
    // NOTE: first character of what I sent via HTTP client library, added header, is missing... not sure which library
    // is causing the problem - so as a workaround I'm going to prepend an '&' character, which is not part of the base64 encoding
    // character-set
    String sMac = request->getHeader(HTTP_CLIENT_MAC_HEADER_NAME)->value();
//prtln("DEBUG: HandleHttpAsyncCanRxReq() got HTTP_CLIENT_MAC_HEADER_NAME: \"" + sMac + "\"");
    if (!sMac.isEmpty() && sMac[0] == '&')
      sMac = sMac.substring(1);
    int iMac = 0;
    int iErr = MyDecodeNum(iMac, sMac, HTTP_TABLE2, FAILSAFE_TOKEN_3, CIPH_CONTEXT_BACKGROUND);
    if (!iErr && iMac > 0){
      uint16_t localMacLT = GetOurDeviceMacLastTwoOctets();
      uint16_t remoteMacLT = (uint16_t)iMac;
      if (localMacLT > 0){
        if (localMacLT != remoteMacLT) // leave -1 if mac-last-two-octets are the same
          weAreMoreMaster = (localMacLT > remoteMacLT) ? 1 : 0;
        else
          prtln("HandleHttpAsyncCanRxReq(): WARNING remote and local MAC last-two-octets are the same! " + String(remoteMacLT));
        if (IML.GetMdnsMAClastTwo(rxIdx) != remoteMacLT){
          IML.SetMdnsMAClastTwo(rxIdx, remoteMacLT);
          prtln("HandleHttpAsyncCanRxReq(): MAC last-two added: " + String(remoteMacLT));
          RefreshGlobalMasterFlagAndIp();
          // purge possible pending command requesting MAC!
//            String sRef = IML.GetStr(rxIdx, MDNS_STRING_SEND_ALL);
//            HMC.StripRangeCommand(CMreqMacMin, CMreqMacMax, sRef);
//            IML.SetStr(rxIdx, MDNS_STRING_SEND_ALL, sRef);
        }
//        else
//          prtln("DEBUG: HandleHttpAsyncCanRxReq(): remoteMacLT " + String(remoteMacLT) + " already in table for " + sIp);
      }
      else
        prtln("HandleHttpAsyncCanRxReq(): error, localMacLT is 0!");
    }
    else
      prtln("HandleHttpAsyncCanRxReq(): can't decode MAC header!");
  }
  else
    prtln("HandleHttpAsyncCanRxReq(): no MAC header!");

  int iCode = HTTPCODE_CANRX_FAIL;
  String sReply = HTTPRESP_CANRX_FAIL;
  
  String sCmd;
  int iTokHigh;
  int iTokDecode = IML.GetToken(rxIdx, MDNS_TOKEN_CANRX_RX);
  
  bool bBadDecode = false;
  int iErr = 0;
    
  if (request->params() != CANRX_PARAMETER_COUNT){
    prtln("SendHttpCanRxReq(): Bad parameter count: " + String(CANRX_PARAMETER_COUNT));
    goto finally;
  }

  if (iTokDecode != NO_TOKEN){
    for (int ii = 0; ii < CANRX_PARAMETER_COUNT; ii++){
      const AsyncWebParameter* p = request->getParam(ii);
      if (!p) continue;
      String sName = String(p->name());
      iErr = MyDecodeStr(sName, HTTP_TABLE2, iTokDecode, CIPH_CONTEXT_BACKGROUND);
      if (iErr < -1){
        prtln("HandleHttpAsyncCanRxReq(): sName Bad decode: " + String(iErr));
        bBadDecode = true;
        break;
      }
      String sValue = String(p->value());
      if (sName == String(HTTP_ASYNCREQ_CANRX_PARAM_COMMAND)){
        iErr = MyDecodeStr(sValue, HTTP_TABLE1, iTokDecode, CIPH_CONTEXT_BACKGROUND);
        if (iErr < -1){
          prtln("HandleHttpAsyncCanRxReq(): sValue Bad decode: " + String(iErr));
          bBadDecode = true;
          break;
        }
        sCmd = sValue;
      }
      else if (sName == String(HTTP_ASYNCREQ_CANRX_PARAM_TOK3BITS)){
        int iValue = 0;
        iErr = MyDecodeNum(iValue, sValue, HTTP_TABLE3, iTokDecode, CIPH_CONTEXT_BACKGROUND);
        if (iErr < -1){
          prtln("HandleHttpAsyncCanRxReq(): iValue Bad decode: " + String(iErr));
          bBadDecode = true;
          break;
        }
        iTokHigh = iValue;
      }
    }
  }
  // designed to fail the first time...
  else{
    iTokDecode = GetRandToken(); // set new Rx token to send back
    IML.SetToken(rxIdx, MDNS_TOKEN_CANRX_RX, iTokDecode);
    bBadDecode = true;
  }

  if (bBadDecode || sCmd.isEmpty()){
    int iToken;
    if (IML.GetFlag(rxIdx, MDNS_FLAG_CANRX_MAIN_DECODE_FAIL))
      iToken = g_origDefToken;
    else{
      IML.SetFlag(rxIdx, MDNS_FLAG_CANRX_MAIN_DECODE_FAIL, true);
      iToken = g_defToken;
    }
    
    String sKey;
    const AsyncWebParameter* p = request->getParam(CANRX_PARAMETER_COUNT-1);
    if (p)
      sKey = String(p->value());

    if (!sKey.isEmpty()){
      // encode with flash-stored value (since g_defToken will change over time...)
      CIP.saveCiphKey(CIPH_CONTEXT_BACKGROUND);
      CIP.setCiphKey(sKey); // use the raw HTTP_ASYNCREQ_CANRX_PARAM_TOK3BITS encoded parameter as key!
      sReply = MyEncodeNum(iTokDecode, HTTP_TABLE2, iToken, CIPH_CONTEXT_BACKGROUND); // send other unit our new random token...
      CIP.restoreCiphKey(CIPH_CONTEXT_BACKGROUND);
      iCode = HTTPCODE_CANRX_DECODE_FAIL;
    }
    else
      prtln("SendHttpCanRxReq(): Bad parameter list!");
  }
  else if (sCmd == HTTP_COMMAND_CANRX){
    // shift until marker-bit shifted out...
    if (iTokHigh >= 1){ // 1 is the minimum because of "always 1" shift-marker bit
      while(!(iTokHigh & 1))
        iTokHigh >>= 1;
      iTokHigh >>= 1;
      
      int iTokCanRxHigh = iTokHigh & 7; // low 3 bits are high part of Canrx token
      int iTokCmdStrHigh = iTokHigh >> 3; // high 3 bits are high part of Command-string token
      
      if (iTokCmdStrHigh >= 0 && iTokCmdStrHigh <= 7 && iTokCanRxHigh >= 0 && iTokCanRxHigh <= 7){
        int iTokCanRxLow = random(0,7+1); // 0-7
        int iTokCmdStrLow = random(0,7+1); // 0-7
        int iTokShifted = (iTokCmdStrLow<<4)|(iTokCanRxLow<<1)|1; // add shift-marker-bit
        iTokShifted <<= random(0,MAX_CANRX_TOKEN_SHIFT+1);
        sReply = MyEncodeNum(iTokShifted, HTTP_TABLE3, iTokDecode, CIPH_CONTEXT_BACKGROUND);
        if (sReply.isEmpty()){
          prtln("SendHttpCanRxReq(): Can't encode sTokLow for tokShifted=" + String(iTokShifted));
          goto finally;
        }
        int iTokRx = (iTokCmdStrHigh<<3)+iTokCmdStrLow;
        IML.SetToken(rxIdx, MDNS_TOKEN_RX, iTokRx);
        iTokRx = (iTokCanRxHigh<<3)+iTokCanRxLow;
        IML.SetToken(rxIdx, MDNS_TOKEN_CANRX_RX, iTokRx);
        IML.SetFlag(rxIdx, MDNS_FLAG_CANRX_MAIN_DECODE_FAIL, false);
        
        iCode = HTTPCODE_CANRX_OK;
      }
      else
        prtln("HandleHttpAsyncCanRxReq() HTTP_COMMAND_CANRX iTokCmdStrHigh or iTokCanRxHigh out of range!");
    }
    else
      prtln("HandleHttpAsyncCanRxReq() HTTP_COMMAND_CANRX iTokHigh < 1 (possible bad decode)!");
  }
  else
    prtln("HandleHttpAsyncCanRxReq() sCmd unrecognized: \"" + sCmd + "\"");

finally:
  if (sReply.isEmpty()){
    sReply = HTTPRESP_CANRX_FAIL;
    prtln("HandleHttpAsyncCanRxReq(): sReply is empty. Set sReply = HTTPRESP_CANRX_FAIL");
  }

  if (sReply == HTTPRESP_CANRX_FAIL){
    iCode = HTTPCODE_CANRX_FAIL;
    iErr = MyEncodeStr(sReply, HTTP_TABLE3, FAILSAFE_TOKEN_1, CIPH_CONTEXT_BACKGROUND);
    if (iErr){
      prtln("SendHttpCanRxReq(): bad encodeStr: " + String(iErr));
      return;
    }
  }
  
  SendHttpClientMacResponse(request, sReply, iCode);
}

// Note: poll in main loop for MDNS_STRING_RECEIVE not empty and
// TSK.QueueTask(TASK_PROCESS_RECEIVE_STRING, 0, httpCode, sRsp, sMac, sRemIP).
// Send reply to sender's callback using: HTTPRESP_PROCESSING_OK, HTTPCODE_PROCESSING_OK
// TODO: move much of below code to TaskProcessReceiveString()
// In this function, do SetString(idx, MDNS_STRING_RECEIVE, sReq);
void HandleHttpAsyncReq(AsyncWebServerRequest *request){

  int iParamCount = request->params();

  if (iParamCount <= 0 || iParamCount > 2)
    return;

  String sParam[2];
  String sReply;
  int iCode, rxIdx, rxTok;
  int iErr = 0;
  
  IPAddress rip = request->client()->remoteIP();

  String sIp = rip.toString();
  
  rxIdx = IML.FindMdnsIp(rip);
  if (rxIdx < 0){
    iCode = HTTPCODE_NOIP_FAIL;
    sReply = HTTPRESP_NOIP_FAIL;
    prtln("HandleHttpAsyncReq(): replying HTTPRESP_NOIP_FAIL, ip is not in mDNS table: " + sIp);
    goto noencode;
  }

  rxTok = IML.GetToken(rxIdx, MDNS_TOKEN_RX); // save before calling HMC.ProcessMsgCommands()!

  if (rxTok == NO_TOKEN){
    iCode = HTTPCODE_RXTOKEN_FAIL;
    sReply = HTTPRESP_RXTOKEN_FAIL;
    prtln("HandleHttpAsyncReq(): replying HTTPRESP_RXTOKEN_FAIL, rxTok == NO_TOKEN:  " + sIp);
    goto noencode;
  }

  if (!g_bSyncRx){
    iCode = HTTPCODE_RXDISABLED;
    sReply = HTTPRESP_RXDISABLED;
    prtln("HandleHttpAsyncReq(): replying HTTPRESP_RXDISABLED, local Rx is disabled: " + sIp);
    goto finally;
  }
  
  for (int ii = 0; ii < iParamCount; ii++){
    const AsyncWebParameter* p = request->getParam(ii);
    if (p != NULL)
      sParam[ii] = String(p->name()) + '=' + String(p->value());
  }

  // NOTE: order of parameters does not matter! 0,1 1,0
  TSK.QueueTask(TASK_PROCESS_RECEIVE_STRING, IpToUInt(sIp), rxTok, sParam[1], sParam[0]);
  
  iCode = HTTPCODE_PROCESSING_OK;
  sReply = HTTPRESP_PROCESSING_OK;
  prtln("HandleHttpAsyncReq(): replying HTTPRESP_PROCESSING_OK: " + sIp);
  
finally:
  iErr = MyEncodeStr(sReply, HTTP_TABLE3, FAILSAFE_TOKEN_1, CIPH_CONTEXT_BACKGROUND);
  
noencode:
  if (iErr)
    prtln("SendHttpCanRxReq(): bad encode: " + String(iErr));
  else
    SendHttpClientResponse(request, sReply, iCode);
}

void SendHttpClientMacResponse(AsyncWebServerRequest *request, String sResp, int code){
  if (!sResp.isEmpty()){
    AsyncWebServerResponse *r = request->beginResponse(code, "text/plain", sResp);
//    String sIp = request->client()->localIP().toString();
//prtln("DEBUG: SendHttpClientMacResponse(): sIp before MyEncodeStr(): " + sIp);
//    sIp = MyEncodeStr(sIp, HTTP_TABLE3, FAILSAFE_TOKEN_4, CIPH_CONTEXT_BACKGROUND);
//prtln("DEBUG: SendHttpClientMacResponse(): sIp after MyEncodeStr() (about to call r->addHeader()): " + sIp);
//    r->addHeader(HTTP_SERVER_IP_HEADER_NAME, sIp.c_str());
    String sMac = MyEncodeNum(GetOurDeviceMacLastTwoOctets(), HTTP_TABLE1, FAILSAFE_TOKEN_5, CIPH_CONTEXT_BACKGROUND);
    r->addHeader(HTTP_SERVER_MAC_HEADER_NAME, sMac.c_str());
    //r->addHeader("Connection", "close");
    request->send(r);
  }
}

void SendHttpClientResponse(AsyncWebServerRequest *request, String sResp, int code){
  if (!sResp.isEmpty()){
    AsyncWebServerResponse *r = request->beginResponse(code, "text/plain", sResp);
//    String sIp = request->client()->localIP().toString();
//prtln("DEBUG: SendHttpClientResponse(): sIp before MyEncodeStr(): " + sIp);
//    sIp = MyEncodeStr(sIp, HTTP_TABLE3, FAILSAFE_TOKEN_4, CIPH_CONTEXT_BACKGROUND);
//prtln("DEBUG: SendHttpClientResponse(): sIp after MyEncodeStr(): " + sIp);
//    r->addHeader(HTTP_SERVER_IP_HEADER_NAME, sIp.c_str());
    //r->addHeader("Connection", "close");
    request->send(r);
  }
}

// handle ON, OFF, AUTO button clicks on index.html/p0.js
void HandleButtonsReq(AsyncWebServerRequest *request){
    if (IsLockedAlertGet(request, INDEX_FILENAME))
      return;

    String sSel;
    if (request->hasParam("A"))
      sSel = "A";
    else if (request->hasParam("B"))
      sSel = "B";
    else if (request->hasParam("C"))
      sSel = "C";
    else if (request->hasParam("D"))
      sSel = "D";
    if (sSel.isEmpty())
      return;
    String sVal = request->getParam(sSel)->value();
    String sDec = B64C.hnShiftDecode(sVal);
    if (sDec.isEmpty())
      return;
    String sSec = sDec.substring(1);
    if (!alldigits(sSec))
      return;
    if (sSec.toInt() != g_sct) // security check
      return;
    String buttonMode = sDec.substring(0,1);

    if (sSel == "A"){
      if (buttonMode == "0"){
        if (g8_ssr1ModeFromWeb != SSR_MODE_ON){
          g8_ssr1ModeFromWeb = SSR_MODE_ON;
          SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
        }
      }
      else if (buttonMode == "1"){
        if (g8_ssr1ModeFromWeb != SSR_MODE_OFF){
          g8_ssr1ModeFromWeb = SSR_MODE_OFF;
          SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
        }
      }
      else if (buttonMode == "2"){
        if (g8_ssr1ModeFromWeb != SSR_MODE_AUTO){
          g8_ssr1ModeFromWeb = SSR_MODE_AUTO;
          SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
        }
      }
    }
    else if (sSel == "B"){
      if (buttonMode == "0"){
        if (g8_ssr2ModeFromWeb != SSR_MODE_ON){
          g8_ssr2ModeFromWeb = SSR_MODE_ON;
          SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
        }
      }
      else if (buttonMode == "1"){
        if (g8_ssr2ModeFromWeb != SSR_MODE_OFF){
          g8_ssr2ModeFromWeb = SSR_MODE_OFF;
          SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
        }
      }
      else if (buttonMode == "2"){
        if (g8_ssr2ModeFromWeb != SSR_MODE_AUTO){
          g8_ssr2ModeFromWeb = SSR_MODE_AUTO;
          SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
        }
      }
    }
#if ENABLE_SSR_C_AND_D
    else if (sSel == "C"){
      if (buttonMode == "0"){
        if (g8_ssr3ModeFromWeb != SSR_MODE_ON){
          g8_ssr3ModeFromWeb = SSR_MODE_ON;
          SetSSRMode(GPOUT_SSR3, g8_ssr3ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_C);
        }
      }
      else if (buttonMode == "1"){
        if (g8_ssr3ModeFromWeb != SSR_MODE_OFF){
          g8_ssr3ModeFromWeb = SSR_MODE_OFF;
          SetSSRMode(GPOUT_SSR3, g8_ssr3ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_C);
        }
      }
      else if (buttonMode == "2"){
        if (g8_ssr3ModeFromWeb != SSR_MODE_AUTO){
          g8_ssr3ModeFromWeb = SSR_MODE_AUTO;
          SetSSRMode(GPOUT_SSR3, g8_ssr3ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_C);
        }
      }
    }
    else if (sSel == "D"){
      if (buttonMode == "0"){
        if (g8_ssr4ModeFromWeb != SSR_MODE_ON){
          g8_ssr4ModeFromWeb = SSR_MODE_ON;
          SetSSRMode(GPOUT_SSR4, g8_ssr4ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_D);
        }
      }
      else if (buttonMode == "1"){
        if (g8_ssr4ModeFromWeb != SSR_MODE_OFF){
          g8_ssr4ModeFromWeb = SSR_MODE_OFF;
          SetSSRMode(GPOUT_SSR4, g8_ssr4ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_D);
        }
      }
      else if (buttonMode == "2"){
        if (g8_ssr4ModeFromWeb != SSR_MODE_AUTO){
          g8_ssr4ModeFromWeb = SSR_MODE_AUTO;
          SetSSRMode(GPOUT_SSR4, g8_ssr4ModeFromWeb);
          TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_D);
        }
      }
    }
#endif

    request->send(204, "text/html", "");
//    request->send(SPIFFS, "/index.html", String(), false, ReplaceHtmlPercentSign);
}

void HandleHeartbeatReq(AsyncWebServerRequest *request){
  if (request->hasParam(PARAM_HEARTBEAT)){
    String s;

    // check for received text (via internal Http client communications between units)
    // and echo it to the connected web-browser...
    for (int ii = 0; ii < IML.GetCount(); ii++){
      String sRxTxt = IML.GetStr(ii, MDNS_STRING_RXTXT);
      if (!sRxTxt.isEmpty()){
        if (sRxTxt.length() < MAXTXTLEN){
          s = TEXT_PREAMBLE + sRxTxt; // prepend "TXT:" - javascript for web-page looks for that!
//prtln("DEBUG: HandleHeartbeatReq(): sending text from " + IML.GetIP(ii).toString() + " to web-browser at " + request->client()->remoteIP().toString() + ": \"" + s + "\"");
        }
        IML.SetStr(ii, MDNS_STRING_RXTXT, "");
        break; // just send one - if there are more, send on subsequent heartbeat-requests...
      }
    }

    // if no text, send the status variable comma-separated values...
    if (s.isEmpty()){
      s = String(g8_lockCount) + "," + String(g_potPercent) + "," + String(g8_wifiModeFromSwitch) + "," + String(g8_potModeFromSwitch);
    }
    request->send(HTTPCODE_OK, "text/html", B64C.hnShiftEncode(s));
  }
}

void HandleIndexReq(AsyncWebServerRequest *request){
  int count = request->params();

  // note: I only send one parameter but get 2 - an underscore is tacked on by jquery for some reason...
  if (count == 0)
    return;

  const AsyncWebParameter* p = request->getParam((int)0);
  if (p == NULL)
    return;

  String sVal = B64C.hnShiftDecode(p->value());

  if (sVal.isEmpty())
    return;

  String sName = p->name();
  
  byte iStatus = 0; // status 0 = no send needed at end, 1 = OK response, 2 = fail response

  String s; // if this in not empty later, we send it as code 200 to the client
  
  // index.html: g_sHostName perMax perVal perUnits
  // p1.html: (via %P1APMODE% we send wifiName wifiPass if in AP wifi mode) phaseSlider dcASlider dcBSlider
  // p2.html: hours minutes ampm onoff delete

  if (sName == PARAM_TEXT){
    if (sVal == PARAM_TEXT_VALUE){
      s = g_text;
      g_text = "";
    }
  }
  else if (sName == PARAM_STATE1){
    // This sends current state, outlet mode, # times on and duty-cycle in percent to
    // javascript in our HTML web-page as 4 comma-seperated sub-strings
    if (sVal == PARAM_STATE1_VALUE){
#if GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
      s = (g_actualStatus & DEV_STATUS_1) ? "ON" : "OFF";
#else
      s = (g_devStatus & DEV_STATUS_1) ? "ON" : "OFF";
#endif
      s += "," + SsrModeToString(g8_ssr1ModeFromWeb);
      s += "," + String(ComputeTimeToOnOrOffA()); // String(g_stats.AOnPrevCount+g_stats.AOnCounter)
      s += "," + PercentOnToString(g_stats.PrevDConA+g_stats.DConA, g_stats.HalfSecondCount+g_stats.HalfSecondCounter);
    }
  }
  else if (sName == PARAM_STATE2){
    if (sVal == PARAM_STATE2_VALUE){
#if GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
      s = (g_actualStatus & DEV_STATUS_2) ? "ON" : "OFF";
#else
      s = (g_devStatus & DEV_STATUS_2) ? "ON" : "OFF";
#endif
      s += "," + SsrModeToString(g8_ssr2ModeFromWeb);
      s += "," + String(ComputeTimeToOnOrOffB()); // String(g_stats.BOnPrevCount+g_stats.BOnCounter)
      s += "," + PercentOnToString(g_stats.PrevDConB+g_stats.DConB, g_stats.HalfSecondCount+g_stats.HalfSecondCounter);
    }
  }
#if ENABLE_SSR_C_AND_D
  else if (sName == PARAM_STATE3){
    if (sVal == PARAM_STATE3_VALUE){
      s = (g_actualStatus & DEV_STATUS_3) ? "ON" : "OFF";
      s += "," + SsrModeToString(g8_ssr3ModeFromWeb);
      s += "," + String(ComputeTimeToOnOrOffC()); // String(g_stats.COnPrevCount+g_stats.COnCounter)
      s += "," + PercentOnToString(g_stats.PrevDConC+g_stats.DConC, g_stats.HalfSecondCount+g_stats.HalfSecondCounter);
    }
  }
  else if (sName == PARAM_STATE4){
    if (sVal == PARAM_STATE4_VALUE){
      s = (g_actualStatus & DEV_STATUS_4) ? "ON" : "OFF";
      s += "," + SsrModeToString(g8_ssr4ModeFromWeb);
      s += "," + String(ComputeTimeToOnOrOffD()); // String(g_stats.DOnPrevCount+g_stats.DOnCounter)
      s += "," + PercentOnToString(g_stats.PrevDConD+g_stats.DConD, g_stats.HalfSecondCount+g_stats.HalfSecondCounter);
    }
  }
#endif
  else if (sName == PARAM_HOSTNAME){
    sVal.trim();
    twiddle(sVal);
    // process commands first... "c somecommand"
    if (sVal.isEmpty())
      s = "Transmission error - please try again!";
    else if (sVal.length() > 2 && (sVal[0] == 'c' || sVal[0] == 'C') && sVal[1] == ' ')
      s = ProcessCommand(request, sVal); // all by reference!
    else if (!IsLocked()) // set WiFi g_sHostName
      s = ProcessWiFiHostName(sVal);
    else
      s = "Interface is locked!";

    if (s != "")
      s = "{\"a\":\"" + s + "\", \"l\":\"" + String(INDEX_FILENAME) + "\", \"o\":\"\"}";
      //s = "<script>alert('" + s + "'); location.href = '" + String(INDEX_FILENAME) + "';</script>";
  }
  else{
    if (IsLockedAlertGetPlain(request, false))
      return;

    // locked commands...

    if (sName == PARAM_LABEL_A || sName == PARAM_LABEL_B || sName == PARAM_LABEL_C || sName == PARAM_LABEL_D){
      sVal.trim();
      if (sVal.isEmpty())
        s = "Transmission error or blank label...";
      else if (sVal.length() > LABEL_MAXLENGTH)
        s = "Max label length is: " + String(LABEL_MAXLENGTH) + "!";
      else{
        int iTask = -1;
        if (sName == PARAM_LABEL_A)
          iTask = TASK_LABEL_A;
        else if (sName == PARAM_LABEL_B)
          iTask = TASK_LABEL_B;
        else if (sName == PARAM_LABEL_C)
          iTask = TASK_LABEL_C;
        else if (sName == PARAM_LABEL_D)
          iTask = TASK_LABEL_D;
        if (iTask >= 0)
          TSK.QueueTask(iTask, sVal);
      }
      
      if (s != "")
        s = "{\"a\":\"" + s + "\", \"l\":\"" + String(INDEX_FILENAME) + "\", \"o\":\"\"}";
//        s = "<script>alert('" + s + "'); location.href = '" + String(INDEX_FILENAME) + "';</script>";
    }
    else{
      int iVal = B64C.hnDecNumOnly(sVal);

      if (sName == PARAM_PERMAX){
        TSK.QueueTask(TASK_PARMS, SUBTASK_PERMAX, iVal);
        iStatus = 1;
      }
      else if (sName == PARAM_PERUNITS){
        TSK.QueueTask(TASK_PARMS, SUBTASK_PERUNITS, iVal);
        iStatus = 1;
      }
      else if (sName == PARAM_PERVAL){
        TSK.QueueTask(TASK_PARMS, SUBTASK_PERVAL, iVal);
        iStatus = 1;
      }
    }
  }

  // 200 = OK
  // 204 = OK but No Content
  if (!s.isEmpty())
    request->send(HTTPCODE_OK, "text/html", B64C.hnShiftEncode(s));
  else if (iStatus == 1)
    request->send(204, "text/html", "");
}

void HandleGetP2Req(AsyncWebServerRequest *request){
  byte iStatus = 0; // status 0 = no send needed at end, 1 = OK response, 2 = fail response

  String s = ""; // if this in not empty later, we send it as code 200 to the client

  int count = request->params();

  // note: I only send one parameter but get 2 - an underscore is tacked on by jquery for some reason...
  if (count == 0)
    return;

  const AsyncWebParameter* p = request->getParam((int)0);
  if (p == NULL)
    return;

  String sVal = B64C.hnShiftDecode(p->value());

  if (sVal.isEmpty())
    return;

  String sName = p->name();

  // used to set our time from time on remote laptop
  if (sName == PARAM_DATETIME){
    s = SetTimeDate(sVal, true);
  }
  else{
    if (IsLockedAlertGetPlain(request, false))
      return;

    // these commands can be locked out...

    if (sName == PARAM_DELINDEX){
      // AsyncWebParameter (file: ESPAsyncWebServer.h)
      // ->name()
      // ->value()
      // ->isFile()
      // ->isPost()
      // ->size()
      // NOTE: slot-numbers requested for deletion are not in order - there could
      // be a g_slotCount of 1 and its delete-index of 99!
      if (g_slotCount){
        int iSlotNum = B64C.hnDecNumOnly(sVal);
        if (TSC.DeleteTimeSlot(iSlotNum))
          s = "Time-slot " + String(iSlotNum) + " deleted!";
        else
          s = "Failed to delete time-slot " + String(iSlotNum) + "!";
      }
      else
        s = "Failed to send, please retry...";
    }
    else if (sName == PARAM_ERASEDATA){
      if (sVal == ERASE_DATA_CONFIRM){
        TSK.QueueTask(TASK_RESET_SLOTS);
        s = "Deleting all time-events!";
      }
      else
        s = "Not able to erase...";
    }
    else if (sName == PARAM_EDITINDEX){
      int idx = B64C.hnDecNumOnly(sVal);

      // Here, the user has pressed Edit and we have the edit item's index -
      // we need to send back a comma-separated list of decoded vars for year, day, month, Etc.
      // We can do it two ways - parse the incomming string that represents a stored time-event,
      // or pull it out of flash where it's stored in time-slots. The latter makes more sense!
      t_event t;
      if (TSC.GetTimeSlot(idx, t)) // by ref
        s = TSC.TimeSlotToCommaSepString(idx, t);
      else
        iStatus = 2;
    }
  }

  // 200 = OK
  // 204 = OK but No Content
  if (iStatus == 2)
    s = "Send failed, please retry...";
  if (s != "")
    request->send(HTTPCODE_OK, "text/html", B64C.hnShiftEncode(s).c_str());
  else if (iStatus == 1)
    request->send(204, "text/html", ""); // OK but no data

  // Give a semaphore that we can check in the loop (just prints the inputMessege)
//    xSemaphoreGiveFromISR(webInputSemaphore, NULL);
}

void HandleAltP1Req(AsyncWebServerRequest *request){
  if (IsLockedAlertGetPlain(request, false))
    return;

  int count = request->params();
  //prtln("#parms: " + String(count));

  // note: I only send one parameter but get 2 - an underscore is tacked on by jquery for some reason...
  if (count == 0)
    return;

  const AsyncWebParameter* p = request->getParam((int)0);
  if (p == NULL)
    return;

  String sName = p->name();
  int iVal = B64C.hnDecNum(p->value());

  // all 8-bit values 0-100% and midi-note or channel
  if (iVal < 0 || iVal > 255){
    prtln("HandleAltP1Req(): iVal out of range for: \"" + sName + "\"");
    return;
  }
    
  if (sName == PARAM_PHASE_B)
    TSK.QueueTask(TASK_PARMS, SUBTASK_PHASEB, iVal);
  else if (sName == PARAM_DC_A)
    TSK.QueueTask(TASK_PARMS, SUBTASK_DCA, iVal);
  else if (sName == PARAM_DC_B)
    TSK.QueueTask(TASK_PARMS, SUBTASK_DCB, iVal);
  else if (sName == PARAM_MIDICHAN){
    if (iVal != g8_midiChan){
      g8_midiChan = iVal;
      TSK.QueueTask(TASK_MIDICHAN);
    }
  }
  else if (sName == PARAM_MIDINOTE_A){
    if (iVal != g8_midiNoteA){
      g8_midiNoteA = iVal;
      TSK.QueueTask(TASK_MIDINOTE_A);
    }
  }
  else if (sName == PARAM_MIDINOTE_B){
    if (iVal != g8_midiNoteB){
      g8_midiNoteB = iVal;
      TSK.QueueTask(TASK_MIDINOTE_B);
    }
  }
#if ENABLE_SSR_C_AND_D
  else if (sName == PARAM_PHASE_C)
    TSK.QueueTask(TASK_PARMS, SUBTASK_PHASEC, iVal);
  else if (sName == PARAM_PHASE_D)
    TSK.QueueTask(TASK_PARMS, SUBTASK_PHASED, iVal);
  else if (sName == PARAM_DC_C)
    TSK.QueueTask(TASK_PARMS, SUBTASK_DCC, iVal);
  else if (sName == PARAM_DC_D)
    TSK.QueueTask(TASK_PARMS, SUBTASK_DCD, iVal);
  else if (sName == PARAM_MIDINOTE_C){
    if (iVal != g8_midiNoteC){
      g8_midiNoteC = iVal;
      TSK.QueueTask(TASK_MIDINOTE_C);
    }
  }
  else if (sName == PARAM_MIDINOTE_D){
    if (iVal != g8_midiNoteD){
      g8_midiNoteD = iVal;
      TSK.QueueTask(TASK_MIDINOTE_D);
    }
  }
#endif

  // 204 = OK but No Content
  request->send(204, "text/html", "");
}

void HandleGetP1Req(AsyncWebServerRequest *request){
  byte iStatus = 0; // status 0 = no send needed at end, 1 = OK response

  String s = ""; // if this in not empty later, we send it as code 200 to the client
  // index.html: g_sHostName perMax perVal perUnits
  // p1.html: (via %P1APMODE% we send wifiName wifiPass if in AP wifi mode) phaseSlider dcASlider dcBSlider
  // p2.html: hours minutes ampm onoff delete

  if (IsLockedAlertGet(request, P1_FILENAME))
    return;

  if (request->hasParam(PARAM_BUTRST)){
    if (g_bSoftAP && B64C.hnShiftDecode(request->getParam(PARAM_BUTRST)->value()) == PARAM_BUTRST_VALUE){
      TSK.QueueTask(TASK_WIFI_RESTORE);
      s = "<script>alert('SSID and password reset to previous values!');";
    }
    else
      iStatus = 1;
  }
  else if (request->hasParam(PARAM_WIFINAME) && request->hasParam(PARAM_WIFIPASS)){
    if (g_bSoftAP){
      String valN, valP;
      String sN = request->getParam(PARAM_WIFINAME)->value();
      int errorCodeN = B64C.hnShiftDecode(sN, valN);
      String sP = request->getParam(PARAM_WIFIPASS)->value();
      int errorCodeP = B64C.hnShiftDecode(sP, valP);

      //!!!!!!!!!!!!!!!!!!!!!!!!
      //prtln("sN: \"" + sN + "\"");
      //prtln("valN: \"" + valN + "\"");
      //prtln("sP: \"" + sP + "\"");
      //prtln("valP: \"" + valP + "\"");

      if (errorCodeN < -1 || errorCodeP < -1){
        s = "<script>alert('Transmission error - please try again!');";
        prtln("errorCodeN = " + String(errorCodeN));
        prtln("errorCodeP = " + String(errorCodeP));
      }
      else
        s = ProcessWifiSsidPass(valN, valP);
    }
    else // send error alert message
      s = "<script>alert('Must be in AP mode!');";
    if (s != "")
      s += "location.href = '" + String(P1_FILENAME) + "';</script>";
  }

  // 200 = OK
  // 204 = OK but No Content
  if (s != "")
    request->send(HTTPCODE_OK, "text/html", B64C.hnShiftEncode(s).c_str());
  else if (iStatus == 1)
    request->send(204, "text/html", "");
}

void HandleP2FormReq(AsyncWebServerRequest *request){
  if (IsLockedAlertPost(request))
    return;

  int count = request->params();
  //prtln("#parms: " + String(count));

  // note: I only send one parameter but get 2 - an underscore is tacked on by jquery for some reason...
  if (count == 0)
    return;

  byte iStatus = 0; // status 0 = no send needed at end, 1 = OK response, 2 = fail response
  String s = ""; // if this in not empty later, we send it as code 200 to the client

  //if (request->multipart())
  //  prtln("request is multipart!");

  t_event t = {0};

  uint16_t tempHour = 0;
  bool bTempPmFlag = false;
  bool bTempIncludeCycleTiming = false; // allows us to record cycle-timing, such as phase and duty-cycle, into this event
  bool bTempCycleTimingInRepeats = false; // apply cycle-timing for repeat-events
  bool bUseOldTimeVals = false; // in a slot-replace after editing, keep old slot's timing cycle values
  int repIndex = -1; // if this is -1 we add an event rather than replace

  // cancel pending refresh task
  if (TSK.CancelTask(TASK_PAGE_REFRESH_REQUEST))
    g_bTellP2WebPageToReload = false;

  for (int ii = 0; ii < count; ii++){
    const AsyncWebParameter* p = request->getParam(ii);

    if (p == NULL)
      continue;

    if(p->isFile()){
      prtln("_FILE: " + p->name() + ", " + p->value());
      continue;
    }

    if(p->isPost()){
      prtln("_POST: " + p->name() + ", " + p->value());
      continue;
    }

    String sName = p->name();
    String sVal = B64C.hnShiftDecode(p->value());

    //prtln("hnDecode: " + sName + ':' + sVal);

    if (sName == PARAM_INCLUDETIMINGCYCLE)
      // this is a checkbox - weirdly, it reports "on" if checked and no parameter is sent
      // at all if unchecked.
      bTempIncludeCycleTiming = (sVal == "on") ? true : false; // include current cycle-timing if 1
    else if (sName == PARAM_TIMINGCYCLEINREPEATS)
      bTempCycleTimingInRepeats = (sVal == "on") ? true : false; // include current cycle-timing if 1
    else if (sName == PARAM_DATE){
      // 2020-08-08
      String sYear = sVal.substring(0,4);
      t.timeDate.year = sYear.toInt();
      String sMonth = sVal.substring(5,7);
      t.timeDate.month = sMonth.toInt();
      if (t.timeDate.month > 12)
        t.timeDate.month = 1;
      String sDay = sVal.substring(8,10);
      t.timeDate.day = sDay.toInt();
      if (t.timeDate.day < 1 || t.timeDate.day > 31)
        t.timeDate.day = 1;
    }
    else{
      int iVal = B64C.hnDecNumOnly(sVal);
      if (iVal >= 0){
        if (sName == PARAM_REPLACEINDEX)
          repIndex = iVal;
        else if (sName == PARAM_HOUR)
          tempHour = iVal; // 12-hour format!
        else if (sName == PARAM_MINUTE)
          t.timeDate.minute = iVal;
        else if (sName == PARAM_SECOND)
          t.timeDate.second = iVal;
        else if (sName == PARAM_AMPM)
          bTempPmFlag = (iVal > 0) ? true : false; // PM if 1, AM if 0
        else if (sName == PARAM_REPEAT_MODE)
          t.repeatMode = iVal;
        else if (sName == PARAM_REPEAT_COUNT)
          t.repeatCount = iVal;
        else if (sName == PARAM_EVERY_COUNT)
          t.everyCount = iVal;
        else if (sName == PARAM_DEVICE_ADDR)
          t.deviceAddr = iVal;
        else if (sName == PARAM_DEVICE_MODE)
          t.deviceMode = iVal;
        else if (sName == PARAM_USEOLDTIMEVALS)
          bUseOldTimeVals = iVal ? true : false; // set in response to javascript "confirm" dialog in p2.js
      }
    }
  }

  // convert 12-hour am/pm to 0-23, 24-hour time
  if (bTempPmFlag){
    if (tempHour != 12)
      tempHour += 12;
  }
  else if (tempHour == 12) // 1-11am is 1-11, 12 midnight is 0
    tempHour = 0;

  t.timeDate.hour = tempHour;

  t.timeDate.dayOfWeek = TSC.MyDayOfWeek(t.timeDate.day, t.timeDate.month, t.timeDate.year);

  t.bIncludeCycleTiming = bTempIncludeCycleTiming;
  t.bCycleTimingInRepeats = bTempCycleTimingInRepeats;

  if (bTempIncludeCycleTiming){
    t_event t2;
    // if user has chosen to keep old cycle-values, load them
    if (bUseOldTimeVals && repIndex >= 0 && TSC.GetTimeSlot(repIndex, t2)){
      // move old slot's cycle-timing vals to new slot
      t.dutyCycleA = t2.dutyCycleA;
      t.dutyCycleB = t2.dutyCycleB;
      t.phaseB = t2.phaseB;
      t.perUnits = t2.perUnits;
      t.perMax = t2.perMax;
      t.perVal = t2.perVal;
#if ENABLE_SSR_C_AND_D
      t.dutyCycleC = t2.dutyCycleC;
      t.dutyCycleD = t2.dutyCycleD;
      t.phaseC = t2.phaseC;
      t.phaseD = t2.phaseD;
#endif
    }
    else{
      t.dutyCycleA = g_perVals.dutyCycleA;
      t.dutyCycleB = g_perVals.dutyCycleB;
      t.phaseB = g_perVals.phaseB;
      t.perUnits = g_perVals.perUnits;
      t.perMax = g_perVals.perMax;
      t.perVal = g_perVals.perVal;
#if ENABLE_SSR_C_AND_D
      t.dutyCycleC = g_perVals.dutyCycleC;
      t.dutyCycleD = g_perVals.dutyCycleD;
      t.phaseC = g_perVals.phaseC;
      t.phaseD = g_perVals.phaseD;
#endif
    }
  }
  else{
    t.dutyCycleA = 0xff;
    t.dutyCycleB = 0xff;
    t.phaseB = 0xff;
    t.perUnits = 0xff;
    t.perVal = 0xff;
    t.perMax = 0xffff;
#if ENABLE_SSR_C_AND_D
    t.dutyCycleC = 0xff;
    t.dutyCycleD = 0xff;
    t.phaseC = 0xff;
    t.phaseD = 0xff;
#endif
  }

  t.bEnable = true;

  if (repIndex >= 0){
    if (repIndex < g_slotCount && TSC.PutTimeSlot(repIndex, t))
      s = "Replaced timeslot...";
    else
      s = "Unable to replace timeslot " + String(repIndex) + "...";
  }
  else if (g_slotCount < MAX_TIME_SLOTS){
    //if (TSC.AddTimeSlot(t, true)) // use this for debugging - has error message printout on USB
    if (TSC.AddTimeSlot(t))
      s = "Added timeslot...";
    else
      s = "Unable to add timeslot...";
  }
  else // timeslots are full
    s = "<script>alert('Timeslots are all full. Delete some! Count is: " + String(MAX_TIME_SLOTS) + "');location.href = '" + String(P2_FILENAME) + "';</script>";

  // 200 = OK
  // 204 = OK but No Content
  if (s != "")
    request->send(HTTPCODE_OK, "text/html", B64C.hnShiftEncode(s).c_str());
  else if (iStatus == 1)
    request->send(204, "text/html", ""); // OK but no data
}

void HandlePostP2Req(AsyncWebServerRequest *request){
  int count = request->params();

  // note: I only send one parameter but get 2 - an underscore is tacked on by jquery for some reason...
  if (count == 0)
    return;

  // disallow erase-data and adding time-events from remote file if locked interface
  if (IsLockedAlertPost(request))
    return;

  String s = ""; // if this in not empty later, we send it as code 200 to the client

//    if (request->multipart())
//      prtln("WARNING: is multipart!");

  if (request->hasParam(PARAM_FILEDATA, true)){
    String sIn = B64C.hnShiftDecode(request->getParam(PARAM_FILEDATA, true)->value());

    //prtln("fileData=" + sIn);

    //inputParam = PARAM_FILEDATA;

    int len = sIn.length();
    if (len == 0)
      s = "error sending, please retry...";
    else if (len > MAX_FILE_SIZE)
      s = "file is too long!";
    else{
      // parse time-events from .txt file and add them. Events are seperated by \n (newline)
      // but we can handle \r\n or \n\r as well (that's handled by p2.js in javascript).
      // What we see here is the file with % as a line-seperator. We still must filter
      // comments (which begin with #) and leading whitespace or empty lines.
      // lines filtered out.
      String sOut = "";
      int iCount = 0;
      for (int ii = 0; ii <= len; ii++){
        char c = ii >= len ? '%' : sIn[ii]; // simulate line-terminator at end of file
        if (c == '%' && sOut.length() > 0){ // line-seperator (it won't decode %0A so tried % alone...)
          yield();
          t_event slotData = {0};
          if (TSC.StringToTimeSlot(sOut, slotData)){
            if (TSC.AddTimeSlot(slotData))
              iCount++; // added event!
          }
          sOut = "";
        }
        else
          sOut += c;
      }
      if (iCount)
        s = String(iCount) + " events added!";
      else
        s = "no events added...";
    }
  }

  if (s != "")
    request->send(HTTPCODE_OK, "text/html", B64C.hnShiftEncode(s).c_str());
}

// Replaces placeholder %VAR% values in HTML being served from here
// (index.html, p1.html, p2.html and style.css files stored in SPIFFS flash-memory) to browser "out there"
String ReplaceHtmlPercentSign(const String& var){
  if (var == PH_P2DELSTYLE)
    // here we need to hide the delete form if no items to show...
    return g_slotCount ? "display:inline" : "display:none";

  if (var == PH_P2DELITEMS){
    // This is in p2.html and allows us to insert "option" tags representing start-stop entries we can delete
    // Ex: <option value="on">on</option>
    String sRet = "";
    if (g_slotCount > 0){
      //TODO - decide how we want to store time-strings in the flash-memory - use the RTC EEPROM??? Store as web-format or ESP32 time-date format???
      // https://www.html.am/tags/html-option-tag.cfm
      for (int ii = 0; ii < MAX_TIME_SLOTS; ii++){
        // get the time-item and format it however we want (it's just for display, we use the index to delete an item)

        t_event slotData = {0};
        // Get the time-slot into slotData
        if (TSC.GetTimeSlot(ii, slotData)){ // by-reference
          sRet += "<option value=" + String(ii) + ">" + TSC.TimeSlotToSpaceSepString(slotData) + "</option>";

          // NO SUCCESS GETTING COLOR TO WORK (FireFox)
          //String sColor = slotData.bEnable ? "black" : "gray";
          // option accepts: value, selected, label, disabled, id - I'm trying style
          //sRet += "<option value='" + String(ii) + "' style='color:" + String(sColor) + "'>" + TSC.TimeSlotToString(slotData) + "</option>";

          //String sLabel = slotData.bEnable ? "" : "disabled";
          // option accepts: value, selected, label, disabled, id - I'm trying style
          //sRet += "<option value='" + String(ii) + "' label='" + String(sLabel) + "'>" + TSC.TimeSlotToString(slotData) + "</option>";

          //sRet += "<option value='" + String(ii) + "'" + (slotData.bEnable ? "" : " disabled") + ">" + TSC.TimeSlotToString(slotData) + "</option>";

          //prtln("sending value:" + String(ii) + ", \"" + sTime + "\"");
        }
      }
    }
    return sRet;
  }

  // appears in p1.html
  if (var == PH_P1APMODE){
    // NOTE tried inserting {% if %P1APMODE% } {% endif %} into webpage but it would not handle it...
    // so - this just substitures the var with ALL the HTML and javascript needed to enter SSID etc.!
    // Both input and label fields are aligned in the CSS file
    String sRet;
    String sDummyPass = ""; // empty string so user knows they MUST enter PW!
    if (g_bSoftAP){
      String sTemp;
      if (g_sSSID.length() != 0)
          sTemp = g_sSSID;
      else
          sTemp = "(not set)";

      RefreshSct();
      B64C.SetB64Table(-1, 0);
      B64C.ShuffleB64ExternalTable();

      // set random MAC for station-mode WiFi scan in AP mode - for security purposes
      // the normal mac gets set back when AP mode exits and we reconnect in
      // router-station mode...
      ProcessRandMAC(WIFI_IF_STA);

      sRet = "<form action='" + String(EP_GET_P1) + "' method='get' name='fName' id='fName'>"
             "<input type='hidden' name='" + PARAM_WIFINAME + "' id='hidName'>WiFi Name:<br>"
             "<input type='text' id='wifiName' list='wifiNames'>"
             "<datalist id='wifiNames'>" + WiFiScan(sTemp) + "</datalist><br>"
             "<input type='hidden' name='" + PARAM_WIFIPASS + "' id='hidPass'>Password:<br>"
             "<input type='password' id='wifiPass' value='" + sDummyPass + "' maxlength='" + MAXPASS + "'>"
             "<input type='submit' value='Submit'></form><br>"
             "<a href='" + String(EP_GET_P1) + "?" + PARAM_BUTRST + "=hnEncode(" + PARAM_BUTRST_VALUE + ")'><button>Restore</button></a>"
             "<script>" + "$('#fName').submit(function(){"
                  "var wfn = document.getElementById('wifiName');"
                  "document.getElementById('hidName').value=hnEncode(wfn.value);"
                  "wfn.value='';"
                  "var wfp=document.getElementById('wifiPass');"
                  "document.getElementById('hidPass').value=hnEncode(wfp.value);"
                  "wfp.value='';"
                "});"
             "</script>";
    }
    else
      sRet = ""; // replace %P1APMODE% with nothing if in STA WiFi mode!
    return sRet;
  }

  // appears in index.html, p1.html, p2.html
  if (var == PH_MAXSCT){
    RefreshSct();
    B64C.SetB64Table(-1, 0);
    B64C.ShuffleB64ExternalTable();
    return "<script>" + getSctMinMaxAsJS() + "</script>";
  }

  // appears in index.html and p2.html
  if (var == PH_HOSTNAME)
    return g_sHostName+".local";

  // appears in index.html
  if (var == PH_LABEL_A)
    return g_sLabelA;
  if (var == PH_LABEL_B)
    return g_sLabelB;
#if ENABLE_SSR_C_AND_D
  if (var == PH_LABEL_C)
    return g_sLabelC;
  if (var == PH_LABEL_D)
    return g_sLabelD;
#endif
  if (var == PH_PERVARS)
    return "<script>var varPerMax=" + String(g_perVals.perMax) +
           ";var varPerUnits=" + String(g_perVals.perUnits) +
           ";var varPerVal=" + String(g_perVals.perVal) + ";</script>";

  // appears in p1.html
  if (var == PH_P1VARS)
    return "<script>var chan=" + String(g8_midiChan) +
           ";var na=" + String(g8_midiNoteA) +
           ";var nb=" + String(g8_midiNoteB) +
#if ENABLE_SSR_C_AND_D
           ";var nc=" + String(g8_midiNoteC) +
           ";var nd=" + String(g8_midiNoteD) +
#else
           ";var nc=0" +
           ";var nd=0" +
#endif
           ";var varPhaseBVal=" + String(g_perVals.phaseB) +
           ";var varDcAVal=" + String(g_perVals.dutyCycleA) +
           ";var varDcBVal=" + String(g_perVals.dutyCycleB) +
#if ENABLE_SSR_C_AND_D
           ";var varPhaseCVal=" + String(g_perVals.phaseC) +
           ";var varPhaseDVal=" + String(g_perVals.phaseD) +
           ";var varDcCVal=" + String(g_perVals.dutyCycleC) +
           ";var varDcDVal=" + String(g_perVals.dutyCycleD) + ";</script>";
#else
           ";var varPhaseCVal=0" +
           ";var varPhaseDVal=0" +
           ";var varDcCVal=0" +
           ";var varDcDVal=0" + ";</script>";
#endif
  prtln("Unknown:" + var);
  return String();
}
