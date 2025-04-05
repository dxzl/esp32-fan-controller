// this file Tasks.cpp
#include "Gpc.h"

TasksClass TSK;

// call at setup()
void TasksClass::InitTasks(){
  tasks.resize(0);
}

bool TasksClass::QueueTask(int iTask, String s1){
  return QueueTask(iTask, 0, 0, s1);
}

bool TasksClass::QueueTask(int iTask, String s1, String s2){
  return QueueTask(iTask, 0, 0, s1, s2);
}

// returns false if queue full
bool TasksClass::QueueTask(int iTask, int i1, int i2, String s1, String s2, String s3){
  // remove previous version(s) of this task!
  CancelTask(iTask);
  // add new...
  int oldCount = GetCount();
  int newCount = oldCount+1;
  if (!SetSize(newCount)){
    prtln("ERROR: TASK QUEUE FULL!!!!");
    return false;
  }
  tasks[oldCount].iTask = iTask;
  tasks[oldCount].i1 = i1;
  tasks[oldCount].i2 = i2;
  tasks[oldCount].s1 = s1;
  tasks[oldCount].s2 = s2;
  tasks[oldCount].s3 = s3;
  return true;
}

// return -1 if not found, index if found
int TasksClass::FindTask(int iTask){
  int iCount = GetCount();
  for (int i=0; i<iCount; i++)
    if (tasks[i].iTask == iTask)
      return i;
  return -1;
}

// remove all copies of iTask from task queue
// return true if any cancelled
bool TasksClass::CancelTask(int iTask){
  bool bRet = false;
  for (int i=0; i < GetCount(); i++){
    if (tasks[i].iTask == iTask){
      DelIdx(i--);
      if (!bRet)
        bRet = true;
    }
  }
  return bRet;
}

void TasksClass::DelIdx(int idx){
  tasks.erase(tasks.begin() + idx);
}

// return true if success
bool TasksClass::SetSize(int newSize)
{
  tasks.resize(newSize);
  return (newSize == GetCount()) ? true : false;
}

int TasksClass::GetCount(void) { return tasks.size(); }

bool TasksClass::RunTasks(){
  if (!GetCount()) return false;
  t_task* p = &tasks[0]; // get pointer to oldest task...
  
  int iTask = p->iTask;
  int i1 = p->i1;
  int i2 = p->i2;  
  String s1 = p->s1;
  String s2 = p->s2;
  String s3 = p->s3;

  DelIdx(0); // delete oldest task...
    
  switch(iTask){
    case TASK_PARMS:
      if (i1 == SUBTASK_RELAY_A)
        PC.PutPrefU8(EE_RELAY_A, g8_ssr1ModeFromWeb);
      else if (i1 == SUBTASK_RELAY_B)
        PC.PutPrefU8(EE_RELAY_B, g8_ssr2ModeFromWeb);
#if ENABLE_SSR_C_AND_D
      else if (i1 == SUBTASK_RELAY_C)
        PC.PutPrefU8(EE_RELAY_C, g8_ssr3ModeFromWeb);
      else if (i1 == SUBTASK_RELAY_D)
        PC.PutPrefU8(EE_RELAY_D, g8_ssr4ModeFromWeb);
#endif
      else{
        SubtaskProcessParams(i1, i2);
        LimitPeriod(); // force end of cycle if it's over 10 sec!
      }
    break;

    case TASK_POTCHANGE:
      PotChangeTask(i1, i2); // in GpcUtils.cpp
    break;

    case TASK_HOSTNAME:
      PC.PutWiFiString(EE_HOSTNAME, g_sHostName);

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
        PC.PutWiFiU8(EE_LOCKCOUNT, g8_lockCount);
        PC.PutWiFiString(EE_LOCKPASS, LOCKPASS_INIT);
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
      PC.GetWiFiPrefs();
    break;

    case TASK_RESET_PREFS:
      if (!PC.ErasePreferences()) // ersae prefs...
        prtln("Error erasing preferences!");
      PC.GetPreferences();
      TSK.QueueTask(TASK_PRINT_PREFERENCES);
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
      PC.PutWiFiString(EE_MAC, g_sMac);
      
      if (g_sMac == "")
        prtln("Restoring hardware MAC address...");
      else if (g_sMac == SC_MAC_RANDOM)
        prtln("Setting random MAC address mode...");
      else
        prtln("Stored new MAC address: " + g_sMac);
        
      // if our MAC address changed, we want to do a global WiFi reconnect...
      // the reconnect will eventually call ProcessMAC()
      if (QueueSynchronizedChange(CD_CMD_RECONNECT, random(0,1<<FLAG_COUNT_CD), CD_RECONNECT_STRING))
        prtln("Requesting synchronized WiFi reconnect due to MAC change...");
      else
        prtln("Can't request WiFi reconnect.");
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
      PC.PutPrefU8(EE_MIDINOTE_A, g8_midiNoteA);
      prt("A: ");
      PrintMidiNote(g8_midiNoteA);
    break;

    case TASK_MIDINOTE_B:
      PC.PutPrefU8(EE_MIDINOTE_B, g8_midiNoteB);
      prt("B: ");
      PrintMidiNote(g8_midiNoteB);
    break;

#if ENABLE_SSR_C_AND_D
    case TASK_MIDINOTE_C:
      PC.PutPrefU8(EE_MIDINOTE_C, g8_midiNoteC);
      prt("C: ");
      PrintMidiNote(g8_midiNoteC);
    break;

    case TASK_MIDINOTE_D:
      PC.PutPrefU8(EE_MIDINOTE_D, g8_midiNoteD);
      prt("D: ");
      PrintMidiNote(g8_midiNoteD);
    break;
#endif

    case TASK_SYNC:
    {
      byte temp = 0;
      if (g_bSyncRx)
        temp |= EE_SYNC_MASK_RX;
      if (g_bSyncTx)        
        temp |= EE_SYNC_MASK_TX;
      if (g_bSyncCycle)
        temp |= EE_SYNC_MASK_CYCLE;
      if (g_bSyncTime)        
        temp |= EE_SYNC_MASK_TIME;
      if (g_bSyncEncrypt)        
        temp |= EE_SYNC_MASK_ENCRYPT;
      PC.PutPrefU8(EE_SYNC, temp);
      prt(SyncFlagStatus());
    }      
    break;
    
    case TASK_LABEL_A:
      g_sLabelA = s1;
      PC.PutPrefString(EE_LABEL_A, g_sLabelA);
      prtln("\nLabel A: \"" + g_sLabelA + "\"");
    break;

    case TASK_LABEL_B:
      g_sLabelB = s1;
      PC.PutPrefString(EE_LABEL_B, g_sLabelB);
      prtln("\nLabel B: \"" + g_sLabelB + "\"");
    break;

#if ENABLE_SSR_C_AND_D
    case TASK_LABEL_C:
      g_sLabelC = s1;
      PC.PutPrefString(EE_LABEL_C, g_sLabelC);
      prtln("\nLabel C: \"" + g_sLabelC + "\"");
    break;

    case TASK_LABEL_D:
      g_sLabelD = s1;
      PC.PutPrefString(EE_LABEL_D, g_sLabelD);
      prtln("\nLabel D: \"" + g_sLabelD + "\"");
    break;
#endif

    case TASK_SETCIPKEY:
    {
      int len = s1.length();
      if (len > 0 && len <= CIPKEY_MAX){
        g_sKey = s1;
        PC.PutPrefString(EE_CIPKEY, g_sKey);
        CIP.setCiphKey(g_sKey);
        prtln("cipher key: \"" + g_sKey + "\" " + PrintCharsWithEscapes(g_sKey));
      }
      else
        prtln("\nCipher Key invalid length <1- " + String(CIPKEY_MAX) + ": " + String(len));
    }
    break;
    
    case TASK_MAX_POWER:
      if (i1 >= MAX_POWER_MIN && i1 <= MAX_POWER_MAX){
        esp_wifi_set_max_tx_power(i1); // 0.25dBm steps [40..82]
        g8_maxPower = (uint8_t)i1;
        PC.PutPrefU8(EE_MAX_POWER, g8_maxPower);
        prtln("\nSet Max Power: " + String(i1));
      }
      else
        prtln("\nMax Power out-of-range: " + String(i1));
    break;
    
    case TASK_SETTOKEN:
      PC.PutPrefU8(EE_TOKEN, (uint8_t)g_defToken);
      g_origDefToken = g_defToken;
      prtln("\nSet Token: " + String(g_defToken));
    break;
    
    case TASK_WIFI_OFF:
      // this is accessed only in AP mode - it enables/disables WiFi for station mode!
      PC.PutPrefU8(EE_WIFI_DIS, g_bWiFiDisabled ? 1 : 0);
      if (g_bWiFiDisabled)
        prtln("WiFi Disabled in station-mode - flip AP/STA switch to turn WiFi off...");
      else
        prtln("WiFi enabled in station-mode - flip AP/STA switch to connect to router!");
    break;
    
    case TASK_SHOW_PREVIOUS_SEND_RESULTS:
      TaskResultsFromPreviousSend(i1, s1);
    break;
    
    case TASK_PROCESS_RECEIVE_STRING:
      TaskProcessReceiveString(i1, i2, s1, s2); // i1=ip, i2=rxToken, s1=sParam1, s2=sParam2
    break;

    case TASK_PROCESS_COMMANDS:
      TaskProcessMsgCommands(s1, s2); // s1=command-string, s2=original sender's IP
    break;

    case TASK_HTTPCALLBACK:
      TaskHttpCallback(i1, i2, s1, s2, s3);
    break;

    case TASK_HTTPCB_DECODE_MAC:
      TaskHttpCB_DecodeMac(s1, s2); // s1 has sMac, s2 has sTxIp (from TaskHttpCallback())
    break;

    case TASK_SET_TIMEDATE:
      SetTimeDate(s1, false); // returns the time/date that was set or empty if not able to read
    break;

    case TASK_SET_SNTP_TIMEZONE:
      g_sTimezone = s1;
      PC.PutPrefString(EE_SNTP_TZ, g_sTimezone);
      if (g16_SNTPinterval != 0)
        SetSntpTimezoneAndStart();
      else
        prtln("\nSNTP is OFF! Use \"c sntp interval " + String(SNTP_INT_INIT) + "\" to turn it on!");
      prtln("\nTimeZone: \"" + g_sTimezone + "\"");
    break;

    case TASK_SET_SNTP_INTERVAL:
      if (SetGlobalSntpInterval(i1))
        PC.PutPrefU16(EE_SNTP_INT, i1);
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
    
    case TASK_ENCODE_CHANGED_PARAMETERS:
      HMC.EncodeChangedParametersForAllIPs();
    break;

    case TASK_POLL_WIFI_SWITCH:
      PollWiFiSwitch();
    break;

    case TASK_QUERY_MDNS_SERVICE:
      IML.QueryMdnsServiceAsync(MDNS_SVCU); // start async mDNS search for ESP32 units advertizing _gpc
    break;
    
    case TASK_CHECK_MDNS_SEARCH_RESULT:
      IML.CheckMdnsSearchResult();
    break;
    
    case TASK_PRINT_MDNS_INFO:
      IML.PrintMdnsInfo();
    break;

    case TASK_SEND_HTTP_REQ:
      SendHttpReq(s1);
    break;

    case TASK_SEND_CANRX_REQ:
      SendHttpCanRxReq(s1);
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
      PC.WritePulseFeaturePreferences();
    break;

    case TASK_PRINT_PREFERENCES:
      PrintPreferences();
      PrintPulseFeaturePreferences();
    break;

    default:
    break;
  };
  return true;
}

// ------------------------ non-class-functions --------------------------------

// A slave processes a CMfrom bundle of commands from another slave...
// or a master processes a CMto bundle of commands addressed to either "all" units (0 IP) or
// addressed specifically to the master from a slave
void TaskProcessMsgCommands(String& sInOut, String& sIp){
  int ipIdx = IML.FindMdnsIp(sIp);
  if (ipIdx >= 0){
    if (g_bMaster)
      prtln("master processing CMto commands from: " + sIp);
    else
      prtln("non-master processing CMfrom commands from: " + sIp);
      
    bool bCMchangeDataWasReceived = false;
    int iErr = HMC.ProcessMsgCommands(sInOut, ipIdx, bCMchangeDataWasReceived);
    
    if (iErr < 0){ // negative if error in HMC.ProcessMsgCommands()!
      prtln("TaskProcessMsgCommands() command processing failed: " + String(iErr));
      return;
    }
    // From TaskProcessReceiveString(): NOTE: here, we are processing "bundled" commands from a CMto/CMfrom...
    // we can't send any reply here... But... that's ok because a CMchangeData command should never be found in a CMto/CMfrom
    // bundle because CMchangeData is only sent from a master to all-slaves and does not need to be bundled...
    // [good Rx! if a pending change happened, we want to reply PROCESSED_CODE_CHANGE_OK instead of PROCESSED_CODE_PARAM_OK]
    if (bCMchangeDataWasReceived){
      // NOTE: this is an error here because a CMchangeData is sent via a sSendAll command from
      // a master to all slaves, never slave-to-slave and so should never be in a command-bundle
      prtln("TaskProcessMsgCommands() ERROR: CMchangeData found in a CMto or CMfrom bundle!");
    }
    else
      prtln("TaskProcessMsgCommands() bundled-commands processing ok!");
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
  if (g32_dutyCycleTimerB && --g32_dutyCycleTimerB == 0)
    if (g8_ssr2ModeFromWeb == SSR_MODE_AUTO && g_perVals.dutyCycleB != 100)
      SetSSR(GPOUT_SSR2, false);
#if ENABLE_SSR_C_AND_D
  if (g32_dutyCycleTimerC && --g32_dutyCycleTimerC == 0)
    if (g8_ssr3ModeFromWeb == SSR_MODE_AUTO && g_perVals.dutyCycleC != 100)
      SetSSR(GPOUT_SSR3, false);
  if (g32_dutyCycleTimerD && --g32_dutyCycleTimerD == 0)
    if (g8_ssr4ModeFromWeb == SSR_MODE_AUTO && g_perVals.dutyCycleD != 100)
      SetSSR(GPOUT_SSR4, false);
#endif

  if (g32_periodTimer && --g32_periodTimer == 0){
    // do this before setting duty-cycle and phase timers!
    // (remember that random values might be being used to create this new period!)
    g32_periodTimer = ComputePeriod(g_perVals.perVal, g_perVals.perMax, g_perVals.perUnits);

    // random mode is 100
    g32_phaseTimerB = g32_nextPhaseB; // need to compute next cycle's potentially random phase in advance for ComputeTimeToOnOrOffB()
    g32_nextPhaseB = ComputePhaseB();
#if ENABLE_SSR_C_AND_D
    g32_phaseTimerC = g32_nextPhaseC;
    g32_nextPhaseC = ComputePhaseC();
    g32_phaseTimerD = g32_nextPhaseD;
    g32_nextPhaseD = ComputePhaseD();
#endif

    // g32_phaseTimerB can compute to 0 - turn on both at same time
    if (g32_phaseTimerB == 0)
      SSR2On(g32_periodTimer);
#if ENABLE_SSR_C_AND_D
    if (g32_phaseTimerC == 0)
      SSR3On(g32_periodTimer);
    if (g32_phaseTimerD == 0)
      SSR4On(g32_periodTimer);
#endif

    SSR1On(g32_periodTimer);

    g32_savePeriod = g32_periodTimer; // save for use computing duty-cycles
  }

  if (g32_phaseTimerB && --g32_phaseTimerB == 0)
    SSR2On(g32_savePeriod);
#if ENABLE_SSR_C_AND_D
  if (g32_phaseTimerC && --g32_phaseTimerC == 0)
    SSR3On(g32_savePeriod);
  if (g32_phaseTimerD && --g32_phaseTimerD == 0)
    SSR4On(g32_savePeriod);
#endif
}

// here we need to decode the sCmd and/or sProcCode strings and initiate a client-send
// of HTTP_ASYNCREQ_PROCESSED with a code PROCESSED_CODE_DECPREV_FAIL, PROCESSED_CODE_DECODE_FAIL
// PROCESSED_CODE_CHANGE_OK, PROCESSED_CODE_PARAM_OK Etc.
void TaskProcessReceiveString(int& iIp, int& rxToken, String& sParam1, String& sParam2){

  if (iIp <= 0){
    prtln("TaskProcessReceiveString() Bad Ip!");
    return;
  }
  
  int iProcCode = 0;
  int iResultCode = 0;
  int rxPrevToken = NO_TOKEN;
  bool bCMchangeDataWasReceived = false;

  String sName[MAINRX_PARAM_COUNT], sValue[MAINRX_PARAM_COUNT];
  String sParam[MAINRX_PARAM_COUNT];
  
  String sCmd, sPrevProcCode;
  bool bBadDecode = false;

  String sTxIp = UIntToIp((uint32_t)iIp);
  int ipIdx = IML.FindMdnsIp(sTxIp);
  
  if (ipIdx < 0){
    prtln("TaskProcessReceiveString() sTxIp no longer in mDNS table: " + sTxIp);
    iProcCode = PROCESSED_CODE_BADIP;
    goto finally;
  }

  if (sParam1.isEmpty() && sParam2.isEmpty()){
    prtln("TaskProcessReceiveString() PROCESSED_CODE_NOPARMS: " + sTxIp);
    iProcCode = PROCESSED_CODE_NOPARMS;
    goto finally;
  }
  
  if (!IML.GetFlag(ipIdx, MDNS_FLAG_LINK_OK)){
    prtln("TaskProcessReceiveString() MDNS_FLAG_LINK_OK is false: " + sTxIp);
    iProcCode = PROCESSED_CODE_NOLINK;
    goto finally;
  }
  
  if (rxToken == NO_TOKEN || rxToken < MIN_TOKEN || rxToken > MAX_TOKEN){
    prtln("TaskProcessReceiveString() rxToken == NO_TOKEN: " + sTxIp);
    iProcCode = PROCESSED_CODE_NORXTOKEN;
    goto finally;
  }

  prtln("TaskProcessReceiveString() rxToken: " + String(rxToken) + ", remote ip: " + sTxIp);

  sParam[0] = sParam1;
  sParam[1] = sParam2;
  
  for (int ii=0; ii < MAINRX_PARAM_COUNT; ii++){
    int len = sParam[ii].length();
    int jj = 0;
    char c;
    for (; jj < len; jj++){
      c = sParam[ii][jj];
      if (c == '='){
        jj++;
        break;
      }
      sName[ii] += c;
    }
    for (; jj < len; jj++){
      c = sParam[ii][jj];
      sValue[ii] += c;
    }

    int iErr = MyDecodeStr(sName[ii], HTTP_TABLE1, rxToken, CIPH_CONTEXT_FOREGROUND);
    if (iErr < -1){
      bBadDecode = true;
      prtln("TaskProcessReceiveString() sName=\"" + sName[ii] + "\" set bBadDecode, iErr=" + String(iErr));
      break;
    }
    int iLenName = sName[ii].length();
    
    iErr = MyDecodeStr(sValue[ii], HTTP_TABLE2, rxToken, CIPH_CONTEXT_FOREGROUND);
    if (iErr < -1){
      bBadDecode = true;
      prtln("TaskProcessReceiveString() sName=\"" + sValue[ii] + "\" set bBadDecode, iErr=" + String(iErr));
      break;
    }
    int iLenValue = sValue[ii].length();
    
    if (iLenName >= MIN_NAME_LENGTH && iLenValue >= MIN_VALUE_LENGTH &&
             iLenName <= MAX_NAME_LENGTH && iLenValue <= MAX_VALUE_LENGTH){
      if (String(HTTP_ASYNCREQ_PARAM_PROCCODE) == sName[ii])
        sPrevProcCode = sValue[ii];
      else if (String(HTTP_ASYNCREQ_PARAM_COMMAND) == sName[ii])
        sCmd = sValue[ii];
    }
  }

  if (bBadDecode){
    rxPrevToken = IML.GetToken(ipIdx, MDNS_TOKEN_RXPREV);
    if (rxPrevToken != NO_TOKEN){
      IML.SetToken(ipIdx, MDNS_TOKEN_RX, rxPrevToken);
      IML.SetToken(ipIdx, MDNS_TOKEN_RXPREV, NO_TOKEN);
      iProcCode = PROCESSED_CODE_DECPREV_FAIL;
      prtln("TaskProcessReceiveString() setting Rx Token to previous (" + String(rxPrevToken) + ") and sending response PROCESSED_RESP_DECPREV_FAIL!");
    }
    else{
      IML.SetToken(ipIdx, MDNS_TOKEN_RX, NO_TOKEN);
      iProcCode = PROCESSED_CODE_DECODE_FAIL;
      prtln("TaskProcessReceiveString() previous Rx token was NO_TOKEN, sending response PROCESSED_RESP_DECODE_FAIL!");
    }
    goto finally;
  }
  
  if (sPrevProcCode.length() <= MAX_PROC_CODE_LENGTH && alldigits(sPrevProcCode)){ // allow for a large decimal number with a sign
    iResultCode = sPrevProcCode.toInt();
    TSK.QueueTask(TASK_SHOW_PREVIOUS_SEND_RESULTS, iResultCode, 0, sTxIp);
  }
  
  if (!sCmd.isEmpty()){
    
    int iErr = HMC.DecodeCommands(sCmd, ipIdx);
    if (iErr){
      prtln("TaskProcessReceiveString() error decoding command-string: " + String(iErr) + " using rxToken=" + String(rxToken));
      iProcCode = PROCESSED_CODE_COMMAND_DECODE_FAIL;
      goto finally;
    }
    
    prtln("Good Rx command string! \"" + CommandStrToPrintable(sCmd) + "\" decoded with rxToken=" + String(rxToken));
    
    // add CMfrom of these commands to non-master units other than rxIdx
    // Example: someone changes a slave's MAC address from the command-line serial interface... we send the
    // CMmac command to the master... but now we need to echo this information to the other slaves so that
    // everybody has the new MAC.
    // Note: we want to filter out all CMto/CMfrom commands
    if (g_bMaster){
      int iCount = IML.GetCount();
      if (iCount > 1){
        String sToAll = sCmd; // make a copy
        HMC.StripCommand(CMto, sToAll);
        HMC.StripCommand(CMfrom, sToAll); // should not be any of these but just in case...
        if (!sToAll.isEmpty()){
          uint32_t uIp = (uint32_t)IML.GetIP(ipIdx); // base 10 string of IP
          for (int jj=0; jj < iCount; jj++)
            if (jj != ipIdx)
              HMC.AddCommand(CMfrom, B64C.hnEncNumOnly(uIp) + CM_SEP + sToAll, jj, MDNS_STRING_SEND_SPECIFIC); // prepend sending non-master's IP and add to sSendSpecific
        }
      }
    }
    
    // bCMchangeDataWasReceived is set by-reference...
    iErr = HMC.ProcessMsgCommands(sCmd, ipIdx, bCMchangeDataWasReceived); // returns decoded string in sCmd by-reference!    
    
    if (iErr){
      iProcCode = PROCESSED_CODE_PARAM_FAIL;
      prtln("TaskProcessReceiveString() PROCESSED_CODE_PARAM_FAIL HMC.ProcessMsgCommands() returned error: " + String(iErr));
    }
    else{ // good Rx!
      // if a pending change happened, we want to reply PROCESSED_CODE_CHANGE_OK instead of PROCESSED_CODE_PARAM_OK
      if (bCMchangeDataWasReceived){
        iProcCode = PROCESSED_CODE_CHANGE_OK;
        prtln("TaskProcessReceiveString() PROCESSED_CODE_CHANGE_OK!");
      }
      else{
        iProcCode = PROCESSED_CODE_PARAM_OK;
        prtln("TaskProcessReceiveString() PROCESSED_CODE_PARAM_OK!");
      }
  
      IML.XferRxTokens(ipIdx); // move rxToken to rxPrevToken, rxNextToken to rxToken
      
      // Each SendHttpReq() following the first will have the results of the previous send
      // in sPrevProcCode processed above. The next iProcCode is stored in MDNS_STRING_RXPROCCODE below.
      // We don't want to send unless we're NOT a master AND we've already sent a successful CanRx request
      // to the other unit and have obtained an Rx/Tx token-pair.
      if (!g_bMaster && IML.GetToken(ipIdx, MDNS_TOKEN_TX) != NO_TOKEN)
        TSK.QueueTask(TASK_SEND_HTTP_REQ, sTxIp);
    }
  }

finally:
  IML.SetStr(ipIdx, MDNS_STRING_RXPROCCODE, String(iProcCode)); // this will be used in SendHttpReq()
}

// PROCESSED_CODE_CHANGE_OK            2 // these codes should all be positive, greater than 0, up to 16
// PROCESSED_CODE_PARAM_OK             3
// PROCESSED_CODE_NOPARMS              4
// PROCESSED_CODE_PARAM_FAIL           5
// PROCESSED_CODE_DECODE_FAIL          6
// PROCESSED_CODE_COMMAND_DECODE_FAIL  7
// PROCESSED_CODE_NORXTOKEN            8
// PROCESSED_CODE_NOLINK               9
// PROCESSED_CODE_BADIP                10
// PROCESSED_CODE_DECPREV_FAIL         11
// NOTE: we call SendHttpReq() as a task and include sPrevProcCode (which can be empty). sPrevProcCode is the
// result code of TASK_PROCESS_RECEIVE_STRING that's set in HandleHttpAsyncReq() in WSHandlers.cpp
// Here's the idea:
// A master does a HttpSendReq() to a particular non-master unit.
// The non-master unit responds by queuing a return SendHttpReq() to the master that includes the received
// processing-result from the master's HttpSendReq() (processed in task TASK_PROCESS_RECEIVE_STRING)...
// so http communication is spread out to prevent network clogs. Note: The non-master will not see the
// results from its send to the master until the next send from the master - which could be a big interval!
void TaskResultsFromPreviousSend(int& iResultCode, String& sIp){
  
  int ipIdx = IML.FindMdnsIp(sIp);
  if (ipIdx < 0){
    prtln("Results: remote ip not in mDNS table: " + sIp);
    return;
  }
  
  prtln("Results: remote ip: " + sIp);
  
  iResultCode >>= 4; // throw away random bits
  
  if (iResultCode <= 0){
    prtln("Results: empty, iResultCode: " + String(iResultCode));
    return;
  }
  
  if (iResultCode == PROCESSED_CODE_BADIP)
    prtln("Results: PROCESSED_CODE_BADIP!");
  else if (iResultCode == PROCESSED_CODE_NOLINK)
    prtln("Results: PROCESSED_CODE_NOLINK!");
  else if (iResultCode == PROCESSED_CODE_NOPARMS)
    prtln("Results: PROCESSED_CODE_NOPARMS!");
  else if (iResultCode == PROCESSED_CODE_NORXTOKEN)
    prtln("Results: PROCESSED_CODE_NORXTOKEN!");
  else if (iResultCode == PROCESSED_CODE_COMMAND_DECODE_FAIL)
    prtln("Results: PROCESSED_CODE_COMMAND_DECODE_FAIL!");
  else if (iResultCode == PROCESSED_CODE_PARAM_OK || iResultCode == PROCESSED_CODE_CHANGE_OK){
    // PROCESSED_CODE_CHANGE_OK and PROCESSED_CODE_PARAM_OK are the same - remote unit successfully received our last 
    // transmit - except PROCESSED_CODE_CHANGE_OK means that the previous transmit contained a CMchangeData command
    IML.XferTxTokens(ipIdx); // move txtoken to txprev and txnext to txtoken
    IML.ClearMdnsSendInfo(ipIdx); // clears sSend and MDNS_FLAG_CHANGE_OK but doesn't affect Rx/Tx tokens

    prtln("Results: xfered txNextToken->txToken: " + String(IML.GetToken(ipIdx, MDNS_TOKEN_TX)));
    
    if (iResultCode == PROCESSED_CODE_CHANGE_OK){
      if(!g_bMaster)
        prtln("Results: non-master should not get PROCESSED_CODE_CHANGE_OK!");
      else{
        IML.SetFlag(ipIdx, MDNS_FLAG_CHANGE_OK, true);
        g16_sendDefTokenTimer = 0; // don't want this unit to start a new token-sync while one is in-progress!
  
        prtln("Results: PROCESSED_CODE_CHANGE_OK, set MDNS_FLAG_CHANGE_OK");
        
        // check to see if all MDNS_FLAG_CHANGE_OK flags are all set (all remotes have their g_sChangeData ready to go)
        if (CNG.GetCount() && IML.IsFlagSetForAllIP(MDNS_FLAG_CHANGE_OK)){
          // g16_changeSyncTimer time should be set big enough to allow every remote unit to receive it -
          // and we space-out sends to each remote by a random 10-30 seconds...
          g16_changeSyncTimer = SEND_HTTP_TIME_MAX*IML.GetCount();
    
          // NOTE: we will replace the down-count in the command send-string with current value at time of transmit!
          HMC.AddCommandAll(CMchangeSet, g16_changeSyncTimer);
          IML.SetFlagForAllIP(MDNS_FLAG_CHANGE_OK, false);
          prtln("Results: PROCESSED_CODE_CHANGE_OK added CMchangeSet command to"
                                    " all IPs! g16_changeSyncTimer=" + String(g16_changeSyncTimer));
        }
      }
    }
    else
      prtln("Results: PROCESSED_CODE_PARAM_OK");
  }
  else if (iResultCode == PROCESSED_CODE_DECODE_FAIL){
    IML.SetToken(ipIdx, MDNS_TOKEN_TX, NO_TOKEN);
    IML.SetToken(ipIdx, MDNS_TOKEN_TXPREV, NO_TOKEN);
    prtln("Results: PROCESSED_CODE_DECODE_FAIL, set txToken/txNextToken = NO_TOKEN");
  }
  else if (iResultCode == PROCESSED_CODE_DECPREV_FAIL){
    prtln("Results: PROCESSED_CODE_DECPREV_FAIL");
    int txPrevToken = IML.GetToken(ipIdx, MDNS_TOKEN_TXPREV);
    if (txPrevToken != NO_TOKEN){
      IML.SetToken(ipIdx, MDNS_TOKEN_TX, txPrevToken);
      IML.SetToken(ipIdx, MDNS_TOKEN_TXPREV, NO_TOKEN);
      prtln("Results: restore txToken to " + String(txPrevToken));
    }
    else{
      IML.SetToken(ipIdx, MDNS_TOKEN_TX, NO_TOKEN);
      IML.SetToken(ipIdx, MDNS_TOKEN_TXNEXT, NO_TOKEN);
      prtln("Results: set txToken/txNextToken = NO_TOKEN");
    }
  }
  else if (iResultCode == PROCESSED_CODE_PARAM_FAIL)
    prtln("Results: PROCESSED_CODE_PARAM_FAIL (doing nothing?)");
  else
    prtln("Results: remote sent unexpected code: " + String(iResultCode));
}

// sMac and u16Spare are not currently used!
void TaskHttpCallback(int& iSpare, int& httpCode, String& sRsp, String& sMac, String& sTxIp){

  if (sTxIp.isEmpty()){
    prtln("TaskHttpCallback() No remote ip in header!");
    return;
  }

  prtln("TaskHttpCallback() Remote ip: " + sTxIp);
  
  int ipIdx = IML.FindMdnsIp(sTxIp);
  if (ipIdx < 0){
    prtln("TaskHttpCallback() custom response header IP not in mDNS table: " + sTxIp);
    return;
  }

  prtln("TaskHttpCallback() custom response header IP: " + sTxIp);

  if (!IML.GetFlag(ipIdx, MDNS_FLAG_LINK_OK)){
    prtln("TaskHttpCallback() LINK NOT OK BUT GETTING (FAKED?) response from : " + sTxIp);
    return;
  }
  
  if (sRsp.isEmpty()){
    prtln("TaskHttpCallback() received sRsp string is empty! response from " + sTxIp);
    return;
  }
  
  if (httpCode == HTTPCODE_PROCESSING_OK || httpCode == HTTPCODE_NOIP_FAIL || httpCode == HTTPCODE_ADDIP_FAIL ||
      httpCode == HTTPCODE_RXDISABLED || httpCode == HTTPCODE_PARAM_TOOLONG || httpCode == HTTPCODE_PARAM_TOOSHORT){
    prtln("TaskHttpCallback() doing nothing... got HTTPCODE_" + GetHttpCodeString(httpCode) + " from: " + sTxIp);
    return;
  }
  
  // decode MAC
  if (!sMac.isEmpty())
    TSK.QueueTask(TASK_HTTPCB_DECODE_MAC, 0, 0, sMac, sTxIp); 

  if (httpCode == HTTPCODE_CANRX_FAIL || httpCode == HTTPCODE_RXTOKEN_FAIL)
    SubHttpCB_CanRxFLNoTokenFL(ipIdx, httpCode, sRsp, sTxIp);
  else if (httpCode == HTTPCODE_CANRX_OK)
    SubHttpCB_CanRxOk(ipIdx, httpCode, sRsp, sTxIp);
  else if (httpCode == HTTPCODE_CANRX_DECODE_FAIL)
    SubHttpCB_CanRxDecodeFail(ipIdx, httpCode, sRsp, sTxIp);
  else
    prtln("TaskHttpCallback() unprocessed httpCode: " + String(httpCode) + " from: " + sTxIp);
}

// sending parameters by-reference saves copying strings - ok to set sMac because
// it just reaches back and sets s1...
void TaskHttpCB_DecodeMac(String& sMac, String& sTxIp){
  int ipIdx = IML.FindMdnsIp(sTxIp);
  if (ipIdx < 0){
    prtln("TaskHttpCB_DecodeMac() IP is not in the mDNS table!: " + sTxIp);
    return;
  }
  int iMac = 0;
  int iErr = MyDecodeNum(iMac, sMac, HTTP_TABLE1, FAILSAFE_TOKEN_5, CIPH_CONTEXT_FOREGROUND);
  if (!iErr && iMac > 0){
    uint16_t remoteMacLT = (uint16_t)iMac;
    if (IML.GetMdnsMAClastTwo(ipIdx) != remoteMacLT){
      IML.SetMdnsMAClastTwo(ipIdx, remoteMacLT);
      prtln("TaskHttpCB_DecodeMac(): MAC last-two added: " + String(remoteMacLT));
      RefreshGlobalMasterFlagAndIp();
//      // purge possible pending command requesting MAC!
//      String sRef = IML.GetStr(ipIdx, MDNS_STRING_SEND_ALL);
//      HMC.StripRangeCommand(CMreqMacMin, CMreqMacMax, sRef);
//      IML.SetStr(ipIdx, MDNS_STRING_SEND_ALL, sRef);
//      prtln("TaskHttpCallback(): MAC last-two added: " + String(remoteMacLT));
    }
  }
  else
    prtln("TaskHttpCB_DecodeMac() failed to decode MAC last-two for: " + sTxIp);
}

void SubHttpCB_CanRxOk(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp){
//  prtln("DEBUG: SubHttpCB_CanRxOk() got HTTPCODE_CANRX_OK: \"" + sRsp + "\" from: " + sTxIp);
  int iTokDecode = IML.GetToken(ipIdx, MDNS_TOKEN_CANRX_TX);
  if (iTokDecode == NO_TOKEN){
    prtln("SubHttpCB_CanRxOk() SubHttpCB_CanRxOk(): Can't decode because MDNS_TOKEN_CANRX_TX is NO_TOKEN!");
    return;    
  }
  // NOTE: decode here SHOULD NOT FAIL because we are decoding with the same token we used to send... so just
  // print a message if it fails. Anything else is a MESS!
  int iTokLow = 0;
  int iErr = MyDecodeNum(iTokLow, sRsp, HTTP_TABLE3, iTokDecode, CIPH_CONTEXT_FOREGROUND); // get low bits of new tokens
  if (!iErr && iTokLow >= 1){
    // shift until marker-bit shifted out...
    while(!(iTokLow & 1))
      iTokLow >>= 1;
    iTokLow >>= 1;
    int iTokCanRxLow = iTokLow & 7;
    int iTokCmdStrLow = iTokLow >> 3;
    if (iTokCanRxLow >= 0 && iTokCanRxLow <= 7 && iTokCmdStrLow >= 0 && iTokCmdStrLow <= 7){
      int iTokHigh = IML.GetToken(ipIdx, MDNS_TOKEN_SAVE);
      if (iTokHigh >= 1){
        // shift until marker-bit shifted out...
        while(!(iTokHigh & 1))
          iTokHigh >>= 1;
        iTokHigh >>= 1;
        int iTokCanRxHigh = iTokHigh & 7;
        int iTokCmdStrHigh = iTokHigh >> 3;
        if (iTokCanRxHigh >= 0 && iTokCanRxHigh <= 7 && iTokCmdStrHigh >= 0 && iTokCmdStrHigh <= 7){
          int txToken = (iTokCanRxHigh<<3) + iTokCanRxLow;
          IML.SetToken(ipIdx, MDNS_TOKEN_CANRX_TX, txToken);
//          prtln("DEBUG: SubHttpCB_CanRxOk() HTTPCODE_CANRX_OK: got low 3-bits, setting MDNS_TOKEN_CANRX_TX: " + String(txToken));
          txToken = (iTokCmdStrHigh<<3) + iTokCmdStrLow;
          IML.SetToken(ipIdx, MDNS_TOKEN_TX, txToken);
//          prtln("DEBUG: SubHttpCB_CanRxOk() HTTPCODE_CANRX_OK: got low 3-bits, setting MDNS_TOKEN_TX: " + String(txToken));
          IML.SetToken(ipIdx, MDNS_TOKEN_TXNEXT, NO_TOKEN);
          IML.SetFlag(ipIdx, MDNS_FLAG_CANRX_CALLBACK_DECODE_FAIL, false);

          // if SubHttpCB_CanRxDecodeFail() had "repaired" the CanRx tx token and CanRx has subsequently succeded,
          // we have at least two networked units with different values for g_bDefToken. So start process for
          // replacing g_bDefToken for all units (synchronously).
          if (IML.IsFlagSetForAnyIP(MDNS_FLAG_REQUEST_DEFAULT_TOKEN_CHANGE)){
            if (g_bWiFiConnected && IML.GetCount()){
              // call StartNewRandomDefaultToken() to start a system-wide default token reset in X minutes (if we are master)
              if (MINUTES_AFTER_DEF_TOKEN_FAIL_TO_INITIATE_RESET){
                int ipCount = IML.GetCount();
                if (ipCount)
                  g16_sendDefTokenTimer = g16_sendDefTokenTime-(ipCount+ MINUTES_AFTER_DEF_TOKEN_FAIL_TO_INITIATE_RESET);
              }
            }
            IML.SetFlagForAllIP(MDNS_FLAG_REQUEST_DEFAULT_TOKEN_CHANGE, false); // clear flag(s)
          }
        }
        else
          prtln("SubHttpCB_CanRxOk() HTTPCODE_CANRX_OK: iTokHigh 3-bits out of range! iTokHigh: " + String(iTokHigh));
      }
      else
        prtln("SubHttpCB_CanRxOk() HTTPCODE_CANRX_OK: iTokHigh < 1 error! iTokHigh: " + String(iTokHigh));
    }
    else
      prtln("SubHttpCB_CanRxOk() HTTPCODE_CANRX_OK: iTokLow 3-bits out of range! iTokLow: " + String(iTokLow));
  }
  else
    prtln("SubHttpCB_CanRxOk() can't do low 3-bits decode HTTPCODE_CANRX_OK using g_defToken of: " + String(g_defToken) +
                                                                 ", error: " + String(iTokLow) + ", from: " + sTxIp);
}

void SubHttpCB_CanRxDecodeFail(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp){
  
  // here, we need to decode sRsp using the key we saved via IML.SetStr(MDNS_STRING_KEY)
  String sKey = IML.GetStr(ipIdx, MDNS_STRING_KEY); // retrieve key to decrypt the new CanRx tx token (set in SendHttpCanRxReq())
//    prtln("DEBUG: TASK_HTTPCALLBACK: HTTPCODE_CANRX_DECODE_FAIL GetStr(MDNS_STRING_KEY): \"" + sKey + "\"");
  if (!sKey.isEmpty()){

    int iToken;
    if (IML.GetFlag(ipIdx, MDNS_FLAG_CANRX_CALLBACK_DECODE_FAIL)){
      iToken = g_origDefToken;
    }
    else{
      IML.SetFlag(ipIdx, MDNS_FLAG_CANRX_CALLBACK_DECODE_FAIL, true);
      iToken = g_defToken;
    }    
    
    CIP.saveCiphKey(CIPH_CONTEXT_FOREGROUND);
    CIP.setCiphKey(sKey);
    // decode with flash-stored value (since g_defToken will change over time...)
    int newTxTok = 0;
    int iErr = MyDecodeNum(newTxTok, sRsp, HTTP_TABLE2, iToken, CIPH_CONTEXT_FOREGROUND);
    CIP.restoreCiphKey(CIPH_CONTEXT_FOREGROUND);
    if (!iErr && newTxTok >= 0){
      IML.SetToken(ipIdx, MDNS_TOKEN_CANRX_TX, newTxTok);

      if (iToken == g_origDefToken && g_bMaster)
        IML.SetFlag(ipIdx, MDNS_FLAG_REQUEST_DEFAULT_TOKEN_CHANGE, true);
      
      prtln("TASK_HTTPCALLBACK: SubHttpCB_CanRxDecodeFail() Decode OK! IML.SetCanrxTxToken() to newTxTok of " + String(newTxTok) + ": \"" + sRsp + "\" from: " + sTxIp);
    }
    else{
      IML.SetToken(ipIdx, MDNS_TOKEN_CANRX_TX, NO_TOKEN);
      prtln("TASK_HTTPCALLBACK: SubHttpCB_CanRxDecodeFail() CanRx Decode FAIL! USE \"c token NN\" to set default tokens the same! from: " + sTxIp);
    }
  }
  else
    prtln("TASK_HTTPCALLBACK: HTTPCODE_CANRX_DECODE_FAIL IML.GetStr(MDNS_STRING_KEY) returned sKey empty from: " + sTxIp);
}

void SubHttpCB_CanRxFLNoTokenFL(int& ipIdx, int& httpCode, String& sRsp, String& sTxIp){
  prtln("SubHttpCB_CanRxFLNoTokenFL() ip: " + sTxIp);
  int iErr = MyDecodeStr(sRsp, HTTP_TABLE3, FAILSAFE_TOKEN_1, CIPH_CONTEXT_FOREGROUND);
  if (iErr < -1){
    IML.SetToken(ipIdx, MDNS_TOKEN_TX, NO_TOKEN);
    IML.SetToken(ipIdx, MDNS_TOKEN_TXNEXT, NO_TOKEN);
    prtln("SubHttpCB_CanRxCanRxFLNoTokenFL() decode failed, httpCode=" + String(httpCode));
    prtln("TASK_HTTPCALLBACK: set txToken/txNextToken = NO_TOKEN");
  }
  else if (sRsp == HTTPRESP_RXTOKEN_FAIL){ // remote unit has NO_TOKEN for Rx token
    IML.SetToken(ipIdx, MDNS_TOKEN_TX, NO_TOKEN);
    IML.SetToken(ipIdx, MDNS_TOKEN_TXNEXT, NO_TOKEN);
    prtln("SubHttpCB_CanRxCanRxFLNoTokenFL() got HTTPRESP_RXTOKEN_FAIL!");
    prtln("SubHttpCB_CanRxCanRxFLNoTokenFL() set txToken/txNextToken = NO_TOKEN");
  }
  else if (sRsp == HTTPRESP_CANRX_FAIL){
    prtln("SubHttpCB_CanRxCanRxFLNoTokenFL() (doing nothing?) got HTTPRESP_CANRX_FAIL!");
  }
  else
    prtln("SubHttpCB_CanRxCanRxFLNoTokenFL() unknown sRsp: \"" + sRsp + "\"");
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
  WiFi.softAP(g_sApSSID.c_str(), PC.GetWiFiString(EE_APPWD, DEF_AP_PWD).c_str(), random(MIN_AP_CHANNEL, MAX_AP_CHANNEL), isHidden, MAX_AP_CLIENTS); // additional parms: int channel (1-11), int ssid_hidden (0 = broadcast SSID, 1 = hide SSID), int max_connection (1-4))

  IPAddress apIP;
  apIP.fromString(DEF_AP_IP);
  IPAddress gateway;
  gateway.fromString(DEF_AP_GATEWAY);
  IPAddress subnet;
  subnet.fromString(DEF_AP_SUBNET_MASK);

  //wait for ARDUINO_EVENT_AP_START
  delay(100);

  WiFi.softAPConfig(apIP, gateway, subnet);

  g_bSoftAP = true; // set before calling GetStringInfo()

  //prtln("Reconnected WiFi as access-point...");
  //prtln("Web-Server IP: 192.168." + String(IP_MIDDLE) + "." + String(DEF_AP_IP));
  prtln(GetStringInfo());
  prtln("Access-point WiFI name: " + String(DEF_AP_SSID));
  prtln("Access-point password: " + String(DEF_AP_PWD));

  dnsAndServerStart();
  FlashSequencerInit(g8_wifiLedMode_ON); // start the sequence of flashing out the last octet of IP address...
  g8_fiveSecondTimer = FIVE_SECOND_TIME-2;
}

void TaskWiFiStaConnect(){
  WiFi.mode(WIFI_STA);
  ProcessMAC(WIFI_IF_STA);
  esp_err_t err = esp_wifi_set_max_tx_power(g8_maxPower); // 0.25dBm steps [40..82]
  if (err == ESP_OK)
    prtln("Max STA power set to: " + String(g8_maxPower));
//String sPass = PC.GetWiFiString(EE_PWD, DEF_PWD);
  //prtln(sPass);
  //prtln("strlen(sPass):" + String(strlen(sPass.c_str())));
  //prtln("sPass.length():" + String(sPass.length()));
  //WiFi.begin(g_sSSID.c_str(), sPass.c_str());
  WiFi.begin(g_sSSID.c_str(), PC.GetWiFiString(EE_PWD, DEF_PWD).c_str());

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
        PC.PutPrefU8(EE_PERUNITS, u8);
      }
    break;

    case SUBTASK_PERVAL:
      if (u8 != g_perVals.perVal && u8 >= PERIOD_MIN && u8 <= PERIOD_MAX){
        g_perVals.perVal = u8;
        PC.PutPrefU8(EE_PERVAL, u8);
      }
    break;

    case SUBTASK_DCA:
      if (u8 != g_perVals.dutyCycleA && u8 >= DUTY_CYCLE_MIN && u8 <= DUTY_CYCLE_MAX){
        g_perVals.dutyCycleA = u8;
        PC.PutPrefU8(EE_DC_A, u8);
      }
    break;

    case SUBTASK_DCB:
      if (u8 != g_perVals.dutyCycleB && u8 >= DUTY_CYCLE_MIN && u8 <= DUTY_CYCLE_MAX){
        g_perVals.dutyCycleB = u8;
        PC.PutPrefU8(EE_DC_B, u8);
      }
    break;

    case SUBTASK_PHASEB:
      if (u8 != g_perVals.phaseB && u8 >= PHASE_MIN && u8 <= PHASE_MAX){
        g_perVals.phaseB = u8;
        g32_nextPhaseB = ComputePhaseB();
        PC.PutPrefU8(EE_PHASE_B, u8);
      }
    break;

#if ENABLE_SSR_C_AND_D
    case SUBTASK_DCC:
      if (u8 != g_perVals.dutyCycleC && u8 >= DUTY_CYCLE_MIN && u8 <= DUTY_CYCLE_MAX){
        g_perVals.dutyCycleC = u8;
        PC.PutPrefU8(EE_DC_C, u8);
      }
    break;

    case SUBTASK_DCD:
      if (u8 != g_perVals.dutyCycleD && u8 >= DUTY_CYCLE_MIN && u8 <= DUTY_CYCLE_MAX){
        g_perVals.dutyCycleD = u8;
        PC.PutPrefU8(EE_DC_D, u8);
      }
    break;

    case SUBTASK_PHASEC:
      if (u8 != g_perVals.phaseC && u8 >= PHASE_MIN && u8 <= PHASE_MAX){
        g_perVals.phaseC = u8;
        g32_nextPhaseC = ComputePhaseC();
        PC.PutPrefU8(EE_PHASE_C, u8);
      }
    break;

    case SUBTASK_PHASED:
      if (u8 != g_perVals.phaseD && u8 >= PHASE_MIN && u8 <= PHASE_MAX){
        g_perVals.phaseD = u8;
        g32_nextPhaseD = ComputePhaseD();
        PC.PutPrefU8(EE_PHASE_D, u8);
      }
    break;
#endif

    default:
    break;
  };
}
