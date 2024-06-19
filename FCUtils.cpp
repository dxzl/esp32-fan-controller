// this file FCUtils.cpp
#include "FanController.h"

// may want to do this on program init!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//std::ios_base::sync_with_stdio(false);
//wcout.imbue(locale("en_US.UTF-8"));
//locale("en_US.UTF-8");

bool isHex(char c)
{
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
      s += ":";
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

String GetStringIP(){
  //Static IP address configuration
  //IPAddress staticIP(192, 168, 43, 90); //ESP static ip
  //IPAddress gateway(192, 168, 43, 1);   //IP Address of your WiFi Router (Gateway)
  //IPAddress subnet(255, 255, 255, 0);  //Subnet mask
  //IPAddress dns(8, 8, 8, 8);  //DNS

  IPAddress apip = WiFi.softAPIP();
  String sInfo = "Host \"" + g_sHostName + "\" (" + apip.toString() + ") ";
  if (g_bSoftAP){
    IpToArray(apip[3]); // needed for IP last-octet flasher!
    sInfo += "on channel " + String(GetWiFiChan()) + " is an access point.";
  }
  else if (g_bWiFiConnected){
    IPAddress loip = WiFi.localIP();
    IpToArray(loip[3]); // needed for IP last-octet flasher!
    sInfo += "on channel " + String(GetWiFiChan()) + " connected to router " + WiFi.SSID() + " at " + loip.toString();
  }
  else
    sInfo += "is not presently connected to WiFi.";
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
    g8_digitArray[0] = 0;
    return;
  }

  uint16_t hundreds = ipLastOctet/100;
  ipLastOctet -= hundreds*100;
  uint16_t tens = ipLastOctet/10;
  ipLastOctet -= tens*10;
  g8_digitArray[0] = ipLastOctet; // 1s
  g8_digitArray[1] = tens;
  g8_digitArray[2] = hundreds;
  g8_digitArray[3] = 0; // end marker
}

void ReadPot1(){
  g16_pot1Value = analogRead(GPAIN_POT1);
  int deltaPotValue = g16_pot1Value - g16_oldpot1Value;
  if (deltaPotValue < 0)
    deltaPotValue = -deltaPotValue;

  if (deltaPotValue > POT_DEBOUNCE){
    //double voltage = (3.3/4095.0) * g16_pot1Value;
    //prt("g16_pot1Value:");
    //prt(g16_pot1Value);
    //prt(" Voltage:");
    //prt(voltage);
    //prtln("V");

    float percent = (float)g16_pot1Value*100.0/4095.0;

    if (g8_potModeFromSwitch == 0){
      // period range: 0 - 100% (of perMax)
      QueueTask(TASK_PARMS, SUBTASK_PERVAL, (int)percent);
//prtln("DEBUG: ReadPot1(): perval changed!: " + String((int)percent));
    }
    else if (g8_potModeFromSwitch == 1){
      // phase range: 0 - 100%
      QueueTask(TASK_PARMS, SUBTASK_PHASE, (int)percent);
//prtln("DEBUG: ReadPot1(): phase changed!: " + String((int)percent));
    }

    g16_oldpot1Value = g16_pot1Value;
  }
}

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

void ReadSpdtSwitches(){
#if ESP32_S3
  bool sw1On = digitalRead(GPIN_POT_MODE_SW1);
  bool sw2On = digitalRead(GPIN_POT_MODE_SW2);
  if (sw1On != g_bOldPotModeSw1On || sw2On != g_bOldPotModeSw2On){
    if (sw1On){
      g8_potModeFromSwitch = 1;
      prtln("Pot Mode Switch: 1");
    }
    else if (sw2On){
      g8_potModeFromSwitch = 2;
      prtln("Pot Mode Switch: 2");
    }
    else{
      g8_potModeFromSwitch = 0;
      prtln("Pot Mode Switch: OFF");
    }

    g_bOldPotModeSw1On = sw1On;
    g_bOldPotModeSw2On = sw2On;
  }

  bool swManOn = digitalRead(GPIN_SSR1_MODE_SW_MAN);
  bool swAutOn = digitalRead(GPIN_SSR1_MODE_SW_AUT);
  if (swManOn != g_bOldSsr1ModeManSwOn || swAutOn != g_bOldSsr1ModeAutSwOn){
    if (swManOn){
      g8_ssr1ModeFromSwitch = SSR_MODE_ON;
      prtln("SSR1 Mode Switch: ON");
    }
    else if (swAutOn){
      g8_ssr1ModeFromSwitch = SSR_MODE_AUTO;
      prtln("SSR1 Mode Switch: AUTO");
    }
    else{
      g8_ssr1ModeFromSwitch = SSR_MODE_OFF;
      prtln("SSR1 Mode Switch: OFF");
    }

    g_bOldSsr1ModeManSwOn = swManOn;
    g_bOldSsr1ModeAutSwOn = swAutOn;
    SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
  }

  swManOn = digitalRead(GPIN_SSR2_MODE_SW_MAN);
  swAutOn = digitalRead(GPIN_SSR2_MODE_SW_AUT);
  if (swManOn != g_bOldSsr2ModeManSwOn || swAutOn != g_bOldSsr2ModeAutSwOn){
    if (swManOn){
      g8_ssr2ModeFromSwitch = SSR_MODE_ON;
      prtln("SSR2 Mode Switch: ON");
    }
    else if (swAutOn){
      g8_ssr2ModeFromSwitch = SSR_MODE_AUTO;
      prtln("SSR2 Mode Switch: AUTO");
    }
    else{
      g8_ssr2ModeFromSwitch = SSR_MODE_OFF;
      prtln("SSR2 Mode Switch: OFF");
    }

    g_bOldSsr2ModeManSwOn = swManOn;
    g_bOldSsr2ModeAutSwOn = swAutOn;
    SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
  }

#else  
  bool bPotModeSwOn = (digitalRead(GPIN_POT_MODE_SW) == HIGH) ? true : false;

  if (bPotModeSwOn != g_bOldPotModeSwOn){
    if (bPotModeSwOn){
      g8_potModeFromSwitch = 1;
      prtln("Potentiometer Mode Switch: ON");
    } else {
      g8_potModeFromSwitch = 0;
      prtln("Potentiometer Mode Switch: OFF");
    }
    g_bOldPotModeSwOn = bPotModeSwOn;
  }
#endif  
}

// Usage: SetSSRMode(GPOUT_SSR2, SSR_MODE_ON) sets g_bSsr2On true 
//        SetSSRMode(GPOUT_SSR2, SSR_MODE_AUTO) sets g_bSsr2On false 
//        SetSSRMode(GPOUT_SSR2, SSR_MODE_OFF) sets g_bSsr2On false 
void SetSSRMode(uint8_t gpout, uint8_t ssrMode){
  if (ssrMode == SSR_MODE_ON)
    SetSSR(gpout, true);
  else
    SetSSR(gpout, false);
}

// sets global g_bSsr1On to bSsrOn's value if val is GPIO32_SSR_1
// sets global g_bSsr2On to bSsrOn's value if val is GPIO23_SSR_2
void SetSSR(uint8_t gpout, bool bSetSsrOn){
  bool bIsOn = (digitalRead(gpout) == HIGH) ? true : false;

#if ESP32_S3
  // for S3 board, manual SPDT switch mode overrides all else...
  if (gpout == GPOUT_SSR1){
    if (bIsOn && g8_ssr1ModeFromSwitch == SSR_MODE_OFF)
      bSetSsrOn = false;
    else if (!bIsOn && g8_ssr1ModeFromSwitch == SSR_MODE_ON)
      bSetSsrOn = true;
  }
  else if (gpout == GPOUT_SSR2){
    if (bIsOn && g8_ssr2ModeFromSwitch == SSR_MODE_OFF)
      bSetSsrOn = false;
    else if (!bIsOn && g8_ssr2ModeFromSwitch == SSR_MODE_ON)
      bSetSsrOn = true;
  }
#endif  

  if (bSetSsrOn){
    if (!bIsOn)
      digitalWrite(gpout, HIGH);
    if (gpout == GPOUT_SSR1){
      g_bSsr1On = true;
      g_stats.AOnCounter++;
#if ESP32_S3
      digitalWrite(GPOUT_SSR1_LED, HIGH);
#endif
    }
    else if (gpout == GPOUT_SSR2){
      g_bSsr2On = true;
      g_stats.BOnCounter++;
#if ESP32_S3
      digitalWrite(GPOUT_SSR2_LED, HIGH);
#endif
    }
  }
  else if (bIsOn){ // Set to OFF or AUTO
    digitalWrite(gpout, LOW);
    if (gpout == GPOUT_SSR1){
      g_bSsr1On = false;
#if ESP32_S3
      digitalWrite(GPOUT_SSR1_LED, LOW);
#endif
    }
    else if (gpout == GPOUT_SSR2){
      g_bSsr2On = false;
#if ESP32_S3
      digitalWrite(GPOUT_SSR2_LED, LOW);
#endif
    }
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

// returns NULL if fail.
// pass in a pointer to a time_t version we export
// or set it NULL if not needed!
// set pTm to struct tm pointer we export
struct tm* ReadInternalTime(time_t* pEpochSeconds, struct tm* pTm){
  struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };

  // fill tv.tv_sec
  if (gettimeofday(&tv, NULL) < 0) // -1 if fail
    return NULL;

  // these return the same - as expected!
  //prtln("time(0)=" + String(time(0)) + ", epochSeconds:" + String(tv.tv_sec));

  // passing in time_t epoch-seconds - getting out time broken into a struct tm
  if (localtime_r(&tv.tv_sec, pTm) == NULL)
    return NULL;

  // export as a time_t if user has supplied a nonzero pointer
  if (pEpochSeconds)
    *pEpochSeconds = tv.tv_sec; //FYI: suseconds_t us = tv.tv_usec;

  mktime(pTm); // set day of week (used in repeat mode)

  return pTm; // return NULL if failure
}

// set internal time to a known "time is not set!" state
// sets year to DEF_YEAR which may be used to indicate "time is unset"
// only call this once - on reset
// return true if success
bool InitTimeManually(){
  prtln("Initializing clock to DEF_YEAR: " + String(DEF_YEAR));

  g_bManualTimeWasSet = false; // we are setting the "not set" time - so disable time-events!
  g_bWiFiTimeWasSet = false;
  g_bRequestManualTimeSync = false;
  g_bRequestWiFiTimeSync = false;

  struct tm timeInfo = {0}; // https://www.cplusplus.com/reference/ctime/tm/

  // set time manually (to an old date so we won't delete any time-slots automatically)
  timeInfo.tm_year = DEF_YEAR - EPOCH_YEAR;
  timeInfo.tm_mon = 0; // 0-11
  timeInfo.tm_mday = 1; // 1-31
  timeInfo.tm_hour = 0;
  timeInfo.tm_min = 0;
  timeInfo.tm_sec = 0;
  timeInfo.tm_isdst = -1; // auto-get DST 0 = DST off, 1 = DST on + 1 hour

  time_t epochSeconds = mktime(&timeInfo); // get timestamp - # seconds since midnight 1-1-1970 (sets day of week also)

  struct timeval tv = { .tv_sec = epochSeconds, .tv_usec = 0  };
  if (settimeofday(&tv, NULL) < 0) // returns -1 if fail
    return false;
  return true;
}

// returns time and date as: 2020-11-31T04:32:00pm
String TimeToString(){

  String sRet;

  // read internal time
  struct tm timeInfo = {0};
  if (ReadInternalTime(NULL, &timeInfo) != NULL) // get current time as struct tm
  {
    bool bPmFlag; // by ref
    int hr12 = Make12Hour(timeInfo.tm_hour, bPmFlag);
    String sPm = bPmFlag ? "pm" : "am";
    sRet = String(timeInfo.tm_year+EPOCH_YEAR) + "-" +
      ZeroPad(timeInfo.tm_mon+1) + "-" + ZeroPad(timeInfo.tm_mday) + "T" +
      String(hr12) + ":" + ZeroPad(timeInfo.tm_min) + ":" + ZeroPad(timeInfo.tm_sec) + sPm;
  }
  return sRet;
}

bool SetTimeManually(int myYear, int myMonth, int myDay, int myHour, int myMinute, int mySecond){
  if (g8_clockSetDebounceTimer != 0)
    return true; // don't return error if just a "bouncy" web-request!

  g8_clockSetDebounceTimer = MANUAL_CLOCK_SET_DEBOUNCE_TIME;

  if (myYear <= DEF_YEAR){
    prtln("Warning! Years less than or equal to DEF_YEAR not allowed: " + String(DEF_YEAR));
    return false;
  }

  if (myYear > MAX_YEAR)
    prtln("Warning! Year being set is over Y2038 MAX_YEAR: " + String(MAX_YEAR));

  struct tm timeInfo = {0}; // https://www.cplusplus.com/reference/ctime/tm/
  timeInfo.tm_year = myYear - EPOCH_YEAR;
  timeInfo.tm_mon = myMonth-1;
  timeInfo.tm_mday = myDay;
  timeInfo.tm_hour = myHour;
  timeInfo.tm_min = myMinute;
  timeInfo.tm_sec = mySecond;
  timeInfo.tm_isdst = -1; // auto-get DST 0 = DST off, 1 = DST on + 1 hour

  time_t epochSeconds = mktime(&timeInfo); // get timestamp - # seconds since midnight 1-1-1970 (sets day of week also)

  struct timeval tv = { .tv_sec = epochSeconds, .tv_usec = 0  };
  if (settimeofday(&tv, NULL) == 0){ // returns -1 if fail
    prtln("Requesting manual time-sync...");
    g_bRequestManualTimeSync = true;
    return true;
  }

  prtln("Failed to set internal clock...");

  // this is used in ProcessTimeSlot() to facilitate
  // the repeat modes (see p2.html in the data folder)
  // I.E. every hour, day, Etc. following a time-event
  // trigger.
  g_prevDateTime = {0};
  g_bManualTimeWasSet = false; // stop processing events on manual time...
  return false;
}

// takes hour in 24 hour and returns 12-hour
// and pmFlag by reference
int Make12Hour(int iHour, bool &pmFlag){
  pmFlag = (iHour >= 12) ? true : false;
  iHour %= 12; // 0-11 we get 0-11, for 12-23 we get 0-11
  if (iHour == 0) iHour = 12; // the hour '0' should be '12'
  return iHour;
}

// eastern  = -5
// central  = -6 (me)
// mountain = -7
// pacific  = -8
// so -6 * 60 * 60 = -6 * 3600 = -21600ms
//const long  gmtOffset_sec = -21600;
//const int   daylightOffset_sec = +3600; // offset +3600 if DST (daylight savings time) (spring forward)

// Before calling printLocalTime() call configTime!
void printLocalTime(struct tm &timeInfo){
  // timeinfo members http://www.cplusplus.com/reference/ctime/tm/
  //tm_sec  int seconds after the minute 0-61 (tm_sec is generally 0-59. The extra range is to accommodate for leap seconds in certain systems.)
  //tm_min  int minutes after the hour 0-59
  //tm_hour int hours since midnight 0-23
  //tm_mday int day of the month 1-31
  //tm_mon  int months since January 0-11
  //tm_year int years since 1900
  //tm_wday int days since Sunday 0-6
  //tm_yday int days since January 1, 0-365
  //tm_isdst  int Daylight Saving Time flag
  //The Daylight Saving Time flag (tm_isdst) is greater than zero if Daylight Saving Time is in effect,
  //zero if Daylight Saving Time is not in effect, and less than zero if the information is not available.

  //%A  returns day of week
  //%B  returns month of year
  //%d  returns day of month
  //%Y  returns year
  //%H  returns hour
  //%M  returns minutes
  //%S  returns seconds
#if PRINT_ON
  Serial.println(&timeInfo, "%A, %B %d %Y %H:%M:%S");
#endif

  //time_t lastSync = NTP.getLastSync();
  //char buff[20];
  //strftime(buff, 20, "%H:%M:%S - %d-%m-%Y ", localtime(&lastSync));

}

void PrintPreferences(){
  prtln("period units: " + String(g_perVals.perUnits));
  prtln("max period: " + String(g_perVals.perMax));
  String sTemp = (g_perVals.perVal == 0) ? "random" : String(g_perVals.perVal) + "%";
  prtln("period: " + sTemp);
  prtln("period (.5 sec units): " + String(g32_savePeriod));
  prtln("A duty-cycle:" + String(g_perVals.dutyCycleA) + "%");
  prtln("B duty-cycle: " + String(g_perVals.dutyCycleB) + "%");
  prtln("phase: " + String(g_perVals.phase) + "%");
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
  prtln("wifi disable flag: " + String(g_bWiFiDisabled));
  prtln("labelA: \"" + g_sLabelA + "\"");
  prtln("labelB: \"" + g_sLabelB + "\"");
  prtln("cipher key: \"" + String(g_sKey) + "\"");
  prtln("token: " + String(g_defToken));
  prtln("max power (.25dBm per step): " + String(g8_maxPower));
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

void ResetPeriod()
{
  g32_dutyCycleTimerA = 0;
  g32_dutyCycleTimerB = 0;
  g32_phaseTimer = 0;
  g32_periodTimer = 1; // restart cycle...
  g32_savePeriod = 1;
}

void LimitPeriod(){
  // g32_dutyCycleTimerA and g32_phaseTimer will reset with new period but
  // g32_dutyCycleTimerB runs at end of g32_phaseTimer expiring...
  if (g32_periodTimer > MIN_PERIOD_TIMER){
    g32_periodTimer = MIN_PERIOD_TIMER;
    g32_savePeriod = MIN_PERIOD_TIMER;
  }
  if (g32_dutyCycleTimerB > MIN_PERIOD_TIMER)
    g32_dutyCycleTimerB = MIN_PERIOD_TIMER;
}

//#include <WiFi.h>
//#include "esp_wifi.h"

// not presently used - this would allow us to get ip-address and MAC
// of networks connected to us when we are an access-point.
//void WiFiApInfo(){
//
//  //WiFi.softAP("MyESP32AP");
//
//  //uint8_t bssid[6] MAC address of AP
//  //uint8_t ssid[33] SSID of AP (S.S. - guess this is in UTF-8 - maybe a 16 unicode char limit?)
//  //uint8_t primary channel of AP
//  //wifi_second_chan_tsecond secondary channel of AP
//  //int8_t rssi signal strength of AP
//  //wifi_auth_mode_tauthmode authmode of AP
//  //wifi_cipher_type_tpairwise_cipher pairwise cipher of AP
//  //wifi_cipher_type_tgroup_cipher group cipher of AP
//  //wifi_ant_tant antenna used to receive beacon from AP
//  //uint32_t phy_11b : 1 bit: 0 flag to identify if 11b mode is enabled or not
//  //uint32_t phy_11g : 1 bit: 1 flag to identify if 11g mode is enabled or not
//  //uint32_t phy_11n : 1 bit: 2 flag to identify if 11n mode is enabled or not
//  //uint32_t phy_lr : 1 bit: 3 flag to identify if low rate is enabled or not
//  //uint32_t wps : 1 bit: 4 flag to identify if WPS is supported or not
//  //uint32_t reserved : 27 bit: 5..31 reserved
//  //wifi_country_tcountry country information of AP
//  wifi_sta_list_t wifi_sta_list;
//
//  tcpip_adapter_sta_list_t adapter_sta_list;
//
//  memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
//  memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));
//
//  esp_wifi_ap_get_sta_list(&wifi_sta_list); // Get information of AP which the ESP32 station is associated with
//  tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);
//
//  for (int i = 0; i < adapter_sta_list.num; i++){
//
//    tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
//
//    prtln("station " + i);
//    prt("MAC: ");
//
//    for(int j = 0; j< 6; j++){
//      Serial.printf("%02X", station.mac[j]);
//      if(j<5)Serial.print(":");
//    }
//
//    prtln("\nIP: ");
//    Serial.print(ip4addr_ntoa(&station.ip));
//  }
//}

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

// start process of sending out new "pending" default token to all
// remote units. As the response "TOKOK" comes back, we set the bTokOk flag for each mDNS IP.
// When all have been set, we set the new token for ourself (in flash)
void StartNewRandomToken(){
  int newToken = g_defToken;
  while (newToken == g_defToken)
    newToken = random(0, MAX_TOKEN+1); // generate new random base 64 default token for currently linked ESP32s
  if (HMC.AddTableCommandAll(CMtoken, newToken) > 0){
    IML.ClearAllTokOkFlags(); // cancel any preceding token-change operation...
    g_pendingDefToken = newToken;
  }
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

void RefreshSct(){
  g_sct = GetSct(g_minSct, g_maxSct); // return g_minSct and g_maxSct by reference
}

int GetSct(int &minSct, int &maxSct)
{
  minSct = MIN_SHIFT_COUNT;
  maxSct = random(MAX_SHIFT_COUNT/4, MAX_SHIFT_COUNT);
  return random(minSct, maxSct);
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

// Channel 1 (A) switches off when g32_dutyCycleTimerA decrements to 0.
// It switches on when g32_periodTimer decrements to 0
int ComputeTimeToOnOrOffA(){
  if (g8_ssr1ModeFromWeb != SSR_MODE_AUTO)
    return -1;
  if (g_bSsr1On)
    return g32_dutyCycleTimerA;
  return g32_periodTimer;
}

// Channel 2 (B) switches off when g32_dutyCycleTimerB decrements to 0.
// It switches on when g32_phaseTimer decrements to 0 if g32_phaseTimer is running, or
// at g32_nextPhase + g32_periodTimer if it's not...
int ComputeTimeToOnOrOffB(){
  if (g8_ssr2ModeFromWeb != SSR_MODE_AUTO)
    return -1;
  if (g_bSsr2On)
    return g32_dutyCycleTimerB;
  if (g32_phaseTimer)
    return g32_phaseTimer;
  return g32_periodTimer+g32_nextPhase;
}

void CheckMasterStatus(){
  bool bIsMaster = IML.AreWeMaster();
  // have to check this every time because a 0 MAC address may now be populated...
  if (bIsMaster && !g_bSyncMaster){
    prtln("This IP is a master...");
    g_bSyncMaster = true;
  }
  else if (!bIsMaster && g_bSyncMaster){
    prtln("This IP is NOT a master...");
    g_bSyncMaster = false;
    if (g16_sendDefTokenTimer)
      g16_sendDefTokenTimer = 0;
  }
}

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
  sOnOff = g_bSyncToken ? ON:OFF;
  sOut += "token sync to remote: " + sOnOff;
  sOnOff = g_bSyncTime ? ON:OFF;
  sOut += "time sync to remote: " + sOnOff;
  sOnOff = g_bSyncEncrypt ? ON:OFF;
  sOut += "HTTP encrypt: " + sOnOff;
  return sOut;
}

void FlashSequencerStop(){
  FlashSequencerInit(g8_ledMode_OFF);
  g8_ledSeqState == LEDSEQ_ENDED;
}

// NOTE: g8_digitArray must have a 0 terminator to mark the end of sequence!
// bStart defaults false
void FlashSequencerInit(uint8_t postFlashMode){
  g8_ledFlashCounter = 0;
  g8_ledDigitCounter = 0;
  g8_ledSaveMode = postFlashMode; // save post-flash mode...
  g8_ledMode = g8_ledMode_PAUSED;
  g8_ledSeqState = LEDSEQ_PAUSED;
#if ESP32_S3
  neopixelWrite(RGB_BUILTIN, 0, 0, 0);  // Off
#else
  digitalWrite(GPOUT_ONBOARD_LED, LOW);
#endif
  g_bLedOn = false;
}

void FlashSequencer(){
  if (g8_ledSeqState == LEDSEQ_PAUSED){
    if (g8_ledFlashCounter >= LED_PAUSE_COUNT){
      g8_ledFlashCounter = 0;
      uint8_t digitCount = g8_digitArray[g8_ledDigitCounter++];
      if (digitCount == 0){ // end of digits in sequence...
        g8_ledMode = g8_ledSaveMode; // return to "connected" flashing
        g8_ledSeqState = LEDSEQ_ENDED;
      }
      else{
        g8_ledFlashCount = digitCount;
        g8_ledSeqState = LEDSEQ_FLASHING;
        g8_ledMode = g8_ledMode_FASTFLASH;
      }
    }
  }
  else if (g8_ledSeqState == LEDSEQ_FLASHING){
    if (g8_ledFlashCounter >= g8_ledFlashCount){
      g8_ledFlashCounter = 0;
      g8_ledMode = g8_ledMode_PAUSED;
      g8_ledSeqState = LEDSEQ_PAUSED;
    }
  }
}

void FlashLED(){
  if (g8_ledMode == g8_ledMode_PAUSED)
    g8_ledFlashCounter++; // count 1/4 sec pause interval
  else if (g8_ledMode == g8_ledMode_OFF){
    if (!g_bLedOn){
#if ESP32_S3
        neopixelWrite(RGB_BUILTIN, 0, 0, 0);  // Off
#else
        digitalWrite(GPOUT_ONBOARD_LED, LOW);
#endif
      g_bLedOn = true;
    }
    if (g8_ledFlashTimer)
      g8_ledFlashTimer = 0;
  }
  else if (g8_ledMode == g8_ledMode_ON){
    if (!g_bLedOn){
#if ESP32_S3
      neopixelWrite(RGB_BUILTIN, 0, LED_GREEN, 0);  // On
#else
      digitalWrite(GPOUT_ONBOARD_LED, HIGH);
#endif
      g_bLedOn = true;
    }
    if (g8_ledFlashTimer)
      g8_ledFlashTimer = 0;
  }
  else if (g8_ledFlashTimer){
    if (--g8_ledFlashTimer == 0){
      if (!g_bLedOn){
#if ESP32_S3
        neopixelWrite(RGB_BUILTIN, 0, LED_GREEN, 0);  // On
#else
        digitalWrite(GPOUT_ONBOARD_LED, HIGH);
#endif
        g_bLedOn = true;
      }
      else{
#if ESP32_S3
        neopixelWrite(RGB_BUILTIN, 0, 0, 0);  // Off
#else
        digitalWrite(GPOUT_ONBOARD_LED, LOW);
#endif
        g_bLedOn = false;
        g8_ledFlashCounter++;
      }
      
      if (g8_ledMode == g8_ledMode_SLOWFLASH)
        g8_ledFlashTimer = LED_SLOWFLASH_TIME;
      else if (g8_ledMode == g8_ledMode_FASTFLASH)
        g8_ledFlashTimer = LED_FASTFLASH_TIME;
    }
  }
  else // start off either slow or fast flash
    g8_ledFlashTimer = 1;
}

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

// generate variable length random message
String genRandMessage(int iMin, int iMax){
  int N = random(iMin, iMax); // message 3-30 chars
  if (N == 0)
    return "";
  int it = 3*N;
  char arr[it+1];
  for (int ii=0; ii < N; ii++){
     int mul = ii*3;
     arr[mul+0] = random('a', 'z');
     arr[mul+1] = random('A', 'Z');
     arr[mul+2] = random('0', '9');
  }
  arr[it] = '\0';
  return String(arr);
}

// positive numbers only (need to test...)
String MyEncodeNum(unsigned int uiIn, int table, int token, int context){
  String s;
  if (g_bSyncEncrypt){
    s = B64C.hnEncNumOnly(uiIn, table, token);
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
    s = B64C.hnEncNum(uiIn, table, token);

  return s;
}

// positive numbers only (need to test...)
// returns negative if error
int MyDecodeNum(String s, int table, int token, int context){
  if (s.isEmpty())
    return -2;
  int iOut;
  if (g_bSyncEncrypt){
//prtln("MyDecodeNum(): 0: \"" + s + "\"");
    s = B64C.hnDecodeStr(s, table, token);
//prtln("MyDecodeNum(): 1: \"" + s + "\"");
    if (s.isEmpty())
      return -3;
    s = CIP.decryptString(s, token, context);
//prtln("MyDecodeNum(): 2: \"" + s + "\"");
    if (s.isEmpty())
      return -4;
    s = SubtractTwoDigitBase16Checksum(s);
//prtln("MyDecodeNum(): 3: \"" + s + "\"");
    if (s.isEmpty())
      return -5;
    iOut = B64C.hnDecNumOnly(s, table, token);
//prtln("MyDecodeNum(): 4: \"" + String(iOut) + "\"");
  }
  else
    iOut = B64C.hnDecNum(s, table, token);
  return iOut;
}

String MyEncodeStr(String s, int table, int token, int context){
  if (s.isEmpty())
    return "";
  if (g_bSyncEncrypt){
    s = AddTwoDigitBase16Checksum(s);
    if (s.isEmpty())
      return "";
    s = CIP.encryptString(s, token, context);
    if (s.isEmpty())
      return "";
    s = B64C.hnEncodeStr(s, table, token);
  }
  else
    s = B64C.hnShiftEncode(s, table, token);
  return s;
}

String MyDecodeStr(String s, int table, int token, int context){
  if (s.isEmpty())
    return "";
  if (g_bSyncEncrypt){
    s = B64C.hnDecodeStr(s, table, token);
    if (s.isEmpty())
      return "";
    s = CIP.decryptString(s, token, context);
    if (s.isEmpty())
      return "";
    s = SubtractTwoDigitBase16Checksum(s);
  }
  else
    s = B64C.hnShiftDecode(s, table, token);
  return s;
}

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
//    prtln("FCUtils.cpp SubtractThreeDigitChecksum(): bad checksum!");
//    return "";
//  }
//
//  return sIn;
//}

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
  if (uCs){
    prtln("FCUtils.cpp SubtractTwoDigitBase16Checksum(): bad checksum!");
    return "";
  }

  return sIn;
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

// Pertains to the FanController.h conditional-compile boolean switches:
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
    g_stats.PrevDConA = g_stats.DConA;
    g_stats.PrevDConB = g_stats.DConB;
    ClearStatCounters();
  }

  // we simply count time in .5 sec units when a channel is on
  // these get copied to g_stats.PrevDConA/B when monitor interval
  // rolls over
  if (g_bSsr1On)
    g_stats.DConA++;
  if (g_bSsr2On)
    g_stats.DConB++;
}

void TaskProcessPulseOffFeatureTiming(){
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
}

void TaskSetPulseOffFeatureVars(){
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
}

// returns mDNS count or negative if error
int SendText(String sIP, String sText){
  if (sIP.isEmpty())
    return -2;  
  IPAddress ip;
  ip.fromString(sIP);
  int idx = IML.FindMdnsIp(ip);
  return SendText(idx, sText);
}
// returns mDNS count or negative if error
int SendText(String sText){
  if (!g_bWiFiConnected)
    return -2;  

  if (sText.isEmpty() || sText.length() > MAXTXTLEN)
    return -3;  

  return HMC.AddTableCommandAll(CMtxt, sText);
}
// returns mDNS count or negative if error
int SendText(int idx, String sText){
  if (!g_bWiFiConnected)
    return -2;  

  if (sText.isEmpty() || sText.length() > MAXTXTLEN)
    return -3;  

  int count = IML.GetCount();
  
  if (!count || idx < 0 || idx >= count)
    return -4;

  HMC.AddTableCommand(CMtxt, sText, idx);
prtln("DEBUG: SendText(): AddTableCommand(): \"" + sText + "\"");
  return count;
}
