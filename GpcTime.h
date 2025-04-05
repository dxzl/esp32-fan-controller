#ifndef GpcTimeH
#define GpcTimeH

#include <Arduino.h>
#include <esp_sntp.h>

bool SetGlobalSntpInterval(int iInterval);
String SetTimeDate(String& sVal, bool bSendTimeDateToRemotes);
String TimeToString(bool b24hr);
int SendTimeDateToAllRemotes();
bool SetTimeManually(int myYear, int myMonth, int myDay, int myHour, int myMinute, int mySecond);
bool InitTime();
void InitOrRestartSNTP();
void StopSNTP();
void SetSntpSyncInterval();
void SetSntpTimezoneAndStart();
struct tm* ReadInternalTime(time_t* pEepochSeconds, struct tm* pTm);
int Make12Hour(int iHour, bool &pmFlag);
void printLocalTime(struct tm &timeInfo);
#endif
