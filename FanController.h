#ifndef FanControllerH
#define FanControllerH

#include <Arduino.h>

// Import required libraries
#include <WiFi.h>

//#include <AsyncTCP.h>
//#include <WiFiClient.h>
//#include <WebServer.h>
#include <ESPmDNS.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>

// I modified preferences to add usedEntries (zip is in the "stuff" folder,
// put it in Arduino libraries folder
#include <MyPreferences.h>
#include <Update.h>

// Import library headers
//#include "FS.h" // add file-system functions https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "IndexValueList.h"
#include "IndexRepeatList.h"
#include "FCUtils.h"

#define DTS_VERSION "Version 1.79 (July 21, 2021)"
#define PRINT_ON true // set true to enable status printing to console
#define CLEAR_PREFS false
#define CLEAR_SLOTS false
#define FORCE_AP_ON false // can also tie SW_SOFT_AP GPI34 to 3.3V for AP (ground it for router-mode)
#define USE_UPDATE_LOGIN false // set true to require password entry after typing "c update" into index.html host-name field

// setting READ_WRITE_CUSTOM_BLK3_MAC true will write to BLK3 and permanently set efuse bits specified (presently written once).
// set it false to use original factory base MAC from BLK1, set true to use MAC shown below as base MAC.
// BE CAREFUL AS THESE CAN ONLY BE SET ONCE - MUST BE DIFFERENT FOR EACH ESP32 YOU HAVE!!!
#define READ_WRITE_CUSTOM_BLK3_MAC false
#define BLK3_VER  0x0a // version next 1a
#define BLK3_MAC0 0xe6
#define BLK3_MAC1 0xcd
#define BLK3_MAC2 0x43 // next 47
#define BLK3_MAC3 0xa3 // next ac
#define BLK3_MAC4 0x6e
#define BLK3_MAC5 0xb5

//----------- experimental - these are relevant only if READ_WRITE_CUSTOM_BLK3_MAC is set true ----------------
// efuse bits are all 0 from factory and can only be set once - once set, say for a serial-number (custom MAC address), then write-protect the bits that are still 0!
// use FORCE_NEW_EFUSE_BITS_ON true one time if READ_WRITE_CUSTOM_BLK3_MAC is true and you want to set new bits that are still 0
#define FORCE_NEW_EFUSE_BITS_ON false // set true to force new bits to be set (if not yet write-protected; you can't unset bits that are already set!)
#define WRITE_PROTECT_BLK3 false // set true and run once to write protect BLK3 data (presently NOT write-protected!)
//-------------------------------------------------------------------------------------------------------------

#define UPLOAD_USERID  "dts7"
#define UPLOAD_USERPW  "1234567890"

#define ERASE_DATA_CONFIRM "hsi83NSehaL9Wht"

#define WIFI_COUNTRY "US" // JP, CN
#define WIFI_MAX_CHANNEL 11 // max is 11 in USA!
#define AP_MAX_CHANNEL 11 // 1-13
#define MAX_AP_CLIENTS 1 // 1-4 allowed but for security allow only one

// used for hnDecode() for web-pages
#define MIN_SHIFT_COUNT 1
#define MAX_SHIFT_COUNT 15

#define CPU_FREQ 160 // 80MHz works ok for WiFi but may need 160MHz or 240MHz for WiFi Scans!

// Data wire is plugged into pin 22 on the ESP32
#define ONE_WIRE_BUS 22 // pin 22
//#define MY_INPUT_PULLDOWN 0x09

// Reset SSID and PWD to defaults if GPIO0 pressed on powerup (same as BOOT button)
// (User LED is GPIO2 - turn that on and off if we restored ssid/pwd)
// (Power LED "might" be on GPIO1)
//#define BTN_RESTORE_SSID_PWD    0 // NOTE: sadly, I can't get this pin to work - it's tied in as BOOT
#define ONBOARD_LED_GPIO2       2

// Input only pins (on left top as usb port faces down, second pin down on left is GPI36)
#define POT_1                   36 // ADC1_0 (2)
#define POT_2                   39 // ADC1_3 (3)
// POT_3 is being used as a switch to boot in softAP mode using
// SPDT no-center-off switch with outer pins tied to 3V3 and ground
// In future, make this a center off DPDT tied to two other GPIO pins - the POT
// pins are input only and I had trouble adding internal pullup/pulldown!
//#define POT_3                   34 // ADC1_6 (4)
#define SW_SOFT_AP              34
#define POT_4                   35 // ADC1_7 (5)

// Inputs with pulldowns (3-states 0,0 1,0 0,1)
#define SW_1                    18
#define SW_2                    19

#define ONE_WIRE_BUS_GPIO27     27

// Outputs
#define SSR_1                   32
#define SSR_2                   23

#define SERVER_PORT             80

#define TIME_SYNC_WAIT_TIME     10000 // time to wait for NTP time after connecting to wifi

#define MANUAL_CLOCK_SET_DEBOUNCE_TIME 3 // 1 second units

#define LED_EEPROM_FLASH_TIME   3 // .5 sec units (indicated a value saved to eeprom)
#define FIVE_SECOND_TIME        5 // one second units
#define ONE_MINUTE_TIME         (60/FIVE_SECOND_TIME) // 5-second units (12)
#define TASK_TIME               2 // .25 second (.25 sec incerments)
#define POT_DEBOUNCE            100 // range 0-4095 (0V - 3.3V)

#define TASK_PERMAX       0
#define TASK_PERUNITS     1
#define TASK_PERVAL       2
#define TASK_PHASE        3
#define TASK_DCA          4
#define TASK_DCB          5
#define TASK_DCA_DCB      6
#define TASK_RELAY_A      7
#define TASK_RELAY_B      8
#define TASK_HOSTNAME     9
#define TASK_RECONNECT    10
#define TASK_MAC          11
#define TASK_RESET_PARMS  12
#define TASK_RESET_SLOTS  13
#define TASK_TOGGLE       14
#define TASK_RESTORE      15
#define TASK_PAGE_REFRESH_REQUEST 16
#define TASK_WIFI_CONNECT 17
#define TASK_MIDICHAN     18
#define TASK_MIDINOTE_A   19
#define TASK_MIDINOTE_B   20
#define TASK_REBOOT       21

// this order can't change unless you change p2.html!
#define SSR_MODE_OFF  0
#define SSR_MODE_ON   1
#define SSR_MODE_AUTO 2

// repeat modes (set on p2.html web-page)
#define RPT_OFF      0
#define RPT_SECONDS  1
#define RPT_MINUTES  2
#define RPT_HOURS    3
#define RPT_DAYS     4
#define RPT_WEEKS    5
#define RPT_MONTHLY  6
#define RPT_YEARS    7

// hardware timers (only 0 used)
#define TIMER_0 0
#define TIMER_1 1
#define TIMER_2 2
#define TIMER_3 3

// ledFlashTimer .25ms resolution
#define LED_FASTFLASH_TIME 1
#define LED_SLOWFLASH_TIME 4

// ledFlashCounter
#define LED_PAUSE_COUNT    10 // pause time between digit flash-sequences

// ledSeqState
#define LEDSEQ_ENDED       0
#define LEDSEQ_FLASHING    1
#define LEDSEQ_PAUSED      2

// ledMode, ledSaveMode
#define LEDMODE_OFF        0
#define LEDMODE_ON         1
#define LEDMODE_SLOWFLASH  2
#define LEDMODE_FASTFLASH  3
#define LEDMODE_PAUSED     4

#define PERIOD_MIN              0 // %
#define PERIOD_MAX              100 // 0 = random mode
#define PHASE_MIN               0 // %
#define PHASE_MAX               100 // 0 = random mode
#define DUTY_CYCLE_MIN          0 // %
#define DUTY_CYCLE_MAX          100 // 0 = random mode
#define MIN_RAND_PERCENT_DUTY_CYCLE 20 // smallest time-on is 20% of period when in random mode!

#define PERUNITS_INIT           1 // 0= 1/2 sec, 1=sec, 2=min, 3=hrs
#define PERMAX_INIT             2 // 0=15, 1=30, 2=60, 3=120... 9=7,680
#define PERIOD_INIT             50 // percent offset of period (0-100)
#define PHASE_INIT              50 // percent offset of period (0-100)
#define DUTY_CYCLE_A_INIT       50 // percent on (0-100)
#define DUTY_CYCLE_B_INIT       50 // percent on (0-100)

#define LOCKCOUNT_INIT          -1 // unlocked
#define LOCKPASS_INIT           "****"
#define MIDICHAN_INIT           MIDICHAN_OFF // off
#define MIDINOTE_A_INIT         60 // middle C
#define MIDINOTE_B_INIT         62 // middle D
#define MIDINOTE_ALL            128 // all notes
#define MIDICHAN_ALL            0 // all channels
#define MIDICHAN_OFF            255 // no channels
#define SSR1_MODE_INIT          SSR_MODE_AUTO // 0 = OFF, 1 = ON, 2 = AUTO
#define SSR2_MODE_INIT          SSR_MODE_AUTO // 0 = OFF, 1 = ON, 2 = AUTO

#define LED_EEPROM_FLASH_TIME   3 // .5 sec units (indicated a value saved to eeprom)

#define MAX_TIME_SLOTS          100 // EE_SLOT_xxx (xxx is 000 to 099)
#define MAX_RECORD_SIZE         300 
#define MAX_FILE_SIZE           (2*MAX_TIME_SLOTS*MAX_RECORD_SIZE) // upper limit for incoming text-file; allow for comment-lines!\r\n"
#define EVENT_LENGTH_SEC        29 // "umtrdssttX2020-06-05T23:59:59"
#define EVENT_LENGTH_NOSEC      26 // "umtrdssttX2020-06-05T23:59"
#define MAX_LOCKPASS_LENGTH     32

// https://en.wikipedia.org/wiki/Year_2038_problem this esp32 system won't work past 2035 sometime it appears!
// Unix epoch is 00:00:00 UTC on 1 January 1970
// NOTE: we will just have to have this fixed and rebuild the code by 2034!!!!!!!!!!!!!
// time_t needs to be changed or the system-wide EPOCH year of 1900...
#define MAX_YEAR                2034 // June 20, 2035 is as high as works... so set to 2034
#define EPOCH_YEAR              1900
// this serves as a flag that "time has not been set"
// years less than this serves as a flag that Y2038 happened!
// I give two years to "live in the past" - for whatever reason... (this is 2020)!
#define DEFAULT_YEAR            2016 // (note: don't think someone made a mistake and failed to set this!) I've seen this year - filled in by the system before...

// not used at present - way to sleep for X seconds then reset, keeping RTC variables
#define uS_TO_S_FACTOR 1000000  //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  5        //Time ESP32 will go to sleep (in seconds)// this is when we don't hook into a router from here, but rather

// log directely into this device
#define DEFAULT_SOFTAP_IP       7 // 192.168.7.7 - (1 part)
#define IP_MIDDLE               7 // 192.168.7.7 - (4 part)

#define EE_PREFS_NAMESPACE "dts-fc01"
#define EE_SLOTS_NAMESPACE "dts-fc02"

#define T_ONE_HOUR (2*60*60)

// function prototypes
void IRAM_ATTR onTimer();
void FlashSequencer(bool bStart=false);
void FlashLED();
void SetWiFiHostName(AsyncWebServerRequest* &request, String &s, String &cmd);
void ProcessCommand(AsyncWebServerRequest* &request, String &s, String &cmd);
void WiFiMonitorConnection(bool bDisconnect=false, bool bEraseOldCredentials=false);
void WiFiDisconnect(bool bEraseOldCredentials);
void WiFiStartAP(bool bDisconnect=false, bool bEraseOldCredentials=false);
void printWiFiEventDetails(WiFiEvent_t event);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
//void WiFiEvent(WiFiEvent_t event);
void dnsAndServerStart(bool bDisconnect=false);
void GetPreferences();
//String hnDecode(String sIn, int &errorCode);
void print_wakeup_reason();
void notFound(AsyncWebServerRequest *request);
String processor(const String& var);
void SetupAndStartHardwareTimeInterrupt();
void HardwareTimerRestart(hw_timer_t * timer);
time_t DoTimeSyncOneSecondStuff(time_t now);
void DoTimeSyncOneSecondStuff(void);
void InitrStats();
void ClearStatCounters();
uint32_t GetTimeInterval(uint8_t perMax, uint8_t perUnits);
uint32_t ComputePeriod(uint8_t perVal, uint8_t perMax, uint8_t perUnits);
//uint32_t ComputeMaxPeriod(uint8_t perMax, uint8_t perUnits);
uint32_t DecodePerMax(uint8_t perMax);
bool CheckEventSpecificTimeCycleParameters(t_event* slotData);
bool ParseIncommingTimeEvent(String sIn);
bool ParseRepeatMode(String &s, int16_t &iRepeatMode);
bool ParseDevAddressAndMode(String &s, int16_t &iDevAddr, int16_t &iDevMode);
bool ParseDate(String &s, int16_t &iMonth, int16_t &iDay, int16_t &iYear);
bool ParseTime(String &s, int16_t &iHour, int16_t &iMinute, int16_t &iSecond);
bool IsLockedAlertGetPlain(AsyncWebServerRequest *request, bool bAllowInAP=false);
bool IsLockedAlertGet(AsyncWebServerRequest *request, String sReloadUrl, bool bAllowInAP=false);
bool IsLockedAlertPost(AsyncWebServerRequest *request, bool bAllowInAP=false);
bool IsLocked();

#endif

//extern const char TIMEZONE[], NTP_SERVER1[], NTP_SERVER2[], VERSION_STR[], WEB_PAGE_INDEX[], WEB_PAGE_P1[], WEB_PAGE_P2[], DEFAULT_MYUSERID[], DEFAULT_MYUSERPWD[];
// these are 15 chars max!
extern const char EE_SLOT_PREFIX[], EE_PERMAX[], EE_PERUNITS[], EE_PERVAL[], EE_DC_A[], EE_DC_B[], EE_PHASE[], EE_RELAY_A[], EE_RELAY_B[];
extern const char EE_HOSTNAME[], EE_SSID[], EE_OLDSSID[], EE_PWD[], EE_OLDPWD[], EE_LOCKCOUNT[], EE_LOCKPASS[];

extern const char DEFAULT_SOFTAP_SSID[], DEFAULT_SOFTAP_PWD[], DEFAULT_HOSTNAME[], DEFAULT_SSID[], DEFAULT_PWD[], OBFUSCATE_STR[];

extern int m_slotCount;
extern uint8_t sw1Value, sw2Value, oldSw1Value, oldSw2Value;
extern uint8_t nvSsrMode1, nvSsrMode2, m_taskMode;
extern uint8_t digitArray[];
extern uint16_t pot1Value, oldPot1Value;
extern bool bManualTimeWasSet, bWiFiTimeWasSet, bSoftAP, bOldApSwOn;
extern bool bRequestManualTimeSync, bRequestWiFiTimeSync;
extern uint16_t dutyCycleTimerA, dutyCycleTimerB, periodTimer, savePeriod, phaseTimer;
extern uint16_t m_taskTimer, m_taskData;
extern uint8_t dutyCycleA, dutyCycleB, phase; // saved in Preferences (units are %)
extern uint8_t perUnits, perMax, perVal; // perUnits and perMax are indexes to index.html select options
extern uint8_t ledFlashTimer, clockSetDebounceTimer, m_lockCount;
extern time_t m_prevNow;
extern t_time_date m_prevDateTime;
extern String ledState, ssr1State, ssr2State, swState, apSwState, inputMessage, inputParam, hostName, m_ssid;
extern Preferences preferences;
extern uint32_t statsAOnCounter, statsBOnCounter;
