// this file TimeSlotsClass.cpp
#include "Gpc.h"

TimeSlotsClass TSC;

// finds next occupied slot - call first with iStart 0
// subsequent iStart values should be the returned index + 1
// returns -1 if none empty
int TimeSlotsClass::FindNextFullTimeSlot(int iStart){
  int iRet = -1;

  PF.begin(EE_SLOTS_NAMESPACE, true); // read-only flag...

  for (int ii = iStart; ii < MAX_TIME_SLOTS; ii++){
    String sName = GetSlotNumAsString(ii);

    int len = PF.getBytesLength(sName.c_str()); // modified this in MyPreferences.cpp to not log an error

    if (len == sizeof(t_event)){
      iRet = ii;
      break;
    }
  }

  PF.end();

  yield();

  return iRet;
}

// returns -1 if no slots
int TimeSlotsClass::FindFirstEmptyTimeSlot(){
  int iSlot = -1;
  bool bNotZeroLength = false;

  PF.begin(EE_SLOTS_NAMESPACE, true); // read-only flag...
  for (int ii = 0; ii < MAX_TIME_SLOTS; ii++){
    String sName = GetSlotNumAsString(ii);

    int len = PF.getBytesLength(sName.c_str()); // modified this in MyPreferences.cpp to not log an error

    if (len != sizeof(t_event)){
      if (len != 0)
        bNotZeroLength = true;
      iSlot = ii;
      break;
    }
  }
  PF.end();

  yield();

  // found a corrupt slot
  if (iSlot >= 0 && bNotZeroLength){
    prtln("Trying to delete corrupt slot: " + String(iSlot));
    if (DeleteTimeSlot(iSlot)){ // try to clear it!
      // recount...
      g_slotCount = CountFullTimeSlots();
      prtln("Corrupt slot deleted! slot-count: " + String(g_slotCount));
    }
  }

  return iSlot;
}

bool TimeSlotsClass::IsTimeSlotEmpty(int slotIndex){
  String sName = GetSlotNumAsString(slotIndex);
  if (sName.isEmpty())
    return false;

  PF.begin(EE_SLOTS_NAMESPACE, true); // read-only flag...;
  int len = PF.getBytesLength(sName.c_str()); // modified this in MyPreferences.cpp to not log an error
  PF.end();

  if (len == sizeof(t_event))
    return false; // not empty
    
  if (len == 0)
    return true; // empty
    
  // found a corrupt slot
  prtln("Trying to delete corrupt slot: " + sName);
  if (DeleteTimeSlot(slotIndex)){ // try to clear it!
    // recount...
    g_slotCount = CountFullTimeSlots();
    prtln("Corrupt slot deleted! slot-count: " + String(g_slotCount));
    return true; // now it's empty!
  }
  
  return false; // can't make it empty...
}


// returns true if success
// Note: call IsTimeSlotEmpty() first... if empty, call AddTimeSlot() instead
// if not empty, calling this will replace a slot's data!
bool TimeSlotsClass::PutTimeSlot(int slotIndex, t_event &t){
  String sName = GetSlotNumAsString(slotIndex);
  if (sName.isEmpty())
    return false;
    
  int bytesWritten = 0;
  PF.begin(EE_SLOTS_NAMESPACE);
  bytesWritten = PF.putBytes(sName.c_str(), &t, sizeof(t_event));
  PF.end();
  yield();
  return (bytesWritten == sizeof(t_event));
}

// returns true if success
bool TimeSlotsClass::GetTimeSlot(int slotIndex, t_event &t){
  String sName = GetSlotNumAsString(slotIndex);
  if (sName.isEmpty())
    return false;

  int bytesRead = 0;

  PF.begin(EE_SLOTS_NAMESPACE, true); // read-only flag...;
  bytesRead = PF.getBytes(sName.c_str(), &t, sizeof(t_event));
  PF.end();

  yield();

  return (bytesRead == sizeof(t_event));
}

bool TimeSlotsClass::DisableTimeSlot(int slotIndex){
  return EnableTimeSlot(slotIndex, false);
}

// bEnable defaults true
bool TimeSlotsClass::EnableTimeSlot(int slotIndex, bool bEnable){
  t_event t;
  if (!GetTimeSlot(slotIndex, t)) // by ref
    return false;
  t.bEnable = bEnable;
  if (!PutTimeSlot(slotIndex, t))
    return false;

  TSK.QueueTask(TASK_PAGE_REFRESH_REQUEST); // delay and tell P2.html to reload
  return true;
}

// returns true if success
bool TimeSlotsClass::DeleteTimeSlot(int slotIndex){
  String sName = GetSlotNumAsString(slotIndex);
  if (sName.isEmpty())
    return false;

  bool bRet = false;

  PF.begin(EE_SLOTS_NAMESPACE);
  bRet = PF.remove(sName.c_str());
  PF.end();

  prt("delete slot " + String(slotIndex) + " ");

  if (bRet){
    IVL.RemoveIndexBySlot(slotIndex); // remove it from non-zero seconds list (if present)
    IRL.RemoveIndexBySlot(slotIndex);
    g_slotCount--;
    TSK.QueueTask(TASK_PAGE_REFRESH_REQUEST); // delay and tell P2.html to reload
    prtln("success!");
  }
  else
    prtln("failed!");
    
  return bRet;
}

//int TimeSlotsClass::GetSlotIndexFromName(String sName){
//  int iStart = String(EE_SLOT_PREFIX).length();
//  if (iStart < sName.length()){
//    String sIdx = sName.substring(iStart);
//    if (alldigits(sIdx))
//      return sIdx.toInt();
//  }
//  return -1;
//}

bool TimeSlotsClass::AddTimeSlot(t_event &slotData, bool bVerbose){
  if (g_slotCount >= MAX_TIME_SLOTS){
    if (bVerbose)
      prtln("Time slots all full - count is: " + String(g_slotCount));
    return false;
  }

  // store new time slot's data
  int iSlot = FindFirstEmptyTimeSlot();

  if (bVerbose)
    prtln("empty slot index: " + String(iSlot));

  if (iSlot >= 0){
    if (PutTimeSlot(iSlot, slotData)){
      if (slotData.timeDate.second != 0 || slotData.repeatMode == RPT_SECONDS)
        IVL.Add(iSlot, slotData.timeDate.second); // add seconds-resolution item to local list (we check this list every second!)
      if (slotData.repeatMode != RPT_OFF)
        IRL.Add(iSlot, false, slotData.repeatCount, slotData.everyCount);
      g_slotCount++; // increment slot-count

      if (bVerbose)
        prtln("new g_slotCount= " + String(g_slotCount));

      TSK.QueueTask(TASK_PAGE_REFRESH_REQUEST); // delay and tell P2.html to reload
      return true;
    }

    if (bVerbose)
      prtln("Unable to write to slot index: " + String(iSlot));

    return false;
  }

  // should have empty slots but don't
  if (bVerbose)
    prtln("Error, should have empty slots but don't...");

  if (!EraseTimeSlots()){
    if (bVerbose)
      prtln("Error erasing time-slots!");
  }
  g_slotCount = 0;

  return false;
}

bool TimeSlotsClass::EraseTimeSlots(){
  bool bRet = false;
  prtln("Erasing time-slots...");

  // clear time-slots namespace
  PF.begin(EE_SLOTS_NAMESPACE);
  bRet = PF.clear();
  PF.end();

  if (bRet){
    // clear lists
    IRL.Clear();
    IVL.Clear();
    g_slotCount = 0;
  }

  yield();

  return bRet;
}

int TimeSlotsClass::CountFullTimeSlots(){
  int iCount = 0;

  PF.begin(EE_SLOTS_NAMESPACE, true);

  for (int ii = 0; ii < MAX_TIME_SLOTS; ii++){
    String sName = GetSlotNumAsString(ii);

    int len = PF.getBytesLength(sName.c_str()); // modified this in MyPF.cpp to not log an error

    if (len == sizeof(t_event))
      iCount++;
  }

  PF.end();

  yield();

  return iCount;
}

// EE_SLOT_xxx (xxx is 000 to 999)
String TimeSlotsClass::GetSlotNumAsString(int val){
  String sSlotNum;
  if (val < 100)
    sSlotNum += '0';
  if (val < 10)
    sSlotNum += '0';
  sSlotNum += String(val);
  return EE_SLOT_PREFIX + sSlotNum;
}

// This is used with PARAM_EDITINDEX to send a slot's data (for p2.html/p2.js) that's to be edited
// Exactly 26 parameters in this exact order are expected by p2.js in a comma separated string! 
String TimeSlotsClass::TimeSlotToCommaSepString(int idx, t_event &t){
  bool pmFlag; // by ref
  int hr12 = Make12Hour(t.timeDate.hour, pmFlag);
  String sOut = String(t.repeatMode) + "," + String(t.deviceMode) + "," + String(t.deviceAddr) + "," + String (t.repeatCount) + "," + String(t.everyCount) + "," +
    String(t.timeDate.dayOfWeek) + "," + String(hr12) + "," + String(t.timeDate.minute) + "," + String (t.timeDate.second) + "," +
      String(t.timeDate.day) + "," + String(t.timeDate.month) + "," + String(t.timeDate.year) + "," + String(pmFlag) + "," +
        String(t.dutyCycleA) + "," + String(t.dutyCycleB) + "," + String(t.dutyCycleC) + "," + String(t.dutyCycleD) + "," +
        String(t.phaseB) + "," + String(t.phaseC) + "," + String(t.phaseD) + "," + String(t.perUnits) + "," +
          String(t.perMax) + "," + String(t.bIncludeCycleTiming) + "," + String(t.bCycleTimingInRepeats) + "," + String(t.bEnable) + "," + String(idx);
  return sOut;
}

// This is used to format the p2.html select drop-down list to choose a slot to delete...
// In the p2.html and p2.js webpage (in the "data" folder!), the user can load a plain text file that has timeslots formatted one per line as follows:
// 100 time-events maximum allowed! Comments are allowed in the file that start with #
//   Format example: (leading * if event is expired): "12/31/2020 12:59:59pm AB:auto r:inf e:65535 t:min"
//      Options: am|pm, A|B|AB, off|on|auto, off|sec|min|hrs|day|wek|mon|yrs
//      Optional cycle-timing: a:40,b:50,p:20,u:0-3,m:0-9,v:0-m,i:y (include cycle-timing), c:y (...in repeat events)
String TimeSlotsClass::TimeSlotToSpaceSepString(t_event &t){
  String sDevAddr;

  if (t.deviceAddr & 1)
    sDevAddr += "A";
  if (t.deviceAddr & 2)
    sDevAddr += "B";
  if (t.deviceAddr & 4)
    sDevAddr += "C";
  if (t.deviceAddr & 8)
    sDevAddr += "D";
  if (t.deviceAddr & 16)
    sDevAddr += "E";
  if (t.deviceAddr & 32)
    sDevAddr += "F";
  if (t.deviceAddr & 64)
    sDevAddr += "G";
  if (t.deviceAddr & 128)
    sDevAddr += "H";

  String sDevMode;

  switch(t.deviceMode){
    case 0:
      sDevMode = "off";
    break;
    case 1:
      sDevMode = "on";
    break;
    case 2:
      sDevMode = "auto";
    break;
    default:
      sDevMode = "???";
    break;
  }

  // if repeatMode not "off", then add repeat count and every count

  String sRptCount = " r:";
    
  if (t.repeatCount == 0)
    sRptCount += "inf";
  else
    sRptCount += String(t.repeatCount);

  String sEveryCount = " e:";

  if (t.everyCount == 0)
    sEveryCount += "off";
  else
    sEveryCount += String(t.everyCount);
      
  //<option value="0">off</option>
  //<option value="1">second</option>
  //<option value="2">minute</option>
  //<option value="3">hour</option>
  //<option value="4">daily</option>
  //<option value="5">weekly</option>
  //<option value="6">monthly</option>
  //<option value="7">yearly</option>
  
  String sRptMode;
  switch(t.repeatMode){
    case 0:
      sRptMode = "off";
    break;
    case 1:
      sRptMode = "sec";
    break;
    case 2:
      sRptMode = "min";
    break;
    case 3:
      sRptMode = "hrs";
    break;
    case 4:
      sRptMode = "day";
    break;
    case 5:
      sRptMode = "wek";
    break;
    case 6:
      sRptMode = "mon";
    break;
    case 7:
      sRptMode = "yrs";
    break;
    default:
      sRptMode = "???";
    break;
  }
  sRptMode = " t:" + sRptMode;
    
//  String sDayOfWeek;
//  switch(t.timeDate.dayOfWeek){
//    case 0:
//      sDayOfWeek = "su";
//    break;
//    case 1:
//      sDayOfWeek = "mn";
//    break;
//    case 2:
//      sDayOfWeek = "tu";
//    break;
//    case 3:
//      sDayOfWeek = "wd";
//    break;
//    case 4:
//      sDayOfWeek = "th";
//    break;
//    case 5:
//      sDayOfWeek = "fr";
//    break;
//    case 6:
//      sDayOfWeek = "sa";
//    break;
//    default:
//      sDayOfWeek = "x";
//    break;
//  }

  bool bPmFlag; // by reference
  int my12Hour = Make12Hour(t.timeDate.hour, bPmFlag);
  String sPm = bPmFlag ? "pm" : "am";
  String sEna = t.bEnable ? "" : "*"; // leading * is "disabled"

  // sDayOfWeek - need to fit in? TODO
  // NOTE: this format will be stored in .txt files when the user saves events via the web-page
  String sRet = sEna + String(t.timeDate.month) + "/" + String(t.timeDate.day) + "/" + String(t.timeDate.year) + ", " +
    String(my12Hour) + ':' + ZeroPad(t.timeDate.minute) + ':' + ZeroPad(t.timeDate.second) + sPm + ", " +
      sDevAddr + ':' + sDevMode + sRptCount + sEveryCount + sRptMode;

  // here are more...
  // dca:40,dcb:50,dcc:50,dcd:50,pb:20,pc:20,pd:20,u:0-3,m:0-9,v:0-m, c:y, i:y
  if (t.perVal != 0xff)
    sRet += " v:" + String(t.perVal);
  if (t.perMax != 0xffff)
    sRet += " m:" + String(t.perMax);
  if (t.perUnits != 0xff)
    sRet += " u:" + String(t.perUnits);
  if (t.phaseB != 0xff)
    sRet += " pb:" + String(t.phaseB);
  if (t.phaseC != 0xff)
    sRet += " pc:" + String(t.phaseC);
  if (t.phaseD != 0xff)
    sRet += " pd:" + String(t.phaseD);
  if (t.dutyCycleA != 0xff)
    sRet += " dca:" + String(t.dutyCycleA);
  if (t.dutyCycleB != 0xff)
    sRet += " dcb:" + String(t.dutyCycleB);
  if (t.dutyCycleC != 0xff)
    sRet += " dcc:" + String(t.dutyCycleC);
  if (t.dutyCycleD != 0xff)
    sRet += " dcd:" + String(t.dutyCycleD);
  if (t.bIncludeCycleTiming != false)
    sRet += " i:y";
  if (t.bCycleTimingInRepeats != false)
    sRet += " c:y";

  return sRet;
}

// returns count added
// NOTE:  before calling this, call CountFullTimeSlots()!
int TimeSlotsClass::InitSecondsList(int slotCount){
  IVL.Clear();

  int iFull = -1;

  // read each record of t_event
  for (int ii = 0; ii < slotCount; ii++){
    iFull = FindNextFullTimeSlot(iFull+1);
    if (iFull < 0)
      break;

    // get time slot's data
    t_event t = {0};
    if (GetTimeSlot(iFull, t)){ // by ref
      // We have a special list for items that need monitoring every second instead of every minute.
      // any item with non-zero seconds or that must repeat every second goes here...
      if (t.timeDate.second != 0 || t.repeatMode == RPT_SECONDS)
        IVL.Add(iFull, t.timeDate.second);
    }
    else{
      prtln("Error in InitSecondsList(). Can't read slot: " + String(iFull));
      break;
    }
  }
  return IVL.GetCount();
}

// returns count added
// NOTE:  before calling this, call CountFullTimeSlots()!
int TimeSlotsClass::InitRepeatList(int slotCount){
  IRL.Clear();

  if (slotCount == 0 || !(g_bManualTimeWasSet || g_bWiFiTimeWasSet))
    return 0;

  int iFull = -1;

  time_t now = time(0);
  struct tm timeInfo = {0};
  if (localtime_r(&now, &timeInfo) == NULL)
    return 0;

  // check for stale events if time has been set...
  bool bCheckStale = (timeInfo.tm_year+EPOCH_YEAR != DEF_YEAR);

  // read each record of t_event
  for (int ii = 0; ii < slotCount; ii++){
    iFull = FindNextFullTimeSlot(iFull+1);
    if (iFull < 0)
      break;

    // get time slot's data
    t_event slotData = {0};
    if (GetTimeSlot(iFull, slotData)){ // by ref
      // repeatCount == 0 is infinite, everyCount 0 == undefined
      if (slotData.repeatMode != RPT_OFF){
        bool bStaleFlag = false;
        if (bCheckStale && slotData.repeatCount != 0){ // prevent going stale if set for infinite repeat
          t_time_date timeDate = CopyTmToTtimeDate(timeInfo);
          // returns 0 if match, 1 if event-time has passed by... 2 if yet to be...
          if (CompareTimeDate(timeDate, slotData.timeDate) == 1)
            bStaleFlag = true;
        }

        IRL.Add(iFull, bStaleFlag, slotData.repeatCount, slotData.everyCount);
      }
    }
    else{
      prtln("Error in InitRepeatList(). Can't read slot: " + String(iFull));
      break;
    }
  }
  return IRL.GetCount();
}

// pass back t by reference
t_time_date TimeSlotsClass::CopyTmToTtimeDate(struct tm &tm){
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
int TimeSlotsClass::CompareTimeDate(t_time_date &timeDate, t_time_date &slotTimeDate){
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
int TimeSlotsClass::MyDayOfWeek(int d, int m, int y){
  static int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};
  y -= m < 3;
  return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

// called when web-page p2.html loads a .txt file of
// time-event strings separated by \n. We process
// an individual event here...
//
//var s = "#100 time-events maximum allowed!\n" +
//"#Format example: (* if event expired)\n" +
//"# 12/31/2020, 12:59:59pm, AB:auto r:65534, e:65534,t:min\n" +
//"# (am, pm), (A, B, AB), (off, on, auto)\n" +
//"# (off/sec/min/hrs/day/wek/mon/yrs)\n" +
//"# (optional cycle-timing: a:40,b:50,p:20,u:0-3,m:0-9,v:0-100,\n" +
//"# i:y include cycle-timing, c:y ...in repeat events)\n";
// NOTE: allow for time in 24-hour format (has no am/pm)
//
// A maximum of 100 time-events are allowed! Parameters may be separated by either a space or a comma.
// Format example: (append a leading * if event is disabled (expired))
// 12/31/2020, 12:59:59pm, AB:auto, r:65534, e:65534, t:min (r is repeat count [0 is inf], e is repeat every, t is repeat units (off|sec|min|hrs|day|wek|mon|yrs))
// Optional cycle-timing: a:0-100,b:0-100,p:0-100,v:0-100,m:1-65535,u:0-3 (a and b are duty cycle, p is phase, v is period in percent)
// (m is max-period 1-65535 of units, u is units [0=half-seconds, 1=seconds, 2=minutes, 3=hours])
// Options: am|pm, A|B|AB, off|on|auto
// i:y include cycle-timing, c:y ...in repeat events)
// Setting 0 for a,b,v is random percentage
// Setting 100 for p is random percentage
// The actual cycle period is computed as (v*m)/100
//
// Example string to set a disabled timeslot:
// *11/02/2024,9:20:00pm,AB:auto,r:inf,e:off,t:off,a:0,b:0,p:100,v:0,m:60,u:1,i:y
//
bool TimeSlotsClass::StringToTimeSlot(String sIn, t_event &slotData){
  sIn.trim();

  int len = sIn.length();
  if (len == 0)
    return false;

  if (len > MAX_RECORD_SIZE){
    prtln("Incomming file-record too long!");
    return false;
  }

  if (sIn[0] == '#')
    return false; // skip comments

  bool bEnable; // we set to disable if leading '*'
  // sOut can have leading * indicating "disabled"
  if (sIn[0] == '*'){
    sIn = sIn.substring(1); // take off the "*"
    sIn.trim();
    len = sIn.length();
    if (len == 0)
      return false;
    bEnable = false;
  }
  else
    bEnable = true;

  // parse time-events from .txt file and add them. Events are seperated by \n (newline)
  // but we can handle \r\n or \n\r as well.
  String sOut = "";
  int16_t iMonth, iDay, iYear, iHour, iMinute, iSecond, iDevAddr, iDevMode;
  //int16_t iDayOfWeek;
  int16_t iDcA, iDcB, iDcC, iDcD, iPhaseB, iPhaseC, iPhaseD, iPerUnits, iPerVal;
  int32_t iPerMax;
  
  // init these to 0 - they are "optional" in input string - but need to be 0 if not present!
  int16_t iRcount = 0; // infinite by default
  int16_t iEcount = 1; // repeat every "one" of iRepeatMode by default
  int16_t iRepeatMode = 0; // off by default
  bool bCycleTimingInRepeats = false;
  bool bIncludeCycleTiming = false;
  int iCount = 0;

  // optional - user can add any or all they want, in any order
  iDcA = iDcB = iDcC = iDcD = iPhaseB = iPhaseC = iPhaseD = iPerUnits = iPerVal = -1; // unused on init...
  iPerMax = -1;
  
  for (int ii = 0 ; ii <= len ; ii++){ // allow ii to go past eof on purpose!
    // impute a record-break if past end of file
    // to force last parameter to be processed
    char c = ii >= len ? ' ' : sIn[ii];

    if (c == ' ' || c == ','){ // field-seperator
      // skip extraneous spaces or commas
      if (ii+1 < len && (sIn[ii+1] == ' ' || sIn[ii+1] == ','))
        continue;

      int lenToken = sOut.length();
      if (lenToken > 0){
        sOut.trim();
        sOut.toLowerCase();

        bool bRandom = (sOut == TSC_RAND || sOut == "random");
        
        if (iCount == 0){
          if (!ParseDate(sOut, iMonth, iDay, iYear))
            return false;
          iCount++;
        }
        else if (iCount == 1){
          if (!ParseTime(sOut, iHour, iMinute, iSecond))
            return false;
          iCount++;
        }
        else if (iCount == 2){
          // iDevAddr will be returned with 0 if no device is selected, 0x0f if A, B, C and D are selected,
          // 0x01 if device A is selected, 0x06 if devices B and C are selected, Etc.
          if (!ParseDevAddressAndMode(sOut, iDevAddr, iDevMode))
            return false;
          iCount++;
        }
        else if (iCount > 2){ // the following are optional and can occur in any order
          int idx = sOut.indexOf(':');
          if (idx > 0){
            // dutyCycleA, dutyCycleB, phaseB, units index 0-3, max slider index 0-9, value of slider 0-max
            // dca:50,dcb:50,dcc:50,dcd:50,pb:50,pc:50,pd:50,u:1,m:9,v:100
            String cmd = sOut.substring(0, idx);
            sOut = sOut.substring(idx+1);
            int iVal = sOut.toInt();
            if (cmd == TSC_REPEAT_MODE)
              ParseRepeatMode(sOut, iRepeatMode); // by ref
            else if (cmd == TSC_REPEAT_COUNT){
              if (sOut == TSC_INF)
                iRcount = 0;
              else if (iVal >= 0)
                iRcount = iVal;
            }
            else if (cmd == TSC_EVERY_COUNT){
              if (sOut == TSC_OFF)
                iEcount = 0;
              else if (iVal >= 0)
                iEcount = iVal;
            }
            else if (cmd == TSC_INCLUDE_CYCLE_TIMING){
              if (sOut == TSC_YES)
                bIncludeCycleTiming = true;
            }
            else if (cmd == TSC_CYCLE_TIMING_IN_REPEATS){
              if (sOut == TSC_YES)
                bCycleTimingInRepeats = true;
            }
            else if (cmd == TSC_DUTY_CYCLE_A){
              if (bRandom)
                iDcA = 0;
              else if (iVal >= 0)
                iDcA = iVal;
            }
            else if (cmd == TSC_DUTY_CYCLE_B){
              if (bRandom)
                iDcB = 0;
              else if (iVal >= 0)
                iDcB = iVal;
            }
            else if (cmd == TSC_DUTY_CYCLE_C){
              if (bRandom)
                iDcC = 0;
              else if (iVal >= 0)
                iDcC = iVal;
            }
            else if (cmd == TSC_DUTY_CYCLE_D){
              if (bRandom)
                iDcD = 0;
              else if (iVal >= 0)
                iDcD = iVal;
            }
            else if (cmd == TSC_PHASE_B){
              if (bRandom)
                iPhaseB = 100;
              else if (iVal >= 0){
                iPhaseB = iVal;
              }
            }
            else if (cmd == TSC_PHASE_C){
              if (bRandom)
                iPhaseC = 100;
              else if (iVal >= 0){
                iPhaseC = iVal;
              }
            }
            else if (cmd == TSC_PHASE_D){
              if (bRandom)
                iPhaseD = 100;
              else if (iVal >= 0){
                iPhaseD = iVal;
              }
            }
            else if (cmd == TSC_UNITS){
              // 0=halfsec,1=sec,2=min,3=hrs
              if (sOut == "halfsec")
                iPerUnits = 0;
              else if (sOut == "sec")
                iPerUnits = 1;
              else if (sOut == "min")
                iPerUnits = 2;
              else if (sOut == "hrs")
                iPerUnits = 3;
              else if (iVal >= 0)
                iPerUnits = iVal;
            }
            else if (cmd == TSC_PERMAX){
              // value of the period (in iPerUnits) when the iPerVal slider (index.html) is at 100%
              // 1-65535
              if (iVal >= 0)
                iPerMax = iVal;
            }
            else if (cmd == TSC_PERVAL){
              if (bRandom)
                iPerVal = 0;
              else if (iVal >= 0)
                iPerVal = iVal;
            }
            else
              return false;
          }
          else
            return false;
        }
        sOut = "";
      }
    }
    else
      sOut += c;
  }

  if (iCount < 3)
    return false;

  // put all of our gleaned info into a t_event
  slotData.timeDate.hour = iHour;
  slotData.timeDate.minute = iMinute;
  slotData.timeDate.second = iSecond;
  slotData.timeDate.year = iYear;
  slotData.timeDate.month = iMonth;
  slotData.timeDate.day = iDay;
  slotData.timeDate.dayOfWeek = TSC.MyDayOfWeek(iDay, iMonth, iYear);

  slotData.repeatMode = iRepeatMode;
  slotData.deviceMode = iDevMode;
  slotData.deviceAddr = iDevAddr;
  slotData.repeatCount = iRcount;
  slotData.everyCount = iEcount;

  slotData.bEnable = bEnable;
  slotData.bIncludeCycleTiming = bIncludeCycleTiming;
  slotData.bCycleTimingInRepeats = bCycleTimingInRepeats;

  // casting from signed to unsigned, some are -1
  slotData.dutyCycleA = (uint8_t)iDcA;
  slotData.dutyCycleB = (uint8_t)iDcB;
  slotData.dutyCycleC = (uint8_t)iDcC;
  slotData.dutyCycleD = (uint8_t)iDcD;
  slotData.phaseB = (uint8_t)iPhaseB;
  slotData.phaseC = (uint8_t)iPhaseC;
  slotData.phaseD = (uint8_t)iPhaseD;
  slotData.perUnits = (uint8_t)iPerUnits;
  slotData.perVal = (uint8_t)iPerVal;
  slotData.perMax = (uint16_t)iPerMax;

  return true;
}

bool TimeSlotsClass::ParseRepeatMode(String &s, int16_t &iRepeatMode){
  if (s == "off")
    iRepeatMode = 0;
  else if (s == "sec")
    iRepeatMode = 1;
  else if (s == "min")
    iRepeatMode = 2;
  else if (s == "hrs")
    iRepeatMode = 3;
  else if (s == "day")
    iRepeatMode = 4;
  else if (s == "wek")
    iRepeatMode = 5;
  else if (s == "mon")
    iRepeatMode = 6;
  else if (s == "yrs")
    iRepeatMode = 7;
  else
    iRepeatMode = -1;

  return true;
}

bool TimeSlotsClass::ParseDevAddressAndMode(String &s, int16_t &iDevAddr, int16_t &iDevMode){
  int idx = s.indexOf(':');
  if (idx < 0)
    return false;

  String sTmp = s.substring(0, idx);
  sTmp.trim();
  sTmp.toLowerCase();
  if (sTmp == "all")
    iDevAddr = 0x0f; // A=bit0, B=bit1, C=bit2, D=bit3
  else{
    iDevAddr = 0;
    if (sTmp.indexOf('a') >= 0)
      iDevAddr |= 0x01;
    if (sTmp.indexOf('b') >= 0)
      iDevAddr |= 0x02;
    if (sTmp.indexOf('c') >= 0)
      iDevAddr |= 0x04;
    if (sTmp.indexOf('d') >= 0)
      iDevAddr |= 0x08;
  }

  sTmp = s.substring(idx+1);
  sTmp.trim();
  if (sTmp == "off")
    iDevMode = 0;
  else if (sTmp == "on")
    iDevMode = 1;
  else if (sTmp == "auto")
    iDevMode = 2;
  else
    iDevMode = -1;

  return true;
}

// s is passed in trimmed
// Examples 2:6:9pm or 23:59:59 or 1AM or 4:03pm or 12:59:00am
// returns true if success
bool TimeSlotsClass::ParseTime(String &s, int16_t &iHour, int16_t &iMinute, int16_t &iSecond){
  bool bHaveHour = false;
  bool bHaveMinute = false;
  bool bHaveSecond = false;
  int iTemp = 0;
  String sOut = "";

  iHour = -1;
  iMinute = -1;
  iSecond = -1;

  s.trim();
  s.toLowerCase();

  int len = s.length();

  bool bAm = false;
  bool bPm = false;
  if (len >= 2 && s[len-1] == 'm'){
    if (s[len-2] == 'p')
      bPm = true;
    else if (s[len-2] == 'a')
      bAm = true;
  }
  int maxHour, minHour;
  if (bPm || bAm){
    s = s.substring(0,len-2);
    len = s.length();
    maxHour = 12;
    minHour = 1;
  }
  else{
    maxHour = 23;
    minHour = 0;
  }
  
  for (int ii = 0; ii < len; ii++){
    char c = s[ii];
    if (c == ':'){
      if (sOut.length() > 0){
        if (bHaveHour){
          iTemp = sOut.toInt();
          if (iTemp >= 0 && iTemp <= 59){
            iMinute = iTemp;
            bHaveMinute = true;
            sOut = "";
          }
          else
            return false;
        }
        else{
          iTemp = sOut.toInt();
          if (iTemp >= minHour && iTemp <= maxHour){
            iHour = iTemp;
            bHaveHour = true;
            sOut = "";
          }
          else
            return false;
        }
      }
      else
        return false;
    }
    else if (isdigit(c))
      sOut += c;
    else
      return false;
  }

  // must have at least one digit in sOut!
  if (sOut.isEmpty())
    return false;
    
  iTemp = sOut.toInt();
  if (!bHaveHour){
    if (iTemp < minHour || iTemp > maxHour)
      return false;
    iHour = iTemp;
    iMinute = 0;
    iSecond = 0;
  }
  else if (iTemp >= 0 && iTemp <= 59){
    if (!bHaveMinute){
      iMinute = iTemp;
      iSecond = 0;
    }
    else
      iSecond = iTemp;
  }
  else
    return false;

  // convert time in 12-hour format to 24-hour
  if (bAm){
    if (iHour == 12)
      iHour = 0;
  }
  else if (bPm){
    if (iHour != 12)
      iHour += 12;
  }
  
  return true;
}

bool TimeSlotsClass::ParseDate(String &s, int16_t &iMonth, int16_t &iDay, int16_t &iYear){
  bool bHaveMonth = false;
  bool bHaveDay = false;
  int iTemp = 0;
  String sOut = "";

  iMonth = -1;
  iDay = -1;
  iYear = -1;

  int len = s.length();

  for (int ii = 0; ii < len; ii++){
    if (bHaveDay && ii == len-1){
      sOut += s[ii];
      iTemp = sOut.toInt();
      if (iTemp >= DEF_YEAR && sOut.length() == 4){ // "2020", Etc.
        iYear = iTemp;
        return true;
      }

      return false;
    }
    else if (s[ii] == '/'){
      if (sOut.length() > 0){
        if (bHaveMonth){
          iTemp = sOut.toInt();
          if (iTemp >= 1 && iTemp <= 31){
            iDay = iTemp;
            bHaveDay = true;
            sOut = "";
          }
          else
            return false;
        }
        else{
          iTemp = sOut.toInt();
          if (iTemp >= 1 && iTemp <= 12){
            iMonth = iTemp;
            bHaveMonth = true;
            sOut = "";
          }
          else
            return false;
        }
      }
    }
    else
      sOut += s[ii];
  }
  return false;
}

// read time-slot info
void TimeSlotsClass::ProcessSecondResolutionTimeSlots(){
  if (g_slotCount == 0 || !(g_bManualTimeWasSet || g_bWiFiTimeWasSet))
    return;

  int count = IVL.GetCount();
  if (count == 0) return; // no non-zero second slots programmed!

  struct tm timeInfo = {0};
  if (ReadInternalTime(NULL, &timeInfo) == NULL) // get current time as both epoch-time_t and struct tm
    return;

  // copy current time and date to format used in each event
  t_time_date timeDate = TSC.CopyTmToTtimeDate(timeInfo);

  // need to check them...
  for (int ii = 0; ii < count; ii++){
    // index might be any number 0 to MAX_TIME_SLOTS, even if only 1 item!
    // (this list has ONLY non-zero seconds slots!)
    int slotIndex = IVL.GetIndex(ii);

    t_event slotData;

    // read full slot into temp-struct slotData
    if (TSC.GetTimeSlot(slotIndex, slotData)) // by ref
      ProcessTimeSlot(slotIndex, timeDate, slotData);
    else{
      prtln("Error in ProcessSecondResolutionTimeSlots(). Can't read slot: " + String(slotIndex));
      break;
    }

    yield(); // let other processes run...
  }

  // g_prevDateTime is used to facilitate
  // the repeat modes (see p2.html in the data folder)
  // I.E. every hour, day, Etc. following a time-event
  // trigger.
  g_prevDateTime = timeDate;
}

void TimeSlotsClass::ProcessMinuteResolutionTimeSlots(){
  if (g_slotCount == 0 || !(g_bManualTimeWasSet || g_bWiFiTimeWasSet))
    return;

  struct tm timeInfo = {0};
  if (ReadInternalTime(NULL, &timeInfo) == NULL) // get current time as both epoch-time_t and struct tm
    return;

  prt("Time: ");
  printLocalTime(timeInfo);

  t_time_date timeDate = TSC.CopyTmToTtimeDate(timeInfo);

  if (timeDate.second != 0){
    prtln("detected drift in seconds, correcting. timeDate.second=" + String(timeDate.second));
    timeDate.second = 0;
  }

  int iFull = -1;

  for (int ii = 0; ii < g_slotCount; ii++){
    iFull = TSC.FindNextFullTimeSlot(iFull+1);
    if (iFull < 0)
      break;

    // if this slot's in the non-zero seconds list, continue - we process it in ProcessSecondResolutionTimeSlots()
    if (IVL.FindIndex(iFull) >= 0)
      continue;

    t_event slotData;
    if (TSC.GetTimeSlot(iFull, slotData)) // by ref
      ProcessTimeSlot(iFull, timeDate, slotData);
    else{
      prtln("Error in ProcessMinuteResolutionTimeSlots(). Can't read slot: " + String(iFull));
      break;
    }
    yield(); // let other processes run...
  }

  // g_prevDateTime is used to facilitate
  // the repeat modes (see p2.html in the data folder)
  // I.E. every hour, day, Etc. following a time-event
  // trigger.
  g_prevDateTime = timeDate;
}

// process all except timeinfo.tm_sec
// timeDate has the RTC time/date
// slotData has the data associated with slotIndex
// timeEpoch is the Epoch (# seconds since midnight 1-1-1970)
void TimeSlotsClass::ProcessTimeSlot(int slotIndex, t_time_date timeDate, t_event slotData){
  // return if time never set or if slot is not enabled (bit0 of flags)
  if (!slotData.bEnable || timeDate.year == DEF_YEAR)
    return;

  //tm_sec  int seconds after the minute  0-61 (usually 0-59 unless a leap-minute)
  //tm_min  int minutes after the hour  0-59
  //tm_hour int hours since midnight  0-23
  //tm_mday int day of the month  1-31
  //tm_mon  int months since January  0-11 (NOTE: in my custom t_event this is 1-12!)
  //tm_year int years since 1900
  //tm_wday int days since Sunday 0-6
  //tm_yday int days since January 1  0-365
  //tm_isdst  int Daylight Saving Time flag
  //zero if Daylight Saving Time is not in effect, and less than zero if the information is not available.
  //The Daylight Saving Time flag (tm_isdst) is greater than zero if Daylight Saving Time is in effect,
  //zero if Daylight Saving Time is not in effect, and less than zero if the information is not available.
  // convert hour to 24-hour
  //Note: for mktime(): Set Daylight Saving Time flag -1 to adjust automatically, +1 if DST, 0 if not DST

  // get elements from slot's time_t
  // (Note: gmtime() converts time since epoch to calendar time
  // expressed as Universal Coordinated Time)
//  struct tm slotData = {0};
//  if (localtime_r((time_t*)&slotData.timeDate, &slotData) == 0)
//    return;

  // diagnostic - print current slot
//  prt("Slot" + String(slotIndex) + "Time: ");
//  printLocalTime(slotData);

  // don't have seconds resolution here -
  // that's done in ProcessSecondResolutionTimeSlots()!
  // (if seconds is, say 50 and we call this routine -
  // then seconds advances to 0 - the minute will have been
  // incremented and no longer matches...)

  // returns 0 if match, 1 if event-time has passed by... 2 if yet to be...
  int compareVal = TSC.CompareTimeDate(timeDate, slotData.timeDate);

  // timestamps match?
  if (compareVal == 0){
    DoEvent(slotData.deviceAddr, slotData.deviceMode);
    prtln("Main Event! Slot index: " + String(slotIndex));

    // init the counters to 0 on first event trigger
    IRL.ResetCountersBySlot(slotIndex);

    // Check for any event-specific time-cycle parameters
    // uint8_t dutyCycleA, dutyCycleB, phase, perUnits, perMax, perVal;
    if (slotData.bIncludeCycleTiming)
      if (CheckEventSpecificTimeCycleParameters(&slotData))
        ResetPeriod();
  }
  else if (compareVal == 1){ // event already ocurred?
    // check to see if this event was out of date at InitRepeatList()
    // we don't want to start repeating on an event stored a year back,
    // so disable it...
    if (slotData.repeatMode == RPT_OFF || IRL.IsStaleBySlot(slotIndex)){
      if (TSC.DisableTimeSlot(slotIndex)) // disable the slot
        TSK.QueueTask(TASK_PAGE_REFRESH_REQUEST); // delay and tell P2.html to reload
    }
    else{
//      struct tm prevTimeInfo = {0};
//      if (localtime_r((time_t*)&g_prevDateTime, &prevTimeInfo) == 0)
//        return;

      // for repeating - everyCounter is set to 0 at the first DoEvent - here,
      // if t.repeatMode != RPT_OFF, we increment the counter and no longer
      // DoEvent if the counter > count. a count of 0 is intinite

      // if the unit we are repeating has changed (i.e. timeInfo.tm_hour != prevTimeInfo.tm_hour),
      // and the sub-units match our event timestamp, we re-trigger the event...
      if ((slotData.repeatMode == RPT_SECONDS && timeDate.second != g_prevDateTime.second) ||
          (slotData.repeatMode == RPT_MINUTES && timeDate.minute != g_prevDateTime.minute && SecMatch(timeDate, slotData)) ||
            (slotData.repeatMode == RPT_HOURS && timeDate.hour != g_prevDateTime.hour && MinSecMatch(timeDate, slotData)) ||
              (slotData.repeatMode == RPT_DAYS && timeDate.day != g_prevDateTime.day && MinSecHoursMatch(timeDate, slotData)) ||
                (slotData.repeatMode == RPT_WEEKS && timeDate.day != g_prevDateTime.day && MinSecHoursDayOfWeekMatch(timeDate, slotData)) ||
                  (slotData.repeatMode == RPT_MONTHLY && timeDate.month != g_prevDateTime.month && MinSecHoursDaysMatch(timeDate, slotData)) ||
                    (slotData.repeatMode == RPT_YEARS && timeDate.year > g_prevDateTime.year && MinSecHoursDaysMonthsMatch(timeDate, slotData)))
        ProcessEvent(slotIndex, slotData);
    }
  }
}

// returns true if we may want to call ResetPeriod()!;
bool TimeSlotsClass::CheckEventSpecificTimeCycleParameters(t_event* slotData){
  bool bChanged = false;;
  if (slotData->perVal != 0xff && slotData->perVal != g_perVals.perVal){
    g_perVals.perVal = slotData->perVal;
    bChanged = true;
  }
  if (slotData->dutyCycleA != 0xff && slotData->dutyCycleA != g_perVals.dutyCycleA){
    g_perVals.dutyCycleA = slotData->dutyCycleA;
    bChanged = true;
  }
  if (slotData->dutyCycleB != 0xff && slotData->dutyCycleB != g_perVals.dutyCycleB){
    g_perVals.dutyCycleB = slotData->dutyCycleB;
    bChanged = true;
  }
  if (slotData->dutyCycleC != 0xff && slotData->dutyCycleC != g_perVals.dutyCycleC){
    g_perVals.dutyCycleC = slotData->dutyCycleC;
    bChanged = true;
  }
  if (slotData->dutyCycleD != 0xff && slotData->dutyCycleD != g_perVals.dutyCycleD){
    g_perVals.dutyCycleD = slotData->dutyCycleD;
    bChanged = true;
  }
  if (slotData->phaseB != 0xff && slotData->phaseB != g_perVals.phaseB){
    g_perVals.phaseB = slotData->phaseB;
    g32_nextPhaseB = ComputePhaseB();
    bChanged = true;
  }
#if ENABLE_SSR_C_AND_D
  if (slotData->phaseC != 0xff && slotData->phaseC != g_perVals.phaseC){
    g_perVals.phaseC = slotData->phaseC;
    g32_nextPhaseC = ComputePhaseC();
    bChanged = true;
  }
  if (slotData->phaseD != 0xff && slotData->phaseD != g_perVals.phaseD){
    g_perVals.phaseD = slotData->phaseD;
    g32_nextPhaseD = ComputePhaseD();
    bChanged = true;
  }
#endif
  if (slotData->perUnits != 0xff && slotData->perUnits != g_perVals.perUnits){
    g_perVals.perUnits = slotData->perUnits;
    bChanged = true;
  }
  if (slotData->perMax != 0xffff && slotData->perMax != g_perVals.perMax){
    g_perVals.perMax = slotData->perMax;
    bChanged = true;
  }
  return bChanged;
}

bool TimeSlotsClass::SecMatch(t_time_date &timeDate, t_event &slotData){
  return timeDate.second == slotData.timeDate.second;
}

bool TimeSlotsClass::MinSecMatch(t_time_date &timeDate, t_event &slotData){
  return timeDate.minute == slotData.timeDate.minute && SecMatch(timeDate, slotData);
}

bool TimeSlotsClass::MinSecHoursMatch(t_time_date &timeDate, t_event &slotData){
  return timeDate.hour == slotData.timeDate.hour && MinSecMatch(timeDate, slotData);
}

bool TimeSlotsClass::MinSecHoursDaysMatch(t_time_date &timeDate, t_event &slotData){
  return timeDate.day == slotData.timeDate.day && MinSecHoursMatch(timeDate, slotData);
}

bool TimeSlotsClass::MinSecHoursDayOfWeekMatch(t_time_date &timeDate, t_event &slotData){
  return timeDate.dayOfWeek == slotData.timeDate.dayOfWeek && MinSecHoursMatch(timeDate, slotData);
}

bool TimeSlotsClass::MinSecHoursDaysMonthsMatch(t_time_date &timeDate, t_event &slotData){
  return timeDate.month == slotData.timeDate.month && MinSecHoursDaysMatch(timeDate, slotData);
}

void TimeSlotsClass::ProcessEvent(int slotIndex, t_event slotData){
  int ir_listIdx = IRL.FindIndexBySlot(slotIndex);
  if (ir_listIdx >= 0){
    // 0 = off (ignore) everyCount, 0 = repeat forever for repeatCount
    if (IRL.IncRepeatCounter(ir_listIdx)){
      DoEvent(slotData.deviceAddr, slotData.deviceMode);
      prtln("Repeat Event! Slot index: " + String(slotIndex));

      if (slotData.bIncludeCycleTiming && slotData.bCycleTimingInRepeats)
        if (CheckEventSpecificTimeCycleParameters(&slotData))
          ResetPeriod();

      // autodelete if not infinite repeat (0) and if finished repeating...
      if(IRL.GetRptCount(ir_listIdx) != 0 && IRL.GetRptCounter(ir_listIdx) >= slotData.repeatCount)
        if (TSC.DisableTimeSlot(slotIndex)) // disable the slot
          TSK.QueueTask(TASK_PAGE_REFRESH_REQUEST); // delay and tell P2.html to reload
    }
  }
}

void TimeSlotsClass::DoEvent(uint8_t deviceAddr, uint8_t deviceMode){
  // deviceAddr bit0=A, bit1=B, Etc.
  if (deviceAddr & 1){
    if (g8_ssr1ModeFromWeb != deviceMode){
      g8_ssr1ModeFromWeb = deviceMode;
      SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
      ResetPeriod();
    }
  }
  if (deviceAddr & 2){
    if (g8_ssr2ModeFromWeb != deviceMode){
      g8_ssr2ModeFromWeb = deviceMode;
      SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
      ResetPeriod();
    }
  }
#if ENABLE_SSR_C_AND_D
  if (deviceAddr & 4){
    if (g8_ssr3ModeFromWeb != deviceMode){
      g8_ssr3ModeFromWeb = deviceMode;
      SetSSRMode(GPOUT_SSR3, g8_ssr3ModeFromWeb);
      ResetPeriod();
    }
  }
  if (deviceAddr & 8){
    if (g8_ssr4ModeFromWeb != deviceMode){
      g8_ssr4ModeFromWeb = deviceMode;
      SetSSRMode(GPOUT_SSR4, g8_ssr4ModeFromWeb);
      ResetPeriod();
    }
  }
#endif
}

String TimeSlotsClass::PrintTimeSlots(){
  String sOut;
  sOut = "Total timeslots: " + String(g_slotCount);
  if (g_slotCount > 0){
    for (int ii = 0; ii < MAX_TIME_SLOTS; ii++){
      t_event slotData = {0};
      if (TSC.GetTimeSlot(ii, slotData)) // Get the time-slot into slotData by-reference
        sOut += "\nTimeSlot " + String(ii) + " = \"" + TimeSlotToSpaceSepString(slotData) + "\"";
    }
  }
  return sOut;
}
