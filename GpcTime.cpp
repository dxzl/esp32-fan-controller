// this file GpcTime.cpp
#include "Gpc.h"

// The ESP32 has 8kB SRAM on the Real-TIme-Clock module called "RTC fast memory". The data saved here is not erased
// during deep sleep. However, it is erased when you press the reset button (the button labeled EN on the ESP32 board).
// To save data in the RTC memory, you just have to add RTC_DATA_ATTR before a variable definition (NOTE: not working 9/23/2024).
// The example saves the bootCount variable on the RTC memory. This variable will count how many times
// the ESP32 has woken up from deep sleep.

const char NTP_SERVER1[] = SNTP_SERVER1; // you can have more than one URL, comma delimited!
const char NTP_SERVER2[] = SNTP_SERVER2;

// set internal time to a known "time is not set!" state
// sets year to DEF_YEAR which may be used to indicate "time is unset"
// only call this once - on reset
// return true if success
bool InitTime(){
  
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

void InitOrRestartSNTP(){
  if (sntp_restart())
    prtln("SNTP restarted!");
  else{
    SetSntpSyncInterval();
    SetSntpTimezoneAndStart();
  }
}

// iInterval is in hours
bool SetGlobalSntpInterval(int iInterval){
  if (iInterval < 0)
    return false;
    
  if (g16_SNTPinterval == 0 && iInterval != 0){ // turn on after being off
    g16_SNTPinterval = iInterval;
    InitOrRestartSNTP();
  }
  else if (g16_SNTPinterval != 0 && iInterval == 0){ // turn off after being on
    g16_SNTPinterval = iInterval;
    StopSNTP();
  }
  else{ // change current interval
    g16_SNTPinterval = iInterval;
    SetSntpSyncInterval();
  }
  return true;
}

// example using usa eastern standard/eastern daylight time
// edt begins the second sunday in march at 0200
// est begins the first sunday in november at 0200
// "EST5EDT4,M3.2.0/02:00:00,M11.1.0/02:00:00"
//
// don't need this if calling configTzTime...
//  setenv("TZ", g_sTimezone.c_str(), 1); // Set timezone
//  tzset();
void SetSntpTimezoneAndStart(){
  prtln("Starting SNTP: " + String(NTP_SERVER1) + ", " + String(NTP_SERVER2) + ", " + g_sTimezone);
  // void configTzTime(const char* TIMEZONE, const char* server1, const char* server2, const char* server3);
  //configTime(gmtOffset_sec, daylightOffset_sec, NTP_SERVER1, NTP_SERVER2); // init the (S)NTP internet time system
  configTzTime(g_sTimezone.c_str(), NTP_SERVER1, NTP_SERVER2);
}

void SetSntpSyncInterval(){
  prtln("Setting SNTP sync hours: " + String(g16_SNTPinterval));
  sntp_set_sync_interval(g16_SNTPinterval*60*60*1000UL);
}

// call InitOrRestartSNTP() to restart!
void StopSNTP(){
  esp_sntp_stop();
}

// set time and date from sVal such as "2022-12-31T23:59:59" (send hour in 24-hour format)
// returns the current time and date: 2020-11-31T04:32:00pmTsetok (or Tlocked, Treload, Tnoset)
// returns an empty string if unable to read the time back after setting it.
// NOTE: if this is called and succeeds, we want to pass sVal over HTTP to other ESP32s which can
// then set their local clock if g_bSyncRx is set.
String SetTimeDate(String& sVal, bool bSendTimeDateToRemotes){
  String sRet;
  bool bSetRequestedButLocked = false;
  bool bTimeSetFailed = false;
  bool bTimeSetSuccess = false;

  // set our time if unlocked and it is present in input string
  if (sVal.length() >= MAXTIMELEN && sVal != "0"){
    if (!IsLocked()){
      //prtln("setting clock to web browser's time (user pressed \"Set\" button!): \"" + sVal + "\"");

      // parse date/time
//      int myMonth = 0;
//      int myDay = 0;
//      int myHour = 0;
//      int myMinute = 0;
//      int mySecond = 0;

      // get year
      int myYear = 0;
      String sT = sVal.substring(0,4);
      if (alldigits(sT))
        myYear = sT.toInt();

      // get month, day, hour, minute, second
      int myTime[5] = {0};
      for (int ii=0, iStart=5; ii<5; ii++, iStart+=3){
        sT = sVal.substring(iStart,iStart+2);
        if (alldigits(sT))
          myTime[ii] = sT.toInt();
      }
      
//      sT = sVal.substring(5,7);
//      if (alldigits(sT))
//        myMonth = sT.toInt();
//      sT = sVal.substring(8,10);
//      if (alldigits(sT))
//        myDay = sT.toInt();
//      sT = sVal.substring(11,13);
//      if (alldigits(sT))
//        myHour = sT.toInt();
//      sT = sVal.substring(14,16);
//      if (alldigits(sT))
//        myMinute = sT.toInt();
//      sT = sVal.substring(17,19);
//      if (alldigits(sT))
//        mySecond = sT.toInt();
//      if (SetTimeManually(myYear, myMonth, myDay, myHour, myMinute, mySecond))

      if (SetTimeManually(myYear, myTime[0], myTime[1], myTime[2], myTime[3], myTime[4]))
        bTimeSetSuccess = true;
      else
        bTimeSetFailed = true;
    }
    else
      bSetRequestedButLocked = true;
  }

  sRet = TimeToString(false); // time and date as: 2020-11-31T04:32:00pm

  if (sRet.length() >= MAXTIMELEN){
    // add a Tcommand to web-page p2.html via the formatted time and date it's polling for each second.
    // That gives us a nice two-way communication path!
    if (bSetRequestedButLocked)
      sRet += "Tlocked"; // show locked alert
    else if (bTimeSetSuccess){
      sRet += "Tsetok"; // show "Time was set!" at web-page Javascript
      if (bSendTimeDateToRemotes)
        SendTimeDateToAllRemotes(); // set CMtime command for each IP in mDNS array
    }
    else if (bTimeSetFailed)
      sRet += "Tnoset"; // show "Time set failed..." at web-page Javascript
    if (g_bTellP2WebPageToReload){
      sRet += "Treload";
      g_bTellP2WebPageToReload = false;
    }
  }

  return sRet;
}

// returns mDNS count or negative if error
// 2020-11-31T23:32:00
int SendTimeDateToAllRemotes(){
  if (!g_bWiFiConnected)
    return -2;  

  String sTime = TimeToString(true); // date/time time as 24-hour format
  
  if (sTime.length() != MAXTIMELEN){
    prtln("SendTimeDateToAllRemotes() length != MAXTIMELEN: \"" + sTime + "\"");
    return -3;
  }
  
  prtln("SendTimeDateToAllRemotes() \"" + sTime + "\"");

  return HMC.AddCommandAll(CMtime, sTime);
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

// returns date and time as: 2020-11-31T12:32:00pm
// or as 24 hour date and time: 2020-11-31T23:32:00
String TimeToString(bool b24hr){

  String sRet;

  // read internal time
  struct tm timeInfo = {0};
  if (ReadInternalTime(NULL, &timeInfo) != NULL){ // get current time as struct tm
    int hr;
    String sPm;
    if (b24hr){
      bool bPmFlag; // by ref
      hr = Make12Hour(timeInfo.tm_hour, bPmFlag);
      sPm = bPmFlag ? "pm" : "am";
    }
    else
      hr = timeInfo.tm_hour;
    sRet = String(timeInfo.tm_year+EPOCH_YEAR) + '-' +
      ZeroPad(timeInfo.tm_mon+1) + '-' + ZeroPad(timeInfo.tm_mday) + 'T' +
      String(hr) + ':' + ZeroPad(timeInfo.tm_min) + ':' + ZeroPad(timeInfo.tm_sec) + sPm;
  }
  return sRet;
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

// const char* ntpServer = "pool.ntp.org";
// configTime(0,0,ntpServer);
//unsigned long getEpochTime(){
//  time_t now;
//  struct tm timeinfo;
//  if (!getLocalTime(&timeinfo))
//    return 0;
//  time(&now);
//  return now;
//}

// obtain time with 1 sec resolution
//String getTime(){
//  time_t now;
//  char buf[64];
//  struct tm timeinfo;
//
//  time(&now);
//  setenv("TZ", "CST-8", 1); // set timezone
//  tzset();
//
//  localtime_r(&now, &timeinfo);
//  strftime(buf, sizeof(buf), "%c", &timeinfo);
//  return String(buf);
//}

// 1us resolution:
// struct timeval tv;
// gettimeofday(&tv, NULL):
// int64_t time_us = (int64_t)tv.tv_sec * 1000000L * (int64_t)tv.tv_usec;

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

// #include "esp_sntp.h"
// sTimeServer: "pool.ntp.org"
// sTz: "AEST-10AEDT,M10.1.0,M4.1.0/3"
// bool esp_sntp_enabled();
// esp_sntp_stop();
// esp_sntp_restart();
//void initSNTP(String sTz, String sTimeServer){
//  sntp_set_sync_interval(1*60*60*1000UL); // every hour
//  sntp_set_time_sync_notification_cb(tz_notify);
//  esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL); ESP_SNTP_OPMODE_LISTENONLY
//  esp_sntp_setservername(0, sTimeServer.c_str());
//  esp_sntp_init();
//  setenv("TZ", sTz.c_str(), 1);
//  tzset();
//}
// sntp_sync_mode_t sm = sntp_get_sync_mode(); // SNTP_SYNC_MODE_IMMED, SNTP_SYNC_MODE_SMOOTH
//void tz_notify(struct timeval* t){
//  prtln("synchronized!");
//}
// to poll it...
// SNTP_SYNC_STATUS_RESET, SNTP_SYNC_STATUS_IN_PROGRESS
//void wait4SNTP(){
//  sntp_sync_status_t ss = sntp_get_sync_status();
//  while (ss != SNTP_SYNC_STATUS_COMPLETED){
//    delay(100);
//    prtln("waiting...");
//  }
//}
//void printTime(){
//  struct ti;
//  getLocalTime(&ti);
//  Serial.println(&ti, "%A, %B %d %Y %H:%M:%S");
//}
