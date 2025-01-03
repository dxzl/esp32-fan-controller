// this file FanController.ino
#include "FanController.h"

/*********
  ------------------------------------------------------------------------ 
  Install instructions:
  https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/windows.md

  Thanks to Rui Santos for his great tutorials and examples!

  WiFi Smart Fan Controller is by Scott Swift, Christian - Jesus is Lord!

  NOTE: Compile with ESP32 build-tools 3.0.5 and up.
  NOTE: you should use the custom-modified library versions I've zipped and put in: misc\AAA_Libraries
  and also the updated "me-no-dev" libraries: https://github.com/me-no-dev/ (ESPAsyncWebServer and ESPAsyncTCP).
  On my PC, I install it into: \Documents\Arduino\libraries.
  My "shetchbook" (projects) folder is: YourDrive:\Documents\Arduino
  I locate this project in: YourDrive:\Documents\Arduino\projects\ESP32\FanControllerS3
  On my PC I create the Output folder: YourDrive:\Documents\Arduino\projects\Output
  I add this line to prererences.txt: build.path=YourDrive:\Documents\Arduino\projects\Output
  On my PC, prererences.txt is located in: C:\Users\(you)\AppData\Local\Arduino15
  The ESP32 build-tools are installed by boardsmanager into:
    C:\Users\(you)\AppData\Local\Arduino15\packages\esp32
  In File->Preferences, Additional Boards Manager URLS add:
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  Tools->Board->Boards Manager search for ESP32 and download and install the latest build tools
  Restart the Arduino IDE then choose your board from the list in Tools->Board (I choose
  either ESP32 Arduino->"Esp32 Dev Module" or "Esp32S3 Dev Module")
  NOTE: For the old ESP Dev Module with 4MB select Tools->Partition Scheme: No OTA 2MB APP/2MB SPIFFS
  (For the new boards with more memory, you can use OTA flash-uploading!)

  (Note: to compile for the S3 Board, set ESP32_S3 true in FanController.h and choose
  Tools->Board->Esp32 Arduino->ESP32S3 Dev Module)
  Compile using Sketch->Export Compiled Binary and you should see the output files in:
    YourDrive:\Documents\Arduino\projects\Output
  You can run pgm.bat to then upload the compiled main program binary to your board
  You can run data.bat to upload the SPIFFS web-server-files to your board
  The files for the web-server are in: YourDrive:\Documents\Arduino\projects\ESP32\FanControllerS3\data
    
  Links related to the new versions of ESP32 (S3) that can handle HTTPS:
  https://forum.arduino.cc/t/esp32-s3-n16r8-new-board/1099353
  https://esp32s3.com/getting-started.html

  Other links:
  https://github.com/vtunr/esp32_binary_merger
  https://github.com/PBearson/Get-Started-With-ESP32-OTA
  ------------------------------------------------------------------------
  
  I use Sketch->Export Compiled Binary then run fixname.bat to change the .bin file to fc.bin

  Next, build the SPIFFS data webserver files as a binary. I run the ESP32 utility "mkspiffs.exe" via fcspiffs.bat
  fcspiffs.bat has "mkspiffs.exe -p 256 -b 4096 -s 0x30000 -c ..\data fc.spiffs.bin"

  here is pgm.bat (flashes the main program int an old ESP32 Dev Kit):
  @echo off
  Set EspToolVer=4.6
  Set MyVer=3.0.5
  Set MyName=FancontrollerS3
  Set /p "MyVer=Main Program Upload. Type version or Enter for %MyVer%: "
  Set MyPort=4
  Set /p "MyPort=Type port # (4,5...) or Enter for 4: "
  Set MyPort=COM%MyPort%
  Set MyDrive=G
  Set /p "MyDrive=Type drive letter for Output folder (E,G, Etc...) or Enter for G: "
  Set MyOut=%MyDrive%:\Documents\Arduino\projects\Output
  Set MyEsp=%UserProfile%\AppData\Local\Arduino15\packages\esp32
  "%MyEsp%\tools\esptool_py\%EspToolVer%\esptool.exe" --chip esp32 --port %MyPort% --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 "%MyOut%\%MyName%.ino.bootloader.bin" 0x8000 "%MyOut%\%MyName%.ino.partitions.bin" 0xe000 "%MyEsp%\hardware\esp32\%MyVer%\tools\partitions\boot_app0.bin" 0x10000 "%MyOut%\%MyName%.ino.bin"
  echo Upload complete!
  pause
    
  here is data.bat (flashes the web-server files into an old ESP32 Dev Kit):
  @echo off
  Set EspToolVer=4.6
  Set MyVer=3.0.5
  Set MyName=FancontrollerS3
  Set /p "MyVer=SPIFFS Upload. Type version or Enter for %MyVer%: "
  Set MyPort=4
  Set /p "MyPort=Type port # (4,5...) or Enter for 4: "
  Set MyPort=COM%MyPort%
  Set MyDrive=G
  Set /p "MyDrive=Type drive letter for Output folder (E,G, Etc...) or Enter for G: "
  Set MyOut=%MyDrive%:\Documents\Arduino\projects\Output
  Set MyEsp=%UserProfile%\AppData\Local\Arduino15\packages\esp32
  "%MyEsp%\tools\esptool_py\%EspToolVer%\esptool.exe" --chip esp32 --port %MyPort% --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x210000 "%MyOut%\%MyName%.spiffs.bin"
  echo Upload complete!
  pause

  
  If you use Partition Scheme (Minimal 1.3MB App,700Kb SPIFFS), use:
  mkspiffs.exe -p 256 -b 4096 -s 1376256 -c ..\data fc.spiffs.bin

*********/

#if COMPILE_WITH_MIDI_LIBRARY
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, RTP_MIDI, "AppleMIDI-ESP32", DEFAULT_CONTROL_PORT);
#endif

const char OBFUSCATE_STR[] = "XqSmvn9CDfexnacQRbtcQM7zJ4jZoyXfeDMcg2atmk4nf3OykMe";

// FYI:
// How to use the flash-string function to conserve SRAM
// client.println(F("my message stored in flash..."));

// references:
// https://esp32developer.com
// https://github.com/cs8425/ESPAsyncWebServer
// https://github.com/me-no-dev/ESPAsyncWebServer
// https://techtutorialsx.com/2018/09/13/esp32-arduino-web-server-receiving-data-from-javascript-websocket-client/
// https://techtutorialsx.com/2018/09/11/esp32-arduino-web-server-sending-data-to-javascript-client-via-websocket/
// https://circuits4you.com/2019/01/12/esp8266-servo-motor-control/
// https://circuits4you.com/2018/11/20/web-server-on-esp32-how-to-update-and-display-sensor-values/
// https://www.allen.dj/esp8266-nodemcu-button-slider-remote-node-js-server/
// https://techtutorialsx.com/2018/10/05/esp32-web-server-template-processing-when-serving-html-from-file-system/
// https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input/submit
// https://stackoverflow.com/questions/3384960/want-html-form-submit-to-do-nothing
// https://startingelectronics.org/tutorials/arduino/ethernet-shield-web-server-tutorial/web-server-read-switch-automatically-using-AJAX/
// https://lastminuteengineers.com/esp32-ota-web-updater-arduino-ide/
// https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/ (handle button ISR)
// https://tzapu.com/esp8266-wifi-connection-manager-library-arduino-ide/
// https://coderwall.com/p/je3uww/get-progress-of-an-ajax-request
// https://arduino-esp8266.readthedocs.io/en/latest/libraries.html
// https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
// Date-Time input example: https://blog.startingelectronics.com/using-html-drop-down-select-boxes-for-hour-and-minute-on-arduino-web-server/
// https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/system/system_time.html
// https://circuits4you.com/2019/03/21/esp8266-url-encode-decode-example/
// https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
// Get WiFi event info: https://techtutorialsx.com/2019/08/15/esp32-arduino-getting-wifi-event-information/
// https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/wifi.html
// https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/

// stackArray[] - Interesting! FYI
//    t_event stackArray[g_slotCount];

#if ESP32_S3
bool g_bOldPotModeSw1On, g_bOldPotModeSw2On;
bool g_bOldSsr1ModeManSwOn, g_bOldSsr1ModeAutSwOn;
bool g_bOldSsr2ModeManSwOn, g_bOldSsr2ModeAutSwOn;
uint8_t g8_ssr1ModeFromSwitch, g8_ssr2ModeFromSwitch;
int bootCount;
#else
bool g_bOldPotModeSwOn;
// RTC_SLOW_ATTR, RTC_FAST_ATTR, RTC_RODATA_ATTR - read-only
// (RTC_IRAM_ATTR can be used in a function declaration to put it in RTC memory!)
RTC_DATA_ATTR int bootCount;
#endif

bool g_bWiFiConnected, g_bWiFiConnecting, g_bSoftAP, g_bMdnsOn, g_bWiFiDisabled;
bool g_bResetOrPowerLoss, g_bTellP2WebPageToReload;
bool g_bManualTimeWasSet, g_bWiFiTimeWasSet, g_bValidated;
bool g_bRequestManualTimeSync, g_bRequestWiFiTimeSync, g_bMidiConnected;
bool g_bTest, g_bLedOn;
bool g_bSyncRx, g_bSyncTx, g_bSyncCycle, g_bSyncTime, g_bSyncEncrypt, g_bMaster;

bool g_bOldWiFiApSwOn, g_bOldWiFiStaSwOn;

uint8_t g8_potModeFromSwitch, g8_wifiModeFromSwitch;

uint8_t g8_ssr1ModeFromWeb, g8_ssr2ModeFromWeb;

// cycle pulse-off feature
uint8_t g8_pulseModeA, g8_pulseModeB;
uint8_t g8_pulseWidthTimerA, g8_pulseWidthA, g8_pulseMinWidthA, g8_pulseMaxWidthA;
uint8_t g8_pulseWidthTimerB, g8_pulseWidthB, g8_pulseMinWidthB, g8_pulseMaxWidthB;
uint16_t g16_pulsePeriodTimerA, g16_pulsePeriodA, g16_pulseMinPeriodA, g16_pulseMaxPeriodA;
uint16_t g16_pulsePeriodTimerB, g16_pulsePeriodB, g16_pulseMinPeriodB, g16_pulseMaxPeriodB;

uint8_t g8_maxPower;
uint8_t g8_midiNoteA, g8_midiNoteB, g8_midiChan;

// used to flash the ip address least-signifigant digit first, 4th (last) number
// 0 is placed at end of sequence, we can flash, say 192.168.1.789 - the 789 part, 9 first...
uint8_t g8_ledFlashCount, g8_ledFlashCounter, g8_ledDigitCounter, g8_ledSaveMode, g8_ledMode, g8_ledSeqState;
uint8_t g8_digitArray[4];

uint8_t g8_quarterSecondTimer, g8_fiveSecondTimer, g8_thirtySecondTimer;
uint8_t g8_ledFlashTimer, g8_clockSetDebounceTimer, g8_lockCount;

uint16_t g16_pot1Value, g16_oldpot1Value; // variable for storing the potentiometer value
uint16_t g16_unlockCounter, g16_changeSyncTimer, g16_sendDefTokenTimer, g16_SNTPinterval;
uint16_t g16_sendDefTokenTime, g16_sendHttpTimer, g16_asyncHttpIndex, g16_oddEvenCounter;

// timers
uint32_t g32_savePeriod, g32_periodTimer, g32_dutyCycleTimerA, g32_dutyCycleTimerB, g32_phaseTimer, g32_nextPhase;

// parameters for web-page hnDecode() routine
int g_sct, g_minSct, g_maxSct;

int g_oldDevStatus, g_devStatus;

int g_slotCount, g_prevMdnsCount, g_taskIdx;
int g_defToken, g_origDefToken;

// previous time-date used to facilitate repeat functions
// following initial time-slot trigger (see p2.html)
time_t g_prevNow;
Preferences PF; // create an instance of Preferences library
t_time_date g_prevDateTime;

PerVals g_perVals, g_oldPerVals;
Stats g_stats;
IPAddress g_IpMaster;

uint16_t g16_restartExpirationTimer; 
String g_sRestartKey, g_sRestartIp;

String g_sHostName, g_sSSID, g_sApSSID, g_sKey, g_sMac, g_sLabelA, g_sLabelB, g_sSerIn, g_text, g_sTimezone;

hw_timer_t * g_HwTimer;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux; // synchronization between main loop and ISR

// Create AsyncWebServer object on port 80
AsyncWebServer webServer(SERVER_PORT);

//StaticJsonDocument<JSONARRAYSIZE> jdoc;
//const size_t jsonCapacity = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(3) + 50;

// see definition in AppleMidi.h
//APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "AppleMIDI-ESP32", 5004);
//                           │       │      │       └──── Local port number
//                           │       │      └──────────── Name
//                           │       └─────────────────── MIDI instance name
//                           └─────────────────────────── Network socket class

void serialEvent() {
  while (Serial.available()){
    char c = Serial.read();
    if (c == '\r' || c == '\n'){
      if (!g_sSerIn.isEmpty()){
        ProcessSerialCommand(g_sSerIn);
        g_sSerIn = "";
      }
    }
    else if (g_sSerIn.length() < SERIAL_PORT_MAX_INPUT)
      g_sSerIn += c;
    else{
      g_sSerIn = "";
      prtln("flushed input buffer!");
    }
  }
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// TODO (someday... but probably will change to a secure web-server library anyway...) 6/2024
// (search for "setTemplateProcessor" below...)
//String wsTemplateProc(const String& sReqFile){
//  prtln("sReqFile=\"" + sReqFile + "\"");  
//  if (sReqFile == HELP2_FILENAME)
//    // "F" means the string is a file-path - backslash chars...
//    // (don't need if just file's name...)
//    return F(HELP2_FILENAME);
//}
//Serving specific file by name
// Serve the file "/www/page.htm" when request url is "/page.htm"
//server.serveStatic("/page.htm", SPIFFS, "/www/page.htm");
//Serving files in directory
//To serve files in a directory, the path to the files should specify a directory in SPIFFS and ends with "/".
// Serve files in directory "/www/" when request url starts with "/"
// Request to the root or if the file doesn't exist will try to server the defualt file name "index.htm" if exists
//server.serveStatic("/", SPIFFS, "/www/");
// Server with different default file
//server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("default.html");
//Serving static files with authentication
//server
//    .serveStatic("/", SPIFFS, "/www/")
//    .setDefaultFile("default.html")
//    .setAuthentication("user", "pass");

// may want to do this on program init????????
//std::ios_base::sync_with_stdio(false);
//wcout.imbue(locale("en_US.UTF-8"));
//locale("en_US.UTF-8");
void setup()
{
  WiFi.disconnect(true, false); // turn off WiFi but don't clear AP credentials from NV memory
  
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Solid-state relay outputs
  pinMode(GPOUT_SSR1, OUTPUT);
  pinMode(GPOUT_SSR2, OUTPUT);

  // SPDT center-off WiFi AP/Off/STA switch. Inputs with pulldowns (3-states 00,01,10)
  pinMode(GPIN_WIFI_AP_SW, INPUT_PULLDOWN);
  pinMode(GPIN_WIFI_STA_SW, INPUT_PULLDOWN);

#if ESP32_S3
  // Analog Input
  pinMode(GPAIN_POT1, INPUT); // (pin 4 left) GPIO04 ADC1_3

  // Inputs with pulldowns (3-states 00,01,10)
  pinMode(GPIN_POT_MODE_SW1, INPUT_PULLDOWN); // (pin 5/44 left) GPIO05 ADC1_4 Set pin to Vcc for POT1 Mode 1
  pinMode(GPIN_POT_MODE_SW2, INPUT_PULLDOWN); // (pin 6/44 left) GPIO06 ADC1_5 Set pin to Vcc for POT1 Mode 2
  
  pinMode(GPBD_ONE_WIRE_BUS_DATA, INPUT_PULLDOWN); // (pin 12/44) GPIO08 ADC1_7
  pinMode(GPOUT_ONE_WIRE_BUS_CLK, OUTPUT); // (pin 15/44 left) GPIO09 ADC1_8

  // SPDT center-off SSR On/Off/Auto switches. Inputs with pulldowns (3-states 00,01,10)
  pinMode(GPIN_SSR1_MODE_SW_MAN, INPUT_PULLDOWN); // (pin 17/44 left) Set pin to Vcc for SSR_1 Manual ON
  pinMode(GPIN_SSR1_MODE_SW_AUT, INPUT_PULLDOWN); // (pin 18/44 left) Set pin to Vcc for SSR_1 Auto ON
  pinMode(GPIN_SSR2_MODE_SW_MAN, INPUT_PULLDOWN); // (pin 19/44 left) Set pin to Vcc for SSR_2 Manual ON
  pinMode(GPIN_SSR2_MODE_SW_AUT, INPUT_PULLDOWN); // (pin 20/44 left) Set pin to Vcc for SSR_2 Auto ON

  pinMode(GPOUT_SSR1_LED, OUTPUT); // (pin 32/44 right) GPIO35
  pinMode(GPOUT_SSR2_LED, OUTPUT); // (pin 33/44 right) GPIO36

  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);  // Off

  prtln(String(DTS_VERSION) + " (for ESP32S3 Dev Module!)");
#else
  // SW_SOFT_AP (all four POT pins are input only, no pullup/down!)
  pinMode(GPIN_POT_MODE_SW, INPUT); // GPIO34 toggle switch where a POT normally would go - used to boot to softAP WiFi mode
  pinMode(GPOUT_ONBOARD_LED, OUTPUT); // set internal LED (blue) as output
  
  digitalWrite(GPOUT_ONBOARD_LED, LOW);

  prtln(String(DTS_VERSION) + " (for ESP32 Dev Module!)");  
#endif

  // I've not had any success reading GPIO0 (it's also the BOOT button)

  // detach system interrupt and configure for polling
  //detachInterrupt(BTN_RESTORE_SSID_PWD);
  //pinMode(BTN_RESTORE_SSID_PWD, INPUT_PULLUP); // Set GPIO0 to input mode (same as boot button)

  //  LOW  Triggers interrupt whenever the pin is LOW
  //  HIGH  Triggers interrupt whenever the pin is HIGH
  //  CHANGE  Triggers interrupt whenever the pin changes value, from HIGH to LOW or LOW to HIGH
  //  FALLING Triggers interrupt when the pin goes from HIGH to LOW
  //  RISING  Triggers interrupt when the pin goes from LOW to HIGH
  //attachInterrupt(BTN_RESTORE_SSID_PWD, ISR, Mode);

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  prtln("Boot number: " + String(++bootCount));

  // insure "old" values for POT the same
  g16_oldpot1Value = g16_pot1Value = analogRead(GPAIN_POT1);

  // insure "old" values for SWITCH different
  g_bOldWiFiApSwOn = ~digitalRead(GPIN_WIFI_AP_SW);
  g_bOldWiFiStaSwOn = ~digitalRead(GPIN_WIFI_STA_SW);

#if ESP32_S3
  g_bOldPotModeSw1On = ~digitalRead(GPIN_POT_MODE_SW1);
  g_bOldPotModeSw2On = ~digitalRead(GPIN_POT_MODE_SW2);
  g_bOldSsr1ModeManSwOn = ~digitalRead(GPIN_SSR1_MODE_SW_MAN);
  g_bOldSsr1ModeAutSwOn = ~digitalRead(GPIN_SSR1_MODE_SW_AUT);
  g_bOldSsr2ModeManSwOn = ~digitalRead(GPIN_SSR2_MODE_SW_MAN);
  g_bOldSsr2ModeAutSwOn = ~digitalRead(GPIN_SSR2_MODE_SW_AUT);
  g8_ssr1ModeFromSwitch = 255;
  g8_ssr2ModeFromSwitch = 255;
#else
  g_bOldPotModeSwOn = ~digitalRead(GPIN_POT_MODE_SW);
#endif

  g8_potModeFromSwitch = 255;
  g8_wifiModeFromSwitch = 255;
  
  g8_fiveSecondTimer = FIVE_SECOND_TIME-2; // call PollApSwitch() in 2-3 seconds

  // init blue LED on the ESP32 daughter-board
  g8_ledMode = g8_ledMode_OFF;
  g8_ledSaveMode = g8_ledMode_OFF;
  g8_ledSeqState = LEDSEQ_ENDED;
  g8_ledFlashTimer = 0;
  g8_ledFlashCount = 0;
  g8_ledFlashCounter = 0;
  g8_ledDigitCounter = 0;
  g8_digitArray[0] = 0;

  g_bLedOn = false;

  g8_clockSetDebounceTimer = 0;
  g16_sendDefTokenTimer = 0;
  g16_changeSyncTimer = 0;
  g16_oddEvenCounter = 0;
  g16_sendDefTokenTime = random(SEND_DEF_TOKEN_TIME_MIN, SEND_DEF_TOKEN_TIME_MAX+1);
  g16_sendHttpTimer = SEND_HTTP_TIME_MIN;

  g16_asyncHttpIndex = 0;

  g16_restartExpirationTimer = 0;

  uint32_t cpu_freq = getCpuFrequencyMhz();
  prtln("Startup cpu freq is " + String(cpu_freq) + "MHz");

  if (cpu_freq != CPU_FREQ)
  {
    prtln("Changing cpu frequency to " + String(CPU_FREQ) + "MHz");
    setCpuFrequencyMhz(CPU_FREQ);
  }

  g_devStatus = 0;
  g_taskIdx = 0;
  g_prevMdnsCount = 0;
  g8_lockCount = 0;
  g16_unlockCounter = 0;
  g_minSct = MIN_SHIFT_COUNT;
  g_maxSct = MAX_SHIFT_COUNT;
  g_sct = g_minSct;
  g_HwTimer = NULL;
  timerMux = portMUX_INITIALIZER_UNLOCKED;

  g_bMidiConnected = false;
  g_bWiFiConnected = false;
  g_bWiFiConnecting = false;
  g_bSoftAP = false;
  g_bMdnsOn = false;
  g_bTellP2WebPageToReload = false;
  g_bValidated = false;
  
  g_bMaster = false;
  
#if HTTP_CLIENT_TEST_MODE
  g_bTest = true;
#else
  g_bTest = false;
#endif
  
  TSK.InitTasks();
  CNG.InitChanges();

  // https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
  //  ESP_SLEEP_WAKEUP_UNDEFINED In case of deep sleep, reset was not caused by exit from deep sleep.
  //  ESP_SLEEP_WAKEUP_ALL Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source.
  //  ESP_SLEEP_WAKEUP_EXT0 Wakeup caused by external signal using RTC_IO.
  //  ESP_SLEEP_WAKEUP_EXT1 Wakeup caused by external signal using RTC_CNTL.
  //  ESP_SLEEP_WAKEUP_TIMER Wakeup caused by timer.
  //  ESP_SLEEP_WAKEUP_TOUCHPAD Wakeup caused by touchpad.
  //  ESP_SLEEP_WAKEUP_ULP Wakeup caused by ULP program.
  //  ESP_SLEEP_WAKEUP_GPIO Wakeup caused by GPIO (light sleep only)
  //  ESP_SLEEP_WAKEUP_UART Wakeup caused by UART (light sleep only)
  g_bResetOrPowerLoss = (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) ? true : false; // if reset or power loss

  // Initialize SPIFFS
  if(!SPIFFS.begin(true))
  {
    prtln("An Error has occurred while mounting SPIFFS");
    return;
  }

  prtln("Embedded Version string: \"" + GetEmbeddedVersionString() + "\"");

  // Pertains to the FanController.h conditional-compile boolean switches:
  // READ_WRITE_CUSTOM_BLK3_MAC, FORCE_NEW_EFUSE_BITS_ON, WRITE_PROTECT_BLK3
  InitMAC();

  // NOTE: GetPreferences() sets period (which is in percent) and nvPeriodMax from
  // SPIFFS non-volitile memory
  #if RESET_PREFS
    PC.ErasePreferences();
  #endif
  #if RESET_WIFI
    PC.EraseWiFiPrefs();
    PC.RestoreDefaultApSsidAndPwd();
  #endif
  #if RESET_SLOTS
    TSC.EraseTimeSlots();
  #endif

  PC.GetPreferences(); // put this BEFORE the use of any preference-variable (such as g_defToken/g_origDefToken)

  // parameters NOT to send upon startup via HTTP client!
  g_oldPerVals = g_perVals;
  g_oldDevStatus = g_devStatus;
  
  PC.GetWiFiPrefs();

  // init stats counter
  InitStats();

  // this might take a while since we have to cycle from 0-MAX_SLOTS
  // trying to read each...
  g_slotCount = TSC.CountFullTimeSlots();
  prtln("Number of stored time-events: " + String(g_slotCount));

  // Note: don't call this between PF.begin and end. It will cause watchdog timer resets!
  //long heap_size = ESP.getFreeHeap();
  //prtln("Heap before variable size array: " + String(heap_size));

  // Route to serve .js files
  webServer.serveStatic(P0JS_FILENAME, SPIFFS, JS_DIRECTORY P0JS_FILENAME);
  webServer.serveStatic(P1JS_FILENAME, SPIFFS, JS_DIRECTORY P1JS_FILENAME);
  webServer.serveStatic(P2JS_FILENAME, SPIFFS, JS_DIRECTORY P2JS_FILENAME);
  webServer.serveStatic(JQUERY_FILENAME, SPIFFS, JS_DIRECTORY JQUERY_FILENAME);
  webServer.serveStatic(SCT_FILENAME, SPIFFS, JS_DIRECTORY SCT_FILENAME);
  
  // Route to serve .css files
  webServer.serveStatic(STYLE1_FILENAME, SPIFFS, CSS_DIRECTORY STYLE1_FILENAME);
  webServer.serveStatic(STYLE2_FILENAME, SPIFFS, CSS_DIRECTORY STYLE2_FILENAME);
  webServer.serveStatic(STYLE3_FILENAME, SPIFFS, CSS_DIRECTORY STYLE3_FILENAME);
  webServer.serveStatic(STYLELED_FILENAME, SPIFFS, CSS_DIRECTORY STYLELED_FILENAME);
  
  webServer.onNotFound(notFound);

  // local message received from remote unit's async http client... we reply and the reply is
  // received and decoded by the HttpClientCallback() function (see above)
  // for HTTP_ASYNCREQ_PARAM_COMMAND:
  // if all is well we send code HTTPCODE_PARAM_OK and HTTPRESP_PARAM_OK
  // if not, we send code HTTPCODE_FAIL along with a new base 36 encoded token
  webServer.on(HTTP_ASYNCREQ_CANRX, HTTP_GET, [](AsyncWebServerRequest *request){
    HandleHttpAsyncCanRxReq(request);
  });
//  webServer.on(HTTP_ASYNCREQ_TEXT, HTTP_GET, [](AsyncWebServerRequest *request){
//    HandleHttpAsyncTextReq(request);
//  });
  webServer.on(HTTP_ASYNCREQ, HTTP_GET, [](AsyncWebServerRequest *request){
    HandleHttpAsyncReq(request);
  });
  
  // Send web page with input fields to client
  // Route for root / web page
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // flag = true saves as attachment! String() is the content-type
    // ReplaceHtmlPercentSign is the handler function, SPIFFS serves page from file-system
    // /index.html is the file to serve based from root '/'
    SendWithHeaders(request, INDEX_FILENAME);
  });

  webServer.on(INDEX_FILENAME, HTTP_GET, [](AsyncWebServerRequest *request){
    SendWithHeaders(request, INDEX_FILENAME);
  });

  webServer.on(HELP1_FILENAME, HTTP_GET, [](AsyncWebServerRequest *request){
    // block help page if locked unless in AP mode
    if (IsLockedAlertGet(request, INDEX_FILENAME, true))
      return;

    SendWithHeaders(request, HELP1_FILENAME);
  });

  webServer.on(HELP2_FILENAME, HTTP_GET, [](AsyncWebServerRequest *request){
    // block help page if locked unless in AP mode
    if (IsLockedAlertGet(request, INDEX_FILENAME, true))
      return;

    SendWithHeaders(request, HELP2_FILENAME);
  });

  webServer.on(P1_FILENAME, HTTP_GET, [](AsyncWebServerRequest *request){
    SendWithHeaders(request, P1_FILENAME);
  });

  webServer.on(P2_FILENAME, HTTP_GET, [](AsyncWebServerRequest *request){
    SendWithHeaders(request, P2_FILENAME);
  });

  // loginIndex is launched when we enter "c update" and press submit while on p1.html in AP mode
  webServer.on(LOGIN_FILENAME, HTTP_GET, [](AsyncWebServerRequest *request){
    // block web update if locked unless in AP mode
    if (IsLockedAlertGet(request, P1_FILENAME, true))
      return;

    // set USE_UPDATE_LOGIN true for secure password-protected login screen required to update
    // set false to simplify and go directly to file-selection screen.
    #if USE_UPDATE_LOGIN
      SendWithHeaders(request, LOGIN_FILENAME);
    #else
      g_bValidated = true;
      SendWithHeaders(request, SERVERINDEX_FILENAME);
    #endif
  });

  webServer.on(SERVERINDEX_FILENAME, HTTP_GET, [](AsyncWebServerRequest *request){
    // block web update if locked unless in AP mode
    if (IsLockedAlertGet(request, P1_FILENAME, true))
      return;

    if (g_bValidated)
      request->send(SPIFFS, SERVERINDEX_FILENAME, "text/javascript");
  });

  #if USE_UPDATE_LOGIN
  webServer.on(EP_GET_LIFORM, HTTP_GET, [] (AsyncWebServerRequest *request){
    // block web update if locked unless in AP mode
    if (IsLockedAlertGet(request, P1_FILENAME, true))
      return;

    if (request->hasParam(PARAM_UDID) && request->hasParam(PARAM_UDPW))
    {
      String sId, sPw;
      int errorCodeId = B64C.hnShiftDecode(request->getParam(PARAM_UDID)->value(), sId);
      int errorCodePw = B64C.hnShiftDecode(request->getParam(PARAM_UDPW)->value(), sPw);
      if (sId == UPDATE_USERID && sPw == UPDATE_USERPW) // change to stored values - maybe use g_sHostName (???)
      {
        g_bValidated = true;
        SendWithHeaders(request, SERVERINDEX_FILENAME);
      }
      else
      {
        g_bValidated = false;
        // hnDecode returns empty string if error
        // returns errorCode -2 if empty string, -3 if bad validation prefix,
        // -4 if bad checksum, -5 if have validation but empty string thereafter
        if (errorCodeId < -1 || errorCodePw < -1)
          request->send(HTTPCODE_OK, "text/html", "<script>alert('Transmission error - please retry!');location.href = '" +
                                                                            String(LOGIN_FILENAME) + "'</script>");
        else
          request->send(HTTPCODE_OK, "text/html", "<script>alert('Incorrect credentials!');location.href = '" +
                                                                        String(LOGIN_FILENAME) + "'</script>");
      };

      sId = OBFUSCATE_STR; // obfuscate memory for security
      sPw = OBFUSCATE_STR;
    }
  });
  #endif

  webServer.on(EP_POST_UPDATE, HTTP_POST, [](AsyncWebServerRequest *request){
    if (IsLockedAlertPost(request, true)) // allow in AP!
      return;

    if (!g_bValidated)
      request->send(204, "text/html", "");
    else
    {
      bool shouldReboot = !Update.hasError();
      AsyncWebServerResponse *r = request->beginResponse(200, "text/plain", shouldReboot ? HTTPRESP_OK : HTTPRESP_FAIL);
      r->addHeader("Connection", "close");
      request->send(r);

      if (shouldReboot)
        TSK.QueueTask(TASK_FIRMWARE_RESTART);
      else
        SendWithHeaders(request, SERVERINDEX_FILENAME);
    }
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
  {
    if(index == 0)
    {
      // https://github.com/espressif/arduino-esp32/blob/master/libraries/Update/src/Update.h
      // https://esp32.com/viewtopic.php?t=3775
      // https://www.instructables.com/id/Set-Up-an-ESP8266-Automatic-Update-Server/
      // https://techtutorialsx.com/2019/07/21/esp32-arduino-updating-firmware-from-the-spiffs-file-system/
      // https://techtutorialsx.com/2019/03/03/esp32-arduino-spiffs-getting-total-bytes-used/
      // bool canBegin = Update.begin(contentLength, U_FLASH); // update OTA partitions
      // bool canBegin = Update.begin(contentLength, U_SPIFFS); // update SPIFFS partition
      // mkspiffs tool - there are only two SPIFFS binaries: a 1M and a 4M version -
      // constructed with the mkspiffs tool - since all the devices have either 1M or 4M flash.
//      updateSpiffs(updateUrl,SPIFFS_VERSION);
//      updateSpiffs(updateUrl,SPIFFS_VERSION);
//      prtln(SPIFFS.usedBytes());
//      prtln(SPIFFS.totalBytes());
      //bool begin(size_t size=UPDATE_SIZE_UNKNOWN, int command = U_FLASH, int ledPin = -1, uint8_t ledOn = LOW);
//      if(!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH, GPOUT_ONBOARD_LED , HIGH))
//      FSInfo fs_info;
//      SPIFFS.info(fs_info);
//      prt("Total bytes:    "); prtln(fs_info.totalBytes);
//      prt("Used bytes:     "); prtln(fs_info.usedBytes);
//      prt("Block size:     "); prtln(fs_info.blockSize);
//      prt("Page size:      "); prtln(fs_info.pageSize);
//      prt("Max open files: "); prtln(fs_info.maxOpenFiles);
//      prt("Max path length:"); prtln(fs_info.maxPathLength);
//      prtln();

      String fnlc = filename;
      fnlc.toLowerCase();
      
      if (fnlc.indexOf(OTA_UPDATE_SPIFFS_VS_PGM_ID) >= 0)
      {
        int maxSpace = SPIFFS.totalBytes();
        prtln("Update SPIFFS data: \"" + filename + "\", Space available: " + String(maxSpace));
        if(!Update.begin(maxSpace, U_SPIFFS))
          Update.printError(Serial);
      }
      else
      {
        int maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        prtln("Update firmware: \"" + filename + "\", Space available: " + String(maxSketchSpace));
        if(!Update.begin(maxSketchSpace, U_FLASH))
          Update.printError(Serial);
      }
    }
    if(!Update.hasError())
    {
      /* flashing firmware to ESP*/
      if(Update.write(data, len) != len)
        Update.printError(Serial);
    }
    if(final)
    {
      if (Update.end(true))
        Serial.printf("Update Success: %uB\n", index+len);
      else
        Update.printError(Serial);
    }
    yield();
  });

//  uint8_t* g_buf = null;
//  uint8_t* p_buf = null;

  // Route to set GPOUT_SSR1/GPOUT_SSR2
  webServer.on(EP_GET_BUTTONS, HTTP_GET, [](AsyncWebServerRequest *request){
    HandleButtonsReq(request);
  });

  // sct.js heartbeat /getHeart
  webServer.on(EP_GET_HEART, HTTP_GET, [] (AsyncWebServerRequest *request){
    HandleHeartbeatReq(request);
  });

  // index.html: g_sHostName perMax perVal perUnits
  // p1.html: (via %ISAPMODE% we send wifiName wifiPass if in AP wifi mode) phaseSlider dcASlider dcBSlider
  // p2.html: hours minutes ampm onoff delete
  webServer.on(EP_GET_INDEX, HTTP_GET, [] (AsyncWebServerRequest *request){
    HandleIndexReq(request);
  });

  webServer.on(EP_GET_P2, HTTP_GET, [] (AsyncWebServerRequest *request){
    HandleGetP2Req(request);
  });

  webServer.on(EP_ALT_P1, HTTP_GET, [] (AsyncWebServerRequest *request){
    HandleAltP1Req(request);
  });

  webServer.on(EP_GET_P1, HTTP_GET, [] (AsyncWebServerRequest *request){
    HandleGetP1Req(request);
  });

  webServer.on(EP_POST_P2FORM, HTTP_POST, [](AsyncWebServerRequest *request){
    HandleP2FormReq(request);
  });

  webServer.on(EP_POST_P2, HTTP_POST, [] (AsyncWebServerRequest *request){
    HandlePostP2Req(request);
 });

  // needed to detect a disconnect...
//  WiFi.ony7Event(WiFiEvent);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
//  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

  // Create webInputSemaphore
//  webInputSemaphore = xSemaphoreCreateBinary();

  // need this before configTime!
  // PROBLEM: this is creating some sort of WiFi broadcast "blip"
//  WiFi.mode(WIFI_STA);

  if (!InitTime())
    prtln("Failed to set initial time!");
  else{
    prtln("Initial Time/Date: \"" + TimeToString(false) + "\"");
    InitOrRestartSNTP();
  }
    
  SetupAndStartHardwareTimeInterrupt();
  prtln("Timer started...");

  InitHttpClientHandlers();
  
  WiFi.mode(WIFI_OFF);

  RefreshSct();
  
  B64C.init(); // initialize B64Class in Encode.cpp
  
  // tests... (in Tests.cpp)
  Tests();
  TSK.QueueTask(TASK_PRINT_PREFERENCES); // make sure InitTasks() is called before this!
  bootCount = 0;
}

//    response->addHeader("Access-Control-Allow-Headers", "origin, content-type, accept, authorization");
//    response->addHeader("Content-Type", "application/json");
//    response->addHeader("Content-Type", "*");
//    response->addHeader("Access-Control-Allow-Origin", "POST, GET, OPTIONS"); // fixes it for edge but not firefox...
void SendWithHeaders(AsyncWebServerRequest *request, String s){
  AsyncWebServerResponse *r = request->beginResponse(SPIFFS, s.c_str(), String(), false, ReplaceHtmlPercentSign);
  r->addHeader("Access-Control-Allow-Headers", "*");
  r->addHeader("Access-Control-Allow-Origin", "*");
  r->addHeader("Access-Control-Allow-Methods", "*");
  r->addHeader("Cache-Control", "no-cache, no-store");
  r->addHeader("Connection", "close");
  request->send(r);
}

//void IRAM_ATTR onTimer(){
void onTimer(){
  // Increment the counter and set the time of ISR
//  portENTER_CRITICAL_ISR(&timerMux);
//  isrCounter++;
//  lastIsrAt = millis();
//  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}

// call this on powerup and if perMax or perUnits changes
// -1 value means "not yet set" and we can display dashes
void InitStats(){
  // Struggling with this - user can set a wide range of max period from 25*.5sec to 1000*365days, and
  // the slider-value on the index.html web-page will go from 0 (random) up to that max range...
  // So this interval is hard to grasp... seems to make more sense to simply report "time on during the past full hour".
  //g_stats.HalfSecondCount = 2*ComputeMaxPeriod(perMax, perUnits);
  g_stats.HalfSecondCount = GetTimeInterval(g_perVals.perMax, g_perVals.perUnits);

  g_stats.AOnPrevCount = 0;
  g_stats.BOnPrevCount = 0;
  g_stats.PrevDConA = 0;
  g_stats.PrevDConB = 0;
  ClearStatCounters();
}

void ClearStatCounters(){
  g_stats.HalfSecondCounter = 0;
  g_stats.AOnCounter = 0;
  g_stats.BOnCounter = 0;
  g_stats.DConA = 0;
  g_stats.DConB = 0;
}

 // We return a simple time for now of 2*60*60 in half-second units
 // so the user isn't confused by a complex, varying statistics interval...
 // Keep in mind the statistic % on also includes manual on/off interactions... but
 // if we set the auto on/off cycling to some large interval in many hours, this
 // number is too small... except for just showing a dependable "time on over past hour plus current fraction of an hour"
uint32_t GetTimeInterval(uint16_t perMax, uint8_t perUnits){
   return T_ONE_HOUR;
}

uint32_t ComputePhase(){
  uint8_t phasePercent = (g_perVals.phase == 100) ? random(PHASE_MIN, PHASE_MAX) : g_perVals.phase;
  return (uint32_t)(float)(phasePercent*g32_periodTimer)/100.0;
}

// returns a time in .5 second units
uint32_t ComputePeriod(uint8_t perVal, uint16_t perMax, uint8_t perUnits){
  // perUnits is 0=.5 sec resolution, 1=sec, 2=min, 3=hrs
  // perVal is slider's value 0-100% of perMax
  uint32_t iPerVal = (perVal == 0) ? random(PERMAX_MIN, perMax) : (float)(perVal*perMax)/100.0;

  uint32_t iTmp;

  switch(perUnits){
    case 0: // .5 sec
      iTmp = iPerVal;
    break;

    case 1: // sec
    default:
      iTmp = iPerVal*2;
    break;

    case 2: // min
      iTmp = iPerVal*2*60;
    break;

    case 3: // hrs
      iTmp = iPerVal*T_ONE_HOUR;
    break;
  }

  return iTmp;
}

// returns the javaScript used by webpages %MAXSCT% for hnEncode() in sct.js
String getSctMinMaxAsJS(){
return "var varMaxSct='" + String(g_sct) + "," + String(g_minSct) +
        "," + String(g_maxSct) + "'; " + "var varB64T='" + String(B64C.GetTable()) + "';"; 
}

void dnsAndServerStart(){
  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin(g_sHostName.c_str()))
    prtln("Error setting up MDNS responder!");
  else{
    webServer.begin();
    prtln("TCP server started");
  
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
    MDNS.addService(MDNS_SVC, "tcp", 80);
  
    prtln("Host Name: " + g_sHostName);
  
    if (g8_midiChan != MIDICHAN_OFF)
      startMIDI(); // start apple-midi and control-surface libraries

    g_bMdnsOn = true;
    prtln("mDNS responder started");
  }
}

void dnsAndServerStop(){
  if (g_bMdnsOn){
    stopMIDI();
    MDNS.end(); // throws exception if called twice!
    webServer.end();
    g_bMdnsOn = false;
  }
}

// esp_deep_sleep_start();
// esp_light_sleep_start();
// esp_sleep_enable_timer_wakeup(); // enable RTC timer's wakeup
// https://github.com/espressif/esp-idf/tree/7d75213/examples/system/deep_sleep
// https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

  switch(wakeupReason){
    case ESP_SLEEP_WAKEUP_EXT0 : prtln("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : prtln("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : prtln("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : prtln("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : prtln("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeupReason); break;
  }
}

//-------------------------------------------------------------

void loop(){
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
//#endif  //Increment boot number (memory-location is in Real-Time-Clock module) and print it every reboot

#if COMPILE_WITH_MIDI_LIBRARY
  // Listen to incoming notes
  RTP_MIDI.read();
  //Control_Surface.loop(); // handle all midi and control-surface messages
  //MIDI.read();
  //AppleMIDI.read();
#endif

  TSK.RunTasks();

  // return if 1/4 sec. timer has not fired
  if (xSemaphoreTake(timerSemaphore, 0) != pdTRUE)
    return;

//  uint32_t isrCount = 0, isrTime = 0;
    // Read the interrupt count and time
//  portENTER_CRITICAL(&timerMux);
//  isrCount = isrCounter;
//  isrTime = lastIsrAt;
//  portEXIT_CRITICAL(&timerMux);

  // -------- do stuff every .25 sec here

#if ESP32_S3
  // For ESP32_S3 custom board (not yet existing) reads the SSR auto/on/off switch for each relay
  ReadSsrSwitches();
#endif
  
  // read potentiometer and associated mode-switch every 1/4 second unless locked
  if (!IsLocked()){
    // For old ESP32 board, reads SPST POT mode switch (select phase or period to change with POT)
    // For ESP32_S3 board, reads the DPDT mode switch for POT1
#if !DISABLE_POTENTIOMETER    
    ReadPotModeSwitch();
    ReadPot1();
#endif
  }

  FlashSequencer();
  FlashLED();

  if (++g8_quarterSecondTimer < 2)
    return;
  g8_quarterSecondTimer = 0; // reset

  //-------- do stuff every .5 sec here
  TSK.QueueTask(TASK_MAIN_TIMING_CYCLE);
  TSK.QueueTask(TASK_PULSEOFF_TIMING_CYCLE);
  TSK.QueueTask(TASK_STATS_MONITOR);

  //----------------------------------------------
  // Read RTC every .5 seconds of the hardware-timer - if it's changed
  // since the last sample, an RTC second's elapsed...
  //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html
  // Start synchronization via sntp:
  //sntp_setoperatingmode(SNTP_OPMODE_POLL);
  //sntp_setservername(0, "pool.ntp.org");
  //sntp_init();
  time_t now = time(0);
  if (now == g_prevNow)
    return;
  g_prevNow = now;

  if (now % 60 == 0){
    // -------- do stuff every 1 minute here

    IML.DeleteExpiredMdnsIps();
    
    int count = IML.GetCount(); // # of remote units connected
    if (count && g_bWiFiConnected){
      if (count == g_prevMdnsCount){
        if (g_bMaster && ++g16_sendDefTokenTimer >= g16_sendDefTokenTime)
          MasterStartRandomDefaultTokenChange(); // start a system-wide default token reset
      }
      else{ // mDNS count changed...
        g_prevMdnsCount = count;
        g16_sendDefTokenTimer = 0;
      }
      TSK.QueueTask(TASK_PRINT_MDNS_INFO);
    }
    else{
      g_prevMdnsCount = 0;
      g16_sendDefTokenTimer = 0;
    }

    // we have a brand new minute here - which means seconds == 00 on the RTC - but
    // we need to sync seconds for the hw timer still, and reset the software timers
    if (g_bRequestManualTimeSync || g_bRequestWiFiTimeSync){
      DoTimeSyncOneSecondStuff();

      // set "time was set" flag before initializeing lists (the init routines check for time-sync!)
      if (g_bRequestManualTimeSync){
        prtln("Time set manually... enabling time-events!");
        g_bRequestManualTimeSync = false;
        g_bManualTimeWasSet = true;
      }
      if (g_bRequestWiFiTimeSync){
        prtln("Time set via internet... enabling time-events!");
        g_bRequestWiFiTimeSync = false;
        g_bWiFiTimeWasSet = true;
      }

      int iNZScount = TSC.InitSecondsList(g_slotCount); // call this AFTER TSC.CountFullTimeSlots()!
      prtln("Number of items with nonzero seconds: " + String(iNZScount));
      // InitRepeatList will check for expired time-events if clock has been set
      int iRPTcount = TSC.InitRepeatList(g_slotCount);
      prtln("Number of items with repeat count: " + String(iRPTcount));
    }

    // do stuff every minute here
    TSK.QueueTask(TASK_PROCESS_ONE_MINUTE_TIME_SLOTS);
  }

  // -------- do stuff every 1 sec here

  if (g8_clockSetDebounceTimer) // prevent multiple manual clock-set requests
    g8_clockSetDebounceTimer--;

  // expiration for "c restart ip key" command
  if (g16_restartExpirationTimer && --g16_restartExpirationTimer == 0){
    g_sRestartKey = "";
    g_sRestartIp = "";
  }


  // this timer is set either locally to set a master's default-token periodically, or it's initiated
  // remotely via the CMchangeSet command (HttpMsgClass.h). The timer value is passed as the data associated with
  // CMchangeSet - we pass whatever remaining time is on the master-unit's g16_changeSyncTimer - and so all networked
  // units will set their g_defToken at exactly the same second.
  if (g16_changeSyncTimer && --g16_changeSyncTimer == 0)
    while (CNG.RunChanges()); // do all queued changes
    
  // set number of services by reference...
  if (g_bWiFiConnected){
    TSK.QueueTask(TASK_CHECK_MDNS_SEARCH_RESULT);
    
    // if esp32s were found on the network, cycle through them one every 5-10 seconds, sending info
    if (--g16_sendHttpTimer == 0){
      int count = IML.GetCount();
      if (count){
        if (g16_asyncHttpIndex >= count)
          g16_asyncHttpIndex = 0;
        // After command "c test on", g16_oddEvenCounter lets us fold-in transmit of a random text
        // message every other 10-30 second interval...
        if (g_bTest && (g16_oddEvenCounter++ & 1))
          SendText(g16_asyncHttpIndex, genRandMessage(1, TEST_TEXT_MAX_LENGTH));
        else{
          String sIp = IML.GetIP(g16_asyncHttpIndex).toString();
          int txToken = IML.GetToken(g16_asyncHttpIndex, MDNS_TOKEN_TX);
          if (txToken == NO_TOKEN)
            // NOTE: this will send our MAC (last-two-octets) to the remote and receive
            // the remote's MAC (last-two-octets). Also it will set this IP's tx-token and canRx tx token
            // as well as the remote's rx-token and canRx rx token. It does not set this IP's rx-token
            // Etc. The remote will send this IP a CanRx requext to establish the remaining tokens.
            // Once all of that finishes, we begin calling SendHttpReq() if we are a master. The MAC addresses
            // are used to determine a master... (highest last-two octets of all the MACs will be the master)
            TSK.QueueTask(TASK_SEND_CANRX_REQ, sIp);
          else if (g_bMaster){
            TSK.QueueTask(TASK_SEND_HTTP_REQ, sIp);
          }
        }
        g16_asyncHttpIndex++;
      }
      g16_sendHttpTimer = random(SEND_HTTP_TIME_MIN, SEND_HTTP_TIME_MAX+1);
    }
  }

  TSK.QueueTask(TASK_PROCESS_ONE_SECOND_TIME_SLOTS);

  if (++g8_fiveSecondTimer >= FIVE_SECOND_TIME){
    // -------- do stuff every 5 sec here

    // this is clumsy bit all I could think of for now - when the p2.html page is displaying time-slots
    // and then back here in the code we auto-delete an event - we need to update the list. So I'm
    // doing that by setting this flag, then if the flag is set, the 1 second time-request from the
    // user's web-browser (from p2.html) will get an extra "Treload" tacked on to the time/date.
    // we clear this flag there, but it can get "stuck on" if the user closes the page randomly...
    // so here, we clear the flag.
    if (g_bTellP2WebPageToReload)
      g_bTellP2WebPageToReload = false;

    // Encoded base 10 changed-parameter transmit strings for each IP in t_ip_time struct in MdnsClass.h
    TSK.QueueTask(TASK_ENCODE_CHANGED_PARAMETERS);

    TSK.QueueTask(TASK_POLL_WIFI_SWITCH); // also monitors WiFi connection!

    TSK.QueueTask(TASK_SET_PULSEOFF_VARS);

    // https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
    // https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/NTP-TZ-DST/NTP-TZ-DST.ino
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html
    // https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/ (gets time from the web)
    // https://en.cppreference.com/w/c/chrono/localtime
    // return from loop if we re-synced!
    if (CheckForWiFiTimeSync())
      return;

    g8_fiveSecondTimer = 0; // reset
  }

  // 30 seconds...
  if (++g8_thirtySecondTimer >= THIRTY_SECOND_TIME){
    // initiate asynchronous search for new esp32s on the network infrequently...
    if (g_bWiFiConnected)
      TSK.QueueTask(TASK_QUERY_MDNS_SERVICE);
    g8_thirtySecondTimer = 0;
  }
}

// C:\Users\Scott\Documents\Arduino\hardware\espressif\esp32\cores\esp32\esp32-hal-timer.h
void SetupAndStartHardwareTimeInterrupt(){
  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();

  // hw_timer_t * timerBegin(uint32_t frequency);
  g_HwTimer = timerBegin(HW_TIMER_FREQ); // 1MHz

  // Attach onTimer function to our timer.
  // void timerAttachInterrupt(hw_timer_t * timer, void (*userFunc)(void));
  timerAttachInterrupt(g_HwTimer, &onTimer);

  // Set alarm to call onTimer function every 1/4 second (value in microseconds).
  // Repeat the alarm (third parameter)
  g_prevNow = DoTimeSyncOneSecondStuff(time(0));

  // Enable and start timer
  // Set alarm to call onTimer function every 1/4 second (value in microseconds).
  // void timerAlarm(hw_timer_t * timer, uint64_t alarm_value, bool autoreload, uint64_t reload_count);
  timerAlarm(g_HwTimer, HW_TIMER_PERIOD, true, 0); // .25 sec
}

time_t DoTimeSyncOneSecondStuff(time_t now){
  // synchronize to RTC seconds
  // NOTE: we sync to minutes also but after this, in loop()
  while (time(0) == now)
    yield();

  DoTimeSyncOneSecondStuff();

  return now+1;
}

void DoTimeSyncOneSecondStuff(void){
  prtln("Synchronizing RTC to internal timer...");

  // this one is important to make "now" - it rolls over at 2...
  g8_quarterSecondTimer = 2;

  // this one's not used for sync and IS used as a "debounce"
  // for web-input, so leave it 0
  g8_fiveSecondTimer = 0;
  g8_thirtySecondTimer = THIRTY_SECOND_TIME-1; // used for mDns async. searches

  //g8_clockSetDebounceTimer = 0; DON'T DO THIS HERE! this negates the debounce!

  ResetPeriod();

  HardwareTimerRestart(g_HwTimer);
}

// call this when the internal clock is changed or set
// FYI: C:\Users\Scott\Documents\Arduino\hardware\espressif\esp32\cores\esp32\esp32-hal-timer.h
void HardwareTimerRestart(hw_timer_t * timer){
  prtln("Restarting hardware timer...");
  timerRestart(timer);
}

void SSR1On(uint32_t iPeriod){
  if (g8_ssr1ModeFromWeb == SSR_MODE_AUTO)
    SetSSR(GPOUT_SSR1, true);

  // random mode is 0
  if (g_perVals.dutyCycleA == 0)
    g32_dutyCycleTimerA = random(MIN_RAND_PERCENT_DUTY_CYCLE, DUTY_CYCLE_MAX);
  else
    g32_dutyCycleTimerA = g_perVals.dutyCycleA;
  g32_dutyCycleTimerA = (float)(g32_dutyCycleTimerA*iPeriod)/100.0;
}

void SSR2On(uint32_t iPeriod){
  if (g8_ssr2ModeFromWeb == SSR_MODE_AUTO)
    SetSSR(GPOUT_SSR2, true);

  // random mode is 0
  if (g_perVals.dutyCycleB == 0)
    g32_dutyCycleTimerB = random(MIN_RAND_PERCENT_DUTY_CYCLE, DUTY_CYCLE_MAX);
  else
    g32_dutyCycleTimerB = g_perVals.dutyCycleB;
  g32_dutyCycleTimerB = (float)(g32_dutyCycleTimerB*iPeriod)/100.0;
}

// return true if timers reset (need to return from loop())
bool CheckForWiFiTimeSync(){
  // don't check unless wifi and time's not yet been set or if
  // in the process of synchronizing minutes...
  if (!g_bWiFiConnected || g_bManualTimeWasSet || g_bWiFiTimeWasSet || g_bRequestWiFiTimeSync || g_bRequestManualTimeSync)
    return false;

  struct tm timeInfo = {0};
  if (ReadInternalTime(NULL, &timeInfo) == NULL) // get current time as struct tm
    return false;

  int myYear = timeInfo.tm_year + EPOCH_YEAR;

  if (myYear > DEF_YEAR){
    prtln("Requesting WiFi time-sync...");
    g_bRequestWiFiTimeSync = true;
    return true;
  }
  //else if (myYear < DEF_YEAR)
  //  prtln("Y2038 Issue??? CheckForTimeSync() year: " + String(myYear) + " less than DEF_YEAR: " + String(DEF_YEAR));
  return false;
}

// ====================================================================================
// Add some MIDI elements for testing
// ====================================================================================

// send Control Change Sustain On
// AppleMIDI.controlChange(0x40, 0x7F, MIDI_CHANNEL);
// send Control Change Sustain Off
// AppleMIDI.controlChange(0x40, 0x00, MIDI_CHANNEL);
// send Note On
// AppleMIDI.noteOn(0x2b, 0x64, MIDI_CHANNEL);
// send Note Off
// AppleMIDI.noteOff(0x2b, 0x64, MIDI_CHANNEL);

//using namespace MIDI_Notes;
// This one works for us - good for testing! - S.S.
// This one works for us - good for testing! - S.S.
//NoteValueLED led = {
//    GPOUT_ONBOARD_LED, note(C, 4),
//};
//NoteButton button = {
//    0, note(C, 4),  // GPIO0 has a push button connected on most boards
//};

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================
#if COMPILE_WITH_MIDI_LIBRARY

void startMIDI(){
    AppleRTP_MIDI.setName(g_sHostName.c_str());

    // Set up some AppleMIDI callback handles
    AppleRTP_MIDI.setHandleConnected(onAppleMidiConnected);
    AppleRTP_MIDI.setHandleDisconnected(onAppleMidiDisconnected);
//    AppleRTP_MIDI.setHandleError(onAppleMidiError);

    // Initialize Control Surface (also calls MIDI.begin())
//    Control_Surface.begin();
//    MIDI.begin(2);
//    MIDI.begin(MIDI_CHANNEL_OMNI);

    RTP_MIDI.setHandleNoteOn(OnMidiNoteOn);
    RTP_MIDI.setHandleNoteOff(OnMidiNoteOff);
//    MIDI.setHandleNoteOn(OnMidiNoteOn);
//    MIDI.setHandleNoteOff(OnMidiNoteOff);
//  MIDI.setHandleAfterTouchPoly(OnAfterTouchPoly);
//  MIDI.setHandleControlChange(OnControlChange);
//  MIDI.setHandleProgramChange(OnProgramChange);
//  MIDI.setHandleAfterTouchChannel(OnAfterTouchChannel);
//  MIDI.setHandlePitchBend(OnPitchBend);
//  MIDI.setHandleSystemExclusive(OnSystemExclusive);
//  MIDI.setHandleTimeCodeQuarterFrame(OnTimeCodeQuarterFrame);
//  MIDI.setHandleSongPosition(OnSongPosition);
//  MIDI.setHandleSongSelect(OnSongSelect);
//  MIDI.setHandleTuneRequest(OnTuneRequest);
//  MIDI.setHandleClock(OnClock);
//  MIDI.setHandleStart(OnStart);
//  MIDI.setHandleContinue(OnContinue);
//  MIDI.setHandleStop(OnStop);
//  MIDI.setHandleActiveSensing(OnActiveSensing);
//  MIDI.setHandleSystemReset(OnSystemReset);

    // MIDI_CHANNEL_OMNI == MIDICHAN_ALL == 0, MIDI_CHANNEL_OFF == MIDICHAN_OFF == 17 and over
    if (g8_midiChan == MIDICHAN_ALL)
      RTP_MIDI.begin(MIDI_CHANNEL_OMNI);
    else
      RTP_MIDI.begin(g8_midiChan);

    // Add service to MDNS-SD
    MDNS.addService("apple-midi", "udp", AppleRTP_MIDI.getPort());

    prtln("AppleMIDI started: " + g_sHostName);
}

void stopMIDI(){
  if (g_bMidiConnected)
    AppleRTP_MIDI.sendEndSession();
  mdns_service_remove("_apple-midi", "_udp");
  prtln("AppleMIDI stopped!");
}

void onAppleMidiConnected(const ssrc_t &ssrc, const char *name){
  g_bMidiConnected  = true;
  prtln("Apple MIDI connected to session " + String(name));
}

void onAppleMidiDisconnected(const ssrc_t &ssrc){
  g_bMidiConnected  = false;
  prtln("Apple MIDI disconnected");
}

void onAppleMidiError(const ssrc_t &ssrc, int32_t err){
  prtln("AppleMidiError: " + String(err));
}

// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks
// MIDI. (handlers)
//void handleNoteOff(byte channel, byte note, byte velocity);
//void handleNoteOn(byte channel, byte note, byte velocity);
//void handleAfterTouchPoly(byte channel, byte note, byte pressure);
//void handleControlChange(byte channel, byte number, byte value);
//void handleProgramChange(byte channel, byte number);
//void handleAfterTouchChannel(byte channel, byte pressure);
//void handlePitchBend(byte channel, int bend);
//void handleSystemExclusive(byte* array, unsigned size);
//void handleTimeCodeQuarterFrame(byte data);
//void handleSongPosition(unsigned int beats);
//void handleSongSelect(byte songnumber);
//void handleTuneRequest(void);
//void handleClock(void);
//void handleStart(void);
//void handleContinue(void);
//void handleStop(void);
//void handleActiveSensing(void);
//void handleSystemReset(void);

void OnMidiNoteOn(uint8_t chan, uint8_t note, uint8_t velocity){

//  Serial.print(F("Incoming NoteOn from channel:"));
//  Serial.print(chan);
//  Serial.print(F(" note:"));
//  Serial.print(note);
//  Serial.print(F(" velocity:"));
//  Serial.print(velocity);
//  Serial.println();

  if (velocity == 0)
    OnMidiNoteOff(chan, note, velocity);
  else{
    if (note == g8_midiNoteA && g8_ssr1ModeFromWeb == SSR_MODE_OFF)
      SetSSR(GPOUT_SSR1, true);
    if (note == g8_midiNoteB && g8_ssr2ModeFromWeb == SSR_MODE_OFF)
      SetSSR(GPOUT_SSR2, true);
  }
}

void OnMidiNoteOff(uint8_t chan, uint8_t note, uint8_t velocity){
//  Serial.print(F("Incoming NoteOff from channel:"));
//  Serial.print(chan);
//  Serial.print(F(" note:"));
//  Serial.print(note);
//  Serial.print(F(" velocity:"));
//  Serial.print(velocity);
//  Serial.println();
//
  if (note == g8_midiNoteA && g8_ssr1ModeFromWeb == SSR_MODE_OFF)
    SetSSR(GPOUT_SSR1, false);
  if (note == g8_midiNoteB && g8_ssr2ModeFromWeb == SSR_MODE_OFF)
    SetSSR(GPOUT_SSR2, false);
}
// has to be in FanController.ino!
void TaskMidiChan(){
  if (g_bWiFiConnected){
    if (g8_midiChan == MIDICHAN_OFF){
      if (g_bMidiConnected)
        stopMIDI();
    }
    else if (!g_bMidiConnected)
      startMIDI();
  }

  PC.PutPrefU8(EE_MIDICHAN, g8_midiChan);
  if (g8_midiChan == MIDICHAN_OFF)
    RTP_MIDI.setInputChannel(MIDI_CHANNEL_OFF);
  else if (g8_midiChan == MIDICHAN_ALL)
    RTP_MIDI.setInputChannel(MIDI_CHANNEL_OMNI);
  else
    RTP_MIDI.setInputChannel(g8_midiChan);
  PrintMidiChan();
}
#else
void stopMIDI(){
  
}
void startMIDI(){
  
}
void TaskMidiChan(){
}
#endif
