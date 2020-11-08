#ifndef FCUtilsH
#define FCUtilsH

#include <Arduino.h>
#include "esp_wifi.h"

typedef struct
{
  uint8_t dayOfWeek; // 0=Sunday, 1=Monday... 6=Saturday
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t day;
  uint8_t month;
  uint16_t year;
} t_time_date;

typedef struct
{
  // off=0,sec,min,hrs,day,wek,mon,yrs; OFF, ON, AUTO; A, B, AB
  uint8_t repeatMode, deviceMode, deviceAddr; // 0xff = unset
  // dutyCycleA, dutyCycleB, phase are 0-100%
  // 0xff = unset (perUnits and perMax are indicies to html select widget!)
  uint8_t dutyCycleA, dutyCycleB, phase, perUnits, perMax;
  uint16_t repeatCount, everyCount, perVal; // 0xffff = unset
  t_time_date timeDate;
  bool bEnable, bIncludeCycleTiming, bCycleTimingInRepeats; // true if enabled
} t_event;

String ZeroPad(byte val);
String GetStringIP();

void ReadPot1();
void ReadModeSwitch();

void RestoreDefaultSsidAndPwd();
void ToggelOldSsidAndPwd();

String SsrModeToString(byte ssrMode);
void SetState(byte val, String s);
void SetState(byte val, byte ssrMode);

bool EraseTimeSlots();
int CountFullTimeSlots();
String GetSlotNumAsString(int val);
bool DisableTimeSlot(int slotIndex);
bool EnableTimeSlot(int slotIndex, bool bEnable=true);
bool AddTimeSlot(t_event &slotData, bool bVerbose=true);
bool DeleteTimeSlot(int slotIndex);
bool GetTimeSlot(int slotIndex, t_event &t);
bool PutTimeSlot(int slotIndex, t_event &t);
int FindFirstEmptyTimeSlot();
int FindNextFullTimeSlot(int iStart);

bool ErasePreferences();
void PutPreferenceU16(const char* s, uint16_t val);
void PutPreferenceString(const char* s, String val);
String GetPreferenceString(const char* s, const char* eeDefault);
void PutPreference(const char* s, byte val);

bool SetTimeManually(int myYear, int myMonth, int myDay, int myHour, int myMinute, int mySecond);
bool InitTimeManually();
struct tm* ReadInternalTime(time_t* pEepochSeconds, struct tm* pTm);
void printLocalTime(struct tm &timeInfo);
int CompareTimeDate(t_time_date &timeDate, t_time_date &slotTimeDate);
t_time_date CopyTmToTtimeDate(struct tm &tm);
void ResetPeriod();
//void WiFiApInfo();
String WiFiScan(String sInit);
String translateEncryptionType(wifi_auth_mode_t encryptionType);

int InitSecondsList(int slotCount);
int InitRepeatList(int slotCount);

int MyDayOfWeek(int d, int m, int y);

void prtln(String s);
void prt(String s);

#endif
