#ifndef TimeSlotsClassH
#define TimeSlotsClassH

#include <Arduino.h>

typedef struct{
  uint8_t dayOfWeek; // 0=Sunday, 1=Monday... 6=Saturday
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t day;
  uint8_t month;
  uint16_t year;
} t_time_date;

typedef struct{
  // off=0,sec,min,hrs,day,wek,mon,yrs; OFF, ON, AUTO; A, B, AB
  uint8_t repeatMode, deviceMode, deviceAddr; // 0xff = unset
  // dutyCycleA, dutyCycleB, phase are 0-100%
  // 0xff = unset (perUnits and perMax are indicies to html select widget!)
  uint8_t dutyCycleA, dutyCycleB, phase, perUnits, perVal;
  uint16_t repeatCount, everyCount, perMax; // 0xffff = unset
  t_time_date timeDate;
  bool bEnable, bIncludeCycleTiming, bCycleTimingInRepeats; // true if enabled
} t_event;

class TimeSlotsClass{
  private:

    void DoEvent(uint8_t deviceAddr, uint8_t deviceMode);
    void ProcessEvent(int slotIndex, t_event slotData);
    bool MinSecHoursDaysMonthsMatch(t_time_date &timeDate, t_event &slotData);
    bool MinSecHoursDayOfWeekMatch(t_time_date &timeDate, t_event &slotData);
    bool MinSecHoursDaysMatch(t_time_date &timeDate, t_event &slotData);
    bool MinSecHoursMatch(t_time_date &timeDate, t_event &slotData);
    bool MinSecMatch(t_time_date &timeDate, t_event &slotData);
    bool SecMatch(t_time_date &timeDate, t_event &slotData);
    bool CheckEventSpecificTimeCycleParameters(t_event* slotData);
    void ProcessTimeSlot(int slotIndex, t_time_date timeDate, t_event slotData);
    bool ParseDate(String &s, int16_t &iMonth, int16_t &iDay, int16_t &iYear);
    bool ParseTime(String &s, int16_t &iHour, int16_t &iMinute, int16_t &iSecond);
    bool ParseDevAddressAndMode(String &s, int16_t &iDevAddr, int16_t &iDevMode);
    bool ParseRepeatMode(String &s, int16_t &iRepeatMode);

  public:
    bool StringToTimeSlot(String sIn, t_event &slotData);
    void ProcessMinuteResolutionTimeSlots();
    void ProcessSecondResolutionTimeSlots();
    t_time_date CopyTmToTtimeDate(struct tm &tm);
    int CompareTimeDate(t_time_date &timeDate, t_time_date &slotTimeDate);
    int MyDayOfWeek(int d, int m, int y);
    int InitSecondsList(int slotCount);
    int InitRepeatList(int slotCount);
    bool EraseTimeSlots();
    int CountFullTimeSlots();
    String GetSlotNumAsString(int val);
    bool DisableTimeSlot(int slotIndex);
    bool EnableTimeSlot(int slotIndex, bool bEnable=true);
    bool AddTimeSlot(t_event &slotData, bool bVerbose=false);
    bool DeleteTimeSlot(int slotIndex);
    bool GetTimeSlot(int slotIndex, t_event &t);
    bool PutTimeSlot(int slotIndex, t_event &t);
    bool IsTimeSlotEmpty(int slotIndex);
    int FindFirstEmptyTimeSlot();
    int FindNextFullTimeSlot(int iStart);
    String TimeSlotToSpaceSepString(t_event &t);
    String TimeSlotToCommaSepString(int idx, t_event &t);
    t_event StringToTimeSlot(String s);
    
};

#endif

extern TimeSlotsClass TSC;
