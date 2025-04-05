//#ifndef GpcH
//#define GpcH
#pragma once

#include <Arduino.h>

#include <esp_system.h>
#include <esp_efuse.h>
#include <esp_efuse_table.h>

// Import required libraries
#include <ESPmDNS.h>
#include <MyPreferences.h> // custom change to disable error-logging in getBytesLength()
#include <Update.h>

// NOTE: there is an additional true/false switch above "COMPILE_WITH_MIDI_LIBRARY"!
#define GPC_VERSION "Version 1.04 (April 4, 2025)"

// Note: to compile for ESP32S3, set Tools->Board->ESP32 Arduino->ESP32S3 Dev Module,
// set Tools->Flash Size->8MB, set Tools->CPU Frequency->240MHz, Partition Scheme->8M with spiffs (3MB app/1.5MB spiffs)
// Note: Specifically select Port->(select port, COM7 usually)
// Note: my S3 board "should" be 16MB - needs more sluthing!
// (For the old ESP32 Dev Module, select it in Boards, use the 4MB RAM size - you need to use
// Partition Scheme->No OTA (2MB app/2MB spiffs) or it won't fit!)

// Notes:
// Board 1 only has a SPST switch for POT-mode and there are no sense inputs so DISABLE_POTENTIOMETER
// may need to be set to avoid spurrious setting of parameters!
//
// For board 2B there are no sense inputs unless a hand-modification has been made! Without sense inputs, the POT
// reading/functionality won't work and the status won't be the "actual" status - manually turning on an SSR can't be
// sensed!
//
// For boards 2C, 3B and 3C you can choose not to install SSR 3 and 4 switches and parts - in that case, 
// set ENABLE_SSR_C_AND_D false!

// For ESP32 Board 1 set GPC_BOARD_2B/2C/3B/3C false and ENABLE_SSR_C_AND_D false (oldest board)
// for board ESP32 Board 2B set GPC_BOARD_2B true and ENABLE_SSR_C_AND_D false
// for board ESP32 Board 3B set GPC_BOARD_3B true and ENABLE_SSR_C_AND_D true
// for board ESP32 Board 3C set GPC_BOARD_3C true and ENABLE_SSR_C_AND_D true
#define GPC_BOARD_2B false // uses the old ESP32 DevKitC and 2 SSRs
#define GPC_BOARD_3B false // uses the ESP32 S3 DevKitC-1 and 4 SSRs (POT ADC to GPIO10, ADC1_9)
// newest boards for both old ESP32 DevKit and for new S3 DevKit-1 module
#define GPC_BOARD_2C false // uses the old ESP32 DevKitC and 4 SSRs
#define GPC_BOARD_3C false // uses the ESP32 S3 DevKitC-1 and 4 SSRs (POT ADCs GPIO06, GPIO08, GPIO09)

#if GPC_BOARD_2C || GPC_BOARD_3B || GPC_BOARD_3C
#define ENABLE_SSR_C_AND_D true
#define DISABLE_POTENTIOMETER false
#else
#define ENABLE_SSR_C_AND_D false
#define DISABLE_POTENTIOMETER true
#endif

#define PRINT_ON true // set true to enable status printing to console
#define RESET_PREFS false // set true to force clear on boot, then set back to false and rebuild...
#define RESET_WIFI  false // ""
#define RESET_SLOTS false // ""
#define FORCE_DEF_CIPHER_KEY false // force use of default cipher key AND default token!
#define FORCE_AP_ON false
#define FORCE_STA_ON false
#define HTTP_CLIENT_TEST_MODE false // send random HTTP strings to each mDNS-discovered remote unit
#define POT_REVERSED false

// NOTE: if you set this true you will need to use the the bigger partition scheme and lose OTA programming
// for the older ESP32 with 4MB flash. When true, you can control the outputs with a midi-keyboard.
// If this is the 8MB flash (ESP32 S3), you can set this true!
//
// NOTE: 8/13/2024 - the memory limit for OTA is exceeded for the old ESP32 boards...
// Now using: "No OTA 2MB App/ 2MB SPIFFS" - may as well set COMPILE_WITH_MIDI_LIBRARY true.
#define COMPILE_WITH_MIDI_LIBRARY true

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

// links for additional boards in Arduino IDE Preferences
// https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
// https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json

#define USE_UPDATE_LOGIN false // set true to require password entry after typing "c update" into index.html host-name field
#define UPDATE_USERID "gpc7" // only when USE_UPDATE_LOGIN is true!
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
// also see the top of Gpc.cpp for constant default strings!!!!!!!

#define MDNS_SVC "gpc"
#define MDNS_SVCU "_gpc"

//----------------------------------------------------------------------------
// defaults for EE_WIFI_NAMESPACE (see PrefsClass.h)

#define DEF_HOSTNAME "gpc7"
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

// .25 sec units (4 ticks per second)
#define PULSE_OFF_MODE_C_INIT 0 // 0=off, 1=on-to-off, 2=off-to-on, 3= both
#define MINWID_C_INIT   (1*4) // when 0, we use only pulse-maxWidth with no variation
#define MAXWID_C_INIT   (4*4) 
#define MINPER_C_INIT   (10*4) // 10 sec
#define MAXPER_C_INIT   (14*4)  // 14 sec

// .25 sec units (4 ticks per second)
#define PULSE_OFF_MODE_D_INIT 0 // 0=off, 1=on-to-off, 2=off-to-on, 3= both
#define MINWID_D_INIT   (1*4) // when 0, we use only pulse-maxWidth with no variation
#define MAXWID_D_INIT   (4*4) 
#define MINPER_D_INIT   (10*4) // 10 sec
#define MAXPER_D_INIT   (14*4)  // 14 sec

#define LABEL_A_INIT            "Outlet A"
#define LABEL_B_INIT            "Outlet B"
#define LABEL_C_INIT            "Outlet C"
#define LABEL_D_INIT            "Outlet D"
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

#define PHASE_B_INIT            100 // percent offset of period (0-100) 100=random
#define PHASE_C_INIT            100
#define PHASE_D_INIT            100
#define PHASE_MIN               0 // %
#define PHASE_MAX               100 // 0 = random mode

#define DUTY_CYCLE_A_INIT       0 // percent on (0-100) 0=random
#define DUTY_CYCLE_B_INIT       0 // percent on (0-100) 0=random
#define DUTY_CYCLE_C_INIT       0 // percent on (0-100) 0=random
#define DUTY_CYCLE_D_INIT       0 // percent on (0-100) 0=random
#define DUTY_CYCLE_MIN          0 // %
#define DUTY_CYCLE_MAX          100 // 0 = random mode
#define MIN_RAND_PERCENT_DUTY_CYCLE 20 // smallest time-on is 20% of period when in random mode!

#define MIDICHAN_INIT           MIDICHAN_OFF // off
#define MIDINOTE_A_INIT         60 // middle C
#define MIDINOTE_B_INIT         62 // middle D
#define MIDINOTE_C_INIT         60 // middle C
#define MIDINOTE_D_INIT         62 // middle D
#define MIDINOTE_ALL            128 // all notes
#define MIDICHAN_ALL            0 // all channels
#define MIDICHAN_OFF            255 // no channels

#define SSR1_MODE_INIT          SSR_MODE_AUTO // 0 = OFF, 1 = ON, 2 = AUTO
#define SSR2_MODE_INIT          SSR_MODE_AUTO // 0 = OFF, 1 = ON, 2 = AUTO
#define SSR3_MODE_INIT          SSR_MODE_AUTO // 0 = OFF, 1 = ON, 2 = AUTO
#define SSR4_MODE_INIT          SSR_MODE_AUTO // 0 = OFF, 1 = ON, 2 = AUTO
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

// prevent a system with no potentiometer present from causing value-changes
#if GPC_BOARD_2B
  #define GPC_BOARD " (for ESP32 DevKitC, DTS Board 2B)"
  #define CPU_FREQ 160 // (S3 is 240MHz) 80MHz works ok for WiFi but may need 160MHz or 240MHz for WiFi Scans!
  #define GPOUT_ONBOARD_LED 2 // GPIO38 for ESP32 Board 3B, GPIO02 for ESP32 Board 1 and ESP32 Board 2B
  #define GPOUT_WIFI_LED 15
  #define GPOUT_COM_LED 27

  // Solid-state relay outputs
  #define GPOUT_SSR1 32
  #define GPOUT_SSR2 23
  
  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_WIFI_AP_SW 21 // Set pin to Vcc for WiFi AP mode
  #define GPIN_WIFI_STA_SW 19 // Set pin to Vcc for WiFi STA mode

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_POT_MODE_SW1 18 // Set pin to Vcc for POT1 Mode 1
  #define GPIN_POT_MODE_SW2 17 // Set pin to Vcc for POT1 Mode 2

  #define GPAIN_POT1 36 // ADC1_0

#elif GPC_BOARD_2C
  #define GPC_BOARD " (for ESP32 DevKitC, DTS Board 2C)"
  #define CPU_FREQ 160
  #define GPOUT_ONBOARD_LED 2
  #define GPOUT_WIFI_LED 5
  #define GPOUT_COM_LED 4

  // Solid-state relay outputs
  #define GPOUT_SSR1 33
  #define GPOUT_SSR2 32
  
  #if ENABLE_SSR_C_AND_D
    #define GPOUT_SSR3 25
    #define GPOUT_SSR4 12
  #endif
  
  // Solid-state relay sense-inputs (no pullup/pulldown)
  #define GPIN_SSR1 34
  #define GPIN_SSR2 35
  
  #if ENABLE_SSR_C_AND_D
    #define GPIN_SSR3 14
    #define GPIN_SSR4 13
  #endif

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_WIFI_AP_SW 17
  #define GPIN_WIFI_STA_SW 18

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_POT_MODE_SW1 15
  #define GPIN_POT_MODE_SW2 16

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_SPARE_MODE_SW1 19
  #define GPIN_SPARE_MODE_SW2 21

  #define GPAIN_POT1 36 // ADC1_0
  #define GPAIN_POT2 39 // ADC1_3

#elif GPC_BOARD_3B

// ESP32 S3 DevKitC-1 Notes (Boards 3B/3C):
// left side top going down (USB connectors at bottom)
// Pin 1, 2 = 3.3V out
// Pin 3 = RST
// Pin 21 = 5V in
// Pin 22 = Ground
// right side bottom going up (USB connectors at bottom)
// Pins 23, 24, 44 = Ground

// NOTE: Do Not Use ADC2, it's used by WiFi module!!!!!

// GPIO 00, 03, 45, 46 (strapping pins - use caution!)
// NOTE can use JTAG GPIO 3 because a special fuse has to be blown for JTAG mode enable
// GPIO 3 has a weak pulldown during reset.
// GPIO 45 SPI flash voltage (0=3.3v, 1=1.8v). has a weak pulldown during reset. [SPI voltage can bypass pin 45 by burning efuse]
// GPIO 46 ROM messages print during booting (0=enable, 1=disable). has a weak pulldown during reset.
// S3 has built-in JTAG USB on GPIO 19 and 20
// Our "sense" inputs should never use these pins because they can be pulled-up hard to 3.3V at reset!
// SSR Outputs can use these because on reset, if anything, they will pull down...

// don't use GPIOs
// 44, 43 (USB0)
// 15, 16 (USB1)
// 19, 20 (USB2)
// 38 builtin LED (LED_BUILTIN)
// 48 rgb LED (RGB_BUILTIN)

//  #define LED_GREEN 50 // 0-255 (for tri-color built-in LED)

// LEDs
// NOTE: The esp32 S3 DevKitC-1 Module has an RGB LED (GPIO 48) and 3 plain LEDs - one plain LED is red power,
// one blinks on flash-programming communications and the 3rd we presume is a blue LED_BUILTIN (GPIO 38)
// (the RGB LED is very nice... shame you can't see it with the board in its enclosure...)

// RGB_BUILTIN (rgb LED on the ESP32 S3 Dev Module) GPIO 48
// LED_BUILTIN (blue LED on the ESP32 S3 Dev Module) GPIO 38
//#ifdef RGB_BUILTIN
//  digitalWrite(RGB_BUILTIN, HIGH);  // Turn the RGB LED white
//  delay(1000);
//  digitalWrite(RGB_BUILTIN, LOW);  // Turn the RGB LED off
//  delay(1000);
//
//  rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);  // Red
//  delay(1000);
//  rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);  // Green
//  delay(1000);
//  rgbLedWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS);  // Blue
//  delay(1000);
//  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);  // Off / black
//  delay(1000);
//#endif

  #define GPC_BOARD " (for ESP32 S3 DevKitC-1, DTS Board 3B)"
  #define CPU_FREQ 240
  #define GPOUT_ONBOARD_LED 38
  #define GPOUT_WIFI_LED 42
  #define GPOUT_COM_LED 21

  // Solid-state relay outputs
  #define GPOUT_SSR1 4
  #define GPOUT_SSR2 6
  
  #if ENABLE_SSR_C_AND_D
    #define GPOUT_SSR3 17
    #define GPOUT_SSR4 8
  #endif
  
  // Solid-state relay sense-inputs (no pullup/pulldown)
  #define GPIN_SSR1 5
  #define GPIN_SSR2 7

  #if ENABLE_SSR_C_AND_D
    #define GPIN_SSR3 18
    #define GPIN_SSR4 9
  #endif

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_WIFI_AP_SW 40
  #define GPIN_WIFI_STA_SW 41

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_POT_MODE_SW1 47
  #define GPIN_POT_MODE_SW2 39

  #define GPAIN_POT1 10 // ADC1_9

#elif GPC_BOARD_3C
  #define GPC_BOARD " (for ESP32 S3 DevKitC-1, DTS Board 3C)"
  #define CPU_FREQ 240
  #define GPOUT_ONBOARD_LED 38
  #define GPOUT_WIFI_LED 42
  #define GPOUT_COM_LED 21

  // Solid-state relay outputs
  #define GPOUT_SSR1 4
  #define GPOUT_SSR2 3 // (JTAG strapping pin but should be ok here...)
  
  #if ENABLE_SSR_C_AND_D
    #define GPOUT_SSR3 17
    #define GPOUT_SSR4 46 // strapping pin with weak pulldown (should work ok...)
  #endif
  
  // Solid-state relay sense-inputs (no pullup/pulldown)
  #define GPIN_SSR1 5
  #define GPIN_SSR2 7
  
  #if ENABLE_SSR_C_AND_D
    #define GPIN_SSR3 18
    #define GPIN_SSR4 14
  #endif

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_WIFI_AP_SW 40 // (pin 24/30) Set pin to Vcc for WiFi AP mode
  #define GPIN_WIFI_STA_SW 41 // (pin 25/30) Set pin to Vcc for WiFi STA mode

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_POT_MODE_SW1 47
  #define GPIN_POT_MODE_SW2 39

  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_SPARE_MODE_SW1 2
  #define GPIN_SPARE_MODE_SW2 1

  // NOTE: these three pads, together with 3.3V and Ground can be used for
  // a continuously rotating digital encoder control with push-in switch. Or - for 3 POTS...
  #define GPAIN_POT1 8 // ADC1_7
  #define GPAIN_POT2 9 // ADC1_8
  #define GPAIN_POT3 6 // ADC1_5

#else
  #define GPC_BOARD " (for ESP32 DevKit, DTS Board 1)"
  #define CPU_FREQ 160
  #define GPOUT_ONBOARD_LED 2

  #define GPOUT_SSR1 32
  #define GPOUT_SSR2 23
  
  #define GPIN_POT_MODE_SW 34 // ADC1_6 (4)
  
  // Inputs with pulldowns (3-states 00,01,10)
  #define GPIN_WIFI_AP_SW 18 // (pin 24/30) Set pin to Vcc for WiFi AP mode
  #define GPIN_WIFI_STA_SW 19 // (pin 25/30) Set pin to Vcc for WiFi STA mode

  #define GPAIN_POT1 36 // ADC1_0
#endif

// g8_potModeFromSwitch values
#define POT_MODE_NONE 255 // 8-bit value!
#define POT_MODE_CENTER 0
#define POT_MODE_LEFT 1
#define POT_MODE_RIGHT 2

#define POT_CHAN_NONE 255 // 8-bit value!
#define POT_CHAN_1 0
#define POT_CHAN_2 1
#define POT_CHAN_3 2
#define POT_CHAN_4 3

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
// bit-masks 1,2,4,8,16,32,64,Etc.
#define DEV_STATUS_1    1
#define DEV_STATUS_2    2
#define DEV_STATUS_3    4
#define DEV_STATUS_4    8

// hardware timer
#define HW_TIMER_FREQ 1000000 // use 1000000 for 1MHz
#define HW_TIMER_PERIOD (HW_TIMER_FREQ/8) // 125ms

#define MS_125_TIME 4 // 125ms timer

// g8_wifiLedFlashTimer 125ms resolution
#define LED_FASTFLASH_TIME 2
#define LED_SLOWFLASH_TIME 8

// g8_wifiLedFlashCounter
#define LED_PAUSE_COUNT    10 // pause time between digit flash-sequences

// g8_wifiLedSeqState
#define LEDSEQ_ENDED       0
#define LEDSEQ_FLASHING    1
#define LEDSEQ_PAUSED      2

// g8_wifiLedMode, g8_wifiLedSaveMode
#define g8_wifiLedMode_OFF        0
#define g8_wifiLedMode_ON         1
#define g8_wifiLedMode_SLOWFLASH  2
#define g8_wifiLedMode_FASTFLASH  3
#define g8_wifiLedMode_PAUSED     4

#define LED_EEPROM_FLASH_TIME   3 // .5 sec units (indicated a value saved to eeprom)

#define MAX_LOCKPASS_LENGTH     32

#define T_ONE_HOUR (2*60*60)

#define SERIAL_PORT_MAX_INPUT   512

// NOTE: "spiffs" in the filename differentiates a SPIFFS data .bin file from a main-program .bin file
// during "over-the-air" programming! The web-server HTML files and javascript are in the fc.spiffs.bin file
// and the main program is in fc.bin (or it might be in Gpc.ino.esp32.bin)
#define OTA_UPDATE_SPIFFS_VS_PGM_ID "spiffs"

//#define CORE0_TASK_STACK_SIZE 10000

// define our custom structs before including our header files!!!!
#define PERVALS_COUNT 10 // number of items in struct (includes duty-cycle for SSR3 and SSR4)
struct PerVals {
  uint8_t dutyCycleA = 0xff, dutyCycleB = 0xff; // saved in Preferences (units are %)
  uint8_t dutyCycleC = 0xff, dutyCycleD = 0xff;
  uint8_t phaseB = 0xff, phaseC = 0xff, phaseD = 0xff; // saved in Preferences (units are %)
  uint8_t perUnits = 0xff, perVal = 0xff; // perUnits is an index to index.html select options
  uint16_t perMax = 0xffff;
};

// we count "on" events up to the max interval in .5 sec units (determined by perMax and perUnits)
// and also time duration "on" within that interval in .5 sec units which can be converted to a percentage on (duty-cycle).
struct Stats {
  uint32_t HalfSecondCounter, HalfSecondCount;
  uint32_t AOnCounter, BOnCounter, COnCounter, DOnCounter, DConA, DConB, DConC, DConD;
  uint32_t AOnPrevCount, BOnPrevCount, COnPrevCount, DOnPrevCount, PrevDConA, PrevDConB, PrevDConC, PrevDConD;
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

#include "GpcUtils.h"
#include "GpcTime.h"
#include "GpcWiFi.h"
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
uint32_t ComputePhaseB();
uint32_t GetTimeInterval(uint16_t perMax, uint8_t perUnits);
void stopMIDI();
void startMIDI();
void TaskMidiChan(); // has to be in Gpc.ino!
void SendWithHeaders(AsyncWebServerRequest *request, String s);
//String wsTemplateProc(const String& var);
//void Core0TaskLoop(void* pvParameters);

#if ENABLE_SSR_C_AND_D
void SSR3On(uint32_t iPeriod);
void SSR4On(uint32_t iPeriod);
uint32_t ComputePhaseC();
uint32_t ComputePhaseD();

extern uint8_t g8_midiNoteC, g8_midiNoteD;
extern uint8_t g8_ssr3ModeFromWeb, g8_ssr4ModeFromWeb;
extern uint8_t g8_pulseModeC, g8_pulseModeD;
extern uint8_t g8_pulseWidthTimerC, g8_pulseWidthC, g8_pulseMinWidthC, g8_pulseMaxWidthC;
extern uint8_t g8_pulseWidthTimerD, g8_pulseWidthD, g8_pulseMinWidthD, g8_pulseMaxWidthD;

extern uint16_t g16_pulsePeriodTimerC, g16_pulsePeriodC, g16_pulseMinPeriodC, g16_pulseMaxPeriodC;
extern uint16_t g16_pulsePeriodTimerD, g16_pulsePeriodD, g16_pulseMinPeriodD, g16_pulseMaxPeriodD;

extern uint32_t g32_dutyCycleTimerC, g32_dutyCycleTimerD, g32_phaseTimerC, g32_phaseTimerD, g32_nextPhaseC, g32_nextPhaseD;

extern String g_sLabelC, g_sLabelD;
#endif

#if GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
extern int g_actualStatus;
#endif

#if GPC_BOARD_2B || GPC_BOARD_3B || GPC_BOARD_2C || GPC_BOARD_3C
extern bool g_bOldPotModeSw1On, g_bOldPotModeSw2On;
#else
extern bool g_bOldPotModeSwOn;
#endif

extern int g_slotCount, g_prevMdnsCount, g_taskIdx;
extern int g_defToken, g_origDefToken;
extern int g_sct, g_minSct, g_maxSct;
extern int g_oldDevStatus, g_devStatus;
extern int g_potPercent;

extern uint8_t g8_maxPower, g8_midiNoteA, g8_midiNoteB, g8_midiChan;

extern uint8_t g8_ms125Timer, g8_fiveSecondTimer, g8_thirtySecondTimer;
extern uint8_t g8_potLedFlashTimer, g8_potLedFlashTime, g8_wifiLedFlashTimer, g8_clockSetDebounceTimer, g8_lockCount;

extern uint8_t g8_wifiLedFlashCount, g8_wifiLedFlashCounter, g8_wifiLedDigitCounter, g8_wifiLedSaveMode, g8_wifiLedMode, g8_wifiLedSeqState;
extern uint8_t g8_wifiLedDigitArray[];

extern uint16_t g16_oldPotValue; // variable for storing the potentiometer value
extern uint16_t g16_unlockCounter, g16_changeSyncTimer, g16_sendDefTokenTimer, g16_SNTPinterval;
extern uint16_t g16_sendDefTokenTime, g16_sendHttpTimer, g16_asyncHttpIndex, g16_oddEvenCounter;

extern uint32_t g32_periodTimer, g32_savePeriod, g32_dutyCycleTimerA, g32_dutyCycleTimerB, g32_phaseTimerB, g32_nextPhaseB;

extern String g_sHostName, g_sSSID, g_sApSSID, g_sKey, g_sMac, g_sLabelA, g_sLabelB, g_sSerIn, g_text, g_sTimezone;

extern bool g_bOldWiFiApSwOn, g_bOldWiFiStaSwOn;

extern uint8_t g8_ssr1ModeFromWeb, g8_ssr2ModeFromWeb;

extern uint8_t g8_potChannel, g8_potModeFromSwitch, g8_wifiModeFromSwitch;

extern bool g_bWiFiConnected, g_bWiFiConnecting, g_bSoftAP, g_bMdnsOn, g_bWiFiDisabled, g_bResetOrPowerLoss, g_bTellP2WebPageToReload;
extern bool g_bManualTimeWasSet, g_bWiFiTimeWasSet, g_bValidated, g_bRequestManualTimeSync, g_bRequestWiFiTimeSync, g_bMidiConnected;
extern bool g_bTest, g_bWiFiLedOn;
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
