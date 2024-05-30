// this file WSHandlers.cpp
#include "FanController.h"

// Web-page placeholders we fill-in dynamically as the html file is "served"
// index.html
const char PH_HOSTNAME[] = "HOSTNAME";
const char PH_MAXSCT[] = "MAXSCT";
const char PH_PERVARS[] = "PERVARS";
const char PH_LABEL_A[] = "LABELA";
const char PH_LABEL_B[] = "LABELB";

// p1.html
const char PH_P1APMODE[] = "P1APMODE";
const char PH_P1VARS[] = "P1VARS"; // combination script tag with vars for g8_midiChan, g8_midiNoteA, g8_midiNoteB

// p2.html
const char PH_P2DELSTYLE[] = "P2DELSTYLE";
const char PH_P2DELITEMS[] = "P2DELITEMS";

// response to async http request to mDNS local ESP32s every 5 sec.
const char HTTP_PARAM_COMMAND[] = "jeisl"; // universal "command" parameter

//const char HTTP_ASYNCFAILSAFEREQ[] = "/leofw";
//const char HTTP_ASYNCFAILSAFEREQ_PARAM_RANDTOKEN[] = "lekcn";
//const char HTTP_ASYNCFAILSAFEREQ_PARAM_MACLT[] = "lwhcb";

const char HTTP_ASYNCCANRXREQ[] = "/qhrfs";
const char HTTP_ASYNCCANRXREQ_PARAM_TOK3BITS[] = "lejcd";

//const char HTTP_ASYNCTEXTREQ[] = "/qhspu";
//const char HTTP_ASYNCTEXTREQ_PARAM_TEXT[] = "lejcy";

const char HTTP_ASYNCREQ[] = "/keuve";
const char HTTP_ASYNCREQ_PARAM_TIMESET[] = "meyvn";

// ------------------ web-server handlers -------------------

// NOTE: These all appear both here and in the .js files for the web-pages served out of SPIFFS file-system
// by the AsyncWebServer library. You can best change via Notepad++ by opening all .html files and .js (in "origdata" folder)
// files as well as this file and WSHandlers.h and use Find/Replace "in all opened documents". Then upload the changed .js files
// to https://obfuscator.io/ and save obfuscated files in the "data" folder.

// sct.js heartbeat /getHeart
const char PARAM_HEARTBEAT[]   = "neihs";

// index.html
const char PARAM_HOSTNAME[]    = "hddsi"; // hnEnc
const char PARAM_PERMAX[]      = "geuge"; // perMax
const char PARAM_PERUNITS[]    = "djeuc"; // perUnits
const char PARAM_PERVAL[]      = "rrksv"; // perVal
const char PARAM_STATE1[]      = "mwial"; // stateTxtA
const char PARAM_STATE2[]      = "uvohh"; // stateTxtB
const char PARAM_LABEL_A[]     = "wjdte"; // labelTxtA
const char PARAM_LABEL_B[]     = "meufw"; // labelTxtB
const char PARAM_TEXT[]        = "ifhrv"; // text-message

// loginIndex.html
const char PARAM_UDID[]   = "bwihl"; // used in firmware update
const char PARAM_UDPW[]   = "pdklt";

// p1.html

// (PARAM_BUTRST, PARAM_WIFINAME and PARAM_WIFIPASS can be changed without reprocessing the spiffs web-data)
const char PARAM_BUTRST[]     = "neuqj"; // restore button press
const char PARAM_WIFINAME[]   = "fgdje"; // set by hidden field on p1.html - variable %P1APMODE% is replaced by two edit-input fields
const char PARAM_WIFIPASS[]   = "rsgyb"; // data submitted is handled in in ReplaceHtmlPercentSign().

// changing these requires serching for the strings in p1.html/p1.js and changeing there too!
const char PARAM_PHASE[]      = "jeita";
const char PARAM_DC_A[]       = "neufb";
const char PARAM_DC_B[]       = "xbmey";
const char PARAM_MIDICHAN[]   = "ahejn";
const char PARAM_MIDINOTE_A[] = "ehwdo";
const char PARAM_MIDINOTE_B[] = "fjezm";

// p2.html
// received from p2.html when Add button pressed
const char PARAM_MINUTE[] = "hriqn";
const char PARAM_HOUR[] = "nwuds";
const char PARAM_SECOND[] = "bbeis";
const char PARAM_AMPM[] = "heidc";
const char PARAM_DEVICE_MODE[] = "whrro";
const char PARAM_DEVICE_ADDR[] = "ajedd";
const char PARAM_REPEAT_MODE[] = "nriwq";
const char PARAM_REPEAT_COUNT[] = "jegsi";
const char PARAM_EVERY_COUNT[] = "ehitm";
const char PARAM_DATE[] = "wjyrc";
// when Edit button pressed on p2.html
const char PARAM_EDITINDEX[] = "ejddo";
const char PARAM_DELINDEX[] = "heifq";
// received from p2.html
const char PARAM_REPLACEINDEX[] = "jfrej";
const char PARAM_INCLUDETIMINGCYCLE[] = "whsin";
const char PARAM_TIMINGCYCLEINREPEATS[] = "ydjyz";
const char PARAM_USEOLDTIMEVALS[] = "vjenw";
const char PARAM_DATETIME[] = "gejcc";
const char PARAM_FILEDATA[] = "wqjun";
const char PARAM_ERASEDATA[] = "lkeism";

//void HandleHttpAsyncTextReq(AsyncWebServerRequest *request){
//
//  IPAddress rip = request->client()->remoteIP();
//
//  if (!g_bSyncRx){
//    SendHttpClientResponse(request, HTTPRESP_RXDISABLED, HTTPCODE_RXDISABLED);
//    prtln("HandleHttpAsyncTextReq(): Local Rx is disabled! (remote ip " + rip.toString());
//    return;
//  }
//  
//  int rxIdx = IML.FindMdnsIp(rip);
//  if (rxIdx < 0){
//    rxIdx = IML.AddMdnsIp(rip);
//    if (rxIdx < 0){ // add incomming IP to mDNS table
//      prtln("HandleHttpAsyncCanRxReq(): UNABLE TO ADD ip to mDNS table: " + rip.toString());
//      SendHttpClientResponse(request, HTTPRESP_ADDIP_FAIL, HTTPCODE_ADDIP_FAIL);
//      return;
//    }
//    prtln("HandleHttpCanRxReq(): ip added to mDNS table: " + rip.toString());
//  }
//
//  String sReply = HTTPRESP_TXT_FAIL;
//  int code = HTTPCODE_TXT_FAIL;
//
//  int rxToken = IML.GetRxToken(rxIdx);
//
//  if (request->hasParam(HTTP_ASYNCTEXTREQ_PARAM_TEXT)){
//    String s1 = request->getParam(HTTP_ASYNCTEXTREQ_PARAM_TEXT)->value();
////prtln("HandleHttpAsyncFailsafeReq(): raw received HTTP_ASYNCTEXTREQ_PARAM_TEXT: \"" + s1 + "\"");
//
//    // HTTP_ASYNCTEXTREQ_PARAM_TEXT is the AES 256-bit encrypted text message
//    s1 = MyDecodeStr(s1, HTTP_TABLE3, rxToken, CIPH_CONTEXT_BACKGROUND);
//    if (!s1.isEmpty()){
//      String sIP = rip.toString();
//      if (g_text.isEmpty()){
//        if (s1.length() <= HTTP_TEXT_MAXCHARS){
//          s1 = sIP + ": \"" + s1 + "\""; // let web-page know who sent the text...
//          prtln("received text: " + s1);
//          g_text = s1; // the g_text may eventually be hnEncoded() and sent to a web-browser via a PARAM_TEXT request
//          code = HTTPCODE_TXT_OK;
//          sReply = HTTPRESP_TXT_OK;
//        }
//        else
//          prtln("ERROR: text received is too long! " + sIP + "\"" + s1 + "\"");
//      }
//      else
//        prtln("ERROR: new text received before previous text sent! " + sIP + "\"" + s1 + "\"");
//    }
//  }
//
//  if (sReply != HTTPRESP_TXT_FAIL){
//    // (NOTE 7/2023: decided to use rxtoken to encode the response, and tx token in the other unit's callback to decode.
//    // This is so that the entire Tx->Rx->Response loop is completed using the same token-set)
//    sReply = MyEncodeStr(sReply, HTTP_TABLE1, rxToken, CIPH_CONTEXT_BACKGROUND);
//    if (sReply.isEmpty()){
//      sReply = HTTPRESP_TXT_FAIL;
//      prtln("HandleHttpAsyncTextReq(): MyEncodeStr(sReply) returned empty string!");
//    }
//  }
//  if (sReply == HTTPRESP_TXT_FAIL){
//    prtln("HandleHttpAsyncTextReq(): Sending HTTPCODE_TXT_FAIL!");
//    code = HTTPCODE_TXT_FAIL;
//    sReply = MyEncodeStr(sReply, HTTP_TABLE1, FAILSAFE_TOKEN_3, CIPH_CONTEXT_BACKGROUND);
//  }
//  SendHttpClientResponse(request, sReply, code);
//}

void HandleHttpAsyncCanRxReq(AsyncWebServerRequest *request){

  IPAddress rip = request->client()->remoteIP();

  if (!g_bSyncRx){
    SendHttpClientResponse(request, HTTPRESP_RXDISABLED, HTTPCODE_RXDISABLED);
    prtln("HandleHttpAsyncCanRxReq(): Local Rx is disabled! (remote ip " + rip.toString());
    return;
  }

  int rxIdx = IML.FindMdnsIp(rip);
  if (rxIdx < 0){
    rxIdx = IML.AddMdnsIp(rip);
    if (rxIdx < 0){ // add incomming IP to mDNS table
      prtln("HandleHttpAsyncCanRxReq(): UNABLE TO ADD ip to mDNS table: " + rip.toString());
      SendHttpClientResponse(request, HTTPRESP_ADDIP_FAIL, HTTPCODE_ADDIP_FAIL);
      return;
    }
    prtln("HandleHttpAsyncCanRxReq(): ip added to mDNS table: " + rip.toString());
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
    int iMac = MyDecodeNum(sMac, HTTP_TABLE2, FAILSAFE_TOKEN_3, CIPH_CONTEXT_BACKGROUND);
    if (iMac > 0){
      uint16_t localMacLT = IML.GetOurDeviceMacLastTwoOctets();
      uint16_t remoteMacLT = (uint16_t)iMac;
      if (localMacLT > 0){
        if (localMacLT != remoteMacLT) // leave -1 if mac-last-two-octets are the same
          weAreMoreMaster = (localMacLT > remoteMacLT) ? 1 : 0;
        else
          prtln("HandleHttpAsyncCanRxReq(): WARNING remote and local MAC last-two-octets are the same! " + String(remoteMacLT));
        if (IML.GetMdnsMAClastTwo(rxIdx) != remoteMacLT){
          IML.SetMdnsMAClastTwo(rxIdx, remoteMacLT);
          prtln("HandleHttpAsyncCanRxReq(): MAC last-two added: " + String(remoteMacLT));
          CheckMasterStatus();
          // purge possible pending command requesting MAC!
//            String sRef = IML.GetSendStr(rxIdx);
//            HMC.StripParam(CMreqMacMin, CMreqMacMax, sRef);
//            IML.SetSendStr(rxIdx, sRef);
        }
      }
    }
    else
      prtln("HandleHttpAsyncCanRxReq(): can't decode MAC header!");
  }
  else
    prtln("HandleHttpAsyncCanRxReq(): no MAC header!");
  
  int code;
  String sReply;

  if (request->hasParam(HTTP_PARAM_COMMAND) &&
          request->hasParam(HTTP_ASYNCCANRXREQ_PARAM_TOK3BITS)){
//prtln("HandleHttpAsyncCanRxReq(): raw received HTTP_PARAM_COMMAND: \"" + s1 + "\"");
    String sTokHigh = request->getParam(HTTP_ASYNCCANRXREQ_PARAM_TOK3BITS)->value(); // get (high 3-bits+1) << 4
    if (sTokHigh.isEmpty()){
      prtln("HandleHttpAsyncCanRxReq(): HTTP_ASYNCCANRXREQ_PARAM_TOK3BITS value is empty!");
      goto finally;
    }
//prtln("DEBUG: HandleHttpAsyncCanRxReq(): : raw HTTP_ASYNCCANRXREQ_PARAM_TOK3BITS (key): \"" + sTokHigh + "\"");
    int iTokHigh = MyDecodeNum(sTokHigh, HTTP_TABLE1, g_defToken, CIPH_CONTEXT_BACKGROUND);
    String sCmd = MyDecodeStr(request->getParam(HTTP_PARAM_COMMAND)->value(), HTTP_TABLE2, g_defToken, CIPH_CONTEXT_BACKGROUND);
    if (iTokHigh < 0 || sCmd.isEmpty()){
//prtln("DEBUG: HandleHttpAsyncCanRxReq(): Can't decode HTTP_PARAM_COMMAND with g_defToken= " + String(g_defToken) + ", ip=" + rip.toString());
      if (weAreMoreMaster == -1){ // we don't know who's "more master" yet (haven't received MAC last-two)
        code = HTTPCODE_CANRX_NOMAC_FAIL;
//prtln("DEBUG: HandleHttpAsyncCanRxReq(): sending HTTPCODE_CANRX_NOMAC_FAIL");
      }
      else{
        code = HTTPCODE_CANRX_DECODE_FAIL;
//prtln("DEBUG: HandleHttpAsyncCanRxReq(): sending HTTPCODE_CANRX_DECODE_FAIL");
      }
      // send back our g_defToken if we "outrank" the remote or NO_TOKEN if not
// NOTE: if both sides are still unset - we'll be in a deadlock... best to wait to send until it's set???
prtln("DEBUG: HandleHttpAsyncCanRxReq(): weAreMoreMaster=" + String(weAreMoreMaster) + " (sends NO_TOKEN if 0 or -1 (unset)!)");
      int iDefTok = (weAreMoreMaster == 1) ? g_defToken : NO_TOKEN;
      CIP.saveCiphKey(CIPH_CONTEXT_BACKGROUND);
      CIP.setCiphKey(sTokHigh); // use the raw HTTP_ASYNCCANRXREQ_PARAM_TOK3BITS encoded parameter as key!
      sReply = MyEncodeNum(iDefTok, HTTP_TABLE2, FAILSAFE_TOKEN_2, CIPH_CONTEXT_BACKGROUND); // send our g_defToken to remote for it to set its g_defToken the same!
      CIP.restoreCiphKey(CIPH_CONTEXT_BACKGROUND);
      // do a system-wide default token reset in 3 minutes
//      if (MINUTES_AFTER_DEF_TOKEN_FAIL_TO_INITIATE_RESET){
//        int ipCount = IML.GetCount();
//        if (ipCount)
//          g16_sendDefTokenTimer = g16_sendDefTokenTime-(ipCount+ MINUTES_AFTER_DEF_TOKEN_FAIL_TO_INITIATE_RESET);
//      }
    }
    else if (sCmd == HTTP_COMMAND_CANRX){
      iTokHigh >>= CANRX_TOKEN_SHIFT; // shift high bits into place (>>4)
      if (iTokHigh >= 1 && iTokHigh <= 8){
        iTokHigh -= 1; // we send token plus one to avoid 0!
        code = HTTPCODE_CANRX_OK;
        int iTokLow = random(1,8+1); // 0-7 plus 1 to avoid 0!
        sReply = MyEncodeNum(iTokLow<<CANRX_TOKEN_SHIFT, HTTP_TABLE3, g_defToken, CIPH_CONTEXT_BACKGROUND); // send back: ((lower 3 bits plus one) << 4)
        int rxToken = ((iTokHigh-1)<<3)+iTokLow;
        IML.SetRxToken(rxIdx, rxToken);
//prtln("DEBUG: HandleHttpAsyncCanRxReq() HTTP_COMMAND_CANRX: got high 3-bits, setting rxToken to: " + String(rxToken));
//prtln("DEBUG: HandleHttpAsyncCanRxReq() HTTP_COMMAND_CANRX: sending back newly generated low 3-bits: " + String(iTokLow-1));
      }
      else
        prtln("DEBUG: HandleHttpAsyncCanRxReq() sCmd unknown/unprocessed: \"" + sCmd + "\"");
    }
  }

finally:
  if (sReply.isEmpty()){
    sReply = HTTPRESP_CANRX_FAIL;
//prtln("DEBUG: HandleHttpAsyncCanRxReq(): sReply is empty. Set sReply = HTTPRESP_CANRX_FAIL");
  }

  if (sReply == HTTPRESP_CANRX_FAIL){
    code = HTTPCODE_CANRX_FAIL;
    sReply = MyEncodeStr(sReply, HTTP_TABLE3, FAILSAFE_TOKEN_1, CIPH_CONTEXT_BACKGROUND);
//prtln("DEBUG: HandleHttpAsyncCanRxReq(): Set code = HTTPCODE_CANRX_FAIL");
  }
  SendHttpClientMacResponse(request, sReply, code);
}

void HandleHttpAsyncReq(AsyncWebServerRequest *request){

  IPAddress rip = request->client()->remoteIP();
  
  if (!g_bSyncRx){
    SendHttpClientResponse(request, HTTPRESP_RXDISABLED, HTTPCODE_RXDISABLED);
    prtln("HandleHttpAsyncReq(): Local Rx is disabled! (remote ip " + rip.toString());
    return;
  }
  
  String sReply;
  int code;


//    // block web update if locked unless in AP mode
//    if (IsLockedAlertGet(request, P1_FILENAME, true))
//      return;

  int rxIdx = IML.FindMdnsIp(rip);
  if (rxIdx < 0){
    rxIdx = IML.AddMdnsIp(rip);
    if (rxIdx < 0){ // add incomming IP to mDNS table (we will update its rxToken below)
      prtln("HandleHttpAsyncReq(): UNABLE TO ADD ip to mDNS table: " + rip.toString());
      SendHttpClientResponse(request, HTTPRESP_ADDIP_FAIL, HTTPCODE_ADDIP_FAIL);
      return; // return -1 for errors that don't require drastic action...
    }
    prtln("HandleHttpAsyncReq(): ip added to mDNS table: " + rip.toString());
  }

  int saveRxTok = IML.GetRxToken(rxIdx); // save before calling HMC.DecodeParameters()!
//prtln("DEBUG: HandleHttpAsyncReq(): saveRxTok: " + String(saveRxTok));    

  if (saveRxTok == NO_TOKEN){
    sReply = MyEncodeStr(HTTPRESP_RXTOKEN_FAIL, HTTP_TABLE3, FAILSAFE_TOKEN_1, CIPH_CONTEXT_BACKGROUND);
// don't send Can Rx query here... we send it if NO_TOKEN on SendHttpReq()
// QueueTask(TASK_SEND_CANRX_QUERY, rip.toString()); // this will get Rx/Tx tokens!
    SendHttpClientResponse(request, sReply, HTTPCODE_RXTOKEN_FAIL);
    prtln("HandleHttpAsyncReq(): Rx or Tx token = NO_TOKEN, sending HTTPCODE_RXTOKEN_FAIL! " + rip.toString());
    return; // return -1 for errors that don't require drastic action...
  }
  
  if (request->hasParam(HTTP_PARAM_COMMAND)){
    String sReq = request->getParam(HTTP_PARAM_COMMAND)->value();
//prtln("debug: raw received HTTP_PARAM_COMMANDS: \"" + s + "\"");

    bool bPendingTokenWasSet; // set flag by-reference...
    int macLastTwo = HMC.DecodeParameters(sReq, rxIdx, bPendingTokenWasSet, false); // returns decoded string in s by-reference!
//prtln("debug: partially decoded after calling HMC.DecodeParameters(): \"" + sReq + "\"");
//prtln("debug: macLastTwo from HMC.DecodeParameters(): " + String(macLastTwo));

    if (macLastTwo < 0){ // negative if error in HMC.DecodeParameters()!
      if (macLastTwo == -3){ // failed to decode
        int rxPrevToken = IML.GetRxPrevToken(rxIdx);
        if (rxPrevToken != NO_TOKEN){
          IML.SetRxToken(rxIdx, rxPrevToken);
          IML.SetRxPrevToken(rxIdx, NO_TOKEN);
          sReq = HTTPRESP_DECPREV_FAIL;
          code = HTTPCODE_DECPREV_FAIL;
          prtln("DEBUG: HandleHttpAsyncReq(): setting Rx Token to previous and sending response HTTPCODE_DECPREV_FAIL!");
        }else{
          IML.SetRxToken(rxIdx, NO_TOKEN);
          sReq = HTTPRESP_DECODE_FAIL;
          code = HTTPCODE_DECODE_FAIL;
          prtln("DEBUG: HandleHttpAsyncReq(): previous Rx token was NO_TOKEN, sending response HTTPCODE_DECODE_FAIL!");
        }
      }
      else{ // checksum or other failure of parameter-commands decoding...
        sReq = HTTPRESP_PARAM_FAIL;
        code = HTTPCODE_PARAM_FAIL;
      }
      prtln("DEBUG: HandleHttpAsyncReq(): HMC.DecodeParameters() returned error: " + String(macLastTwo));
    }
    else{
      //s = "Decoded: \"" + s + '"';
//prtln("DEBUG: HandleHttpAsyncReq(): rxToken after DecodeParameters(): " + String(IML.GetRxToken(rxIdx)));    

      // if a pending new token was received, we want to reply "TOKOK" instead of "PARAMOK"
      if (bPendingTokenWasSet){
        sReq = HTTPRESP_TOK_OK;
        code = HTTPCODE_TOK_OK;
      }else{
        sReq = HTTPRESP_PARAM_OK;
        code = HTTPCODE_PARAM_OK;
      }

      if (macLastTwo > 0){ // positive if a mac-string was decoded
        IML.SetMdnsMAClastTwo(rxIdx, macLastTwo);
        CheckMasterStatus();
      }
      
      prtln("HandleHttpAsyncReq(): HTTP_PARAM_COMMAND OK!: decoded macLastTwo=" + String(macLastTwo));
    }
//prtln("debug: sending HTTP_PARAM_COMMAND response code and string: " + String(code) + ", s=\"" + s + "\"");
    sReply = sReq;
  }

  if (request->hasParam(HTTP_ASYNCREQ_PARAM_TIMESET) && (code == HTTPCODE_PARAM_OK || code == HTTPCODE_TOK_OK)){
    String sTmp = MyDecodeStr(request->getParam(HTTP_ASYNCREQ_PARAM_TIMESET)->value(), HTTP_TABLE1, saveRxTok, CIPH_CONTEXT_BACKGROUND);
    if (!sTmp.isEmpty()){
      if (g_bSyncTime)
        sTmp = SetTimeDate(sTmp); // returns the time/date that was set or empty if not able to read
      else
        prtln("HandleHttpAsyncReq(): got remote time-set, but sync is off!");
    }

    if (sTmp.isEmpty()){
      code = HTTPCODE_TIMESET_FAIL; // time-set failed!
      prtln("HandleHttpAsyncReq(): time was not set!");
    }
    else
      prtln("HandleHttpAsyncReq(): time remotely set: " + sTmp);

    // don't add anything more to sReply here - just means it must be parsed by the HTTP client callback!
  }

  if (sReply == HTTPRESP_DECODE_FAIL || sReply == HTTPRESP_DECPREV_FAIL)
    sReply = MyEncodeStr(sReply, HTTP_TABLE3, FAILSAFE_TOKEN_1, CIPH_CONTEXT_BACKGROUND);
  else if (sReply != HTTPRESP_PARAM_FAIL)
    // (NOTE 7/2023: decided to use rxtoken to encode the response, and tx token in the other unit's callback to decode.
    // This is so that the entire Tx->Rx->Response loop is completed using the same token-set)
    sReply = MyEncodeStr(sReply, HTTP_TABLE1, saveRxTok, CIPH_CONTEXT_BACKGROUND);

  if (sReply.isEmpty()){
    sReply = HTTPRESP_PARAM_FAIL;
    prtln("HandleHttpAsyncReq(): MyEncodeStr(sReply) returned empty string!");
  }
  if (sReply == HTTPRESP_PARAM_FAIL){
    sReply = MyEncodeStr(sReply, HTTP_TABLE3, FAILSAFE_TOKEN_1, CIPH_CONTEXT_BACKGROUND);
    code = HTTPCODE_PARAM_FAIL;
    prtln("HandleHttpAsyncReq(): sending HTTPRESP_PARAM_FAIL!");
  }
  SendHttpClientResponse(request, sReply, code);
}

void SendHttpClientMacResponse(AsyncWebServerRequest *request, String sResp, int code){
  if (!sResp.isEmpty()){
    AsyncWebServerResponse *r = request->beginResponse(code, "text/plain", sResp);
    String sIp = request->client()->localIP().toString();
//prtln("DEBUG: SendHttpClientMacResponse(): sIp before MyEncodeStr(): " + sIp);
    sIp = MyEncodeStr(sIp, HTTP_TABLE3, FAILSAFE_TOKEN_4, CIPH_CONTEXT_BACKGROUND);
//prtln("DEBUG: SendHttpClientMacResponse(): sIp after MyEncodeStr() (about to call r->addHeader()): " + sIp);
    r->addHeader(HTTP_SERVER_IP_HEADER_NAME, sIp.c_str());
    String sMac = MyEncodeNum(IML.GetOurDeviceMacLastTwoOctets(), HTTP_TABLE1, FAILSAFE_TOKEN_5, CIPH_CONTEXT_BACKGROUND);
    r->addHeader(HTTP_SERVER_MAC_HEADER_NAME, sMac.c_str());
    //r->addHeader("Connection", "close");
    request->send(r);
  }
}

void SendHttpClientResponse(AsyncWebServerRequest *request, String sResp, int code){
  if (!sResp.isEmpty()){
    AsyncWebServerResponse *r = request->beginResponse(code, "text/plain", sResp);
    String sIp = request->client()->localIP().toString();
//prtln("DEBUG: SendHttpClientResponse(): sIp before MyEncodeStr(): " + sIp);
    sIp = MyEncodeStr(sIp, HTTP_TABLE3, FAILSAFE_TOKEN_4, CIPH_CONTEXT_BACKGROUND);
//prtln("DEBUG: SendHttpClientResponse(): sIp after MyEncodeStr(): " + sIp);
    r->addHeader(HTTP_SERVER_IP_HEADER_NAME, sIp.c_str());
    //r->addHeader("Connection", "close");
    request->send(r);
  }
}

void HandleButtonsReq(AsyncWebServerRequest *request){
    if (IsLockedAlertGet(request, INDEX_FILENAME))
      return;

    if (request->hasParam("A")){
      String buttonMode = B64C.hnDecode(request->getParam("A")->value());
      if (buttonMode == "0"){
        if (g8_nvSsrMode1 != SSR_MODE_OFF){
          g8_nvSsrMode1 = SSR_MODE_OFF;
          SetSSRMode(GPIO32_SSR_1, g8_nvSsrMode1);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
        }
      }
      else if (buttonMode == "1"){
        if (g8_nvSsrMode1 != SSR_MODE_ON){
          g8_nvSsrMode1 = SSR_MODE_ON;
          SetSSRMode(GPIO32_SSR_1, g8_nvSsrMode1);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
        }
      }
      else if (buttonMode == "2"){
        if (g8_nvSsrMode1 != SSR_MODE_AUTO){
          g8_nvSsrMode1 = SSR_MODE_AUTO;
          SetSSRMode(GPIO32_SSR_1, g8_nvSsrMode1);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
        }
      }
    }
    else if (request->hasParam("B")){
      String buttonMode = B64C.hnDecode(request->getParam("B")->value());
      if (buttonMode == "0"){
        if (g8_nvSsrMode2 != SSR_MODE_OFF){
          g8_nvSsrMode2 = SSR_MODE_OFF;
          SetSSRMode(GPIO23_SSR_2, g8_nvSsrMode2);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
        }
      }
      else if (buttonMode == "1"){
        if (g8_nvSsrMode2 != SSR_MODE_ON){
          g8_nvSsrMode2 = SSR_MODE_ON;
          SetSSRMode(GPIO23_SSR_2, g8_nvSsrMode2);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
        }
      }
      else if (buttonMode == "2"){
        if (g8_nvSsrMode2 != SSR_MODE_AUTO){
          g8_nvSsrMode2 = SSR_MODE_AUTO;
          SetSSRMode(GPIO23_SSR_2, g8_nvSsrMode2);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
        }
      }
    }

    request->send(204, "text/html", "");
//    request->send(SPIFFS, "/index.html", String(), false, ReplaceHtmlPercentSign);
}

void HandleHeartbeatReq(AsyncWebServerRequest *request){
  if (request->hasParam(PARAM_HEARTBEAT)){
    String s;

    // check for received text (via internal Http client communications between units)
    // and echo it to the connected web-browser...
    for (int ii = 0; ii < IML.GetCount(); ii++){
      String sRxTxt = IML.GetRxTxtStr(ii);
      if (!sRxTxt.isEmpty()){
        if (sRxTxt.length() < MAXTXTLEN){
          s = TEXT_PREAMBLE + g_sHostName + ": \"" + sRxTxt + "\"";
//prtln("DEBUG: HandleHeartbeatReq(): sending text from " + IML.GetIP(ii).toString() + " to web-browser at " + request->client()->remoteIP().toString() + ": \"" + s + "\"");
        }
        IML.SetRxTxtStr(ii, "");
        break; // just send one - if there are more, send on subsequent heartbeat-requests...
      }
    }

    // if no text, send the status variable comma-separated values...
    if (s.isEmpty()){
      s = String(g8_lockCount) + "," + String(g16_pot1Value) + "," + String(g8_wifiSwState) + "," + String(g8_modeSwState);
    }
    request->send(200, "text/html", B64C.hnEncode(s).c_str());
  }
}

void HandleIndexReq(AsyncWebServerRequest *request){
  int count = request->params();

  // note: I only send one parameter but get 2 - an underscore is tacked on by jquery for some reason...
  if (count == 0)
    return;

  AsyncWebParameter* p = request->getParam(0);
  if (p == NULL)
    return;

  String sVal = B64C.hnDecode(p->value());

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
      s = (g_bSsr1On ? "ON" : "OFF");
      s += "," + SsrModeToString(g8_nvSsrMode1);
      s += "," + String(ComputeTimeToOnOrOffA()); // String(g_stats.AOnPrevCount+g_stats.AOnCounter)
      s += "," + PercentOnToString(g_stats.PrevDConA+g_stats.DConA, g_stats.HalfSecondCount+g_stats.HalfSecondCounter);
    }
  }
  else if (sName == PARAM_STATE2){
    if (sVal == PARAM_STATE2_VALUE){
      s = (g_bSsr2On ? "ON" : "OFF");
      s += "," + SsrModeToString(g8_nvSsrMode2);
      s += "," + String(ComputeTimeToOnOrOffB()); // String(g_stats.AOnPrevCount+g_stats.AOnCounter)
      s += "," + PercentOnToString(g_stats.PrevDConB+g_stats.DConB, g_stats.HalfSecondCount+g_stats.HalfSecondCounter);
    }
  }
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
      s = "<script>alert('" + s + "'); location.href = '" + String(INDEX_FILENAME) + "';</script>";
  }
  else{
    if (IsLockedAlertGetPlain(request, false))
      return;

    // locked commands...

    if (sName == PARAM_LABEL_A || sName == PARAM_LABEL_B){
      sVal.trim();
      if (sVal.isEmpty())
        s = "Transmission error or blank label...";
      else if (sVal.length() > LABEL_MAXLENGTH)
        s = "Max label length is: " + String(LABEL_MAXLENGTH) + "!";
      else{
        int iTask = (sName == PARAM_LABEL_A) ? TASK_LABEL_A : TASK_LABEL_B;
        QueueTask(iTask, sVal);
      }
      
      if (s != "")
        s = "<script>alert('" + s + "'); location.href = '" + String(INDEX_FILENAME) + "';</script>";
    }
    else{
      int iVal = B64C.hnDecNumOnly(sVal);

      if (sName == PARAM_PERMAX){
        QueueTask(TASK_PARMS, SUBTASK_PERMAX, iVal);
        iStatus = 1;
      }
      else if (sName == PARAM_PERUNITS){
        QueueTask(TASK_PARMS, SUBTASK_PERUNITS, iVal);
        iStatus = 1;
      }
      else if (sName == PARAM_PERVAL){
        QueueTask(TASK_PARMS, SUBTASK_PERVAL, iVal);
        iStatus = 1;
      }
    }
  }

  // 200 = OK
  // 204 = OK but No Content
  if (!s.isEmpty())
    request->send(200, "text/html", B64C.hnEncode(s).c_str());
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

  AsyncWebParameter* p = request->getParam(0);
  if (p == NULL)
    return;

  String sVal = B64C.hnDecode(p->value());

  if (sVal.isEmpty())
    return;

  String sName = p->name();

  // used to set our time from time on remote laptop
  if (sName == PARAM_DATETIME){
    s = SetTimeDate(sVal);
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
        QueueTask(TASK_RESET_SLOTS);
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
    request->send(200, "text/html", B64C.hnEncode(s).c_str());
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

  AsyncWebParameter* p = request->getParam(0);
  if (p == NULL)
    return;

  String sName = p->name();
  int iVal = B64C.hnDecNum(p->value());

  // all 8-bit values 0-100% and midi-note or channel
  if (iVal < 0 || iVal > 255){
    prtln("HandleAltP1Req(): iVal out of range for: \"" + sName + "\"");
    return;
  }
    
  if (sName == PARAM_PHASE)
    QueueTask(TASK_PARMS, SUBTASK_PHASE, iVal);
  else if (sName == PARAM_DC_A)
    QueueTask(TASK_PARMS, SUBTASK_DCA, iVal);
  else if (sName == PARAM_DC_B)
    QueueTask(TASK_PARMS, SUBTASK_DCB, iVal);
  else if (sName == PARAM_MIDICHAN){
    if (iVal != g8_midiChan){
      g8_midiChan = iVal;
      QueueTask(TASK_MIDICHAN);
    }
  }
  else if (sName == PARAM_MIDINOTE_A){
    if (iVal != g8_midiNoteA){
      g8_midiNoteA = iVal;
      QueueTask(TASK_MIDINOTE_A);
    }
  }
  else if (sName == PARAM_MIDINOTE_B){
    if (iVal != g8_midiNoteB){
      g8_midiNoteB = iVal;
      QueueTask(TASK_MIDINOTE_B);
    }
  }

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
    if (g_bSoftAP && B64C.hnDecode(request->getParam(PARAM_BUTRST)->value()) == PARAM_BUTRST_VALUE){
      QueueTask(TASK_WIFI_RESTORE);
      s = "<script>alert('SSID and password reset to previous values!');";
    }
    else
      iStatus = 1;
  }
  else if (request->hasParam(PARAM_WIFINAME) && request->hasParam(PARAM_WIFIPASS)){
    if (g_bSoftAP){
      String valN, valP;
      String sN = request->getParam(PARAM_WIFINAME)->value();
      int errorCodeN = B64C.hnDecode(sN, valN);
      String sP = request->getParam(PARAM_WIFIPASS)->value();
      int errorCodeP = B64C.hnDecode(sP, valP);

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
    request->send(200, "text/html", B64C.hnEncode(s).c_str());
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
  if (CancelTask(TASK_PAGE_REFRESH_REQUEST))
    g_bTellP2WebPageToReload = false;

  for (int ii = 0; ii < count; ii++){
    AsyncWebParameter* p = request->getParam(ii);

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
    String sVal = B64C.hnDecode(p->value());

    //prtln("hnDecode: " + sName + ":" + sVal);

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
      t.phase = t2.phase;
      t.perUnits = t2.perUnits;
      t.perMax = t2.perMax;
      t.perVal = t2.perVal;
    }
    else{
      t.dutyCycleA = g_perVals.dutyCycleA;
      t.dutyCycleB = g_perVals.dutyCycleB;
      t.phase = g_perVals.phase;
      t.perUnits = g_perVals.perUnits;
      t.perMax = g_perVals.perMax;
      t.perVal = g_perVals.perVal;
    }
  }
  else{
    t.dutyCycleA = 0xff;
    t.dutyCycleB = 0xff;
    t.phase = 0xff;
    t.perUnits = 0xff;
    t.perVal = 0xff;
    t.perMax = 0xffff;
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
    request->send(200, "text/html", B64C.hnEncode(s).c_str());
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
    String sIn = B64C.hnDecode(request->getParam(PARAM_FILEDATA, true)->value());

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
    request->send(200, "text/html", B64C.hnEncode(s).c_str());
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
             "<input type='hidden' name='" + String(PARAM_WIFINAME) + "' id='hidName'>WiFi Name:<br>"
             "<input type='text' id='wifiName' list='wifiNames'>"
             "<datalist id='wifiNames'>" + WiFiScan(sTemp) + "</datalist><br>"
             "<input type='hidden' name='" + String(PARAM_WIFIPASS) + "' id='hidPass'>Password:<br>"
             "<input type='password' id='wifiPass' value='" + sDummyPass + "' maxlength='" + String(MAXPASS) + "'>"
             "<input type='submit' value='Submit'></form><br>"
             "<a href='" + EP_GET_P1 + "?" + String(PARAM_BUTRST) + "=hnEncode(" + PARAM_BUTRST_VALUE + ")'><button>Restore</button></a>"
             "<script>" + "$('#fName').submit(function(){"
                  "var wfn = document.getElementById('wifiName');"
                  "document.getElementById('hidName').value=hnEncode(wfn.value);"
                  "wfn.value='';"
                  "var wfp=document.getElementById('wifiPass');"
                  "document.getElementById('hidPass').value=hnEncode(wfp.value);"
                  "wfp.value='';"
                "});"
             "</script>"
             "<script src='sct.js' type='text/javascript'></script>";
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
  if (var == PH_PERVARS)
    return "<script>var varPerMax=" + String(g_perVals.perMax) +
           ";var varPerUnits=" + String(g_perVals.perUnits) +
           ";var varPerVal=" + String(g_perVals.perVal) + ";</script>";

  // appears in p1.html
  if (var == PH_P1VARS)
    return "<script>var chan=" + String(g8_midiChan) +
           ";var na=" + String(g8_midiNoteA) +
           ";var nb=" + String(g8_midiNoteB) +
           ";var varPhaseVal=" + String(g_perVals.phase) +
           ";var varDcAVal=" + String(g_perVals.dutyCycleA) +
           ";var varDcBVal=" + String(g_perVals.dutyCycleB) + ";</script>";

  prtln("Unknown:" + var);
  return String();
}
