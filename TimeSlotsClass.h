#ifndef TimeSlotsClassH
#define TimeSlotsClassH

#include <Arduino.h>

// TimeSlot field chars
#define TSC_YES                      "y"
#define TSC_OFF                      "off"
#define TSC_INF                      "inf"
#define TSC_REPEAT_MODE              't'
#define TSC_REPEAT_COUNT             'r'
#define TSC_EVERY_COUNT              'e'
#define TSC_INCLUDE_CYCLE_TIMING     'i'
#define TSC_CYCLE_TIMING_IN_REPEATS  'c'
#define TSC_DUTY_CYCLE_A             'a'
#define TSC_DUTY_CYCLE_B             'b'
#define TSC_PHASE                    'p'
#define TSC_UNITS                    'u'
#define TSC_PERMAX                   'm'
#define TSC_PERVAL                   'v'

// repeat modes (set on p2.html web-page)
#define RPT_OFF      0
#define RPT_SECONDS  1
#define RPT_MINUTES  2
#define RPT_HOURS    3
#define RPT_DAYS     4
#define RPT_WEEKS    5
#define RPT_MONTHLY  6
#define RPT_YEARS    7

#define MAX_TIME_SLOTS          100 // EE_SLOT_xxx (xxx is 000 to 099)
#define MAX_RECORD_SIZE         300
#define MAX_FILE_SIZE           (2*MAX_TIME_SLOTS*MAX_RECORD_SIZE) // upper limit for incoming text-file; allow for comment-lines!\r\n"
#define EVENT_LENGTH_SEC        29 // "umtrdssttX2020-06-05T23:59:59"
#define EVENT_LENGTH_NOSEC      26 // "umtrdssttX2020-06-05T23:59"

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
    String PrintTimeSlots();
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
//    int GetSlotIndexFromName(String sName);
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
