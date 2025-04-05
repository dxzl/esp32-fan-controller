// this file GpcUtils.cpp
#include "Gpc.h"

void RefreshGlobalMasterFlagAndIp(){
  int iHighestMacIndex = -1; // get by-reference...
  uint16_t uMacHighest = GetHighestMac(iHighestMacIndex);
  if (iHighestMacIndex < 0){
    g_bMaster = false;
    g_IpMaster = (int)0;
    return;
  }

  bool bIsMaster = WeAreMaster();
  
  if ((int)g_IpMaster == 0){
    prtln("RefreshGlobalMasterFlagAndIp(): Initializing...");
    g_bMaster = !bIsMaster;
  }
  
  if (!bIsMaster && g_bMaster){
    prtln("RefreshGlobalMasterFlagAndIp(): This IP is NOT a master...");
    g_bMaster = false;
    g_IpMaster = IML.GetIP(iHighestMacIndex);
    g16_sendDefTokenTimer = 0;
  }
  else if (bIsMaster && !g_bMaster){
    prtln("RefreshGlobalMasterFlagAndIp(): This IP is a master...");
    g_bMaster = true;
    g_IpMaster = GetLocalIp(); // this will be 0.0.0.0 unless connected!
  }
  prtln("RefreshGlobalMasterFlagAndIp(): g_IpMaster is: " + g_IpMaster.toString());
}

//void InitGlobalIpAndMasterFlag(){
//  g_bIsMaster = WeAreMaster();
//  if (g_bIsMaster)
//    g_IpMaster = GetLocalIp();
//  else{
//    int iHighestMacIndex = -1; // get by-reference...
//    GetHighestMac(iHighestMacIndex);
//    g_IpMaster = (iHighestMacIndex >= 0) ? IML.GetIP(iHighestMacIndex) : (int)0;
//  }
//}

bool WeAreMaster(){
  // if we're the only unit, no need for "master"
  if (IML.GetCount() == 0)
    return false;
  // if any IPs in list with no MAC, we can't determine the master yet...
  if (IsAnyMacZero())
    return false;
  int iIdx = -1;
  uint16_t uMacHighest = GetHighestMac(iIdx);
  return GetOurDeviceMacLastTwoOctets() > uMacHighest;
}

//  uint8_t local_buf[6] = {0}; // order 5:4:3:2:1:0
//  uint8_t uni_buf[6] = {0}; // order 5:4:3:2:1:0
//  esp_derive_local_mac(local_buf, uni_buf);
// this just does same as WiFi.macAddress(buf)
//  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, buf);
//  if (ret != ESP_OK)
//    return 0;
// this returns 0 if offline!
//  WiFi.macAddress(buf);
//  Serial.println(local_buf[5],16);
//  Serial.println(local_buf[4],16);
//  Serial.println(local_buf[3],16);
//  Serial.println(local_buf[2],16);
//  Serial.println(local_buf[1],16);
//  Serial.println(local_buf[0],16);
uint16_t GetOurDeviceMacLastTwoOctets(){
  uint8_t base_buf[6] = {0}; // order 5:4:3:2:1:0
  esp_efuse_mac_get_default(base_buf);
  esp_read_mac(base_buf, ESP_MAC_WIFI_STA);
  
//  Serial.println(base_buf[5],16);
//  Serial.println(base_buf[4],16);
//  Serial.println(base_buf[3],16);
//  Serial.println(base_buf[2],16);
//  Serial.println(base_buf[1],16);
//  Serial.println(base_buf[0],16);

  return (base_buf[1] << 8) | base_buf[0];
}

// return highest "last two octets" of MACs stored in arr
// return index by-reference
uint16_t GetHighestMac(int& iIdx){
  uint16_t highestMac = 0;
  int count = IML.GetCount();
  iIdx = -1;
  for (int ii=0; ii<count; ii++){
    uint16_t thisMac = IML.GetMdnsMAClastTwo(ii);
    if (thisMac > highestMac){
      highestMac = thisMac;
      iIdx = ii;
    }
  }
  return highestMac;  
}

// return true if any stored MAC is 0 (remote has not sent it to us)
bool IsAnyMacZero(){
  int count = IML.GetCount();
  for (int ii=0; ii<count; ii++)
    if (IML.GetMdnsMAClastTwo(ii) == 0)
      return true;
  return false;  
}

void ReadPot1(bool bForceRead){
  if (!bForceRead && (g8_potChannel == POT_CHAN_NONE || g8_potModeFromSwitch == POT_MODE_NONE))
    return;
    
  uint16_t potValue = analogRead(GPAIN_POT1);
  int deltaPotValue = potValue - g16_oldPotValue;
  if (deltaPotValue < 0)
    deltaPotValue = -deltaPotValue;

  if (deltaPotValue > POT_DEBOUNCE || bForceRead){
    //double voltage = (3.3/4095.0) * g16_pot1Value;
    //prt("g16_pot1Value:");
    //prt(g16_pot1Value);
    //prt(" Voltage:");
    //prt(voltage);
    //prtln("V");
#if GPC_BOARD_2B || GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
    g_potPercent = (int)((float)potValue*100.0/4095.0);
  #if POT_REVERSED
    g_potPercent = 100-g_potPercent; // POT direction reversed!
  #endif

    g8_potLedFlashTime = (110-g_potPercent)/10;

    // if pot is "near" the min/max, set it to the min/max...
    if (g_potPercent <= 3)
      g_potPercent = 0;
    else if (g_potPercent >= 97)
      g_potPercent = 100;
    
#endif
    g16_oldPotValue = potValue;
  }
}

void PotChangeTask(int iModeAndChan, int iPercent){
  int iMode = (iModeAndChan >> 8);
  int iChan = (iModeAndChan & 0xff);
  prtln("DEBUG: PotChangeTask(): iMode=" + String(iMode) + ", iChan=" + String(iChan) + ", iPercent=" + String(iPercent));
  if (iMode == POT_MODE_LEFT){
    if (iChan == POT_CHAN_1)
      TSK.QueueTask(TASK_PARMS, SUBTASK_PERVAL, iPercent);
    else if (iChan == POT_CHAN_2)
      TSK.QueueTask(TASK_PARMS, SUBTASK_PHASEB, iPercent);
      
    #if ENABLE_SSR_C_AND_D
    else if (iChan == POT_CHAN_3)
      TSK.QueueTask(TASK_PARMS, SUBTASK_PHASEC, iPercent);
    else if (iChan == POT_CHAN_4)
      TSK.QueueTask(TASK_PARMS, SUBTASK_PHASED, iPercent);
    #endif
    
  }
  else if (iMode == POT_MODE_RIGHT){
    if (iChan == POT_CHAN_1)
      TSK.QueueTask(TASK_PARMS, SUBTASK_DCA, iPercent);
    else if (iChan == POT_CHAN_2)
      TSK.QueueTask(TASK_PARMS, SUBTASK_DCB, iPercent);
      
    #if ENABLE_SSR_C_AND_D
    else if (iChan == POT_CHAN_3)
      TSK.QueueTask(TASK_PARMS, SUBTASK_DCC, iPercent);
    else if (iChan == POT_CHAN_4)
      TSK.QueueTask(TASK_PARMS, SUBTASK_DCD, iPercent);
    #endif
    
  }
  else if (iMode == POT_MODE_CENTER){
    if (iChan == POT_CHAN_1){
      // NOTE: perMax is NOT a percentage but the POT value is... so we need to map the pot to
      // perMax...
      int iActual;
      if (iPercent == 0)
        iActual = PERMAX_MIN;     
      else
        iActual = iPercent * PERMAX_MAX / 100;
      TSK.QueueTask(TASK_PARMS, SUBTASK_PERMAX, iActual);
    }
    else if (iChan == POT_CHAN_2){
      // NOTE: perUnits is NOT a percentage but the POT value is... so we need to map the pot to
      // perUnits...
      int iActual;
      if (iPercent == 0)
        iActual = PERUNITS_MIN;     
      else
        iActual = iPercent * PERUNITS_MAX / 100;
      TSK.QueueTask(TASK_PARMS, SUBTASK_PERUNITS, iActual);
    }
    
    #if ENABLE_SSR_C_AND_D
//    else if (iChan == POT_CHAN_3)
//      TSK.QueueTask(TASK_PARMS, SUBTASK_???, iPercent); // unused presently...
//    else if (iChan == POT_CHAN_4)
//      TSK.QueueTask(TASK_PARMS, SUBTASK_???, iPercent); // unused presently...
    #endif
  
  }
}

void ReadPotModeSwitch(){
    
#if GPC_BOARD_2B || GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
  bool sw1On = digitalRead(GPIN_POT_MODE_SW1);
  bool sw2On = digitalRead(GPIN_POT_MODE_SW2);
  if (sw1On != g_bOldPotModeSw1On || sw2On != g_bOldPotModeSw2On){
    ReadPot1(true); // force a read
    
    if (sw1On){
      g8_potModeFromSwitch = POT_MODE_LEFT;
      prtln("Pot Mode Switch: Left");
    }
    else if (sw2On){
      g8_potModeFromSwitch = POT_MODE_RIGHT;
      prtln("Pot Mode Switch: Right");
    }
    else{
      g8_potModeFromSwitch = POT_MODE_CENTER;
      prtln("Pot Mode Switch: Center");
    }

    g_bOldPotModeSw1On = sw1On;
    g_bOldPotModeSw2On = sw2On;

    SetPotChannelFromSenseStatus();
  }
#else  
  bool bPotModeSwOn = (digitalRead(GPIN_POT_MODE_SW) == HIGH) ? true : false;
  if (bPotModeSwOn != g_bOldPotModeSwOn){
    ReadPot1(true); // force a read

    if (bPotModeSwOn){
      g8_potModeFromSwitch = POT_MODE_LEFT;
      prtln("Potentiometer Mode Switch: Left");
    } else {
      g8_potModeFromSwitch = POT_MODE_RIGHT;
      prtln("Potentiometer Mode Switch: Right");
    }
    g_bOldPotModeSwOn = bPotModeSwOn;

    SetPotChannelFromSenseStatus();
  }
#endif  
}

void SetPotChannelFromSenseStatus(){
#if GPC_BOARD_2B || GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
#endif

#if GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
  // NOTE: we activate POT-changes by changing the pot-mode switch... we can't use
  // the SSR-state from the sense inputs to activate because then auto-mode will activate POT-changes
  // erroneously!
  
  // enable POT by assigning it a channel. NOTE: user must set only ONE SSR's toggle-switch to manual ON
  // to enable the POT control to set only that channel's phase/duty-cycle/Etc.
  if (g_actualStatus == DEV_STATUS_1)
    g8_potChannel = POT_CHAN_1;
  else if (g_actualStatus == DEV_STATUS_2)
    g8_potChannel = POT_CHAN_2;
  #if ENABLE_SSR_C_AND_D
  else if (g_actualStatus == DEV_STATUS_3)
    g8_potChannel = POT_CHAN_3;
  else if (g_actualStatus == DEV_STATUS_4)
    g8_potChannel = POT_CHAN_4;
  #endif

// don't do this here - because we need a valid channel to save the changed pot-value!!!!
//  else
//    g8_potChannel = POT_CHAN_NONE;
#else
  // original board 1 has no way to sense the SSR's state (could mod it to do that... TODO)
  // Also - the POT mode switch is SPST, only LEFT/RIGHT, no CENTER...
  // so we'll just set the channel to SSR1 all the time for now...
  g8_potChannel = POT_CHAN_1;
#endif    
}

#if GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
bool ShouldComLedIndicateInfinite(){
  bool bSetLedOn = false;
  if (g8_potModeFromSwitch == POT_MODE_LEFT){
    if (g8_potChannel == POT_CHAN_1){
      // on channel 1, if pot is full-left and switch is full left we are setting random period
      if (g_potPercent == 0)
        bSetLedOn = true;
    }
    // on channels 2, 3 and 4, if pot is full-right and switch is full left we are setting random phase
    else if (g_potPercent == 100)
      bSetLedOn = true;
  }
  // if pot is full-left and switch is full right we are setting random duty-cycle for the selected channel
  else if (g8_potModeFromSwitch == POT_MODE_RIGHT){
    if (g_potPercent == 0)
      bSetLedOn = true;
  }
  return bSetLedOn;
}

void ReadActualSSRState(){
  int iOldActualStatus = g_actualStatus;

  //------------------------------------------------------------------
  bool bIsOn = digitalRead(GPIN_SSR1);
  bool bStatus = (g_actualStatus & DEV_STATUS_1) ? true : false;
  if (bIsOn && !bStatus)
    g_actualStatus |= DEV_STATUS_1;
  else if (!bIsOn && bStatus)
    g_actualStatus &= ~DEV_STATUS_1;
    
  bIsOn = digitalRead(GPIN_SSR2);
  bStatus = (g_actualStatus & DEV_STATUS_2) ? true : false;
  if (bIsOn && !bStatus)
    g_actualStatus |= DEV_STATUS_2;
  else if (!bIsOn && bStatus)
    g_actualStatus &= ~DEV_STATUS_2;

  #if ENABLE_SSR_C_AND_D
  bIsOn = digitalRead(GPIN_SSR3);
  bStatus = (g_actualStatus & DEV_STATUS_3) ? true : false;
  if (bIsOn && !bStatus)
    g_actualStatus |= DEV_STATUS_3;
  else if (!bIsOn && bStatus)
    g_actualStatus &= ~DEV_STATUS_3;

  bIsOn = digitalRead(GPIN_SSR4);
  bStatus = (g_actualStatus & DEV_STATUS_4) ? true : false;
  if (bIsOn && !bStatus)
    g_actualStatus |= DEV_STATUS_4;
  else if (!bIsOn && bStatus)
    g_actualStatus &= ~DEV_STATUS_4;
  #endif

  //------------------------------------------------------------------
  if (iOldActualStatus){
    if (g_actualStatus > iOldActualStatus)
      // cancel pot-programming if user (or auto) has turned on a second SSR!!!!
      CancelPotProgramming();
    else if (!g_actualStatus){
      // if we were changing the pot and the SSR SPDT switch is changed to center-off (or auto mode turns all off)
      // we initiate storage of the new parameter's value in the EEPROM (flash)
//      prtln("DEBUG: ReadActualSSRState(): actual status now off!!!!");
      if (IsPotProgrammingOn()){
//        prtln("DEBUG: ReadActualSSRState(): Queuing TASK_POTCHANGE. Chan=" + String(g8_potChannel) + ", Mode=" + String(g8_potModeFromSwitch));
        int iModeAndChan = ((int)g8_potModeFromSwitch)<<8;
        iModeAndChan |= g8_potChannel;
        TSK.QueueTask(TASK_POTCHANGE, iModeAndChan, g_potPercent);
      }
      CancelPotProgramming();
    }
  }
}

void CancelPotProgramming(){
  digitalWrite(GPOUT_COM_LED, false);
  g8_potModeFromSwitch = POT_MODE_NONE;
  g8_potChannel = POT_CHAN_NONE;
  g8_potLedFlashTime = 255;
}

bool IsPotProgrammingOn(){
  return g8_potLedFlashTime != 255 && g8_potModeFromSwitch != POT_MODE_NONE && g8_potChannel != POT_CHAN_NONE;
}
#endif

void ReadWiFiSwitch(){
#if FORCE_AP_ON
  g8_wifiModeFromSwitch = WIFI_SW_MODE_AP;
#elif FORCE_STA_ON
  g8_wifiModeFromSwitch = WIFI_SW_MODE_STA;
#else
  bool apSwOn = digitalRead(GPIN_WIFI_AP_SW);
  bool staSwOn = digitalRead(GPIN_WIFI_STA_SW);
  if (apSwOn != g_bOldWiFiApSwOn || staSwOn != g_bOldWiFiStaSwOn){
    if (apSwOn){
      g8_wifiModeFromSwitch = WIFI_SW_MODE_AP;
      prtln("WiFi Mode Switch: AP");
    }
    else if (staSwOn){
      g8_wifiModeFromSwitch = WIFI_SW_MODE_STA;
      prtln("WiFi Mode Switch: STA");
    }
    else{
      g8_wifiModeFromSwitch = WIFI_SW_MODE_OFF;
      prtln("WiFi Mode Switch: OFF");
    }

    g_bOldWiFiStaSwOn = staSwOn;
    g_bOldWiFiApSwOn = apSwOn;
  }
#endif
}

// Usage: SetSSRMode(GPOUT_SSR2, SSR_MODE_ON) sets DEV_STATUS_2 bit
//        SetSSRMode(GPOUT_SSR2, SSR_MODE_AUTO) clears DEV_STATUS_2 bit
//        SetSSRMode(GPOUT_SSR2, SSR_MODE_OFF) clears DEV_STATUS_2 bit
void SetSSRMode(uint8_t gpout, uint8_t ssrMode){
  if (ssrMode == SSR_MODE_ON)
    SetSSR(gpout, true);
  else
    SetSSR(gpout, false);
}

// sets global g_devStatus (DEV_STATUS_1/DEV_STATUS2/DEV_STATUS3/DEV_STATUS4)
// bits to reflect the GPOUT_SSR1/GPOUT_SSR2/GPOUT_SSR3/GPOUT_SSR4 states
void SetSSR(uint8_t gpout, bool bSetSsrOn){
  bool bIsOn = (digitalRead(gpout) == HIGH) ? true : false;

  if (bSetSsrOn){
    if (!bIsOn)
      digitalWrite(gpout, HIGH);
    if (gpout == GPOUT_SSR1){
      g_devStatus |= DEV_STATUS_1;
      g_stats.AOnCounter++;
    }
    else if (gpout == GPOUT_SSR2){
      g_devStatus |= DEV_STATUS_2;
      g_stats.BOnCounter++;
    }
#if ENABLE_SSR_C_AND_D
    else if (gpout == GPOUT_SSR3){
      g_devStatus |= DEV_STATUS_3;
      g_stats.COnCounter++;
    }
    else if (gpout == GPOUT_SSR4){
      g_devStatus |= DEV_STATUS_4;
      g_stats.DOnCounter++;
    }
#endif
  }
  else if (bIsOn){ // Set to OFF or AUTO
    digitalWrite(gpout, LOW);
    if (gpout == GPOUT_SSR1){
      g_devStatus &= ~DEV_STATUS_1;
    }
    else if (gpout == GPOUT_SSR2){
      g_devStatus &= ~DEV_STATUS_2;
    }
#if ENABLE_SSR_C_AND_D
    else if (gpout == GPOUT_SSR3){
      g_devStatus &= ~DEV_STATUS_3;
    }
    else if (gpout == GPOUT_SSR4){
      g_devStatus &= ~DEV_STATUS_4;
    }
#endif
  }
}

String PercentOnToString(uint32_t totalDCon, uint32_t totalTime){
  uint8_t pOn = (uint8_t)(100.0*totalDCon/(float)totalTime);
  return String(pOn);
}

// ssr1Mode can be "OFF, "ON" or "AUTO"
String SsrModeToString(uint8_t ssrMode){
  if (ssrMode == SSR_MODE_OFF)
    return "OFF";
  if (ssrMode == SSR_MODE_ON)
    return "ON";
  if (ssrMode == SSR_MODE_AUTO)
    return "AUTO";
  return "";
}

void ResetPeriod()
{
  g32_dutyCycleTimerA = 0;
  g32_dutyCycleTimerB = 0;
  g32_phaseTimerB = 0;
#if ENABLE_SSR_C_AND_D
  g32_phaseTimerC = 0;
  g32_phaseTimerD = 0;
  g32_dutyCycleTimerC = 0;
  g32_dutyCycleTimerD = 0;
#endif
  g32_periodTimer = 1; // restart cycle...
  g32_savePeriod = 1;
}

void LimitPeriod(){
  // g32_dutyCycleTimerA and the phase-timers will reset with new period but
  // g32_dutyCycleTimerB,C,D keep running...
  if (g32_periodTimer > MIN_PERIOD_TIMER){
    g32_periodTimer = MIN_PERIOD_TIMER;
    g32_savePeriod = MIN_PERIOD_TIMER;
  }
  if (g32_dutyCycleTimerB > MIN_PERIOD_TIMER)
    g32_dutyCycleTimerB = MIN_PERIOD_TIMER;
#if ENABLE_SSR_C_AND_D
  if (g32_dutyCycleTimerC > MIN_PERIOD_TIMER)
    g32_dutyCycleTimerC = MIN_PERIOD_TIMER;
  if (g32_dutyCycleTimerD > MIN_PERIOD_TIMER)
    g32_dutyCycleTimerD = MIN_PERIOD_TIMER;
#endif
}

// This provides an HTML select list of available wifi networks.
String WiFiScan(String sInit){
  String s = "<option value='" + sInit + "'>";
  int n = WiFi.scanNetworks();
//  prtln("network scan count: " + String(n));
  if (n == 0) return s;
  for (int ii = 0; ii < n; ++ii){
    String ssid = String(WiFi.SSID(ii));
//    prtln("ssid: \"" + ssid + "\"");
    // skip adding current wifi router since it's added above...
    if (ssid != sInit)
      s += "<option value='" + ssid + "'>"; // WiFi.RSSI(i), WiFi.channel(i)
  }
  return s;
}

//bool isIp(String sIn){
//  IPAddress ip;
//  ip.fromString(sIn);
//  if ((uint32_t)ip == 0)
//    return false;
//  return true;
//}

bool alldigits(String &sIn){
  int len = sIn.length();
  
  if (len == 0)
    return false;
    
  // handle case of a leading +/- sign
  int startIdx = 0;
  if (len >= 2 && (sIn[0] == '+' || sIn[0] == '-') && isdigit(sIn[1]))
    startIdx = 2;
    
  for (int ii=startIdx; ii<len; ii++)
    if (!isdigit(sIn[ii]))
      return false;
      
  return true;
}
  
// start process of sending out new "pending" default token to all
// remote units. As the response "TOKOK" comes back, we set the bTokOk flag for each mDNS IP.
// When all have been set, we set the new token for ourself (in flash)
// StartNewRandomDefaultToken() is usually called when a g_bMaster has incrimented g16_sendDefTokenTimer
// up to g16_sendDefTokenTime... But - a non-master WILL call this when CanRx succeeds after first failing and
// being "repaired" with a new CanRx tx token... (see Tasks.cpp)
// Only call if master!
bool MasterStartRandomDefaultTokenChange(){
  if (!g_bWiFiConnected || !g_bMaster)
    return false;
    
  int newToken = g_defToken;
  while (newToken == g_defToken)
    newToken = GetRandToken(); // generate new random base 64 default token for currently linked ESP32s
  // Master will send CMchangeData command to all non-master units
  return MasterStartSynchronizedChange(CD_CMD_TOKEN, 0, String(newToken));
}

int GetRandToken(){
  return random(MIN_TOKEN, MAX_TOKEN+1);
}

// called when a user sets a new default token from the command-interface while
// connected to the WiFi router or from MasterStartRandomDefaultTokenChange(), CD_CMD_TOKEN,
// or from Tasks.cpp TASK_MAC after a new MAC address has been saved in flash CD_CMD_RECONNECT.
// Also called for the command "c synchronize"
// Call if either master or slave
bool QueueSynchronizedChange(int iChangeCmd, int iChangeFlags, String sChangeData){
  if (!g_bWiFiConnected || !IML.GetCount())
    return false;
    
  if (g_bMaster)
    return MasterStartSynchronizedChange(iChangeCmd, iChangeFlags, sChangeData);

  // non-masters need to send a request to the master to initiate a default token-change
  int iIdx = IML.FindMdnsIp(g_IpMaster);
  if (iIdx < 0)
    return false;
    
  switch(iChangeCmd){
    case CD_CMD_TOKEN:
      prtln("this non-master is requesting system-wide default token-change...");
    break;
    
    case CD_CMD_RECONNECT:
      prtln("this non-master is requesting system-wide WiFi reset and mDNS IP purge...");
    break;
    
    case CD_CMD_CYCLE_PARMS:
      prtln("this non-master is requesting system-wide cycle-timing propigation...");
    break;
    
    default:
      return false;
    break;
  };

  // send CMchangeReq to master
  HMC.AddCMchangeReq(iChangeCmd, iChangeFlags, sChangeData, iIdx, MDNS_STRING_SEND_SPECIFIC);
  
  return true;
}

// Only call if master
bool MasterStartSynchronizedChange(int iChangeCmd, int iChangeFlags, String sChangeData){
  if (!g_bWiFiConnected || !g_bMaster)
    return false;
    
  bool bRet;
  // Master will send CMchangeData command to all non-master units
  if (HMC.AddCMchangeDataAll(iChangeCmd, iChangeFlags, sChangeData) > 0){
    IML.SetFlagForAllIP(MDNS_FLAG_CHANGE_OK, false); // cancel any preceding token-change operation...
    CNG.QueueChange(iChangeCmd, iChangeFlags, sChangeData);
    bRet = true;
  }
  else
    bRet = false;
    
  switch(iChangeCmd){
    case CD_CMD_TOKEN:
      g16_sendDefTokenTimer = 0;
      g16_sendDefTokenTime = random(SEND_DEF_TOKEN_TIME_MIN, SEND_DEF_TOKEN_TIME_MAX+1);
    break;
    
    case CD_CMD_RECONNECT:
    break;
    
    case CD_CMD_CYCLE_PARMS:
    break;

    default:
    break;
  };
  
  return bRet;
}

void RefreshSct(){
  g_sct = GetSct(g_minSct, g_maxSct); // return g_minSct and g_maxSct by reference
}

int GetSct(int &minSct, int &maxSct)
{
  minSct = MIN_SHIFT_COUNT;
  maxSct = random(MAX_SHIFT_COUNT/4, MAX_SHIFT_COUNT);
  return random(minSct, maxSct);
}

// Channel 1 (A) switches off when g32_dutyCycleTimerA decrements to 0.
// It switches on when g32_periodTimer decrements to 0
int ComputeTimeToOnOrOffA(){
  if (g8_ssr1ModeFromWeb != SSR_MODE_AUTO)
    return -1;
  if (g_devStatus & DEV_STATUS_1)
    return g32_dutyCycleTimerA;
  return g32_periodTimer;
}

// Channel 2 (B) switches off when g32_dutyCycleTimerB decrements to 0.
// It switches on when g32_phaseTimerB decrements to 0 if g32_phaseTimerB is running, or
// at g32_nextPhase + g32_periodTimer if it's not...
int ComputeTimeToOnOrOffB(){
  if (g8_ssr2ModeFromWeb != SSR_MODE_AUTO)
    return -1;
  if (g_devStatus & DEV_STATUS_2)
    return g32_dutyCycleTimerB;
  if (g32_phaseTimerB)
    return g32_phaseTimerB;
  return g32_periodTimer+g32_nextPhaseB;
}

#if ENABLE_SSR_C_AND_D
int ComputeTimeToOnOrOffC(){
  if (g8_ssr3ModeFromWeb != SSR_MODE_AUTO)
    return -1;
  if (g_devStatus & DEV_STATUS_3)
    return g32_dutyCycleTimerC;
  if (g32_phaseTimerC)
    return g32_phaseTimerC;
  return g32_periodTimer+g32_nextPhaseC;
}

int ComputeTimeToOnOrOffD(){
  if (g8_ssr4ModeFromWeb != SSR_MODE_AUTO)
    return -1;
  if (g_devStatus & DEV_STATUS_4)
    return g32_dutyCycleTimerD;
  if (g32_phaseTimerD)
    return g32_phaseTimerD;
  return g32_periodTimer+g32_nextPhaseD;
}
#endif

// set sReloadUrl to P2_FILENAME Etc.
bool IsLockedAlertGetPlain(AsyncWebServerRequest *request, bool bAllowInAP){
  if (bAllowInAP && g_bSoftAP)
    return false;

  if (IsLocked()){
    request->send(HTTPCODE_OK, "text/html", "System is locked!");
    return true;
  }
  return false;
}

// set sReloadUrl to P2_FILENAME Etc.
bool IsLockedAlertGet(AsyncWebServerRequest *request, String sReloadUrl, bool bAllowInAP){
  if (bAllowInAP && g_bSoftAP)
    return false;

  if (IsLocked()){
    String s = "<script>alert('System is locked!');";
    if (sReloadUrl != "")
      s += "location.href='" + String(sReloadUrl) + "';";
    s += "</script>";
    request->send(HTTPCODE_OK, "text/html", s);
    return true;
  }
  return false;
}

bool IsLockedAlertPost(AsyncWebServerRequest *request, bool bAllowInAP){
  if (bAllowInAP && g_bSoftAP)
    return false;

  if (IsLocked()){
    request->send(HTTPCODE_OK, "text/html", "System is locked!");
    return true;
  }
  return false;
}

bool IsLocked(){
  return g8_lockCount != 0xff;
}

String SyncFlagStatus(){
  const char ON[] = "ON\n";
  const char OFF[] = "OFF\n";
  String sOut, sOnOff;
  sOnOff = g_bSyncRx ? ON:OFF;
  sOut += "rx sync to remote: " + sOnOff;
  sOnOff = g_bSyncTx ? ON:OFF;
  sOut += "tx sync to remote: " + sOnOff;
  sOnOff = g_bSyncCycle ? ON:OFF;
  sOut += "cycle sync to remote: " + sOnOff;
  sOnOff = g_bSyncTime ? ON:OFF;
  sOut += "time sync to remote: " + sOnOff;
  sOnOff = g_bSyncEncrypt ? ON:OFF;
  sOut += "HTTP encrypt: " + sOnOff;
  return sOut;
}

void FlashSequencerStop(){
  FlashSequencerInit(g8_wifiLedMode_OFF);
  g8_wifiLedSeqState == LEDSEQ_ENDED;
}

// NOTE: g8_wifiLedDigitArray must have a 0 terminator to mark the end of sequence!
// bStart defaults false
void FlashSequencerInit(uint8_t postFlashMode){
  g8_wifiLedFlashCounter = 0;
  g8_wifiLedDigitCounter = 0;
  g8_wifiLedSaveMode = postFlashMode; // save post-flash mode...
  g8_wifiLedMode = g8_wifiLedMode_PAUSED;
  g8_wifiLedSeqState = LEDSEQ_PAUSED;
#if GPC_BOARD_2B || GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
  digitalWrite(GPOUT_WIFI_LED, LOW);
#endif
  digitalWrite(GPOUT_ONBOARD_LED, LOW);
  g_bWiFiLedOn = false;
}

void FlashSequencer(){
  if (g8_wifiLedSeqState == LEDSEQ_PAUSED){
    if (g8_wifiLedFlashCounter >= LED_PAUSE_COUNT){
      g8_wifiLedFlashCounter = 0;
      uint8_t digitCount = g8_wifiLedDigitArray[g8_wifiLedDigitCounter++];
      if (digitCount == 0){ // end of digits in sequence...
        g8_wifiLedMode = g8_wifiLedSaveMode; // return to "connected" flashing
        g8_wifiLedSeqState = LEDSEQ_ENDED;
      }
      else{
        g8_wifiLedFlashCount = digitCount;
        g8_wifiLedSeqState = LEDSEQ_FLASHING;
        g8_wifiLedMode = g8_wifiLedMode_FASTFLASH;
      }
    }
  }
  else if (g8_wifiLedSeqState == LEDSEQ_FLASHING){
    if (g8_wifiLedFlashCounter >= g8_wifiLedFlashCount){
      g8_wifiLedFlashCounter = 0;
      g8_wifiLedMode = g8_wifiLedMode_PAUSED;
      g8_wifiLedSeqState = LEDSEQ_PAUSED;
    }
  }
}

void FlashLED(){
  if (g8_wifiLedMode == g8_wifiLedMode_PAUSED)
    g8_wifiLedFlashCounter++; // count 1/4 sec pause interval
  else if (g8_wifiLedMode == g8_wifiLedMode_OFF){
    if (!g_bWiFiLedOn){
#if GPC_BOARD_2B || GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
      digitalWrite(GPOUT_WIFI_LED, LOW);
#endif
      digitalWrite(GPOUT_ONBOARD_LED, LOW);
      g_bWiFiLedOn = true;
    }
    if (g8_wifiLedFlashTimer)
      g8_wifiLedFlashTimer = 0;
  }
  else if (g8_wifiLedMode == g8_wifiLedMode_ON){
    if (!g_bWiFiLedOn){
#if GPC_BOARD_2B || GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
      digitalWrite(GPOUT_WIFI_LED, HIGH);
#endif
      digitalWrite(GPOUT_ONBOARD_LED, HIGH);
      g_bWiFiLedOn = true;
    }
    if (g8_wifiLedFlashTimer)
      g8_wifiLedFlashTimer = 0;
  }
  else if (g8_wifiLedFlashTimer){
    if (--g8_wifiLedFlashTimer == 0){
      if (!g_bWiFiLedOn){
#if GPC_BOARD_2B || GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
        digitalWrite(GPOUT_WIFI_LED, HIGH);
#endif
        digitalWrite(GPOUT_ONBOARD_LED, HIGH);
        g_bWiFiLedOn = true;
      }
      else{
#if GPC_BOARD_2B || GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
        digitalWrite(GPOUT_WIFI_LED, LOW);
#endif
        digitalWrite(GPOUT_ONBOARD_LED, LOW);
        g_bWiFiLedOn = false;
        g8_wifiLedFlashCounter++;
      }
      
      if (g8_wifiLedMode == g8_wifiLedMode_SLOWFLASH)
        g8_wifiLedFlashTimer = LED_SLOWFLASH_TIME;
      else if (g8_wifiLedMode == g8_wifiLedMode_FASTFLASH)
        g8_wifiLedFlashTimer = LED_FASTFLASH_TIME;
    }
  }
  else // start off either slow or fast flash
    g8_wifiLedFlashTimer = 1;
}

// generate variable length random message
String genRandMessage(int iMin, int iMax){
  int N = random(iMin, iMax+1); // message iMin-iMax chars
  if (N == 0)
    return "";
  char arr[N+1];
  for (int ii=0; ii < N; ii++)
     arr[ii] = random('\x20', '\x7f');
  arr[N] = '\0';
  return String(arr);
}

// returns empty string if failure
String MyEncodeNum(int iIn, int table, int token, int context){
  String s;
  if (g_bSyncEncrypt){
    s = B64C.hnEncNumOnly(iIn, table, token);
//prtln("MyEncodeNum(): 1: \"" + s + "\"");
    if (s.isEmpty())
      return "";
    s = AddTwoDigitBase16Checksum(s);
//prtln("MyEncodeNum(): 2: \"" + s + "\"");
    if (s.isEmpty())
      return "";
    s = CIP.encryptString(s, token, context);
//prtln("MyEncodeNum(): 3: \"" + s + "\"");
    if (s.isEmpty())
      return "";
    s = B64C.hnEncodeStr(s, table, token);
//prtln("MyEncodeNum(): 4: \"" + s + "\"");
  }else
    s = B64C.hnEncNum(iIn, table, token);

  return s;
}

// positive numbers only (need to test...)
// returns negative if error, 0 if success, result is in iOut
int MyDecodeNum(int& iOut, String s, int table, int token, int context){
  iOut = 0;
  if (s.isEmpty())
    return -1;
  if (g_bSyncEncrypt){
//prtln("MyDecodeNum(): 0: \"" + s + "\"");
    s = B64C.hnDecodeStr(s, table, token);
//prtln("MyDecodeNum(): 1: \"" + s + "\"");
    if (s.isEmpty())
      return -2;
    s = CIP.decryptString(s, token, context);
//prtln("MyDecodeNum(): 2: \"" + s + "\"");
    if (s.isEmpty())
      return -3;
    s = SubtractTwoDigitBase16Checksum(s);
//prtln("MyDecodeNum(): 3: \"" + s + "\"");
    if (s.isEmpty()){
      prtln("GpcUtils.cpp MyDecodeNum() SubtractTwoDigitBase16Checksum(): bad checksum!");
      return -4;
    }
    iOut = B64C.hnDecNumOnly(s, table, token);
//prtln("MyDecodeNum(): 4: \"" + String(iOut) + "\"");
  }
  else
    iOut = B64C.hnDecNum(s, table, token);
  return 0;
}

int MyEncodeStr(String& sInOut, int table, int token, int context){
  
  if (sInOut.isEmpty())
    return -1;
    
  if (g_bSyncEncrypt){
    sInOut = AddTwoDigitBase16Checksum(sInOut);
    if (sInOut.isEmpty())
      return -2;
    sInOut = CIP.encryptString(sInOut, token, context);
    if (sInOut.isEmpty())
      return -3;
    sInOut = B64C.hnEncodeStr(sInOut, table, token);
    if (sInOut.isEmpty())
      return -4;
  }
  else{
    sInOut = B64C.hnShiftEncode(sInOut, table, token);
    if (sInOut.isEmpty())
      return -5;
  }
  return 0;
}

// returns 0 if no error, negative error code
int MyDecodeStr(String& sInOut, int table, int token, int context){
  
  if (sInOut.isEmpty())
    return -1;
    
  if (g_bSyncEncrypt){
    sInOut = B64C.hnDecodeStr(sInOut, table, token);
    if (sInOut.isEmpty())
      return -2;
    sInOut = CIP.decryptString(sInOut, token, context);
    if (sInOut.isEmpty())
      return -3;
    sInOut = SubtractTwoDigitBase16Checksum(sInOut);
    if (sInOut.isEmpty()){
      prtln("GpcUtils.cpp MyDecodeStr() SubtractTwoDigitBase16Checksum(): bad checksum!");
      return -4;
    }
  }
  else{
    sInOut = B64C.hnShiftDecode(sInOut, table, token);
    if (sInOut.isEmpty())
      return -5;
  }
  return 0;
}

// add a two-digit base-16 checksum (00-ff)
String AddTwoDigitBase16Checksum(String sIn){
  uint8_t cs = 0;
  int len = sIn.length();
  for (int i=0; i<len; i++)
    cs += (uint8_t)sIn[i];
  cs = ~cs+1;
  String sCs = String(cs, 16);
  sCs.toLowerCase();
//prtln("DEBUG: AddTwoDigitBase16Checksum(): " + sCs);

  len = sCs.length();
  if (len == 1)
    sCs = "0" + sCs;
  else if (len != 2)
    return "";
  return sIn + sCs;
}

// removes and checks a uint8_t 0-255 checksum expected at the end of the
// string as two ASCII base-16 digits 00-ff
// returns empty string if bad checksum
String SubtractTwoDigitBase16Checksum(String sIn){
  uint8_t cs = 0;
  int len = sIn.length();
  if (len < 3) // 00-ff
    return "";
  String sCs = sIn.substring(len-2);
//prtln("DEBUG: SubtractTwoDigitBase16Checksum(): " + sCs);

  int iCs = (int)strtol(sCs.c_str(), NULL, 16); // convert base 16 ascii to int
  if (iCs < 0 || iCs > 255)
    return "";
  sIn = sIn.substring(0, len-2);
  len = sIn.length();
  uint8_t uCs = 0;
  for (int i=0; i<len; i++)
    uCs += (uint8_t)sIn[i];
  uCs += (uint8_t)iCs;
  if (uCs)
    return "";

  return sIn;
}

// Pertains to the Gpc.h conditional-compile boolean switches:
// READ_WRITE_CUSTOM_BLK3_MAC, FORCE_NEW_EFUSE_BITS_ON, WRITE_PROTECT_BLK3
void InitMAC(){
  // It appears that we can write to BLK3 using esp_efuse_write_field_blob() and it will
  // persist through power-cycling and resets. But if you reflash the program it is erased.
  // To PERMANENTLY write it, set BURN_EFUSE_ENABLED true
  // aplication version is stored in esp_app_desc_t in DROM
  // set PROJECT_VER in CMakeLists.txt... insert "set(PROJECT_VER, "0.1.0.1") before "project.cmake"
  // (or put version string in versions.txt in project root)
  // app can access the project version with esp_ota_get_app_description()
  // esp_efuse_mac_get_default() read factory-programmed MAC
  // esp_efuse_mac_get_custom() read from EFUSE_BLK3_RDATA0 to EFUSE_BLK3_RDATA5
  //  returns ESP_OK, ESP_ERR_INVALID_VERSION, ESP_ERR_INVALID_CRC
  //  returns: Version 1 byte
  //           Reserved 16 bytes (128 bits)
  //           Mac Addr 6 bytes (48 bits)
  //           Mac CRC 1 byte
  // esp_err_t esp_base_mac_addr_set/get(const uint8_t *pMac) sets a new base mac (call early in app_main)
  // returns ESP_OK or ESP_ERR_INVALID_ARG, pMac points to 6-byte array
  // esp_read_mac() get base_mac returns factory mac in BLK0
  // esp_wifi_get_mac()
  // esp_efuse_mac_get_default()
  // esp_read_mac(uint8_t *pMac, esp_mac_type_t type) returns 6 bytes, type code 0=wifi,1=softap,2=bluetooth,3=ethernet
  // Wi-Fi Station base_mac
  // Wi-Fi SoftAP base_mac + 1 to the last octet
  // Bluetooth base_mac + 1 to the last octet
  // Ethernet base_mac + 1 to the last octet
  // NOTE: MAC must be "unicast". lsb of first byte must be 0)
  // set the "locally administered" bit 1 of first byte)(0x02)
  // esp_derive_local_mac(uint8_t *localmac, uint8_t *universalmac); returns ESP_OK if success

  // One-time burn MAC address
  // uint8_t mac[6]; // = {0xe6, 0xcd, 0x43, 0xa3, 0x6e, 0xb5};
  // esp_err_t esp_efuse_read_field_blob(ESP_EFUSE_MAC_CUSTOM, mac, 48); // check first to make sure empty (all 0s)!
  // esp_err_t esp_efuse_write_field_blob(const esp_efuse_desc_t *field[], const void *src, size_t src_size_bits)
  // returns ESP_OK
  // void esp_efuse_burn_new_values(void)
  // esp_base_mac_addr_set(); // set all MAC addresses to BLK3 Mac
  // https://docs.espressif.com/projects/esp-idf/en/v3.1.7/api-reference/system/base_mac_address.html
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/efuse.html

  //int numBits = esp_efuse_get_field_size(ESP_EFUSE_MAC_CUSTOM);
  //prtln("bits in custom MAC field = " + String(numBits)); // 48 bits
  //int numBytes = numBits/8;
#if READ_WRITE_CUSTOM_BLK3_MAC
  uint8_t mac[6] = {0};
  if (esp_efuse_read_field_blob(ESP_EFUSE_MAC_CUSTOM, mac, 48) == ESP_OK){
    bool bIsEmpty = true;
    for (int ii=0; ii < 6; ii++){
      prtln("MAC[" + String(ii) + "] = " + String(mac[ii]));
      if (bIsEmpty && mac[ii] != 0)
        bIsEmpty = false;
    }
  #if !FORCE_NEW_EFUSE_BITS_ON
    if (bIsEmpty){
  #endif
      mac[0] = BLK3_MAC0;
      mac[1] = BLK3_MAC1;
      mac[2] = BLK3_MAC2;
      mac[3] = BLK3_MAC3;
      mac[4] = BLK3_MAC4;
      mac[5] = BLK3_MAC5;
      // burn the efuse permanently (be careful!)
      if (esp_efuse_write_field_blob(ESP_EFUSE_MAC_CUSTOM, mac, 48) == ESP_OK)
        prtln("wrote new efuse MAC to BLK3 registers!");
      else
        prtln("error writing new efuse MAC into BLK3...");
  #if !FORCE_NEW_EFUSE_BITS_ON
    }
    else
      prtln("successfully read custom base MAC address from BLK3 efuse...");
  #endif

    prtln("setting custom base MAC address from BLK3 efuse...");
    esp_base_mac_addr_set(mac); // set all MAC addresses to BLK3 Mac
  }
  else if (esp_efuse_mac_get_default(mac) == ESP_OK)
  {
    prtln("setting default base MAC address...");
    esp_base_mac_addr_set(mac); // set all MAC addresses to BLK3 Mac
  }
  else
    prtln("error reading default base MAC address...");

#endif // end #if READ_WRITE_CUSTOM_BLK3_MAC

#if WRITE_PROTECT_BLK3

    if (esp_efuse_set_write_protect(EFUSE_BLK3) == ESP_OK)
      prtln("BLK3 efuse write-protect set!");
    else
      prtln("BLK3 efuse already write-protected!");

#endif
}

// call every .5 second to update stats
void TaskStatisticsMonitor(){
  if (++g_stats.HalfSecondCounter > g_stats.HalfSecondCount){
    g_stats.AOnPrevCount = g_stats.AOnCounter;
    g_stats.BOnPrevCount = g_stats.BOnCounter;
    g_stats.COnPrevCount = g_stats.COnCounter;
    g_stats.DOnPrevCount = g_stats.DOnCounter;
    g_stats.PrevDConA = g_stats.DConA;
    g_stats.PrevDConB = g_stats.DConB;
    g_stats.PrevDConC = g_stats.DConC;
    g_stats.PrevDConD = g_stats.DConD;
    ClearStatCounters();
  }

#if GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
  if (g_actualStatus & DEV_STATUS_1)
    g_stats.DConA++;
  if (g_actualStatus & DEV_STATUS_2)
    g_stats.DConB++;
  #if ENABLE_SSR_C_AND_D
    if (g_actualStatus & DEV_STATUS_3)
      g_stats.DConC++;
    if (g_actualStatus & DEV_STATUS_4)
      g_stats.DConD++;
  #endif
#else
  // we simply count time in .5 sec units when a channel is on
  // these get copied to g_stats.PrevDConA/B when monitor interval
  // rolls over
  if (g_devStatus & DEV_STATUS_1)
    g_stats.DConA++;
  if (g_devStatus & DEV_STATUS_2)
    g_stats.DConB++;
#endif
}

void TaskProcessPulseOffFeatureTiming(){
  // SSR1
  if (g8_ssr1ModeFromWeb == SSR_MODE_AUTO && g8_pulseModeA != PULSE_MODE_OFF){
    if (g16_pulsePeriodTimerA == 0){
      if (g8_pulseModeA == PULSE_MODE_OFF_IF_ON){
        if (g32_dutyCycleTimerA)
          SetSSR(GPOUT_SSR1, false); // turn off A
      }
      else if (g8_pulseModeA == PULSE_MODE_ON_IF_OFF){
        if (!g32_dutyCycleTimerA)
          SetSSR(GPOUT_SSR1, true); // turn on A
      }
      else if (g8_pulseModeA == PULSE_MODE_ON_OR_OFF){
        if (g32_dutyCycleTimerA)
          SetSSR(GPOUT_SSR1, false); // turn off A
        else
          SetSSR(GPOUT_SSR1, true); // turn on A
      }
      g16_pulsePeriodTimerA = g16_pulsePeriodA;
      g8_pulseWidthTimerA = g8_pulseWidthA;
    }
    else
      g16_pulsePeriodTimerA--;

    if (g8_pulseWidthTimerA == 0){
      if (g8_pulseModeA == PULSE_MODE_OFF_IF_ON){
        if (g32_dutyCycleTimerA)
          SetSSR(GPOUT_SSR1, true); // turn on A
      }
      else if (g8_pulseModeA == PULSE_MODE_ON_IF_OFF){
        if (!g32_dutyCycleTimerA)
          SetSSR(GPOUT_SSR1, false); // turn off A
      }
      else if (g8_pulseModeA == PULSE_MODE_ON_OR_OFF){
        if (g32_dutyCycleTimerA)
          SetSSR(GPOUT_SSR1, true); // turn on A
        else
          SetSSR(GPOUT_SSR1, false); // turn off A
      }
    }
    else
      g8_pulseWidthTimerA--;
  }
  
  // SSR2
  if (g8_ssr2ModeFromWeb == SSR_MODE_AUTO && g8_pulseModeB != PULSE_MODE_OFF){
    if (g16_pulsePeriodTimerB == 0){
      if (g8_pulseModeB == PULSE_MODE_OFF_IF_ON){
        if (g32_dutyCycleTimerB)
          SetSSR(GPOUT_SSR2, false); // turn off B
      }
      else if (g8_pulseModeB == PULSE_MODE_ON_IF_OFF){
        if (!g32_dutyCycleTimerB)
          SetSSR(GPOUT_SSR2, true); // turn on B
      }
      else if (g8_pulseModeB == PULSE_MODE_ON_OR_OFF){
        if (g32_dutyCycleTimerB)
          SetSSR(GPOUT_SSR2, false); // turn off B
        else
          SetSSR(GPOUT_SSR2, true); // turn on B
      }
      g16_pulsePeriodTimerB = g16_pulsePeriodB;
      g8_pulseWidthTimerB = g8_pulseWidthB;
    }
    else
      g16_pulsePeriodTimerB--;

    if (g8_pulseWidthTimerB == 0){
      if (g8_pulseModeB == PULSE_MODE_OFF_IF_ON){
        if (g32_dutyCycleTimerB)
          SetSSR(GPOUT_SSR2, true); // turn on B
      }
      else if (g8_pulseModeB == PULSE_MODE_ON_IF_OFF){
        if (!g32_dutyCycleTimerB)
          SetSSR(GPOUT_SSR2, false); // turn off B
      }
      else if (g8_pulseModeB == PULSE_MODE_ON_OR_OFF){
        if (g32_dutyCycleTimerB)
          SetSSR(GPOUT_SSR2, true); // turn on B
        else
          SetSSR(GPOUT_SSR2, false); // turn off B
      }
    }
    else
      g8_pulseWidthTimerB--;
  }

#if ENABLE_SSR_C_AND_D
  // SSR3
  if (g8_ssr3ModeFromWeb == SSR_MODE_AUTO && g8_pulseModeC != PULSE_MODE_OFF){
    if (g16_pulsePeriodTimerC == 0){
      if (g8_pulseModeC == PULSE_MODE_OFF_IF_ON){
        if (g32_dutyCycleTimerC)
          SetSSR(GPOUT_SSR3, false); // turn off C
      }
      else if (g8_pulseModeC == PULSE_MODE_ON_IF_OFF){
        if (!g32_dutyCycleTimerC)
          SetSSR(GPOUT_SSR3, true); // turn on C
      }
      else if (g8_pulseModeC == PULSE_MODE_ON_OR_OFF){
        if (g32_dutyCycleTimerC)
          SetSSR(GPOUT_SSR3, false); // turn off C
        else
          SetSSR(GPOUT_SSR3, true); // turn on C
      }
      g16_pulsePeriodTimerC = g16_pulsePeriodC;
      g8_pulseWidthTimerC = g8_pulseWidthC;
    }
    else
      g16_pulsePeriodTimerC--;

    if (g8_pulseWidthTimerC == 0){
      if (g8_pulseModeC == PULSE_MODE_OFF_IF_ON){
        if (g32_dutyCycleTimerC)
          SetSSR(GPOUT_SSR3, true); // turn on C
      }
      else if (g8_pulseModeC == PULSE_MODE_ON_IF_OFF){
        if (!g32_dutyCycleTimerC)
          SetSSR(GPOUT_SSR3, false); // turn off C
      }
      else if (g8_pulseModeC == PULSE_MODE_ON_OR_OFF){
        if (g32_dutyCycleTimerC)
          SetSSR(GPOUT_SSR3, true); // turn on C
        else
          SetSSR(GPOUT_SSR3, false); // turn off C
      }
    }
    else
      g8_pulseWidthTimerC--;
  }

  // SSR4
  if (g8_ssr4ModeFromWeb == SSR_MODE_AUTO && g8_pulseModeD != PULSE_MODE_OFF){
    if (g16_pulsePeriodTimerD == 0){
      if (g8_pulseModeD == PULSE_MODE_OFF_IF_ON){
        if (g32_dutyCycleTimerD)
          SetSSR(GPOUT_SSR4, false); // turn off D
      }
      else if (g8_pulseModeD == PULSE_MODE_ON_IF_OFF){
        if (!g32_dutyCycleTimerD)
          SetSSR(GPOUT_SSR4, true); // turn on D
      }
      else if (g8_pulseModeD == PULSE_MODE_ON_OR_OFF){
        if (g32_dutyCycleTimerD)
          SetSSR(GPOUT_SSR3, false); // turn off D
        else
          SetSSR(GPOUT_SSR3, true); // turn on D
      }
      g16_pulsePeriodTimerD = g16_pulsePeriodD;
      g8_pulseWidthTimerD = g8_pulseWidthD;
    }
    else
      g16_pulsePeriodTimerD--;

    if (g8_pulseWidthTimerD == 0){
      if (g8_pulseModeD == PULSE_MODE_OFF_IF_ON){
        if (g32_dutyCycleTimerD)
          SetSSR(GPOUT_SSR4, true); // turn on D
      }
      else if (g8_pulseModeD == PULSE_MODE_ON_IF_OFF){
        if (!g32_dutyCycleTimerD)
          SetSSR(GPOUT_SSR4, false); // turn off D
      }
      else if (g8_pulseModeD == PULSE_MODE_ON_OR_OFF){
        if (g32_dutyCycleTimerD)
          SetSSR(GPOUT_SSR4, true); // turn on D
        else
          SetSSR(GPOUT_SSR4, false); // turn off D
      }
    }
    else
      g8_pulseWidthTimerD--;
  }
#endif
}

void TaskSetPulseOffFeatureVars(){
  // SSR1
  if (g8_pulseModeA != PULSE_MODE_OFF){
    if (g8_pulseMinWidthA > 0 && g8_pulseMaxWidthA > 0){
      g8_pulseWidthA = random(g8_pulseMinWidthA, g8_pulseMaxWidthA+1);
    }
    else if (g8_pulseMaxWidthA > 0 && g8_pulseWidthA != g8_pulseMaxWidthA && g8_pulseMinWidthA == 0){
      g8_pulseWidthA = g8_pulseMaxWidthA;
    }
    else if (g8_pulseWidthA != 0)
      g8_pulseWidthA = 0;
      
    if (g16_pulseMinPeriodA > 0 && g16_pulseMaxPeriodA > 0){
      g16_pulsePeriodA = random(g16_pulseMinPeriodA, g16_pulseMaxPeriodA+1);
    }
    else if (g16_pulseMaxPeriodA > 0 && g16_pulsePeriodA != g16_pulseMaxPeriodA && g16_pulseMinPeriodA == 0){
      g16_pulsePeriodA = g16_pulseMaxPeriodA;
    }
    else if (g16_pulsePeriodA != 0)
      g16_pulsePeriodA = 0;
  }else{
    if (g8_pulseWidthTimerA != 0)
      g8_pulseWidthTimerA = 0;
    if (g16_pulsePeriodTimerA != 0)
      g16_pulsePeriodTimerA = 0;
  }

  // SSR2
  if (g8_pulseModeB != PULSE_MODE_OFF){
    if (g8_pulseMinWidthB > 0 && g8_pulseMaxWidthB > 0){
      g8_pulseWidthB = random(g8_pulseMinWidthB, g8_pulseMaxWidthB+1);
    }
    else if (g8_pulseMaxWidthB > 0 && g8_pulseWidthB != g8_pulseMaxWidthB && g8_pulseMinWidthB == 0){
      g8_pulseWidthB = g8_pulseMaxWidthB;
    }
    else if (g8_pulseWidthB != 0)
      g8_pulseWidthB = 0;
      
    if (g16_pulseMinPeriodB > 0 && g16_pulseMaxPeriodB > 0){
      g16_pulsePeriodB = random(g16_pulseMinPeriodB, g16_pulseMaxPeriodB+1);
    }
    else if (g16_pulseMaxPeriodB > 0 && g16_pulsePeriodB != g16_pulseMaxPeriodB && g16_pulseMinPeriodB == 0){
      g16_pulsePeriodB = g16_pulseMaxPeriodB;
    }
    else if (g16_pulsePeriodB != 0)
      g16_pulsePeriodB = 0;
  }else{
    if (g8_pulseWidthTimerB != 0)
      g8_pulseWidthTimerB = 0;
    if (g16_pulsePeriodTimerB != 0)
      g16_pulsePeriodTimerB = 0;
  }

#if ENABLE_SSR_C_AND_D
  // SSR3
  if (g8_pulseModeC != PULSE_MODE_OFF){
    if (g8_pulseMinWidthC > 0 && g8_pulseMaxWidthC > 0){
      g8_pulseWidthC = random(g8_pulseMinWidthC, g8_pulseMaxWidthC+1);
    }
    else if (g8_pulseMaxWidthC > 0 && g8_pulseWidthC != g8_pulseMaxWidthC && g8_pulseMinWidthC == 0){
      g8_pulseWidthC = g8_pulseMaxWidthC;
    }
    else if (g8_pulseWidthC != 0)
      g8_pulseWidthC = 0;
      
    if (g16_pulseMinPeriodC > 0 && g16_pulseMaxPeriodC > 0){
      g16_pulsePeriodC = random(g16_pulseMinPeriodC, g16_pulseMaxPeriodC+1);
    }
    else if (g16_pulseMaxPeriodC > 0 && g16_pulsePeriodC != g16_pulseMaxPeriodC && g16_pulseMinPeriodC == 0){
      g16_pulsePeriodC = g16_pulseMaxPeriodC;
    }
    else if (g16_pulsePeriodC != 0)
      g16_pulsePeriodC = 0;
  }else{
    if (g8_pulseWidthTimerC != 0)
      g8_pulseWidthTimerC = 0;
    if (g16_pulsePeriodTimerC != 0)
      g16_pulsePeriodTimerC = 0;
  }

  // SSR4
  if (g8_pulseModeD != PULSE_MODE_OFF){
    if (g8_pulseMinWidthD > 0 && g8_pulseMaxWidthD > 0){
      g8_pulseWidthD = random(g8_pulseMinWidthD, g8_pulseMaxWidthD+1);
    }
    else if (g8_pulseMaxWidthD > 0 && g8_pulseWidthD != g8_pulseMaxWidthD && g8_pulseMinWidthD == 0){
      g8_pulseWidthD = g8_pulseMaxWidthD;
    }
    else if (g8_pulseWidthD != 0)
      g8_pulseWidthD = 0;
      
    if (g16_pulseMinPeriodD > 0 && g16_pulseMaxPeriodD > 0){
      g16_pulsePeriodD = random(g16_pulseMinPeriodD, g16_pulseMaxPeriodD+1);
    }
    else if (g16_pulseMaxPeriodD > 0 && g16_pulsePeriodD != g16_pulseMaxPeriodD && g16_pulseMinPeriodD == 0){
      g16_pulsePeriodD = g16_pulseMaxPeriodD;
    }
    else if (g16_pulsePeriodD != 0)
      g16_pulsePeriodD = 0;
  }else{
    if (g8_pulseWidthTimerD != 0)
      g8_pulseWidthTimerD = 0;
    if (g16_pulsePeriodTimerD != 0)
      g16_pulsePeriodTimerD = 0;
  }
#endif
}

// returns mDNS count or negative if error
int SendText(String sText){
  int count = IML.GetCount();
  
  if (!count || !g_bWiFiConnected)
    return -2;  

  if (sText.isEmpty() || sText.length() > MAXTXTLEN)
    return -3;  

  return HMC.AddCommandAll(CMtxt, sText);
}

// returns mDNS count or negative if error
int SendText(int idx, String sText){
  int count = IML.GetCount();
  
  if (!count || !g_bWiFiConnected)
    return -2;  

  if (sText.isEmpty() || sText.length() > MAXTXTLEN)
    return -3;  

  if (idx < 0 || idx >= count)
    return -4;

  HMC.AddCommand(CMtxt, sText, idx, MDNS_STRING_SEND_SPECIFIC); // add to sSendSpecific string rather than sSendAll!
//prtln("DEBUG: SendText(): AddCommand(): \"" + sText + "\"");
  return count;
}

// returns mDNS count or negative if error
int SendText(String sIp, String sText){
  IPAddress ip;
  ip.fromString(sIp);
  if ((uint32_t)ip == 0)
    return -5;
  int idx = IML.FindMdnsIp(ip);
  if (idx < 0)
    return -6;
  return SendText(idx, sText);
}

//---------------------------------------------------------------------------
// Conversions to/from Int/String and print routines
//---------------------------------------------------------------------------

String CommandStrToPrintable(String sIn){
  String sOut;
  for (int ii=0; ii<sIn.length(); ii++){
    char c = sIn[ii];
    if (c > 0 && c <= CMSPECIALRANGESMAX)
      sOut += "0x" + String((int)c, HEX);
    else
      sOut += c;
  }
  return sOut;
}

// perMax is 16 bit, all else is 8 bits
// CNG.QueueChange(CD_CMD_CYCLE_PARMS, 0, PerValsToString(perVals));
String PerValsToString(PerVals perVals){
  return String(perVals.perMax) + ':' + 
         String(perVals.phaseB) + ':' +
         String(perVals.phaseC) + ':' +
         String(perVals.phaseD) + ':' +
         String(perVals.perVal) + ':' +
         String(perVals.perUnits) + ':' +  
         String(perVals.dutyCycleA) + ':' +
         String(perVals.dutyCycleB) + ':' +
         String(perVals.dutyCycleC) + ':' +
         String(perVals.dutyCycleD) + ':'; // add terminating ':'
}

// perMax is 16 bit, all else are 8 bits
// sPerVals format 0:0:0:0:0:0:
// returns negative if error
// returns perVals by reference
int StringToPerVals(String sPerVals, PerVals& perVals){
  int vals[PERVALS_COUNT] = {0};
  int ii;
  for (ii=0; ii < PERVALS_COUNT; ii++){
    int idx = sPerVals.indexOf(':');
    if (idx < 1)
      return -2;
    String sParam = sPerVals.substring(0, idx);
    if (!alldigits(sParam))
      return -3;
    vals[ii] = sParam.toInt();
    sPerVals = sPerVals.substring(idx+1);
  }
  if (ii < PERVALS_COUNT)
    return -4;
  perVals.perMax = (uint16_t)vals[0];
  perVals.phaseB = (uint8_t)vals[1];
  perVals.phaseC = (uint8_t)vals[2];
  perVals.phaseD = (uint8_t)vals[3];
  perVals.perVal = (uint8_t)vals[4];
  perVals.perUnits = (uint8_t)vals[5];
  perVals.dutyCycleA = (uint8_t)vals[6];
  perVals.dutyCycleB = (uint8_t)vals[7];
  perVals.dutyCycleC = (uint8_t)vals[8];
  perVals.dutyCycleD = (uint8_t)vals[9];
  return 0;
}

// 32 chars in DROM sector
// set using PROJECT_VER
String GetEmbeddedVersionString(){
  const esp_app_desc_t *app_desc = esp_app_get_description();
  // char version[32], char date[16]
  if (app_desc == NULL)
    return "";
  return String(app_desc->version, 32);
}

// from index.html
//  <option value="0">1/2 sec</option>
//  <option value="1">sec</option>
//  <option value="2">min</option>
//  <option value="3">hrs</option>
String GetPerUnitsString(int perUnitsIndex){
  String sRet;
  switch(perUnitsIndex){
    case 0:
      sRet = "half-second";
    break;
    case 1:
      sRet = "one second";
    break;
    case 2:
      sRet = "one minute";
    break;
    case 3:
      sRet = "one hour";
    break;
    default:
      sRet = "Unknown!";
    break;
  };
  return sRet + " units";
}

String GetPhaseString(int phase){
  if (phase == 100)
    return "random";
  return String(phase);
}

String GetPerDCString(int iVal){
  if (iVal == 0)
    return "random";
  return String(iVal);
}

// return by-reference...
void twiddle(String& s){
  int len = s.length();
  if (len > 2){
    char s0, s1, s2;
    if (len & 1){
      s0 = 'e';
      s1 = 'm';
      s2 = 'R';
    }
    else{
      s0 = 'R';
      s1 = 'e';
      s2 = 'm';
    }
    char c1 = s[0];
    char c2 = s[1];
    if (c1 == 'c')
      s[0] = s0;
    else if (c1 == s0)
      s[0] = 'c';
    else if (c1 == 'C')
      s[0] = s1;
    else if (c1 == s1)
      s[0] = 'C';
    if (c2 == ' ')
      s[1] = s2;
    else if (c2 == s2)
      s[1] = ' ';
  }
}

uint32_t IpToUInt(IPAddress ip){
  return (uint32_t)ip;
}

uint32_t IpToUInt(String sIp){
  IPAddress ip;
  ip.fromString(sIp);
//  uint32_t acc = ip[3];
//  for (int ii=2; ii>=0; ii--){
//    acc <<= 8;
//    acc |= ip[ii];
//  }
//  return acc;

  return (uint32_t)ip;
}

String UIntToIp(uint32_t ip_uint){
  return IPAddress(ip_uint).toString();
}

bool isHex(char c){
  return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

String ZeroPad(byte val){
  String s = (val < 10) ? "0" : "";
  s += String(val);
  return s;
}

// copied this function from CodeProject's site...
// https://www.codeproject.com/Articles/35103/Convert-MAC-Address-String-into-Bytes
String MacArrayToString(uint8_t* pMacArray){
  String s;
  char cbuf[3];
  for(int ii = 0; ii < 6; ii++){
    sprintf(cbuf,"%02X", *pMacArray++);
    s += String(cbuf);
    if (ii < 5)
      s += ':';
  }
  return s;
}

uint8_t* MacStringToByteArray(const char *pMac, uint8_t* pBuf){
  char cSep = ':';

  for (int ii = 0; ii < 6; ii++){
    unsigned int iNumber = 0;

    //Convert letter into lower case.
    char ch = tolower(*pMac++);

    if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
      return NULL;

    //Convert into number.
    // a. If chareater is digit then ch - '0'
    // b. else (ch - 'a' + 10) it is done because addition of 10 takes correct value.
    iNumber = isdigit(ch) ? (ch - '0') : (ch - 'a' + 10);
    ch = tolower(*pMac);

    if ((ii < 5 && ch != cSep) || (ii == 5 && ch != '\0' && !isspace(ch))){
      pMac++;

      if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
        return NULL;

      iNumber <<= 4;
      iNumber += isdigit(ch) ? (ch - '0') : (ch - 'a' + 10);
      ch = *pMac;

      if (ii < 5 && ch != cSep)
        return NULL;
    }
    /* Store result.  */
    pBuf[ii] = (uint8_t)iNumber;
    /* Skip cSep.  */
    pMac++;
  }
  return pBuf;
}

String GetStringInfo(){
  //Static IP address configuration
  //IPAddress staticIP(192, 168, 43, 90); //ESP static ip
  //IPAddress gateway(192, 168, 43, 1);   //IP Address of your WiFi Router (Gateway)
  //IPAddress subnet(255, 255, 255, 0);  //Subnet mask
  //IPAddress dns(8, 8, 8, 8);  //DNS

  String sInfo = "Our hostname: \"" + g_sHostName + "\"\r\n";
  sInfo += "Current WiFi channel: " + String(GetWiFiChan()) + "\r\n";
  sInfo += "Our access-point SSID: " + g_sApSSID + ", IP: " + String(DEF_AP_IP) +
                        ", pass: \"" + PC.GetWiFiString(EE_APPWD, DEF_AP_PWD) + "\"\r\n";
  sInfo += "Expected router SSID: " + g_sSSID + ", pass: \"" + PC.GetWiFiString(EE_PWD, DEF_PWD) + "\"\r\n";
  if (g_bSoftAP){
    IPAddress apip = WiFi.softAPIP();
    IpToArray(apip[3]); // needed for IP last-octet flasher!
    sInfo += "Acces-point ON! Soft AP IP: " + apip.toString() + "\r\n";
  }
  if (g_bWiFiConnected){
    IPAddress loip = WiFi.localIP();
    IpToArray(loip[3]); // needed for IP last-octet flasher!
    sInfo += "Connected to router IP: " + loip.toString() + "\r\n";
    sInfo += "Router SSID: " + WiFi.SSID() + ", gateway IP: " + WiFi.gatewayIP().toString() + "\r\n";
    int iCount = IML.GetCount();
    if (iCount){
      sInfo += String(iCount+1) + " total units are networked. The master is " + g_IpMaster.toString() + "\r\n";
      if (g_bMaster)
        sInfo += " This unit is the master.";
      else
        sInfo += " This unit is not the master.";
    }
  }
  else
    sInfo += " is not presently connected to WiFi.";
  return sInfo;
}

int GetWiFiChan(){
  uint8_t chan;
  wifi_second_chan_t chan2;
  if (esp_wifi_get_channel(&chan, &chan2) == ESP_OK)
    return chan;
  return -1;
}

// Used to flash out the last octet of the IP address on the ESP32 built-in LED
// puts the last octet of an IP Address into a global array
// so that we can flash it out on the LED
// For example:
//   .8 => [0] = 8, [1] = 0
//   .32 => [0] = 2, [1] = 3, [2] = 0
//   .192 => [0] = 2, [1] = 9, [2] = 1, [3] = 0
void IpToArray(uint16_t ipLastOctet){
  if (ipLastOctet == 0){
    g8_wifiLedDigitArray[0] = 0;
    return;
  }

  uint16_t hundreds = ipLastOctet/100;
  ipLastOctet -= hundreds*100;
  uint16_t tens = ipLastOctet/10;
  ipLastOctet -= tens*10;
  g8_wifiLedDigitArray[0] = ipLastOctet; // 1s
  g8_wifiLedDigitArray[1] = tens;
  g8_wifiLedDigitArray[2] = hundreds;
  g8_wifiLedDigitArray[3] = 0; // end marker
}

String PrintCharsWithEscapes(String sIn){
  String sOut, sTemp;
  int len = sIn.length();
  for (int ii=0; ii < len; ii++){
    sTemp = String(sIn[ii], 16);
    if (sTemp.length() == 1)
      sTemp = '0' + sTemp;
    sOut += "\\x" + sTemp;
  }
  return sOut;
}

// convert \xff hex escape codes
// in a cipher key, all \ chars must be escaped as \\
// Examples "hello\x00" will return an error message since a null char isn't allowed in a string
//          "hello\\" will become "hello\"
//          "hello\\x00" will become "hello\x00"
//          "hello\\\x20" will become "hello\ "
String Unescape(String sIn){
  String sOut;
  int len = sIn.length();
  for (int ii=0; ii < len; ii++){
    if (ii <= len-2 && sIn[ii] == '\\' && sIn[ii+1] == '\\'){
      sOut += '\\';
      ii += 1;
    }
    else if (ii <= len-4 && sIn[ii] == '\\' && sIn[ii+1] == 'x' && isHexOrDigit(sIn[ii+2]) && isHexOrDigit(sIn[ii+3])){
      int n = hexCharToInt(sIn[ii+2])*16 + hexCharToInt(sIn[ii+3]);
      if (n <= 0) // error if \x00
        return "";
      if (n <= 255){
        sOut += char(n);
        ii += 3;
      }
      else
        sOut += sIn[ii];
    }
    else
      sOut += sIn[ii];
  }
  return sOut;
}

// returns 0-15 or -1 if not hex
int hexCharToInt(char c){
  if (c >= '0' && c <= '9')
    return c-'0'; 
  if (c >= 'a' && c <= 'f')
    return c-'a'+10; 
  if (c >= 'A' && c <= 'F')
    return c-'A'+10; 
  return -1;
}

bool isHexOrDigit(char c){
  return isHex(c) || isDigit(c);
}

//---------------------------------------------------------------------------
// Print routines
//---------------------------------------------------------------------------

void PrintPreferences(){
  PrintCycleTiming();
  prt(SyncFlagStatus());
  prt("SSR1 Mode: ");
  PrintSsrMode(g8_ssr1ModeFromWeb);
  prt("SSR2 Mode: ");
  PrintSsrMode(g8_ssr2ModeFromWeb);
  PrintMidiChan();
  prt("A: ");
  PrintMidiNote(g8_midiNoteA);
  prt("B: ");
  PrintMidiNote(g8_midiNoteB);
#if ENABLE_SSR_C_AND_D
  prt("C: ");
  PrintMidiNote(g8_midiNoteC);
  prt("D: ");
  PrintMidiNote(g8_midiNoteD);
#endif
  prtln("wifi disable flag: " + String(g_bWiFiDisabled));
  prtln("labelA: \"" + g_sLabelA + "\"");
  prtln("labelB: \"" + g_sLabelB + "\"");
#if ENABLE_SSR_C_AND_D
  prtln("labelC: \"" + g_sLabelC + "\"");
  prtln("labelD: \"" + g_sLabelD + "\"");
#endif
  prtln("cipher key: \"" + String(g_sKey) + "\"");
  prtln("token: " + String(g_defToken));
  prtln("max power (.25dBm per step): " + String(g8_maxPower));
  prtln("SNTP interval (hours): " + String(g16_SNTPinterval));
  prtln("SNTP Timezone: " + g_sTimezone);
}

void PrintCycleTiming(){
  prtln("period units: " + String(g_perVals.perUnits));
  prtln("max period: " + String(g_perVals.perMax));
  String sTemp = (g_perVals.perVal == 0) ? "random" : String(g_perVals.perVal) + "%";
  prtln("period: " + sTemp);
  prtln("period (.5 sec units): " + String(g32_savePeriod));
  sTemp = (g_perVals.dutyCycleA == 0) ? "random" : String(g_perVals.dutyCycleA) + "%";
  prtln("A duty-cycle: " + sTemp);
  sTemp = (g_perVals.dutyCycleB == 0) ? "random" : String(g_perVals.dutyCycleB) + "%";
  prtln("B duty-cycle: " + sTemp);
#if ENABLE_SSR_C_AND_D
  sTemp = (g_perVals.dutyCycleC == 0) ? "random" : String(g_perVals.dutyCycleC) + "%";
  prtln("C duty-cycle: " + sTemp);
  sTemp = (g_perVals.dutyCycleD == 0) ? "random" : String(g_perVals.dutyCycleD) + "%";
  prtln("D duty-cycle: " + sTemp);
#endif
  sTemp = (g_perVals.phaseB == 100) ? "random" : String(g_perVals.phaseB) + "%";
  prtln("phaseB: " + sTemp);
#if ENABLE_SSR_C_AND_D
  sTemp = (g_perVals.phaseC == 100) ? "random" : String(g_perVals.phaseC) + "%";
  prtln("phaseC: " + sTemp);
  sTemp = (g_perVals.phaseD == 100) ? "random" : String(g_perVals.phaseD) + "%";
  prtln("phaseD: " + sTemp);
#endif
}

void PrintPulseFeaturePreferences(){
  prtln("pulse-off mode A: " + String(g8_pulseModeA));
  prtln("pulse-off min width A: " + String(g8_pulseMinWidthA));
  prtln("pulse-off max width A: " + String(g8_pulseMaxWidthA));
  prtln("pulse-off min period A: " + String(g16_pulseMinPeriodA));
  prtln("pulse-off max period A: " + String(g16_pulseMaxPeriodA));
  
  prtln("pulse-off mode B: " + String(g8_pulseModeB));
  prtln("pulse-off min width B: " + String(g8_pulseMinWidthB));
  prtln("pulse-off max width B: " + String(g8_pulseMaxWidthB));
  prtln("pulse-off min period B: " + String(g16_pulseMinPeriodB));
  prtln("pulse-off max period B: " + String(g16_pulseMaxPeriodB));
  
#if ENABLE_SSR_C_AND_D
  prtln("pulse-off mode C: " + String(g8_pulseModeC));
  prtln("pulse-off min width C: " + String(g8_pulseMinWidthC));
  prtln("pulse-off max width C: " + String(g8_pulseMaxWidthC));
  prtln("pulse-off min period C: " + String(g16_pulseMinPeriodC));
  prtln("pulse-off max period C: " + String(g16_pulseMaxPeriodC));
  
  prtln("pulse-off mode D: " + String(g8_pulseModeD));
  prtln("pulse-off min width D: " + String(g8_pulseMinWidthD));
  prtln("pulse-off max width D: " + String(g8_pulseMaxWidthD));
  prtln("pulse-off min period D: " + String(g16_pulseMinPeriodD));
  prtln("pulse-off max period D: " + String(g16_pulseMaxPeriodD));
#endif
}

void PrintSsrMode(uint8_t ssrMode){
  String s;
  if (ssrMode == SSR_MODE_OFF)
    s = "OFF";
  else if (ssrMode == SSR_MODE_ON)
    s = "ON";
  else if (ssrMode == SSR_MODE_AUTO)
    s = "AUTO";
  else
    s = "(unknown)";
  prtln(s);
}

void PrintMidiNote(uint8_t note){
  String s;
  if (note == MIDINOTE_ALL)
    s = "ALL";
  else
    s = String(note);
  prtln("Midi note set to:" + s);
}

void PrintMidiChan(){
  String s;
  if (g8_midiChan == MIDICHAN_OFF)
    s = "OFF";
  else if (g8_midiChan == MIDICHAN_ALL)
    s = "ALL";
  else
    s = String(g8_midiChan);
  prtln("Midi channel set to:" + s);
}

// sFile: "/test.txt"
void PrintSpiffs(String sFile){
  File file = SPIFFS.open(sFile);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("File Content:");
  while(file.available())
    Serial.write(file.read());
  file.close();
}

// print wrappers
void prtln(String s){
#if PRINT_ON
  Serial.println(s);
#endif
}

void prt(String s){
#if PRINT_ON
  Serial.print(s);
#endif
}

//    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
//    Serial.println(encryptionTypeDescription);
//String translateEncryptionType(wifi_auth_mode_t encryptionType) {
//  switch (encryptionType) {
//    case (0):
//      return "Open";
//    case (1):
//      return "WEP";
//    case (2):
//      return "WPA_PSK";
//    case (3):
//      return "WPA2_PSK";
//    case (4):
//      return "WPA_WPA2_PSK";
//    case (5):
//      return "WPA2_ENTERPRISE";
//    default:
//      return "UNKOWN";
//  }
//}

// decode unicode hex: /u00e1, etc
//String convertUnicode(String unicodeStr){
//  String out = "";
//  int len = unicodeStr.length();
//  char iChar;
//  char* endPtr; // will point to next char after numeric code
//  for (int i = 0; i < len; i++){
//     iChar = unicodeStr[i];
//     if(iChar == '\\'){ // got escape char
//       iChar = unicodeStr[++i];
//       if(iChar == 'u'){ // got unicode hex
//         char unicode[6];
//         unicode[0] = '0';
//         unicode[1] = 'x';
//         for (int j = 0; j < 4; j++){
//           iChar = unicodeStr[++i];
//           unicode[j + 2] = iChar;
//         }
//         long unicodeVal = strtol(unicode, &endPtr, 16); //convert the string
//         out += (char)unicodeVal;
//       } else if(iChar == '/'){
//         out += iChar;
//       } else if(iChar == 'n'){
//         out += '\n';
//       }
//     } else {
//       out += iChar;
//     }
//  }
//  return out;
//}

// ensconses sIn "somewhere" within a random message
//String genRandPositioning(String sIn, int iMin, int iMax){
//  String sRand = genRandMessage(iMin, iMax);
//  int iLen = sRand.length();
//  if (iLen == 0)
//    return sIn;
//  int iIdx = random(0, iLen+1);
//  if (iIdx == iLen)
//    return sIn + sRand;
//  if (iIdx == 0)
//    return sRand + sIn;
//  String sSub1, sSub2; 
//  sSub1 = sRand.substring(0, iIdx); 
//  sSub2 = sRand.substring(iIdx+1);
//  return sSub1 + sIn + sSub2;
//}

// add a three-digit base-10 checksum (000-255)
//String AddThreeDigitBase10Checksum(String sIn){
//  uint8_t cs = 0;
//  int len = sIn.length();
//  for (int i=0; i<len; i++)
//    cs += (uint8_t)sIn[i];
//  cs = ~cs+1;
//  String sCs = String(cs, 10);
//  len = sCs.length();
//  if (len == 1)
//    sCs = "00" + sCs;
//  else if (len == 2)  
//    sCs = "0" + sCs;
//  return sIn + sCs;
//}

// removes and checks a uint8_t 0-255 checksum expected at the end of the
// string as three ASCII base-10 digits 000-255
// returns empty string if bad checksum
//String SubtractThreeDigitBase10Checksum(String sIn){
//  uint8_t cs = 0;
//  int len = sIn.length();
//  if (len < 4)
//    return "";
//  String sCs = sIn.substring(len-3);
//  
//  int iCs = sCs.toInt();
//  if (iCs < 0 || iCs > 255)
//    return "";
//  sIn = sIn.substring(0, len-3);
//  len = sIn.length();
//  uint8_t uCs = 0;
//  for (int i=0; i<len; i++)
//    uCs += (uint8_t)sIn[i];
//  uCs += (uint8_t)iCs;
//  if (uCs){
//    prtln("GpcUtils.cpp SubtractThreeDigitChecksum(): bad checksum!");
//    return "";
//  }
//
//  return sIn;
//}
