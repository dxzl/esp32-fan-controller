// this file Tasks.cpp
#include "FanController.h"

t_task g_tasks[MAX_TASKS];

void RunTasks(){
  t_task* p = GetFirstFullTaskSlot();
  if (!p) return;
  
  int iTask = p->iTask;
  int i1 = p->i1;
  int i2 = p->i2;  
  String s1 = p->s1;
  String s2 = p->s2;
  String s3 = p->s3;

  p->s3 = "";
  p->s2 = "";
  p->s1 = "";
  p->i2 = 0;
  p->i1 = 0;
  p->iTask = TASK_NONE; // don't forget to clear task! (do this last in case QueueTask() is being called from an ISR or event!)
  
  switch(iTask){
    case TASK_SEND_CANRX_QUERY:
      TaskSendCanRxQuery(s1);
    break;
    
    case TASK_PARMS:
      if (i1 == SUBTASK_RELAY_A)
        PC.PutPrefByte(EE_RELAY_A, g8_ssr1ModeFromWeb);
      else if (i1 == SUBTASK_RELAY_B)
        PC.PutPrefByte(EE_RELAY_B, g8_ssr2ModeFromWeb);
      else{
        SubtaskProcessParams(i1, i2);
        LimitPeriod(); // force end of cycle if it's over 10 sec!
      }
    break;

    case TASK_HOSTNAME:
      PC.PutWiFiPrefString(EE_HOSTNAME, g_sHostName);

      WiFiMonitorConnection(true); // disconnect STA mode (it will reconnect)

      prtln("New g_sHostName has been set!");
    break;

    case TASK_RECONNECT:

      WiFiMonitorConnection(true); // disconnect
      prtln("Reconnecting by TASK_RECONNECT...");

    break;

    case TASK_FIRMWARE_RESTART:
      if (g_bSoftAP){
        //PC.ErasePreferences(); // erase lock pw and prefs if AP mode...

        // only remove locked condition if we updated firmware while in AP mode
        // NOTE: we do NOT remove a lock condition if updated via a router/internet
        g8_lockCount = 0xff;
        PC.PutWiFiPrefByte(EE_LOCKCOUNT, g8_lockCount);
        PC.PutWiFiPrefString(EE_LOCKPASS, LOCKPASS_INIT);
      }
      ESP.restart();
    break;

    case TASK_RESTART:
      ESP.restart();
    break;

    case TASK_RESET_WIFI:
      if (!PC.EraseWiFiPrefs()) // erase prefs...
        prtln("Error erasing wifi preferences!");
      PC.RestoreDefaultApSsidAndPwd();
      GetWiFiPrefs();
    break;

    case TASK_RESET_PREFS:
      if (!PC.ErasePreferences()) // ersae prefs...
        prtln("Error erasing preferences!");
      GetPreferences();
      QueueTask(TASK_PRINT_PREFERENCES);
    break;

    case TASK_RESET_SLOTS:
      if (!TSC.EraseTimeSlots())
        prtln("Error erasing time-slots!");
    break;

    case TASK_WIFI_TOGGLE:
      PC.ToggleOldSsidAndPwd();
      WiFiMonitorConnection(true); // disconnect
    break;

    case TASK_WIFI_RESTORE:
      prtln("Received command to restore WiFi credentials!");
      PC.RestoreDefaultSsidAndPwd();
      WiFiMonitorConnection(true); // disconnect (reconnect)
    break;

    case TASK_WIFI_AP_RESTORE:
      prtln("Received command to restore WiFi AP credentials!");
      PC.RestoreDefaultApSsidAndPwd();
      WiFiMonitorConnection(true); // disconnect (reconnect)
    break;

    case TASK_PAGE_REFRESH_REQUEST:
      // this task is set when something on p2.html has been
      // affected and the page needs to be reloaded
      g_bTellP2WebPageToReload = true;
      g8_fiveSecondTimer = 0; // this will reset the flag as a "failsafe" in 5-sec.
    break;

    case TASK_MAC:
      PC.PutWiFiPrefString(EE_MAC, g_sMac);
      if (g_sMac == "")
        prtln("Restoring hardware MAC address...");
      else if (g_sMac == SC_MAC_RANDOM)
        prtln("Setting random MAC address mode...");
      else
        prtln("Stored new MAC address: " + g_sMac);
      WiFiMonitorConnection(true); // disconnect (reconnect)
    break;

    case TASK_WIFI_STA_CONNECT:
      TaskWiFiStaConnect();
    break;

    case TASK_WIFI_AP_CONNECT:
      TaskWiFiApConnect();
    break;
    
    case TASK_MIDICHAN:
      TaskMidiChan();
    break;

    case TASK_MIDINOTE_A:
      PC.PutPrefByte(EE_MIDINOTE_A, g8_midiNoteA);
      prt("A: ");
      PrintMidiNote(g8_midiNoteA);
    break;

    case TASK_MIDINOTE_B:
      PC.PutPrefByte(EE_MIDINOTE_B, g8_midiNoteB);
      prt("B: ");
      PrintMidiNote(g8_midiNoteB);
    break;

    case TASK_SYNC:
    {
      byte temp = 0;
      if (g_bSyncRx)
        temp |= EE_SYNC_MASK_RX;
      if (g_bSyncTx)        
        temp |= EE_SYNC_MASK_TX;
      if (g_bSyncCycle)
        temp |= EE_SYNC_MASK_CYCLE;
      if (g_bSyncToken)
        temp |= EE_SYNC_MASK_TOKEN;
      if (g_bSyncTime)        
        temp |= EE_SYNC_MASK_TIME;
      if (g_bSyncEncrypt)        
        temp |= EE_SYNC_MASK_ENCRYPT;
      PC.PutPrefByte(EE_SYNC, temp);
      prt(SyncFlagStatus());
    }      
    break;
    
    case TASK_LABEL_A:
      g_sLabelA = s1;
      PC.PutPrefString(EE_LABEL_A, g_sLabelA);
      prtln("\nLabel A: \"" + g_sLabelA + "\"");
    break;

    case TASK_SETCIPKEY:
    {
      int len = s1.length();
      if (len >= 1 && len <= CIPKEY_MAX){
        g_sKey = s1;
        PC.PutPrefString(EE_CIPKEY, g_sKey);
        CIP.setCiphKey(g_sKey);
        prtln("cipher key: \"" + String(g_sKey) + "\"");
      }
      else
        prtln("\nCipher Key invalid length <1- " + String(CIPKEY_MAX) + ": " + String(len));
    }
    break;
    
    case TASK_LABEL_B:
      g_sLabelB = s1;
      PC.PutPrefString(EE_LABEL_B, g_sLabelB);
      prtln("\nLabel B: \"" + g_sLabelB + "\"");
    break;
    
    case TASK_MAX_POWER:
      if (i1 >= MAX_POWER_MIN && i1 <= MAX_POWER_MAX){
        esp_wifi_set_max_tx_power(i1); // 0.25dBm steps [40..82]
        g8_maxPower = (uint8_t)i1;
        PC.PutPrefByte(EE_MAX_POWER, g8_maxPower);
        prtln("\nSet Max Power: " + String(i1));
      }
      else
        prtln("\nMax Power out-of-range: " + String(i1));
    break;
    
    case TASK_SETTOKEN:
      PC.PutPrefByte(EE_TOKEN, (uint8_t)g_defToken);
      prtln("\nSet Token: " + String(g_defToken));
    break;
    
    case TASK_WIFI_OFF:
      // this is accessed only in AP mode - it enables/disables WiFi for station mode!
      PC.PutPrefByte(EE_WIFI_DIS, g_bWiFiDisabled ? 1 : 0);
      if (g_bWiFiDisabled)
        prtln("WiFi Disabled in station-mode - flip AP/STA switch to turn WiFi off...");
      else
        prtln("WiFi enabled in station-mode - flip AP/STA switch to connect to router!");
    break;
    
    case TASK_HTTPCALLBACK:
      TaskHttpCallback(i1, i2, s1, s2, s3);
    break;

    case TASK_HTTPCB_DECODE_MAC:
      TaskHttpCB_DecodeMac(s1, s2); // s1 has sMac, s2 has sTxIp (from TaskHttpCallback())
    break;

    case TASK_MAIN_TIMING_CYCLE:
      TaskMainTimingCycle();
    break;
    
    case TASK_PROCESS_ONE_SECOND_TIME_SLOTS:
      TSC.ProcessSecondResolutionTimeSlots();
    break;
    
    case TASK_PROCESS_ONE_MINUTE_TIME_SLOTS:
      TSC.ProcessMinuteResolutionTimeSlots();
    break;
    
    case TASK_QUERY_MDNS_SERVICE:
      IML.QueryMdnsServiceAsync(MDNS_SVCU); // start async mDNS search for ESP32 units advertizing _dts
    break;
    
    case TASK_ENCODE_CHANGED_PARAMETERS:
      HMC.EncodeChangedParametersForAllIPs();
    break;

    case TASK_POLL_WIFI_SWITCH:
      PollWiFiSwitch();
    break;

    case TASK_CHECK_MDNS_SEARCH_RESULT:
      IML.CheckMdnsSearchResult();
    break;

    case TASK_CYCLE_THROUGH_MDNS_IPS:
    {
      int count = IML.GetCount();
      if (count){
        if (g16_asyncHttpIndex >= count)
          g16_asyncHttpIndex = 0;
        // After command "c test on", g16_oddEvenCounter lets us fold-in transmit of a random text message every other 10-30 second interval...
        if (g_bTest && (g16_oddEvenCounter++ & 1))
          SendText(g16_asyncHttpIndex, genRandMessage(1, 10));
        else if (SendHttpReq(g16_asyncHttpIndex)) // sent to different ESP32 every 10-to-30 seconds
          g16_asyncHttpIndex++;
      }
    }
    break;

    case TASK_PULSEOFF_TIMING_CYCLE:
      TaskProcessPulseOffFeatureTiming();
    break;

    case TASK_STATS_MONITOR:
      TaskStatisticsMonitor();
    break;

    case TASK_SET_PULSEOFF_VARS:
      TaskSetPulseOffFeatureVars();
    break;

    case TASK_WRITE_PULSE_EE_VALUES:
      TaskWritePulseFeaturePreferences();
    break;

    case TASK_PRINT_PREFERENCES:
      PrintPreferences();
      PrintPulseFeaturePreferences();
    break;

    default:
    break;
  };
}

void TaskSendCanRxQuery(String sIp){
  int ipIdx = IML.FindMdnsIp(sIp);
  if (ipIdx >= 0){
    SendHttpCanRxReq(ipIdx);
    prtln("DEBUG: SendHttpReq() called SendHttpCanRxReq() for: " + sIp);
  }
}

void TaskMainTimingCycle(){
  // Algorithm: We set g32_periodTimer from potentiometer and also reset
  // phase and dutyCycle timers to 0, and turn on device A if it's
  // in AUTO mode. This happens each timer-timeout of period-timer.
  // At period-timer timeout, start phase-timer and start the duty-cycle
  // timer for device A.
  // When duty-cycle timer for device A times out, turn off the device.
  // When phase-timer times out, turn on device B if mode is AUTO and
  // start duty-cycle timer B.

  // check duty-cycle before period and set any pending "off" event
  // if duty-cycle is 100 we stay on all the time
  if (g32_dutyCycleTimerA && --g32_dutyCycleTimerA == 0)
    if (g8_ssr1ModeFromWeb == SSR_MODE_AUTO && g_perVals.dutyCycleA != 100)
      SetSSR(GPOUT_SSR1, false);

  // check duty-cycle before period and set any pending "off" event
  // if duty-cycle is 100 we stay on all the time
  if (g32_dutyCycleTimerB && --g32_dutyCycleTimerB == 0)
    if (g8_ssr2ModeFromWeb == SSR_MODE_AUTO && g_perVals.dutyCycleB != 100)
      SetSSR(GPOUT_SSR2, false);

  if (g32_periodTimer && --g32_periodTimer == 0){
    // do this before setting duty-cycle and phase timers!
    // (remember that random values might be being used to create this new period!)
    g32_periodTimer = ComputePeriod(g_perVals.perVal, g_perVals.perMax, g_perVals.perUnits);

    // random mode is 100
    g32_phaseTimer = g32_nextPhase; // need to compute next cycle's potentially random phase in advance for ComputeTimeToOnOrOffB()
    g32_nextPhase = ComputePhase();

    // g32_phaseTimer can compute to 0 - turn on both at same time
    if (g32_phaseTimer == 0)
      SSR2On(g32_periodTimer);

    SSR1On(g32_periodTimer);

    g32_savePeriod = g32_periodTimer; // save for use computing duty-cycles
  }

  if (g32_phaseTimer && --g32_phaseTimer == 0)
    SSR2On(g32_savePeriod);
}

// sMac and u16Spare are not currently used!
void TaskHttpCallback(int& iSpare, int& httpCode, String& sRsp, String& sMac, String& sTxIp){

  if (sTxIp.isEmpty()){
    sTxIp = g_httpTxIP.toString();
    prtln("HttpClientCallback() No remote ip in header, using g_httpTxIP!");
  }
  else{
//prtln("DEBUG: TaskHttpCallback(): sTxIp before MyDecodeStr(): " + sTxIp);
    sTxIp = MyDecodeStr(sTxIp, HTTP_TABLE3, FAILSAFE_TOKEN_4, CIPH_CONTEXT_FOREGROUND);
    if (sTxIp.isEmpty() || sTxIp == "0.0.0.0"){
      prtln("HttpClientCallback(): error decoding sTxIp response from " + sTxIp);
      sTxIp = g_httpTxIP.toString();
    }
  }
  prtln("HttpClientCallback() Remote ip: " + sTxIp);
  
  int ipIdx = IML.FindMdnsIp(sTxIp);
  if (ipIdx < 0){
    prtln("TASK_HTTPCALLBACK: custom response header IP not in mDNS table: " + sTxIp);
    return;
  }

  prtln("TASK_HTTPCALLBACK: custom response header IP: " + sTxIp);

  if (!IML.GetLinkOkFlag(ipIdx)){
    prtln("TASK_HTTPCALLBACK: LINK NOT OK BUT GETTING (FAKED?) response from : " + sTxIp);
    return;
  }
  if (sRsp.isEmpty()){
    prtln("TASK_HTTPCALLBACK: received string is empty! response from " + sTxIp);
    return;
  }

  if (httpCode == HTTPCODE_ADDIP_FAIL){
    prtln("TASK_HTTPCALLBACK: got HTTPCODE_ADDIP_FAIL: \"" + sRsp + "\" from: " + sTxIp);
    return;
  }
  if (httpCode == HTTPCODE_RXDISABLED){
    prtln("TASK_HTTPCALLBACK: got HTTPCODE_RXDISABLED: \"" + sRsp + "\" from: " + sTxIp);
    return;
  }

  // decode MAC
  if (!sMac.isEmpty())
    QueueTask(TASK_HTTPCB_DECODE_MAC, 0, 0, sMac, sTxIp); 

//  if (httpCode == HTTPCODE_DECODE_FAIL || httpCode == HTTPCODE_DECPREV_FAIL || httpCode == HTTPCODE_PARAM_FAIL ||
//                         httpCode == HTTPCODE_TXT_FAIL || httpCode == HTTPCODE_CANRX_FAIL || httpCode == HTTPCODE_RXTOKEN_FAIL)
  if (httpCode == HTTPCODE_DECODE_FAIL || httpCode == HTTPCODE_DECPREV_FAIL || httpCode == HTTPCODE_PARAM_FAIL ||
                         httpCode == HTTPCODE_CANRX_FAIL || httpCode == HTTPCODE_RXTOKEN_FAIL)
    SubHttpCB_DecFLDecPrevFLParamFLTxtFlCanRxFLNoTokenFL(ipIdx, httpCode, sRsp, sTxIp);
  else if (httpCode == HTTPCODE_CANRX_OK)
    SubHttpCB_CanRxOk(ipIdx, httpCode, sRsp, sTxIp);
  else if (httpCode == HTTPCODE_CANRX_DECODE_FAIL || httpCode == HTTPCODE_CANRX_NOMAC_FAIL)
    SubHttpCB_CanRxDecodeFail(ipIdx, httpCode, sRsp, sTxIp);
  else
    SubHttpCB_ParamOkTxtOkTokOkTimesetFail(ipIdx, httpCode, sRsp, sTxIp);
}

// sending parameters by-reference saves copying strings - ok to set sMac because
// it just reaches back and sets s1...
void TaskHttpCB_DecodeMac(String& sMac, String& sTxIp){
  int ipIdx = IML.FindMdnsIp(sTxIp);
  if (ipIdx < 0){
    prtln("TaskHttpCB_DecodeMac: IP is not in the mDNS table!: " + sTxIp);
    return;
  }
  int iMac = MyDecodeNum(sMac, HTTP_TABLE1, FAILSAFE_TOKEN_5, CIPH_CONTEXT_FOREGROUND);
  if (iMac > 0){
    uint16_t remoteMacLT = (uint16_t)iMac;
    if (IML.GetMdnsMAClastTwo(ipIdx) != remoteMacLT){
      IML.SetMdnsMAClastTwo(ipIdx, remoteMacLT);
      CheckMasterStatus();
//      // purge possible pending command requesting MAC!
//      String sRef = IML.GetSendStr(ipIdx);
//      HMC.StripParam(CMreqMacMin, CMreqMacMax, sRef);
//      IML.SetSendStr(ipIdx, sRef);
//      prtln("TaskHttpCallback(): MAC last-two added: " + String(remoteMacLT));
    }
  }
  else
    prtln("TaskHttpCB_DecodeMac: failed to decode MAC last-two for: " + sTxIp);
}

void SubHttpCB_CanRxOk(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp){
//  prtln("DEBUG: TASK_HTTPCALLBACK: got HTTPCODE_CANRX_OK: \"" + sRsp + "\" from: " + sTxIp);
  int iTokLow = MyDecodeNum(sRsp, HTTP_TABLE3, g_defToken, CIPH_CONTEXT_FOREGROUND); // get low bits of new token, it can be NO_TOKEN
  if (iTokLow >= 0){
    iTokLow >>= CANRX_TOKEN_SHIFT; // shift high bits into place (>>4)
    if (iTokLow >= 1 && iTokLow <= 8){
      iTokLow -= 1; // subtract 1 since we send as (1-8) << 4 to avoid 0
      int iTokHigh = IML.GetSaveToken(ipIdx);
      if (iTokHigh >= 1 && iTokHigh <= 8){
        int txToken = ((iTokHigh-1)<<3) + iTokLow; // subtract 1 since we send as (1-8) << 4 to avoid 0
        IML.SetTxToken(ipIdx, txToken);
        IML.SetTxNextToken(ipIdx, NO_TOKEN);
        prtln("DEBUG: TaskHttpCallback() HTTPCODE_CANRX_OK: got low 3-bits, setting txToken to: " + String(txToken));
      }
      else
        prtln("DEBUG: TaskHttpCallback() HTTPCODE_CANRX_OK: iTokHigh 3bits out of range!" + String(iTokHigh));
    }
    else
      prtln("DEBUG: TaskHttpCallback() HTTPCODE_CANRX_OK: iTokLow 3bits out of range!" + String(iTokLow));
  }
  else
    prtln("DEBUG: TASK_HTTPCALLBACK: can't do low 3-bits decode HTTPCODE_CANRX_OK using g_defToken of: " + String(g_defToken) +
                                                                           ", error: " + String(iTokLow) + ", from: " + sTxIp);
  IML.SetSaveToken(ipIdx, NO_TOKEN); // clear our save data
}

void SubHttpCB_CanRxDecodeFail(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp){
  
  if (httpCode == HTTPCODE_CANRX_NOMAC_FAIL){
    prtln("TASK_HTTPCALLBACK: HTTPCODE_CANRX_NOMAC_FAIL can't set g_defToken yet because we don't know who's \"more master\" yet!) from: " + sTxIp);
    IML.SetSaveToken(ipIdx, NO_TOKEN); // clear our save data
    return;
  }
  
  // here, we need to decode sRsp using the key we saved via IML.SetUtilStr()
  String sKey = IML.GetUtilStr(ipIdx); // retrieve key to decrypt g_defToken (set in SendHttpCanRxReq())
//    prtln("DEBUG: TASK_HTTPCALLBACK: HTTPCODE_CANRX_DECODE_FAIL GetUtilStr(): \"" + sKey + "\"");
  if (!sKey.isEmpty()){
    CIP.saveCiphKey(CIPH_CONTEXT_FOREGROUND);
    CIP.setCiphKey(sKey);
    int newTok = MyDecodeNum(sRsp, HTTP_TABLE2, FAILSAFE_TOKEN_2, CIPH_CONTEXT_FOREGROUND);
//      prtln("DEBUG: TASK_HTTPCALLBACK: HTTPCODE_CANRX_DECODE_FAIL: decoded new defToken of " + String(newTok) + " with key: \"" + CIP.getKey() + "\"");
    CIP.restoreCiphKey(CIPH_CONTEXT_FOREGROUND);
    if (newTok == NO_TOKEN)
      prtln("TASK_HTTPCALLBACK: HTTPCODE_CANRX_DECODE_FAIL MyDecodeNum() returned NO_TOKEN (which simply means remote is not MASTER relative to us!) from: " + sTxIp);
    else if (newTok >= 0){
      g_defToken = newTok;
      prtln("TASK_HTTPCALLBACK: HTTPCODE_CANRX_DECODE_FAIL set g_defToken to newTok of " + String(g_defToken) + ": \"" + sRsp + "\" from: " + sTxIp);
    }
    else
      prtln("TASK_HTTPCALLBACK: HTTPCODE_CANRX_DECODE_FAIL MyDecodeNum() returned an errorCode for newTok! error=" + String(newTok) + " from: " + sTxIp);
  }
  else
    prtln("TASK_HTTPCALLBACK: HTTPCODE_CANRX_DECODE_FAIL IML.GetUtilStr() returned sKey empty from: " + sTxIp);
    
  IML.SetSaveToken(ipIdx, NO_TOKEN); // clear our save data
}

void SubHttpCB_ParamOkTxtOkTokOkTimesetFail(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp){
  int txToken = IML.GetTxToken(ipIdx);
  // HTTP receive handlers in WShandlers.cpp send an encoded response using table 1 and rxToken (our current txToken!)
  // Unencoded responses: HTTPRESP_RXDISABLED, HTTPRESP_ADDIP_FAIL, HTTPCODE_PARAM_FAIL (processed above...)
  String sDec = MyDecodeStr(sRsp, HTTP_TABLE1, txToken, CIPH_CONTEXT_FOREGROUND);
  if (sDec.isEmpty()){
    // setting NO_TOKEN will initiate "Can Rx?" query on next SendHttpReq() and generate a new token
    prtln("TASK_HTTPCALLBACK: Setting NO_TOKEN for txToken. Can't decode HTTP response using txTok of: " + String(txToken) +", from: " + sTxIp);
    prtln("TASK_HTTPCALLBACK: sRsp = \"" + sRsp + "\"");
    IML.SetTxToken(ipIdx, NO_TOKEN);
    IML.SetTxNextToken(ipIdx, NO_TOKEN);
  }else{
//prtln("DEBUG: TASK_HTTPCALLBACK: sDec = \"" + sDec + "\", httpCode=" + String(httpCode) + " from: " + sTxIp);
//    if (httpCode == HTTPCODE_TXT_OK){
//      if (sDec == HTTPRESP_TXT_OK)
//        prtln("TASK_HTTPCALLBACK: Got HTTPCODE_TXT_OK from: " + sTxIp);
//      else
//        prtln("TASK_HTTPCALLBACK: Got HTTPCODE_TXT_OK but bad sDec from: " + sTxIp);
//    }
// else if...
    if (httpCode == HTTPCODE_PARAM_OK || httpCode == HTTPCODE_TOK_OK || httpCode == HTTPCODE_TIMESET_FAIL){
      // NOTE: you can have HTTPCODE_TIMESET_FAIL and still have sDec == HTTPRESP_PARAM_OK or HTTPRESP_TOK_OK!
      
      if (httpCode == HTTPCODE_TIMESET_FAIL)
        prtln("TASK_HTTPCALLBACK: got HTTPCODE_TIMESET_FAIL!");        
        
      if (sDec == HTTPRESP_TOK_OK){ // don't put "else" on this!
        IML.SetSendOkFlag(ipIdx, true);
        IML.SetTokOkFlag(ipIdx, true);
        prtln("TASK_HTTPCALLBACK: Got HTTPCODE_TOK_OK response!, set bSendOk and bTokOk!");
        // check to see if all bTokOk flags are set (all remotes have their g_pendingDefToken ready to go)
        if (g_pendingDefToken != NO_TOKEN && IML.AreAllTokOkFlagsSet()){
          // g16_tokenSyncTimer time should be set big enough to allow every remote unit to receive it -
          // and we space-out sends to each remote by a random 10-30 seconds...
          g16_tokenSyncTimer = SEND_HTTP_TIME_MAX*IML.GetCount();
    
          // NOTE: we will replace the down-count in the command send-string with current value at time of transmit!
          HMC.AddTableCommandAll(CMsetToken, g16_tokenSyncTimer);
          IML.ClearAllTokOkFlags();
          prtln("TASK_HTTPCALLBACK: added CMsetToken command to all IPs!");
        }
      }
      else if (sDec == HTTPRESP_PARAM_OK){
        IML.ClearMdnsSendInfo(ipIdx); // clears sSend, bSendOk and bTokOk but doesn't affect Rx/Tx tokens
        IML.SetSendOkFlag(ipIdx, true);
        IML.XferTxTokens(ipIdx); // move txtoken to txprev and txnext to txtoken
        prtln("TASK_HTTPCALLBACK: Got HTTPCODE_PARAM_OK, set bSendOk: xfered txNextToken->txToken: " + String(IML.GetTxToken(ipIdx)));
      }
      else
        prtln("TASK_HTTPCALLBACK: unknown sDec for httpCode: " + String(httpCode));
    }
    else
      prtln("TASK_HTTPCALLBACK: remote sent unexpected code: " + String(httpCode));
  }
}

void SubHttpCB_DecFLDecPrevFLParamFLTxtFlCanRxFLNoTokenFL(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp){
  String sDec = MyDecodeStr(sRsp, HTTP_TABLE3, FAILSAFE_TOKEN_1, CIPH_CONTEXT_FOREGROUND);
  if (sDec == HTTPRESP_DECODE_FAIL){
    IML.SetTxToken(ipIdx, NO_TOKEN);
    IML.SetTxPrevToken(ipIdx, NO_TOKEN);
    prtln("TASK_HTTPCALLBACK: got HTTPRESP_DECODE_FAIL! from: " + sTxIp);
    prtln("TASK_HTTPCALLBACK: set txToken/txNextToken = NO_TOKEN");
  }
  else if (sDec == HTTPRESP_DECPREV_FAIL){
    prtln("TASK_HTTPCALLBACK: got HTTPRESP_DECPREV_FAIL! from: " + sTxIp);
    int txPrevToken = IML.GetTxPrevToken(ipIdx);
    if (txPrevToken != NO_TOKEN){
      IML.SetTxToken(ipIdx, txPrevToken);
      IML.SetTxPrevToken(ipIdx, NO_TOKEN);
      prtln("TASK_HTTPCALLBACK: set txToken = txPrevToken: " + String(txPrevToken));
      prtln("TASK_HTTPCALLBACK: set txPrevToken = NO_TOKEN");
    }else{
      IML.SetTxToken(ipIdx, NO_TOKEN);
      IML.SetTxNextToken(ipIdx, NO_TOKEN);
      prtln("TASK_HTTPCALLBACK: set txToken/txNextToken = NO_TOKEN");
    }
  }
  else if (sDec == HTTPRESP_RXTOKEN_FAIL){ // remote unit has NO_TOKEN for Rx token
    IML.SetTxToken(ipIdx, NO_TOKEN);
    IML.SetTxNextToken(ipIdx, NO_TOKEN);
    prtln("TASK_HTTPCALLBACK: got HTTPRESP_RXTOKEN_FAIL! from: " + sTxIp);
    prtln("TASK_HTTPCALLBACK: set txToken/txNextToken = NO_TOKEN");
  }
  else if (sDec == HTTPRESP_PARAM_FAIL){
    prtln("TASK_HTTPCALLBACK: (doing nothing?) got HTTPRESP_PARAM_FAIL! from: " + sTxIp);
  }
//  else if (sDec == HTTPRESP_TXT_FAIL){
//    prtln("TASK_HTTPCALLBACK: got HTTPRESP_TXT_FAIL! from: " + sTxIp);
//  }
  else if (sDec == HTTPRESP_CANRX_FAIL){
    prtln("TASK_HTTPCALLBACK: (doing nothing?) got HTTPRESP_CANRX_FAIL! from: " + sTxIp);
  }
  else{
    IML.SetTxToken(ipIdx, NO_TOKEN);
    IML.SetTxNextToken(ipIdx, NO_TOKEN);
    prtln("TASK_HTTPCALLBACK: decode failed, httpCode=" + String(httpCode) + " from: " + sTxIp);
    prtln("TASK_HTTPCALLBACK: set txToken/txNextToken = NO_TOKEN");
  }
}

void TaskWiFiApConnect(){
  prtln("Starting WiFi AP mode...");

  ProcessMAC(WIFI_IF_AP);

  // order of steps below is important!!!

//    WiFi.mode(WIFI_AP);
  WiFi.mode(WIFI_AP_STA); // need this because we may do a scan for stations...

  esp_err_t err = esp_wifi_set_max_tx_power(g8_maxPower); // 0.25dBm steps [40..82]
  if (err == ESP_OK)
    prtln("Max AP power set to: " + String(g8_maxPower));

  //SSID (defined earlier): maximum of 63 characters;
  //password(defined earlier): minimum of 8 characters; set to NULL if you want the access point to be open
  //channel: Wi-Fi channel number (1-13)
  //ssid_hidden: (0 = broadcast SSID, 1 = hide SSID)
  //max_connection: maximum simultaneous connected clients (1-4)
  //WiFi.softAP(const char* ssid, const char* password, int channel, int ssid_hidden, int max_connection)
  String sTemp = g_sApSSID.substring(0,6); // parse off "HIDDEN"
  sTemp.toLowerCase();
  int isHidden = (sTemp == "hidden") ? 1 : HIDE_AP_SSID; // hide broadcast AP SSID if it begins with HIDDEN
  WiFi.softAP(g_sApSSID.c_str(), PC.GetWiFiPrefString(EE_APPWD, DEF_AP_PWD).c_str(), random(MIN_AP_CHANNEL, MAX_AP_CHANNEL), isHidden, MAX_AP_CLIENTS); // additional parms: int channel (1-11), int ssid_hidden (0 = broadcast SSID, 1 = hide SSID), int max_connection (1-4))

  IPAddress apIP;
  apIP.fromString(DEF_AP_IP);
  IPAddress gateway;
  gateway.fromString(DEF_AP_GATEWAY);
  IPAddress subnet;
  subnet.fromString(DEF_AP_SUBNET_MASK);

  //wait for ARDUINO_EVENT_AP_START
  delay(100);

  WiFi.softAPConfig(apIP, gateway, subnet);

  g_bSoftAP = true; // set before calling GetStringIP()

  //prtln("Reconnected WiFi as access-point...");
  //prtln("Web-Server IP: 192.168." + String(IP_MIDDLE) + "." + String(DEF_AP_IP));
  prtln(GetStringIP());
  prtln("Access-point WiFI name: " + String(DEF_AP_SSID));
  prtln("Access-point password: " + String(DEF_AP_PWD));

  dnsAndServerStart();
  FlashSequencerInit(g8_ledMode_ON); // start the sequence of flashing out the last octet of IP address...
  g8_fiveSecondTimer = FIVE_SECOND_TIME-2;
}

void TaskWiFiStaConnect(){
  WiFi.mode(WIFI_STA);
  ProcessMAC(WIFI_IF_STA);
  esp_err_t err = esp_wifi_set_max_tx_power(g8_maxPower); // 0.25dBm steps [40..82]
  if (err == ESP_OK)
    prtln("Max STA power set to: " + String(g8_maxPower));
//String sPass = PC.GetWiFiPrefString(EE_PWD, DEF_PWD);
  //prtln(sPass);
  //prtln("strlen(sPass):" + String(strlen(sPass.c_str())));
  //prtln("sPass.length():" + String(sPass.length()));
  //WiFi.begin(g_sSSID.c_str(), sPass.c_str());
  WiFi.begin(g_sSSID.c_str(), PC.GetWiFiPrefString(EE_PWD, DEF_PWD).c_str());

  prtln("\nConnecting to WiFi...");
}

void SubtaskProcessParams(int& subTask, int& iData){
  uint8_t u8 = (uint8_t)iData;
  switch(subTask){
    case SUBTASK_PERMAX:
    {
      uint16_t u16 = (uint16_t)iData;
      if (u16 != g_perVals.perMax && u16 >= PERMAX_MIN && u16 <= PERMAX_MAX){
        g_perVals.perMax = u16;
        PC.PutPrefU16(EE_PERMAX, u16);
      }
    }
    break;

    case SUBTASK_PERUNITS:
      if (u8 != g_perVals.perUnits){
        g_perVals.perUnits = u8;
        PC.PutPrefByte(EE_PERUNITS, u8);
      }
    break;

    case SUBTASK_PERVAL:
      if (u8 != g_perVals.perVal && u8 >= PERIOD_MIN && u8 <= PERIOD_MAX){
        g_perVals.perVal = u8;
        PC.PutPrefByte(EE_PERVAL, u8);
      }
    break;

    case SUBTASK_DCA:
      if (u8 != g_perVals.dutyCycleA && u8 >= DUTY_CYCLE_MIN && u8 <= DUTY_CYCLE_MAX){
        g_perVals.dutyCycleA = u8;
        PC.PutPrefByte(EE_DC_A, u8);
      }
    break;

    case SUBTASK_DCB:
      if (u8 != g_perVals.dutyCycleB && u8 >= DUTY_CYCLE_MIN && u8 <= DUTY_CYCLE_MAX){
        g_perVals.dutyCycleB = u8;
        PC.PutPrefByte(EE_DC_B, u8);
      }
    break;

    case SUBTASK_PHASE:
      if (u8 != g_perVals.phase && u8 >= PHASE_MIN && u8 <= PHASE_MAX){
        g_perVals.phase = u8;
        g32_nextPhase = ComputePhase();
        PC.PutPrefByte(EE_PHASE, u8);
      }
    break;

    default:
    break;
  };
}

// call at setup()
void InitTasks(){
  for (int i=0;i<MAX_TASKS;i++)
    g_tasks[i].iTask = TASK_NONE;
}

bool QueueTask(int iTask, String s1){
  return QueueTask(iTask, 0, 0, s1);
}

// returns false if queue full
bool QueueTask(int iTask, int i1, int i2, String s1, String s2, String s3){
  // remove previous version(s) of this task!
  CancelTask(iTask);
  // add new...
  for (int i=0;i<MAX_TASKS;i++){
    if (g_tasks[i].iTask == TASK_NONE){
      g_tasks[i].iTask = iTask;
      g_tasks[i].i1 = i1;
      g_tasks[i].i2 = i2;
      g_tasks[i].s1 = s1;
      g_tasks[i].s2 = s2;
      g_tasks[i].s3 = s3;
      return true;
    }
  }
  prtln("ERROR: TASK QUEUE FULL!!!!");
  return false;
}

// return -1 if not found, index if found
int FindTask(int iTask){
  for (int i=0;i<MAX_TASKS;i++)
    if (g_tasks[i].iTask == iTask)
      return i;
  return -1;
}

// remove all copies of iTask from task queue
// return true if any cancelled
bool CancelTask(int iTask){
  bool bRet = false;
  for (int i=0;i<MAX_TASKS;i++){
    if (g_tasks[i].iTask == iTask){
      g_tasks[i].iTask = TASK_NONE;
      bRet = true;
    }
  }
  return bRet;
}

// returns NULL if no tasks found
t_task* GetFirstFullTaskSlot(){
  for (int i=0;i<MAX_TASKS;i++)
    if (g_tasks[i].iTask != TASK_NONE)
      return &g_tasks[i];
  return NULL;
}
