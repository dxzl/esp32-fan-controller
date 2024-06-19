//#ifndef FanControllerH
//#define FanControllerH
#pragma once

// links for additional boards in Arduino IDE Preferences
// https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
// https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json

#include <Arduino.h>

#include <esp_system.h>
#include <esp_efuse.h>
#include <esp_efuse_table.h>

// Import required libraries
#include <ESPmDNS.h>
#include <MyPreferences.h>
#include <Update.h>
#include <AppleMIDI.h>

// note: AsyncTCP_SSL is available, AsyncHTTPSRequest_Generic is available and esp32_https_server is at
// https://github.com/fhessel/esp32_https_server 
// ESPAsyncWebServer has no working SSL version at this time... October 3, 2022
//#include <WiFiUdp.h>
//#include <AsyncUDP.h>
//#include <MIDI.h>
//#include <AsyncTCP.h>
//#include <WiFiClient.h>
//#include <WebServer.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>
//#define ARDUINOJSON_DECODE_UNICODE 0 // don't decode unicode escape sequences
//#include <ArduinoJson.h> // define before MsgPack.h
//#include <AsyncJson.h>
//#include "FS.h" // add file-system functions https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html

#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"

#include "CipherClass.h" // aes32-Encrypt library requires Seed_Arduino_mbedtls library
#include "B64Class.h"
#include "ValueListClass.h"
#include "RepeatListClass.h"
#include "MdnsListClass.h"
#include "PrefsClass.h"
#include "TimeSlotsClass.h"
#include "HttpMsgClass.h"

#include "FCUtils.h"
#include "FCWiFI.h"
#include "WSHandlers.h"
#include "HttpClientHandlers.h"
#include "Cmd.h"
#include "Tasks.h"
#include "Tests.h"

USING_NAMESPACE_APPLEMIDI

#define DTS_VERSION "Version 3.04 (June 17, 2024)"
#define ESP32_S3 false // set true if using ESP32 S3 board
#define PRINT_ON true // set true to enable status printing to console
#define RESET_PREFS false // set true to force clear on boot, then set back to false and rebuild...
#define RESET_WIFI  false // ""
#define RESET_SLOTS false // ""
#define FORCE_DEF_CIPHER_KEY false
#define FORCE_AP_ON false
#define FORCE_STA_ON false
#define HTTP_CLIENT_TEST_MODE false // send random HTTP strings to each mDNS-discovered remote unit
#define DISABLE_POTENTIOMETER true // prevent a system with no potentiometer present from causing value-changes

#define USE_UPDATE_LOGIN false // set true to require password entry after typing "c update" into index.html host-name field
#define UPDATE_USERID "dts7" // only when USE_UPDATE_LOGIN is true!
#define UPDATE_USERPW "1234567890" // only when USE_UPDATE_LOGIN is true!

#define WIFI_COUNTRY "US" // JP, CN
#define WIFI_MIN_CHANNEL 1
#define WIFI_MAX_CHANNEL 11 // 1-13 possible but max is 11 in USA!
// NOTE: see PrefsClass.h for WIFI_MAX_POWER_INIT

#define DEF_SSID "MyRouter"
#define DEF_PWD "MyRouterPass"
#define DEF_HOSTNAME "dts7"

// log directely into this device via access-point mode at 192.168.7.7
#define DEF_AP_SSID DEF_HOSTNAME
#define DEF_AP_PWD "1234567890"
#define DEF_AP_IP "192.168.7.7"
#define DEF_AP_GATEWAY "192.168.7.7"
#define DEF_AP_SUBNET_MASK "255.255.255.0"
#define MIN_AP_CHANNEL 1
#define MAX_AP_CHANNEL 11 // // 1-13 possible but max is 11 in USA!
#define MAX_AP_CLIENTS 1 // 1-4 allowed but for security allow only one
#define HIDE_AP_SSID 0 // set to 1 to hide
// also see the top of FanController.cpp for constant default strings!!!!!!!

#define MDNS_SVC "dts"
#define MDNS_SVCU "_dts"

// technically these can be 0-63 but better to keep them small for less processing overhead...
// and 0 is probably not advised...
#define FAILSAFE_TOKEN_1 4 // MAC from HTTP client send to the remote web-server
#define FAILSAFE_TOKEN_2 1 // CanRx Decode fail
#define FAILSAFE_TOKEN_3 3 // CanRx fail, text fail, param fail...
#define FAILSAFE_TOKEN_4 5 // IP address string
#define FAILSAFE_TOKEN_5 2 // MAC response from web-server to HTTP client callback

// most signifigant byte bit 0 is multicast bit - do not set
// most signifigant byte bit 1 is locally administered bit - set this.
// least signifigant three bytes are unique to manufacturer
#define DEF_MAC "" // not set we use chip's MAC... format 42:ad:f2:23:d0

// https://en.wikipedia.org/wiki/Year_2038_problem this esp32 system won't work past 2035 sometime it appears!
// Unix epoch is 00:00:00 UTC on 1 January 1970
// NOTE: we will just have to have this fixed and rebuild the code by 2034!!!!!!!!!!!!!
// time_t needs to be changed or the system-wide EPOCH year of 1900...
#define MAX_YEAR    2034 // June 20, 2035 is as high as works... so set to 2034
#define EPOCH_YEAR  1900
// this serves as a flag that "time has not been set"
// years less than this serves as a flag that Y2038 happened!
// I give two years to "live in the past" - for whatever reason... (this is 2020)!
#define DEF_YEAR    2016 // (note: don't think someone made a mistake and failed to set this!) I've seen this year - filled in by the system before...

// NOTE: if you want custom security for your particular implementation, you can change the order of letters in _HttpCommandTable[]
// in HttpMsgClass.h and you can change ENCODE_TABLE2 and ENCODE_TABLE3 in B64Class.h
// You will also want to change TOKEN_INIT in PrefsClass.h! Also change HTTP_ASYNCREQ and HTTP_ASYNCREQ_PARAM_COMMAND in WSHandlers.cpp.

// setting READ_WRITE_CUSTOM_BLK3_MAC true will write to BLK3 and permanently set efuse bits specified (presently written once).
// set it false to use original factory base MAC from BLK1, set true to use MAC shown below as base MAC.
// BE CAREFUL AS THESE CAN ONLY BE SET ONCE - MUST BE DIFFERENT FOR EACH ESP32 YOU HAVE!!!
#define READ_WRITE_CUSTOM_BLK3_MAC false
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

// at some point, these character-string limits might go up to 64 and 128...
#define MAXSSID     32
#define MAXPASS     64
#define MAXAPSSID   32
#define MAXAPPASS   64
#define MAXHOSTNAME 32

// limit length of text sent via CMtxt command!
#define MAXTXTLEN 80

// used for hnEncode()/hnDecode() for web-pages
#define MIN_SHIFT_COUNT 1
#define MAX_SHIFT_COUNT 12

#if ESP32_S3
  // left side (USB at bottom)
  // Pin 1 = 3.3V out
  // Pin 21 = 5V in
  // Pin 22 = Ground

  // right side (USB at bottom)
  // Pin 23, 24, 44 = Ground
  // Pin 31 = boot (MUST BE TIED TO 3.3V)!!!!!
  
  // don't use GPIO 15, 16, 19, 20, 43, 44 (3 USB ports)
  // GPIO 0, 3, 45, 46 (system bootstrap)

  #define LED_GREEN 50 // 0-255 (for tri-color built-in LED)
  #define CPU_FREQ 240 // 80MHz works ok for WiFi but may need 160MHz or 240MHz for WiFi Scans!

  // Solid-state relay outputs
  #define GPOUT_SSR1    1 // (pin 41 right) GPIO01 ADC1_0
  #define GPOUT_SSR2    2 // (pin 40 right) GPIO02 ADC1_1
  
  // Analog Input
  #define GPAIN_POT1    4 // (pin 4 left) GPIO04 ADC1_3

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_POT_MODE_SW1 5 // (pin 5/44 left) GPIO05 ADC1_4 Set pin to Vcc for POT1 Mode 1
  #define GPIN_POT_MODE_SW2 6 // (pin 6/44 left) GPIO06 ADC1_5 Set pin to Vcc for POT1 Mode 2
  
  #define GPBD_ONE_WIRE_BUS_DATA  8  // (pin 12) GPIO08 ADC1_7
  #define GPOUT_ONE_WIRE_BUS_CLK   9 // (pin 15) GPIO09 ADC1_8

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_WIFI_AP_SW      7 // (pin 7/44) GPIO07 ADC1_6 Set pin to Vcc for WiFi AP mode
  #define GPIN_WIFI_STA_SW    10 // (pin 16/44) GPIO10 ADC1_9 Set pin to Vcc for WiFi STA mode

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_SSR1_MODE_SW_MAN   11 // (pin 17/44) Set pin to Vcc for SSR_1 Manual ON
  #define GPIN_SSR1_MODE_SW_AUT   12 // (pin 18/44) Set pin to Vcc for SSR_1 Auto ON
  
  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_SSR2_MODE_SW_MAN   13 // (pin 19/44) Set pin to Vcc for SSR_2 Manual ON
  #define GPIN_SSR2_MODE_SW_AUT   14 // (pin 20/44) Set pin to Vcc for SSR_2 Auto ON

  #define GPOUT_SSR1_LED  35 // (pin 32 right) GPIO35
  #define GPOUT_SSR2_LED  36 // (pin 33 right) GPIO36
#else
  #define CPU_FREQ 160 // 80MHz works ok for WiFi but may need 160MHz or 240MHz for WiFi Scans!
  #define GPOUT_ONBOARD_LED 2

  // Solid-state relay outputs
  #define GPOUT_SSR1    32
  #define GPOUT_SSR2    23

  // Input only pins (on left top as usb port faces down, second pin down on left is GPI36)
  #define GPAIN_POT1    36 // ADC1_0 (2)
  //#define GPAIN_POT_2    39 // ADC1_3 (3)
  // the POT_3 center-pin on the custom PC-board is being used to solder a wire for a SPST POT-Mode switch...
  //#define GPAIN_POT_3    34 // ADC1_6 (4)
  #define GPIN_POT_MODE_SW 34 // ADC1_6 (4)
  //#define GPAIN_POT_4    35 // ADC1_7 (5)

// Data wire is plugged into GPIO 22 on the ESP32
//  #define GPBD_ONE_WIRE_BUS_DATA  22
//  #define GPOUT_ONE_WIRE_BUS_CLK   27

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_WIFI_AP_SW  18 // (pin 24/30) Set pin to Vcc for WiFi AP mode
  #define GPIN_WIFI_STA_SW 19 // (pin 25/30) Set pin to Vcc for WiFi STA mode
#endif

#define WIFI_SW_MODE_OFF  0
#define WIFI_SW_MODE_AP   1
#define WIFI_SW_MODE_STA  2

#define SERVER_PORT             80

#define TIME_SYNC_WAIT_TIME     10000 // time to wait for NTP time after connecting to wifi

#define MIN_PERIOD_TIMER        10 // 5 seconds

#define MANUAL_CLOCK_SET_DEBOUNCE_TIME 3 // 1 second units

#define SEND_HTTP_TIME_MIN      10 // 1-second resolution
#define SEND_HTTP_TIME_MAX      30

// we pick a random time from range below...
#define SEND_DEF_TOKEN_TIME_MIN (60*12) // 1-minute resolution 16-bits - 0.5 days
#define SEND_DEF_TOKEN_TIME_MAX (60*36) // 1-minute resolution 16-bits - 1.5 days

#define LED_EEPROM_FLASH_TIME   3   // .5 sec units (indicated a value saved to eeprom)
#define FIVE_SECOND_TIME        5   // one second units
#define THIRTY_SECOND_TIME      30  // one second units
#define POT_DEBOUNCE            150 // range 0-4095 (0V - 3.3V)

// this order can't change unless you change p2.html!
#define SSR_MODE_OFF  0
#define SSR_MODE_ON   1
#define SSR_MODE_AUTO 2

// independant cycle that pulses the relay off if on, on if off, or both
#define PULSE_MODE_OFF       0
#define PULSE_MODE_OFF_IF_ON 1
#define PULSE_MODE_ON_IF_OFF 2
#define PULSE_MODE_ON_OR_OFF 3

// repeat modes (set on p2.html web-page)
#define RPT_OFF      0
#define RPT_SECONDS  1
#define RPT_MINUTES  2
#define RPT_HOURS    3
#define RPT_DAYS     4
#define RPT_WEEKS    5
#define RPT_MONTHLY  6
#define RPT_YEARS    7

// hardware timer
#define HW_TIMER_FREQ 1000000 // use 1000000 for 1MHz
#define HW_TIMER_PERIOD (HW_TIMER_FREQ/4) // .25 sec

// g8_ledFlashTimer .25ms resolution
#define LED_FASTFLASH_TIME 1
#define LED_SLOWFLASH_TIME 4

// g8_ledFlashCounter
#define LED_PAUSE_COUNT    10 // pause time between digit flash-sequences

// g8_ledSeqState
#define LEDSEQ_ENDED       0
#define LEDSEQ_FLASHING    1
#define LEDSEQ_PAUSED      2

// g8_ledMode, g8_ledSaveMode
#define g8_ledMode_OFF        0
#define g8_ledMode_ON         1
#define g8_ledMode_SLOWFLASH  2
#define g8_ledMode_FASTFLASH  3
#define g8_ledMode_PAUSED     4

#define LED_EEPROM_FLASH_TIME   3 // .5 sec units (indicated a value saved to eeprom)

#define MAX_TIME_SLOTS          100 // EE_SLOT_xxx (xxx is 000 to 099)
#define MAX_RECORD_SIZE         300
#define MAX_FILE_SIZE           (2*MAX_TIME_SLOTS*MAX_RECORD_SIZE) // upper limit for incoming text-file; allow for comment-lines!\r\n"
#define EVENT_LENGTH_SEC        29 // "umtrdssttX2020-06-05T23:59:59"
#define EVENT_LENGTH_NOSEC      26 // "umtrdssttX2020-06-05T23:59"
#define MAX_LOCKPASS_LENGTH     32

#define T_ONE_HOUR (2*60*60)

#define SER_IN_MAX             200

// NOTE: "spiffs" in the filename differentiates a SPIFFS data .bin file from a main-program .bin file
// during "over-the-air" programming! The web-server HTML files and javascript are in the fc.spiffs.bin file
// and the main program is in fc.bin (or it might be in DTS_SMART_FAN_CONTROLLER.ino.esp32.bin)
#define OTA_UPDATE_SPIFFS_VS_PGM_ID "spiffs"

struct PerVals {
  uint8_t dutyCycleA = 0xff, dutyCycleB = 0xff, phase = 0xff; // saved in Preferences (units are %)
  uint8_t perUnits = 0xff, perVal = 0xff; // perUnits is an index to index.html select options
  uint16_t perMax = 0xffff;
};

// we count "on" events up to the max interval in .5 sec units (determined by perMax and perUnits)
// and also time duration "on" within that interval in .5 sec units which can be converted to a percentage on (duty-cycle).
struct Stats {
  uint32_t HalfSecondCounter, HalfSecondCount;
  uint32_t AOnCounter, BOnCounter, DConA, DConB;
  uint32_t AOnPrevCount, BOnPrevCount, PrevDConA, PrevDConB;
};

// function prototypes
void SSR1On(uint32_t iPeriod);
void SSR2On(uint32_t iPeriod);
void serialEvent(); // event hndler for Serial
void IRAM_ATTR onTimer();
String SetTimeDate(String sVal);
void dnsAndServerStart();
void dnsAndServerStop();
void print_wakeup_reason();
void notFound(AsyncWebServerRequest *request);
void SetupAndStartHardwareTimeInterrupt();
void HardwareTimerRestart(hw_timer_t * timer);
time_t DoTimeSyncOneSecondStuff(time_t now);
void DoTimeSyncOneSecondStuff(void);
String getSctMinMaxAsJS();
void InitStats();
void ClearStatCounters();
uint32_t ComputePeriod(uint8_t perVal, uint16_t perMax, uint8_t perUnits);
uint32_t ComputePhase();
uint32_t GetTimeInterval(uint16_t perMax, uint8_t perUnits);
void stopMIDI();
void startMIDI();
bool SendHttpReq(int idx);
void TaskMidiChan(); // has to be in FanController.ino!
//String wsTemplateProc(const String& var);
//#endif

extern int g_slotCount, g_prevMdnsCount;
extern int g_defToken, g_pendingDefToken, g_oldDefToken;
extern int g_sct, g_minSct, g_maxSct;
extern uint8_t g8_maxPower, g8_midiNoteA, g8_midiNoteB, g8_midiChan;
extern uint8_t g8_ledFlashCount, g8_ledFlashCounter, g8_ledDigitCounter, g8_ledSaveMode, g8_ledMode, g8_ledSeqState;
extern uint8_t g8_quarterSecondTimer, g8_fiveSecondTimer, g8_thirtySecondTimer, g8_ledFlashTimer, g8_clockSetDebounceTimer, g8_lockCount;
extern uint8_t g8_digitArray[];
extern uint16_t g16_pot1Value, g16_oldpot1Value; // variable for storing the potentiometer value
extern uint16_t g16_oldMacLastTwo, g16_unlockCounter, g16_tokenSyncTimer, g16_sendDefTokenTimer;
extern uint16_t g16_sendDefTokenTime, g16_sendHttpTimer, g16_asyncHttpIndex, g16_oddEvenCounter;
extern uint32_t g32_periodTimer, g32_savePeriod, g32_dutyCycleTimerA, g32_dutyCycleTimerB, g32_phaseTimer, g32_nextPhase;
extern String g_sHostName, g_sSSID, g_sApSSID, g_sKey, g_sMac, g_sLabelA, g_sLabelB, g_sSerIn, g_text;

extern bool g_bOldWiFiApSwOn, g_bOldWiFiStaSwOn;

#if ESP32_S3
extern bool g_bOldPotModeSw1On, g_bOldPotModeSw2On;
extern bool g_bOldSsr1ModeManSwOn, g_bOldSsr1ModeAutSwOn;
extern bool g_bOldSsr2ModeManSwOn, g_bOldSsr2ModeAutSwOn;
extern uint8_t g8_ssr1ModeFromSwitch, g8_ssr2ModeFromSwitch;
#else
extern bool g_bOldPotModeSwOn;
#endif

extern uint8_t g8_potModeFromSwitch, g8_wifiModeFromSwitch;

extern uint8_t g8_ssr1ModeFromWeb, g8_ssr2ModeFromWeb;

extern bool g_bWiFiConnected, g_bWiFiConnecting, g_bSoftAP, g_bMdnsOn, g_bWiFiDisabled, g_bResetOrPowerLoss, g_bTellP2WebPageToReload;
extern bool g_bManualTimeWasSet, g_bWiFiTimeWasSet, g_bValidated, g_bRequestManualTimeSync, g_bRequestWiFiTimeSync, g_bMidiConnected;
extern bool g_bSsr1On, g_bSsr2On, g_bOldSsr1On, g_bOldSsr2On, g_bTest, g_bLedOn;
extern bool g_bSyncRx, g_bSyncTx, g_bSyncCycle, g_bSyncToken, g_bSyncTime, g_bSyncEncrypt, g_bSyncMaster;

// cycle pulse-off
extern uint8_t g8_pulseModeA, g8_pulseModeB;
extern uint8_t g8_pulseWidthTimerA, g8_pulseWidthA, g8_pulseMinWidthA, g8_pulseMaxWidthA;
extern uint8_t g8_pulseWidthTimerB, g8_pulseWidthB, g8_pulseMinWidthB, g8_pulseMaxWidthB;
extern uint16_t g16_pulsePeriodTimerA, g16_pulsePeriodA, g16_pulseMinPeriodA, g16_pulseMaxPeriodA;
extern uint16_t g16_pulsePeriodTimerB, g16_pulsePeriodB, g16_pulseMinPeriodB, g16_pulseMaxPeriodB;

extern time_t g_prevNow;
extern Preferences PF;
extern t_time_date g_prevDateTime;
extern IPAddress g_httpTxIP;
extern PerVals g_perVals, g_oldPerVals;
extern Stats g_stats;

extern const char EE_SLOT_PREFIX[], EE_HOSTNAME[], EE_MAC[], EE_SSID[], EE_OLDSSID[], EE_PWD[], EE_OLDPWD[], EE_APSSID[], EE_OLDAPSSID[], EE_APPWD[], EE_OLDAPPWD[], EE_LOCKPASS[], EE_LOCKCOUNT[];
extern const char EE_PERMAX[], EE_PERUNITS[], EE_PERVAL[], EE_DC_A[], EE_DC_B[], EE_PHASE[], EE_RELAY_A[], EE_RELAY_B[], EE_MIDICHAN[], EE_MIDINOTE_A[], EE_MIDINOTE_B[];
extern const char EE_SYNC[], EE_WIFI_DIS[], EE_LABEL_A[], EE_LABEL_B[], EE_TOKEN[], EE_MAX_POWER[], EE_CIPKEY[];
extern const char EE_PULSE_OFF_MODE_A[], EE_PULSE_MINWID_A[], EE_PULSE_MAXWID_A[], EE_PULSE_MINPER_A[], EE_PULSE_MAXPER_A[];
extern const char EE_PULSE_OFF_MODE_B[], EE_PULSE_MINWID_B[], EE_PULSE_MAXWID_B[], EE_PULSE_MINPER_B[], EE_PULSE_MAXPER_B[];

extern const char OBFUSCATE_STR[], SC_MAC_RANDOM[];
