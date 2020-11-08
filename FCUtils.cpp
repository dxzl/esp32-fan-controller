#include "FanController.h"

String ZeroPad(byte val)
{
  String s = (val < 10) ? "0" : "";    
  s += String(val);
  return s;
}

String GetStringIP()
{
  //Static IP address configuration
  //IPAddress staticIP(192, 168, 43, 90); //ESP static ip
  //IPAddress gateway(192, 168, 43, 1);   //IP Address of your WiFi Router (Gateway)
  //IPAddress subnet(255, 255, 255, 0);  //Subnet mask
  //IPAddress dns(8, 8, 8, 8);  //DNS
  
  IPAddress apip = WiFi.softAPIP();
  IPAddress loip = WiFi.localIP();
  String sInfo = hostName + " (" + apip.toString() + "}";
  if (bSoftAP)
    sInfo = " is an access point.";
  else
    sInfo = " connected to router " + m_ssid + " at IP " + loip.toString();
  return sInfo;
}

void ReadPot1()
{
  pot1Value = analogRead(POT_1);
  int deltaPotValue = pot1Value - oldPot1Value;
  if (deltaPotValue < 0)
    deltaPotValue = -deltaPotValue;
    
  if (deltaPotValue > POT_DEBOUNCE)
  {
    //double voltage = (3.3/4095.0) * pot1Value;
    //prt("pot1Value:");
    //prt(pot1Value);
    //prt(" Voltage:");
    //prt(voltage);
    //prtln("V");

    float percent = (float)pot1Value*100.0/4095.0;
    
    if (swState == "0")
    {
      // period range: 0 - perMax
      perVal = (int)(percent*(float)perMax);
      m_taskMode = TASK_PERVAL;
      m_taskTimer = TASK_TIME;
    }
    else if (swState == "1")
    {
      // duty cycle: 0 - 100%
      dutyCycleA = (int)percent;
      dutyCycleB = dutyCycleA; // POT can't set them seperately (web-interface only!)
      m_taskMode = TASK_DCA_DCB;
      m_taskTimer = TASK_TIME;
    }
    else if (swState == "2")
    {
      // phase range: 0 - 100%
      phase = (int)percent;
      m_taskMode = TASK_PHASE;
      m_taskTimer = TASK_TIME;
    }
    
    oldPot1Value = pot1Value;
  }
}

void ReadModeSwitch()
{
  sw1Value = digitalRead(SW_1);
  sw2Value = digitalRead(SW_2);
  if (sw1Value != oldSw1Value || sw2Value != oldSw2Value)
  {
    if (sw1Value > 0)
      swState = "1";
    else if (sw2Value > 0)
      swState = "2";
    else
      swState = "0";
      
    prtln("swValue:" + swState);
    
    oldSw1Value = sw1Value;
    oldSw2Value = sw2Value;

    // switch changes and no auto mode, force it
    if (nvSsrMode1 != SSR_MODE_AUTO)
    {
      nvSsrMode1 = SSR_MODE_AUTO;
      ResetPeriod();
      PutPreference(EE_RELAY_A, nvSsrMode1);
      prtln("Switch forced SSR1 mode to auto:" + SsrModeToString(nvSsrMode1));
    }
    if (nvSsrMode2 != SSR_MODE_AUTO)
    {
      nvSsrMode2 = SSR_MODE_AUTO;
      ResetPeriod();
      PutPreference(EE_RELAY_B, nvSsrMode2);
      prtln("Switch forced SSR2 mode to auto:" + SsrModeToString(nvSsrMode2));
    }
  }
}

void PutPreference(const char* s, byte val)
{
  preferences.begin(EE_PREFS_NAMESPACE);
  preferences.putUChar(s, val);
  preferences.end();
  
  prtln(String(s) + ":" + String(val));

  // reset period
  ResetPeriod();

  // give blue built-in LED a flash... (might be a nich hack-warning!)
  ledFlashTimer = LED_EEPROM_FLASH_TIME;
  digitalWrite(ONBOARD_LED_GPIO2, HIGH);
  
  yield();
}

String GetPreferenceString(const char* s, const char* eeDefault)
{
  preferences.begin(EE_PREFS_NAMESPACE, false);
  String sTemp = preferences.getString(s, eeDefault);
  preferences.end();
  
  yield();
  return sTemp;
}

void PutPreferenceString(const char* s, String val)
{
  preferences.begin(EE_PREFS_NAMESPACE);
  preferences.putString(s, val);
  preferences.end();

  prtln(String(s) + ":" + String(val));

  // reset period
  ResetPeriod();
  
  yield();
}

void PutPreferenceU16(const char* s, uint16_t val)
{
  preferences.begin(EE_PREFS_NAMESPACE);
  preferences.putUShort(s, val);
  preferences.end();

  prtln(String(s) + ":" + String(val));

  // reset period
  ResetPeriod();
  
  yield();
}

// finds next occupied slot - call first with iStart 0
// subsequent iStart values should be the returned index + 1
// returns -1 if none empty
int FindNextFullTimeSlot(int iStart)
{
  int iRet = -1;

  preferences.begin(EE_SLOTS_NAMESPACE);

  for (int ii = iStart; ii < MAX_TIME_SLOTS; ii++)
  {
    String sName = EE_SLOT_PREFIX + GetSlotNumAsString(ii);
    
    int len = preferences.getBytesLength(sName.c_str()); // modified this in MyPreferences.cpp to not log an error
    
    if (len == sizeof(t_event))
    {
      iRet = ii;
      break;
    }
  }
 
  preferences.end();
    
  yield();

  return iRet;
}

// returns -1 if no slots
int FindFirstEmptyTimeSlot()
{
  int iSlot = -1;
  bool bNotZeroLength = false;

  preferences.begin(EE_SLOTS_NAMESPACE);
  for (int ii = 0; ii < MAX_TIME_SLOTS; ii++)
  {
    String sName = EE_SLOT_PREFIX + GetSlotNumAsString(ii);
    
    int len = preferences.getBytesLength(sName.c_str()); // modified this in MyPreferences.cpp to not log an error
    
    if (len != sizeof(t_event))
    {
      if (len != 0)
        bNotZeroLength = true;
      iSlot = ii;
      break;
    }
  }
  preferences.end();
  
  yield();

  // found a corrupt slot
  if (iSlot >= 0 && bNotZeroLength)
  {
    prtln("Try to delete corrupt slot: " + String(iSlot));
    if (DeleteTimeSlot(iSlot)) // try to clear it!
    {
      // recount...
      m_slotCount = CountFullTimeSlots();
      prtln("Corrupt slot deleted! slot-count: " + String(m_slotCount));
    }
  }

  return iSlot;
}

// returns true if success
bool PutTimeSlot(int slotIndex, t_event &t)
{
  // key names are 15 chars max length
  String sName = EE_SLOT_PREFIX + GetSlotNumAsString(slotIndex);
  int bytesWritten = 0;
  
  preferences.begin(EE_SLOTS_NAMESPACE);
  bytesWritten = preferences.putBytes(sName.c_str(), &t, sizeof(t_event));
  preferences.end();
  
  yield();

  return (bytesWritten == sizeof(t_event));
}

// returns true if success
bool GetTimeSlot(int slotIndex, t_event &t)
{
  String sName = EE_SLOT_PREFIX + GetSlotNumAsString(slotIndex);
  int bytesRead = 0;
  
  preferences.begin(EE_SLOTS_NAMESPACE);
  bytesRead = preferences.getBytes(sName.c_str(), &t, sizeof(t_event));
  preferences.end();
  
  yield();

  return (bytesRead == sizeof(t_event));
}

bool DisableTimeSlot(int slotIndex)
{
  return EnableTimeSlot(slotIndex, false);
}

// bEnable defaults true
bool EnableTimeSlot(int slotIndex, bool bEnable)
{
  t_event t;
  if (!GetTimeSlot(slotIndex, t)) // by ref
    return false;
  t.bEnable = bEnable;
  if (!PutTimeSlot(slotIndex, t))
    return false;
    
  m_taskMode = TASK_PAGE_REFRESH_REQUEST; // delay and tell P2.html to reload
  m_taskTimer = TASK_TIME;      
  return true;
}

// returns true if success
bool DeleteTimeSlot(int slotIndex)
{
  bool bRet = false;
  String sName = EE_SLOT_PREFIX + GetSlotNumAsString(slotIndex);
  
  preferences.begin(EE_SLOTS_NAMESPACE);
  bRet = preferences.remove(sName.c_str());
  preferences.end();
  
  if (bRet)
  {
    IV_RemoveIndexBySlot(slotIndex); // remove it from non-zero seconds list (if present)
    IR_RemoveIndexBySlot(slotIndex);
    m_slotCount--;
    m_taskMode = TASK_PAGE_REFRESH_REQUEST; // delay and tell P2.html to reload
    m_taskTimer = TASK_TIME;      
  }
  
  prtln("deleted time-slot: " + String(slotIndex));
    
  return bRet;
}

bool AddTimeSlot(t_event &slotData, bool bVerbose)
{
  if (m_slotCount >= MAX_TIME_SLOTS)
  {
    if (bVerbose)
      prtln("Time slots all full - count is: " + String(m_slotCount));
    return false;
  }
  
  // store new time slot's data
  int iSlot = FindFirstEmptyTimeSlot();

  if (bVerbose)
    prtln("empty slot index: " + String(iSlot));

  if (iSlot >= 0)
  {
    if (PutTimeSlot(iSlot, slotData))
    {
      if (slotData.timeDate.second != 0 || slotData.repeatMode == RPT_SECONDS)
        IV_Add(iSlot, slotData.timeDate.second); // add seconds-resolution item to local list (we check this list every second!)
      if (slotData.repeatMode != RPT_OFF)
        IR_Add(iSlot, false, slotData.repeatCount, slotData.everyCount);
      m_slotCount++; // increment slot-count
      
      if (bVerbose)
        prtln("new m_slotCount= " + String(m_slotCount));
        
      m_taskMode = TASK_PAGE_REFRESH_REQUEST; // delay and tell P2.html to reload
      m_taskTimer = TASK_TIME;
      return true;
    }
    
    if (bVerbose)
      prtln("Unable to write to slot index: " + String(iSlot));
      
    return false;
  }
  
  // should have empty slots but don't
  if (bVerbose)
    prtln("Error, should have empty slots but don't...");
    
  if (!EraseTimeSlots())
  {
    if (bVerbose)
      prtln("Error erasing time-slots!");
  }
  m_slotCount = 0;
  
  return false;
}

bool EraseTimeSlots()
{
  bool bRet = false;
  prtln("Erasing time-slots...");
  
  // clear time-slots namespace
  preferences.begin(EE_SLOTS_NAMESPACE);
  bRet = preferences.clear();
  preferences.end();

  if (bRet)
  {
    // clear lists
    IR_Clear();
    IV_Clear();
    m_slotCount = 0;
  }
    
  yield();

  return bRet;
}

bool ErasePreferences()
{
  bool bRet = false;
  prtln("Erasing preferences...");
  
  // clear prefs namespace
  preferences.begin(EE_PREFS_NAMESPACE);
  bRet = preferences.clear();
  preferences.end();

  yield();

  return bRet;
}

int CountFullTimeSlots()
{
  int iCount = 0;

  preferences.begin(EE_SLOTS_NAMESPACE);

  for (int ii = 0; ii < MAX_TIME_SLOTS; ii++)
  {
    String sName = EE_SLOT_PREFIX + GetSlotNumAsString(ii);
    
    int len = preferences.getBytesLength(sName.c_str()); // modified this in MyPreferences.cpp to not log an error
    
    if (len == sizeof(t_event))
      iCount++;
  }
 
  preferences.end();
    
  yield();

  return iCount;
}

// EE_SLOT_xxx (xxx is 000 to 099)
String GetSlotNumAsString(int val)
{
  String sSlotNum;
  if (val < 100)
    sSlotNum += '0';
  if (val < 10)
    sSlotNum += '0';
  sSlotNum += String(val);
  return sSlotNum;
}

// ssr1State can be "OFF" or "ON"
void SetState(byte val, byte ssrMode)
{
  if (ssrMode == SSR_MODE_ON)
    SetState(val, "ON");
  else
    SetState(val, "OFF");
}

// ssr1State can be "OFF" or "ON"
void SetState(byte val, String s)
{
  if (s == "ON")
  {
    digitalWrite(val, HIGH);

    if (val == SSR_1)
      ssr1State = "ON";
    else if (val == SSR_2)
      ssr2State = "ON";
  }
  else // OFF or AUTO
  {
    digitalWrite(val, LOW);
    
    if (val == SSR_1)
      ssr1State = "OFF";
    else if (val == SSR_2)
      ssr2State = "OFF";
  }
}

// ssr1Mode can be "OFF, "ON" or "AUTO"
String SsrModeToString(byte ssrMode)
{
  if (ssrMode == SSR_MODE_OFF)
    return("OFF");
  if (ssrMode == SSR_MODE_ON)
    return("ON");
  if (ssrMode == SSR_MODE_AUTO)
    return("AUTO");
  return("");
}
 
void ToggelOldSsidAndPwd()
{
  String sOld = GetPreferenceString(EE_OLDPWD, DEFAULT_PWD);
  String sNew = GetPreferenceString(EE_PWD, DEFAULT_PWD);
  PutPreferenceString(EE_OLDPWD, sNew);
  PutPreferenceString(EE_PWD, sOld);
  sOld = GetPreferenceString(EE_OLDSSID, DEFAULT_SSID);
  sNew = GetPreferenceString(EE_SSID, DEFAULT_SSID);
  PutPreferenceString(EE_OLDSSID, sNew);
  PutPreferenceString(EE_SSID, sOld);
  m_ssid = sOld;
}

void RestoreDefaultSsidAndPwd()
{
  PutPreferenceString(EE_PWD, DEFAULT_PWD);
  PutPreferenceString(EE_OLDPWD, DEFAULT_PWD);
  PutPreferenceString(EE_SSID, DEFAULT_SSID);
  PutPreferenceString(EE_OLDSSID, DEFAULT_SSID);
  m_ssid = DEFAULT_SSID;
}

// returns NULL if fail.
// pass in a pointer to a time_t version we export
// or set it NULL if not needed!
// set pTm to struct tm pointer we export
struct tm* ReadInternalTime(time_t* pEpochSeconds, struct tm* pTm)
{
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
// sets year to DEFAULT_YEAR which may be used to indicate "time is unset"
// only call this once - on reset
// return true if success
bool InitTimeManually()
{
  prtln("Initializing clock to DEFAULT_YEAR...");
  
  bManualTimeWasSet = false; // we are setting the "not set" time - so disable time-events!
  bWiFiTimeWasSet = false;
  bRequestManualTimeSync = false;
  bRequestWiFiTimeSync = false;

  struct tm timeInfo = {0}; // https://www.cplusplus.com/reference/ctime/tm/
  
  // set time manually (to an old date so we won't delete any time-slots automatically)
  timeInfo.tm_year = DEFAULT_YEAR - EPOCH_YEAR;
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

bool SetTimeManually(int myYear, int myMonth, int myDay, int myHour, int myMinute, int mySecond)
{
  if (clockSetDebounceTimer != 0)
    return true; // don't return error if just a "bouncy" web-request!
    
  clockSetDebounceTimer = MANUAL_CLOCK_SET_DEBOUNCE_TIME;
  
  if (myYear <= DEFAULT_YEAR)
  {
    prtln("Warning! Years less than or equal to DEFAULT_YEAR not allowed: " + String(DEFAULT_YEAR));
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
  if (settimeofday(&tv, NULL) == 0) // returns -1 if fail
  {
    prtln("Requesting manual time-sync...");
    bRequestManualTimeSync = true;
    return true;
  }
  
  prtln("Failed to set internal clock...");
  
  // this is used in ProcessTimeSlot() to facilitate
  // the repeat modes (see p2.html in the data folder)
  // I.E. every hour, day, Etc. following a time-event
  // trigger.
  m_prevDateTime = {0};
  bManualTimeWasSet = false; // stop processing events on manual time...
  return false;
}

// eastern  = -5
// central  = -6 (me)
// mountain = -7
// pacific  = -8
// so -6 * 60 * 60 = -6 * 3600 = -21600ms
//const long  gmtOffset_sec = -21600;
//const int   daylightOffset_sec = +3600; // offset +3600 if DST (daylight savings time) (spring forward)

// Before calling printLocalTime() call configTime!
void printLocalTime(struct tm &timeInfo)
{
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

// Replaces placeholder with SSR_1 state value
void ResetPeriod()
{
  dutyCycleTimerA = 0;
  dutyCycleTimerB = 0;
  phaseTimer = 0;
  periodTimer = 1; // restart cycle...
  savePeriod = 1;
}

// returns count added
// NOTE:  before calling this, call CountFullTimeSlots()!
int InitSecondsList(int slotCount)
{
  IV_Clear();

  int iFull = -1;
  
  // read each record of t_event
  for (int ii = 0; ii < slotCount; ii++)
  {
    iFull = FindNextFullTimeSlot(iFull+1);
    if (iFull < 0)
      break;
      
    // get time slot's data
    t_event t = {0};
    if (GetTimeSlot(iFull, t)) // by ref
    {
      // We have a special list for items that need monitoring every second instead of every minute.
      // any item with non-zero seconds or that must repeat every second goes here...
      if (t.timeDate.second != 0 || t.repeatMode == RPT_SECONDS)
        IV_Add(iFull, t.timeDate.second);
    }
    else
    {
      prtln("Error in InitSecondsList(). Can't read slot: " + String(iFull)); 
      break;
    }
  }
  return IV_GetCount();
}

// returns count added
// NOTE:  before calling this, call CountFullTimeSlots()!
int InitRepeatList(int slotCount)
{
  IR_Clear();

  if (slotCount == 0 || !(bManualTimeWasSet || bWiFiTimeWasSet))
    return 0;
  
  int iFull = -1;

  time_t now = time(0);
  struct tm timeInfo = {0};
  if (localtime_r(&now, &timeInfo) == NULL)
    return 0;  

  // check for stale events if if time has been set...
  bool bCheckStale = (timeInfo.tm_year+EPOCH_YEAR != DEFAULT_YEAR);
  
  // read each record of t_event
  for (int ii = 0; ii < slotCount; ii++)
  {
    iFull = FindNextFullTimeSlot(iFull+1);
    if (iFull < 0)
      break;
      
    // get time slot's data
    t_event slotData = {0};
    if (GetTimeSlot(iFull, slotData)) // by ref
    {
      // repeatCount == 0 is infinite, everyCount 0 == undefined
      if (slotData.repeatMode != RPT_OFF)
      {
        // can't go stale if set for infinite repeat???
        // what if the device is in a closet for 10 years and you pull it out and plug
        // it in and things start going on and off by themselves???
        // What about if we just mark them disabled in some way and let the user review the
        // events list?
        
        bool bStaleFlag;
        if (bCheckStale)
        {
          t_time_date timeDate = CopyTmToTtimeDate(timeInfo);
          // returns 0 if match, 1 if event-time has passed by... 2 if yet to be...
          int compareVal = CompareTimeDate(timeDate, slotData.timeDate);
          if (compareVal == 1)
            bStaleFlag = true;
          else
            bStaleFlag = false;
        }
        else
          bStaleFlag = false;
          
        IR_Add(iFull, bStaleFlag, slotData.repeatCount, slotData.everyCount);
      }
        
    }
    else
    {
      prtln("Error in InitRepeatList(). Can't read slot: " + String(iFull)); 
      break;
    }
  }
  return IR_GetCount();
}

// pass back t by reference
t_time_date CopyTmToTtimeDate(struct tm &tm)
{
  t_time_date t;  
  t.second = tm.tm_sec;
  t.minute = tm.tm_min;
  t.hour = tm.tm_hour;
  t.day = tm.tm_mday;
  t.month = tm.tm_mon+1;
  t.year = tm.tm_year+EPOCH_YEAR;
  t.dayOfWeek = tm.tm_wday;
  return t;
}

// returns 0 if a match
// 1 if time has passed by the slot's time
// 2 if time has not yet ocurred.
int CompareTimeDate(t_time_date &timeDate, t_time_date &slotTimeDate)
{
  bool bYearSame = timeDate.year == slotTimeDate.year;
  bool bMonthSame = timeDate.month == slotTimeDate.month;
  bool bDaySame = timeDate.day == slotTimeDate.day;
  bool bHourSame = timeDate.hour == slotTimeDate.hour;
  bool bMinuteSame = timeDate.minute == slotTimeDate.minute;
  bool bSecondSame = timeDate.second == slotTimeDate.second;

  // pass back 0 if time equals slot's time
  if (bYearSame && bMonthSame && bDaySame && bHourSame && bMinuteSame && bSecondSame)
    return 0;
    
  // pass back 1 if time > slot's time
  if (timeDate.year > slotTimeDate.year)
    return 1;
  bool bYearGe = timeDate.year >= slotTimeDate.year;
  if (bYearGe && timeDate.month > slotTimeDate.month)
    return 1;
  bool bMonthGe = timeDate.month >= slotTimeDate.month;
  if (bYearGe && bMonthGe && timeDate.day > slotTimeDate.day)
    return 1;
  bool bDayGe = timeDate.day >= slotTimeDate.day;
  if (bYearGe && bMonthGe && bDayGe && timeDate.hour > slotTimeDate.hour)
    return 1;
  bool bHourGe = timeDate.hour >= slotTimeDate.hour;
  if (bYearGe && bMonthGe && bDayGe && bHourGe && timeDate.minute > slotTimeDate.minute)
    return 1;
  bool bMinuteGe = timeDate.minute >= slotTimeDate.minute;
  if (bYearGe && bMonthGe && bDayGe && bHourGe && bMinuteGe && timeDate.second > slotTimeDate.second)
    return 1;
  
  return 2; // time less than event's time
}

// https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
// https://www.geeksforgeeks.org/find-day-of-the-week-for-a-given-date/
// The y -= m < 3 is a nice trick. It creates a "virtual year" that starts
// on March 1 and ends on February 28 (or 29), putting the extra day (if any)
// at the end of the year; or rather, at the end of the previous year.
int MyDayOfWeek(int d, int m, int y)  
{
  static int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};  
  y -= m < 3;  
  return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;  
}

// print wrappers
void prtln(String s)
{
#if PRINT_ON
  Serial.println(s);
#endif
}

void prt(String s)
{
#if PRINT_ON
  Serial.print(s);
#endif
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
  String s = "<option value='0'>" + sInit + "</option>";  
  int n = WiFi.scanNetworks();
  if (n == 0) return s;
  int count = 1;
  for (int ii = 0; ii < n; ++ii){
    String ssid = String(WiFi.SSID(ii));
    // skip adding current wifi router since it's added above...
    if (ssid != sInit)
      s += "<option value='" + String(count++) + "'>" + ssid + "</option>"; // WiFi.RSSI(i), WiFi.channel(i)
  }
  return s;
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
