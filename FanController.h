//#ifndef FanControllerH
//#define FanControllerH
#pragma once

// needed by AsyncHTTPRequest_Generic.h library
#define ESP32 true

#include <Arduino.h>

#include <esp_system.h>
#include <esp_efuse.h>
#include <esp_efuse_table.h>

// Import required libraries
#include <ESPmDNS.h>
#include <MyPreferences.h> // custom change to disable error-logging in getBytesLength()
#include <Update.h>

// NOTE: if you set this true you will need to use the the bigger partition scheme and lose OTA programming
// for the older ESP32 with 4MB flash. When true, you can control the outputs with a midi-keyboard.
// If this is the 8MB flash (ESP32 S3), you can set this true!
//
// NOTE: 8/13/2024 - the memory limit for OTA is exceeded for the old ESP32 boards...
// Now using: "No OTA 2MB App/ 2MB SPIFFS" - may as well set COMPILE_WITH_MIDI_LIBRARY true.
#define COMPILE_WITH_MIDI_LIBRARY true

// links for additional boards in Arduino IDE Preferences
// https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
// https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json

// NOTE: there is an additional true/false switch above "COMPILE_WITH_MIDI_LIBRARY"!
#define DTS_VERSION "Version 3.45 (December 16, 2024)"

// Note: to compile for ESP32S3, set Tools->Board->ESP32 Arduino->ESP32S3 Dev Module,
// set Tools->Flash Size->8MB, set Tools->CPU Frequency->240MHz, Partition Scheme->8M with spiffs (3MB app/1.5MB spiffs)
// Note: Specifically select Port->(select port, COM7 usually)
// Note: my S3 board "should" be 16MB - needs more sluthing!
// (For the old ESP32 Dev Module, select it in Boards, use the 4MB RAM size - you need to use
// Partition Scheme->No OTA (2MB app/2MB spiffs) or it won't fit!)
#define ESP32_S3 false // set true if using ESP32 S3 board

#define PRINT_ON true // set true to enable status printing to console
#define RESET_PREFS false // set true to force clear on boot, then set back to false and rebuild...
#define RESET_WIFI  false // ""
#define RESET_SLOTS false // ""
#define FORCE_DEF_CIPHER_KEY false // force use of default cipher key AND default token!
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

// log directely into this device via access-point mode at 192.168.7.7
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

//----------------------------------------------------------------------------
// defaults for EE_WIFI_NAMESPACE (see PrefsClass.h)

#define DEF_HOSTNAME "dts7"
#define DEF_SSID "MyRouter"
#define DEF_PWD "MyRouterPass"
#define DEF_AP_SSID DEF_HOSTNAME
#define DEF_AP_PWD "1234567890"
// most signifigant byte bit 0 is multicast bit - do not set
// most signifigant byte bit 1 is locally administered bit - set this.
// least signifigant three bytes are unique to manufacturer
#define DEF_MAC "" // not set we use chip's MAC... format 42:ad:f2:23:d0
#define LOCKCOUNT_INIT          -1 // unlocked
#define LOCKPASS_INIT           "****"

//----------------------------------------------------------------------------
// defaults for EE_PREFS_NAMESPACE (see PrefsClass.h)

// set timezone
// https://remotemonitoringsystems.ca/time-zone-abbreviations.php
//Australia   Melbourne,Canberra,Sydney   EST-10EDT-11,M10.5.0/02:00:00,M3.5.0/03:00:00
//Australia   Perth   WST-8
//Australia   Brisbane  EST-10
//Australia   Adelaide  CST-9:30CDT-10:30,M10.5.0/02:00:00,M3.5.0/03:00:00
//Australia   Darwin  CST-9:30
//Australia   Hobart  EST-10EDT-11,M10.1.0/02:00:00,M3.5.0/03:00:00
//Europe  Amsterdam,Netherlands   CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Athens,Greece   EET-2EEST-3,M3.5.0/03:00:00,M10.5.0/04:00:00
//Europe  Barcelona,Spain   CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Berlin,Germany  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Brussels,Belgium  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Budapest,Hungary  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Copenhagen,Denmark  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Dublin,Ireland  GMT+0IST-1,M3.5.0/01:00:00,M10.5.0/02:00:00
//Europe  Geneva,Switzerland  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Helsinki,Finland  EET-2EEST-3,M3.5.0/03:00:00,M10.5.0/04:00:00
//Europe  Kyiv,Ukraine  EET-2EEST,M3.5.0/3,M10.5.0/4
//Europe  Lisbon,Portugal   WET-0WEST-1,M3.5.0/01:00:00,M10.5.0/02:00:00
//Europe  London,GreatBritain   GMT+0BST-1,M3.5.0/01:00:00,M10.5.0/02:00:00
//Europe  Madrid,Spain  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Oslo,Norway   CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Paris,France  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Prague,CzechRepublic  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Roma,Italy  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//Europe  Moscow,Russia   MSK-3MSD,M3.5.0/2,M10.5.0/3
//Europe  St.Petersburg,Russia  MST-3MDT,M3.5.0/2,M10.5.0/3
//Europe  Stockholm,Sweden  CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
//New Zealand   Auckland, Wellington  NZST-12NZDT-13,M10.1.0/02:00:00,M3.3.0/03:00:00
//USA & Canada  Hawaii Time   HAW10
//USA & Canada  Alaska Time   AKST9AKDT
//USA & Canada  Pacific Time  PST8PDT
//USA & Canada  Mountain Time   MST7MDT
//USA & Canada  Mountain Time (Arizona, no DST)   MST7
//USA & Canada  Central Time  CST6CDT
//USA & Canada  Eastern Time  EST5EDT
//Atlantic  Atlantic Time   AST4ADT
//Asia  Jakarta   WIB-7
//Asia  Jerusalem   GMT+2
//Asia  Singapore   SGT-8
//Asia  Ulaanbaatar, Mongolia   ULAT-8ULAST,M3.5.0/2,M9.5.0/2
//Central and South America   Brazil,Sao Paulo  BRST+3BRDT+2,M10.3.0,M2.3.0
//Central and South America   Argentina   UTC+3
//Central and South America   Central America   CST+6
#define SNTP_TZ_INIT "CST6CDT"
#define SNTP_INT_INIT 7 // 0=off (hours)

// presently, tokens are int (0-63) 255 = NO_TOKEN
// uint8_t, range MIN_TOKEN to MAX_TOKEN (should be under the base64 table-length...)
// you can change TOKEN_INIT but I'd advise no greater than 18 or so because the higher the
// number the greater the processing time to do shifts...
#define TOKEN_INIT 8 // NOTE: to set token use command "c token <token>"
#define MIN_TOKEN  0
#define MAX_TOKEN  (B64_TABLE_SIZE-1) // 63

// 40 = 10dBm, 82 = 20dBm,  0.25dBm steps [40..82]
#define WIFI_MAX_POWER_INIT 40
#define MAX_POWER_MIN 40
#define MAX_POWER_MAX 82

// Flag masks for HTTP communication between other units through a router
// (flag masks 32, 64, 128 available...)(stored in flash as a uint8_t EE_SYNC/SYNC_INIT)
// g_bSyncRx, g_bSyncTx, g_bSyncCycle, g_bSyncTime, g_bSyncEncrypt
#define SYNC_INIT               (1+2+4+8+16) // EE_SYNC_MASK_RX|EE_SYNC_MASK_TX|EE_SYNC_MASK_CYCLE|EE_SYNC_MASK_TIME|EE_SYNC_MASK_ENCRYPT
#define EE_SYNC_MASK_RX         1
#define EE_SYNC_MASK_TX         2
#define EE_SYNC_MASK_CYCLE      4
#define EE_SYNC_MASK_TIME       8
#define EE_SYNC_MASK_ENCRYPT    16

// .25 sec units (4 ticks per second)
#define PULSE_OFF_MODE_A_INIT 0 // 0=off, 1=on-to-off, 2=off-to-on, 3= both
#define MINWID_A_INIT   (1*4) // when 0, we use only pulse-maxWidth with no variation
#define MAXWID_A_INIT   (4*4) 
#define MINPER_A_INIT   (10*4) // 10 sec
#define MAXPER_A_INIT   (14*4)  // 14 sec

// .25 sec units (4 ticks per second)
#define PULSE_OFF_MODE_B_INIT 0 // 0=off, 1=on-to-off, 2=off-to-on, 3= both
#define MINWID_B_INIT   (1*4) // when 0, we use only pulse-maxWidth with no variation
#define MAXWID_B_INIT   (4*4) 
#define MINPER_B_INIT   (10*4) // 10 sec
#define MAXPER_B_INIT   (14*4)  // 14 sec

#define LABEL_A_INIT            "Outlet A"
#define LABEL_B_INIT            "Outlet B"
#define LABEL_MAXLENGTH         32 // max length of index.html labelTxtA and labelTxtB contenteditable HTML5 fields

#define PERUNITS_INIT           1 // 0= 1/2 sec, 1=sec, 2=min, 3=hrs
#define PERUNITS_MIN            0 // index represents .5 second units for perMax
#define PERUNITS_MAX            3 // index represents 1 hour units for perMax

#define PERMAX_INIT             60 // 60 (sec, min, hrs, Etc.)
#define PERMAX_MIN              1
#define PERMAX_MAX              65535

#define PERIOD_INIT             0 // percent offset of period (0-100) 0=random
#define PERIOD_MIN              0 // %
#define PERIOD_MAX              100 // 0 = random mode

#define PHASE_INIT              100 // percent offset of period (0-100) 100=random
#define PHASE_MIN               0 // %
#define PHASE_MAX               100 // 0 = random mode

#define DUTY_CYCLE_A_INIT       0 // percent on (0-100) 0=random
#define DUTY_CYCLE_B_INIT       0 // percent on (0-100) 0=random
#define DUTY_CYCLE_MIN          0 // %
#define DUTY_CYCLE_MAX          100 // 0 = random mode
#define MIN_RAND_PERCENT_DUTY_CYCLE 20 // smallest time-on is 20% of period when in random mode!

#define MIDICHAN_INIT           MIDICHAN_OFF // off
#define MIDINOTE_A_INIT         60 // middle C
#define MIDINOTE_B_INIT         62 // middle D
#define MIDINOTE_ALL            128 // all notes
#define MIDICHAN_ALL            0 // all channels
#define MIDICHAN_OFF            255 // no channels

#define SSR1_MODE_INIT          SSR_MODE_AUTO // 0 = OFF, 1 = ON, 2 = AUTO
#define SSR2_MODE_INIT          SSR_MODE_AUTO // 0 = OFF, 1 = ON, 2 = AUTO
//----------------------------------------------------------------------------

// NOTE: if you want custom security for your particular implementation, you can change the order of letters in _HttpCommandTable[]
// in HttpMsgClass.h and you can change ENCODE_TABLE2 and ENCODE_TABLE3 in B64Class.h
// You will also want to change TOKEN_INIT in PrefsClass.h! Also change HTTP_ASYNCREQ and HTTP_ASYNCREQ_PARAM_COMMAND in WSHandlers.cpp.

// technically these can be 0-63 but better to keep them small for less processing overhead...
// and 0 is probably not advised...
#define FAILSAFE_TOKEN_1 4 // MAC from HTTP client send to the remote web-server
#define FAILSAFE_TOKEN_2 1 // CanRx Decode fail
#define FAILSAFE_TOKEN_3 3 // CanRx fail, text fail, param fail...
#define FAILSAFE_TOKEN_4 5 // IP address string
#define FAILSAFE_TOKEN_5 2 // MAC response from web-server to HTTP client callback

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

#define SNTP_SERVER1  "pool.ntp.org"
#define SNTP_SERVER2  "time.nist.gov"

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
#define MAXTXTLEN 512
#define MAXTIMELEN 19 // 2020-11-31T23:32:00

// "c test on" - max length of random text
#define TEST_TEXT_MAX_LENGTH 10

// used for hnEncode()/hnDecode() for web-pages
#define MIN_SHIFT_COUNT 1
#define MAX_SHIFT_COUNT 12

#define RESTART_KEY_MIN_LENGTH 5
#define RESTART_KEY_MAX_LENGTH 10

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

#define RESTART_EXPIRATION_TIME (5*60) // 5 minutes to type "c restart ip key" command after arming with "c restart all"

#define TIME_SYNC_WAIT_TIME     10000 // time to wait for NTP time after connecting to wifi

#define MIN_PERIOD_TIMER        10 // 5 seconds

#define MANUAL_CLOCK_SET_DEBOUNCE_TIME 3 // 1 second units

#define SEND_HTTP_TIME_MIN      3 // 1-second resolution
#define SEND_HTTP_TIME_MAX      7

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

// relay status in g_devStatus (up to 32 relays)
#define DEV_STATUS_1    0
#define DEV_STATUS_2    1

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

#define MAX_LOCKPASS_LENGTH     32

#define T_ONE_HOUR (2*60*60)

#define SERIAL_PORT_MAX_INPUT   512

// NOTE: "spiffs" in the filename differentiates a SPIFFS data .bin file from a main-program .bin file
// during "over-the-air" programming! The web-server HTML files and javascript are in the fc.spiffs.bin file
// and the main program is in fc.bin (or it might be in DTS_SMART_FAN_CONTROLLER.ino.esp32.bin)
#define OTA_UPDATE_SPIFFS_VS_PGM_ID "spiffs"

#if COMPILE_WITH_MIDI_LIBRARY
#include <AppleMIDI.h>
USING_NAMESPACE_APPLEMIDI
#endif

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

// define our custom structs before including our header files!!!!
#define PERVALS_COUNT 6 // number of items in struct
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

#include "CipherClass.h" // aes32-Encrypt library requires Seed_Arduino_mbedtls library
#include "B64Class.h"
#include "ValueListClass.h"
#include "RepeatListClass.h"
#include "MdnsListClass.h"
#include "PrefsClass.h"
#include "TimeSlotsClass.h"
#include "HttpMsgClass.h"
#include "TaskClass.h"
#include "ChangeClass.h"

#include "FCUtils.h"
#include "FCTime.h"
#include "FCWiFi.h"
#include "WSHandlers.h"
#include "HttpClientHandlers.h"
#include "Cmd.h"
#include "Tests.h"

// function prototypes
void SSR1On(uint32_t iPeriod);
void SSR2On(uint32_t iPeriod);
void serialEvent(); // event hndler for Serial
//void IRAM_ATTR onTimer();
void onTimer();
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
void TaskMidiChan(); // has to be in FanController.ino!
void SendWithHeaders(AsyncWebServerRequest *request, String s);
//String wsTemplateProc(const String& var);

extern int g_slotCount, g_prevMdnsCount, g_taskIdx;
extern int g_defToken, g_origDefToken;
extern int g_sct, g_minSct, g_maxSct;
extern int g_oldDevStatus, g_devStatus;

extern uint8_t g8_maxPower, g8_midiNoteA, g8_midiNoteB, g8_midiChan;
extern uint8_t g8_ledFlashCount, g8_ledFlashCounter, g8_ledDigitCounter, g8_ledSaveMode, g8_ledMode, g8_ledSeqState;
extern uint8_t g8_quarterSecondTimer, g8_fiveSecondTimer, g8_thirtySecondTimer, g8_ledFlashTimer, g8_clockSetDebounceTimer, g8_lockCount;
extern uint8_t g8_digitArray[];

extern uint16_t g16_pot1Value, g16_oldpot1Value; // variable for storing the potentiometer value
extern uint16_t g16_unlockCounter, g16_changeSyncTimer, g16_sendDefTokenTimer, g16_SNTPinterval;
extern uint16_t g16_sendDefTokenTime, g16_sendHttpTimer, g16_asyncHttpIndex, g16_oddEvenCounter;

extern uint32_t g32_periodTimer, g32_savePeriod, g32_dutyCycleTimerA, g32_dutyCycleTimerB, g32_phaseTimer, g32_nextPhase;

extern String g_sHostName, g_sSSID, g_sApSSID, g_sKey, g_sMac, g_sLabelA, g_sLabelB, g_sSerIn, g_text, g_sTimezone;

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
extern bool g_bTest, g_bLedOn;
extern bool g_bSyncRx, g_bSyncTx, g_bSyncCycle, g_bSyncTime, g_bSyncEncrypt, g_bMaster;

// cycle pulse-off
extern uint8_t g8_pulseModeA, g8_pulseModeB;
extern uint8_t g8_pulseWidthTimerA, g8_pulseWidthA, g8_pulseMinWidthA, g8_pulseMaxWidthA;
extern uint8_t g8_pulseWidthTimerB, g8_pulseWidthB, g8_pulseMinWidthB, g8_pulseMaxWidthB;
extern uint16_t g16_pulsePeriodTimerA, g16_pulsePeriodA, g16_pulseMinPeriodA, g16_pulseMaxPeriodA;
extern uint16_t g16_pulsePeriodTimerB, g16_pulsePeriodB, g16_pulseMinPeriodB, g16_pulseMaxPeriodB;

extern uint16_t g16_restartExpirationTimer; 
extern String g_sRestartKey, g_sRestartIp;

extern time_t g_prevNow;
extern Preferences PF;
extern t_time_date g_prevDateTime;

extern PerVals g_perVals, g_oldPerVals;
extern Stats g_stats;
extern IPAddress g_IpMaster;

extern const char OBFUSCATE_STR[], SC_MAC_RANDOM[];
extern const char NTP_SERVER1[], NTP_SERVER2[];

//extern RTC_DATA_ATTR int bootCount;
extern int bootCount;
