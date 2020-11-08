/*********
  Install instructions: 
  https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/windows.md
  
  Thanks to Rui Santos for his great tutorials and examples!
  
  WiFi Smart Fan Controller is by Scott Swift, Christian.
  
  NOTE: Use the Arduino->Tools->Partition Scheme Minimal SPIFFS (1.9MB APP with OTA, 190Kb SPIFFS)

  I use Sketch->Export Compiled Binary then run FixName.bat to change the .bin file to fc.bin
  Next, I run the ESP32 utility "mkspiffs.exe" via fcspiffs.bat
  fcspiffs.bat has "mkspiffs.exe -p 256 -b 4096 -s 0x30000 -c ..\data fc.spiffs.bin"

  If you use Partition Scheme (Minimal 1.3MB App,700Kb SPIFFS), use:
  mkspiffs.exe -p 256 -b 4096 -s 1376256 -c ..\data fc.spiffs.bin
  
*********/
#include "FanController.h"

// FYI:
// Using the flash-string function to conserve SRAM
// client.println(F("<H3>Arduino with Ethernet Shield</H2>"));

// web-server reference
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

// https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
// The ESP32 has 8kB SRAM on the RTC part, called RTC fast memory. The data saved here is not erased
// during deep sleep. However, it is erased when you press the reset button (the button labeled EN on the ESP32 board).
// To save data in the RTC memory, you just have to add RTC_DATA_ATTR before a variable definition.
// The example saves the bootCount variable on the RTC memory. This variable will count how many times
// the ESP32 has woken up from deep sleep.

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
const char TIMEZONE[] = "CST6CDT";
const char NTP_SERVER1[] = "pool.ntp.org"; // you can have more than one URL, comma delimited!
const char NTP_SERVER2[] = "time.nist.gov";

const char VERSION_STR[] = DTS_VERSION;

const char WEB_PAGE_LIFORM[] = "/serverIndex.html";
const char WEB_PAGE_INDEX[] = "/index.html";
const char WEB_PAGE_P1[] = "/p1.html";
const char WEB_PAGE_P2[] = "/p2.html";

// these are 15 chars max!
const char EE_SLOT_PREFIX[]    = "EE_SLOT_"; // will become EE_SLOT_000
const char EE_PERMAX[]         = "EE_PERMAX";
const char EE_PERUNITS[]       = "EE_PERUNITS";
const char EE_PERVAL[]         = "EE_PERVAL";
const char EE_DC_A[]           = "EE_DC_A";
const char EE_DC_B[]           = "EE_DC_B";
const char EE_PHASE[]          = "EE_PHASE";
const char EE_RELAY_A[]        = "EE_RELAY_A";
const char EE_RELAY_B[]        = "EE_RELAY_B";
const char EE_HOSTNAME[]       = "EE_HOSTNAME";
const char EE_SSID[]           = "EE_SSID";
const char EE_OLDSSID[]        = "EE_OLDSSID";
const char EE_PWD[]            = "EE_PWD";
const char EE_OLDPWD[]         = "EE_OLDPWD";
const char EE_LOCKCOUNT[]      = "EE_LOCKCOUNT"; // 0xff=unlocked, 0x00=locked
const char EE_LOCKPASS[]       = "EE_LOCKPASS";

const char DEFAULT_SOFTAP_SSID[] = "dts7";
const char DEFAULT_SOFTAP_PWD[]  = "1234567890";

const char DEFAULT_HOSTNAME[] = "dts7";
const char DEFAULT_SSID[]     = "YourRouterSSID";
const char DEFAULT_PWD[]      = "YourRouterPassword";
const char OBFUSCATE_STR[]    = "somerandomstringtoobfuscate";

// you can type these commands into the HOSTNAME web-edit field and submit the command!
const char COMMAND_RESET[]   = "reset";
const char SC_RESET_PARMS[]  = "parms";
const char SC_RESET_SLOTS[]  = "slots";

const char COMMAND_WIFI[]    = "wifi";
const char SC_WIFI_TOGGLE[]  = "toggle";
const char SC_WIFI_RESTORE[] = "restore";
const char SC_WIFI_DISABLE[] = "disable";

const char COMMAND_VERSION[] = "version";
const char COMMAND_INFO[]    = "info";
const char COMMAND_UPDATE[]  = "update";
const char COMMAND_LOCK[]    = "lock";
const char COMMAND_UNLOCK[]  = "unlock";


// Web-page placeholders we fill-in dynamically as the html file is "served"
// index.html
const char PH_HOSTNAME[] = "HOSTNAME";
const char PH_MAXSCT[] = "MAXSCT";
const char PH_PERMAX[] = "PERMAX";
const char PH_PERUNITS[] = "PERUNITS";
const char PH_PERVAL[] = "PERVAL";
const char PH_STATE1[] = "STATE1";
const char PH_STATE2[] = "STATE2";
const char PH_MODE1[] = "MODE1";
const char PH_MODE2[] = "MODE2";
// p1.html
const char PH_ISAPMODE[] = "ISAPMODE";
const char PH_PHASE[] = "PHASE";
const char PH_DC_A[] = "DC_A";
const char PH_DC_B[] = "DC_B";

// p2.html
const char PH_DELSTYLE[] = "DELSTYLE";
const char PH_DELETEITEMS[] = "DELETEITEMS";

// index.html
const char PARAM_HOSTNAME[]    = "hnEnc";
const char PARAM_PERMAX[]      = "perMax";
const char PARAM_PERUNITS[]    = "perUnits";
const char PARAM_PERVAL[]      = "perVal";
const char PARAM_STATE1[]      = "stateVal1";
const char PARAM_STATE2[]      = "stateVal2";

// loginIndex.html
const char PARAM_UDID[]   = "hidUdId"; // used un firmware update
const char PARAM_UDPW[]   = "hidUdPw";

// p1.html
const char PARAM_BUTRST[]     = "butRst"; // restore button press
const char PARAM_WIFINAME[]   = "hidName"; // set by hidden field on p1.html - variable %ISAPMODE% is replaced by two edit-input fields
const char PARAM_WIFIPASS[]   = "hidPass"; // data submitted is handled in in processor().
const char PARAM_PHASE[]      = "phaseVal";
const char PARAM_DC_A[]       = "dcAVal";
const char PARAM_DC_B[]       = "dcBVal";

// p2.html
// received from p2.html when Add button pressed
const char PARAM_MINUTE[] = "minutes";
const char PARAM_HOUR[] = "hours";
const char PARAM_SECOND[] = "seconds";
const char PARAM_AMPM[] = "pmFlag";
const char PARAM_DEVICE_MODE[] = "deviceMode";
const char PARAM_DEVICE_ADDR[] = "deviceAddr";
const char PARAM_REPEAT_MODE[] = "repeatMode";
const char PARAM_REPEAT_COUNT[] = "repeatCount";
const char PARAM_EVERY_COUNT[] = "everyCount";
const char PARAM_DATE[] = "myDate";
// when Edit button pressed on p2.html
const char PARAM_EDITITEM[] = "editItem";
const char PARAM_EDITINDEX[] = "editIndex";
const char PARAM_DELINDEX[] = "delIndex";
// received from p2.html
const char PARAM_REPLACEINDEX[] = "replaceIndex";
const char PARAM_INCLUDETIMINGCYCLE[] = "cbCycle";
const char PARAM_TIMINGCYCLEINREPEATS[] = "cbRepeats";
const char PARAM_USEOLDTIMEVALS[] = "useOldTimeVals";
const char PARAM_DELITEMS[] = "delItems";
const char PARAM_DATETIME[] = "dateTime";
const char PARAM_FILEDATA[] = "fileData";
const char PARAM_ERASEDATA[] = "eraseData";

// RTC_SLOW_ATTR, RTC_FAST_ATTR, RTC_RODATA_ATTR - read-only
// (RTC_IRAM_ATTR can be used in a function declaration to put it in RTC memory!)
RTC_DATA_ATTR int bootCount;

// stackArray[] - Interesting! FYI
//    t_event stackArray[m_slotCount];

int m_slotCount;

// three-position switch with pulldowns on the GPIO pins
uint8_t sw1Value, sw2Value, oldSw1Value, oldSw2Value;
uint8_t nvSsrMode1, nvSsrMode2, m_taskMode, m_sct, m_max;

// variable for storing the potentiometer value
uint16_t pot1Value, oldPot1Value;

bool bWiFiConnected, bWiFiConnecting, bSoftAP, bWiFiDisabled;
bool bResetOrPowerLoss, bTellP2WebPageToReload;
bool bManualTimeWasSet, bWiFiTimeWasSet, bValidated;
bool bRequestManualTimeSync, bRequestWiFiTimeSync;

// timers
uint16_t dutyCycleTimerA, dutyCycleTimerB, periodTimer, savePeriod, phaseTimer;
uint16_t m_taskTimer, m_taskData;
uint8_t quarterSecondTimer, fiveSecondTimer;
uint8_t dutyCycleA, dutyCycleB, phase; // saved in Preferences (units are %)
uint8_t perUnits, perMax, perVal; // perUnits and perMax are indices into selected option in index.html
uint8_t ledFlashTimer, clockSetDebounceTimer, m_lockCount;

// previous time-date used to facilitate repeat functions
// following initial time-slot trigger (see p2.html)
time_t m_prevNow;
t_time_date m_prevDateTime;

// Stores states
String ledState, ssr1State, ssr2State, swState, inputMessage, inputParam, hostName, m_ssid;

hw_timer_t * m_timer;
volatile SemaphoreHandle_t timerSemaphore, webInputSemaphore;
portMUX_TYPE timerMux; // synchronization between main loop and ISR

// Create AsyncWebServer object on port 80
AsyncWebServer webServer(SERVER_PORT);

// create an instance of Preferences library
Preferences preferences;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  // SW_SOFT_AP (all four POT pins are input only, no pullup/down!)
  pinMode(SW_SOFT_AP, INPUT); // GPIO34 toggel switch where a POT normally would go - used to boot to softAP WiFi mode
  
  pinMode(SW_1, INPUT_PULLDOWN);
  pinMode(SW_2, INPUT_PULLDOWN);
  pinMode(ONBOARD_LED_GPIO2, OUTPUT); // set internal LED (blue) as output
  pinMode(SSR_1, OUTPUT);
  pinMode(SSR_2, OUTPUT);
  
  //Increment boot number (memory-location is in Real-Time-Clock module) and print it every reboot
  prtln("Boot number: " + String(++bootCount));
  
  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  
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
  
  // insure "old" values for POT the same
  oldPot1Value = pot1Value = analogRead(POT_1);

  // insure "old" values for SWITCH different
  sw1Value = digitalRead(SW_1);
  oldSw1Value = ~sw1Value;
  sw2Value = digitalRead(SW_2);
  oldSw2Value = ~sw2Value;
      
  digitalWrite(ONBOARD_LED_GPIO2, LOW);
  ledState = "OFF";
  
  bootCount = 0;
  clockSetDebounceTimer = 0;

  m_taskTimer = 0;
  m_taskMode = 0;
  m_taskData = 0;
  m_max = 1;
  m_sct = 1;
  
  m_timer = NULL;
  timerMux = portMUX_INITIALIZER_UNLOCKED;
  
  bWiFiConnected = false;
  bWiFiConnecting = false;
  bWiFiDisabled = false;
  bSoftAP = false;
  bTellP2WebPageToReload = false;
  bValidated = false;
  
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
  bResetOrPowerLoss = (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) ? true : false; // if reset or power loss

// NOTE: GetPreferences() sets period (which is in percent) and nvPeriodMax from
// SPIFFS non-volitile memory
#if CLEAR_PREFS
  ErasePreferences();
#endif
#if CLEAR_SLOTS
  EraseTimeSlots();
#endif

  GetPreferences();
  
  // example using usa eastern standard/eastern daylight time
  // edt begins the second sunday in march at 0200
  // est begins the first sunday in november at 0200
  // "EST5EDT4,M3.2.0/02:00:00,M11.1.0/02:00:00"
  
  //setenv("TZ", "EST+5", 1); // Set timezone

// don't need this if calling configTzTime????
//  setenv("TZ", TIMEZONE, 1); // Set timezone
//  tzset();

  // Initialize SPIFFS
  if(!SPIFFS.begin(true))
  {
    prtln("An Error has occurred while mounting SPIFFS");
    return;
  }

//    WL_CONNECTED: assigned when connected to a WiFi network;
//    WL_NO_SHIELD: assigned when no WiFi shield is present;
//    WL_IDLE_STATUS: it is a temporary status assigned when WiFi.begin() is called and
//      remains active until the number of attempts expires (resulting in WL_CONNECT_FAILED)
//      or a connection is established (resulting in WL_CONNECTED);
//    WL_NO_SSID_AVAIL: assigned when no SSID are available;
//    WL_SCAN_COMPLETED: assigned when the scan networks is completed;
//    WL_CONNECT_FAILED: assigned when the connection fails for all the attempts;
//    WL_CONNECTION_LOST: assigned when the connection is lost;
//    WL_DISCONNECTED: assigned when disconnected from a network;   // Connect to Wi-Fi
  //WiFi.mode(WIFI_STA);
  webServer.onNotFound(notFound);
  
  //Let’s say we are designing a network application. Let’s list down few
  //URIs and their purpose to get better understanding when to use POST 
  //and when to use PUT operations.
  //
  //GET   /device-management/devices : Get all devices
  //POST  /device-management/devices : Create a new device
  //
  //GET   /device-management/devices/{id} : Get the device information identified by "id"
  //PUT   /device-management/devices/{id} : Update the device information identified by "id"
  //DELETE  /device-management/devices/{id} : Delete device by "id"
  
  // Send web page with input fields to client
  // Route for root / web page
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    // flag = true saves as attachment! String() is the content-type
    // processor is the handler function, SPIFFS serves page from file-system
    // /index.html is the file to serve based from root '/'
//    request->send(SPIFFS, "/index.html", String(), false, processor);

    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", String(), false, processor);
    SetHeaders(response);
    request->send(response);
  });
  
  webServer.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", String(), false, processor);
    SetHeaders(response);
    request->send(response);
  });

  // Route for p1 web page
  webServer.on("/p1.html", HTTP_GET, [](AsyncWebServerRequest *request)
  {
//    request->send(SPIFFS, "/p1.html", String(), false, processor);
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/p1.html", String(), false, processor);
    SetHeaders(response);
    request->send(response);
  });
  
  // Route for p2 web page
  webServer.on("/p2.html", HTTP_GET, [](AsyncWebServerRequest *request)
  {
//    request->send(SPIFFS, "/p2.html", String(), false, processor);
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/p2.html", String(), false, processor);
    SetHeaders(response);
    request->send(response);
  });
  
  // Route to load p2.js file
  webServer.on("/p2.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/p2.js", "text/javascript");
//    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/p2.js", "text/javascript");
//    SetHeaders(response);
//    request->send(response);
  });

  // Route to load sct.js file
  webServer.on("/sct.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/sct.js", "text/javascript");
//    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/sct.js", "text/javascript");
//    SetHeaders(response);
//    request->send(response);
  });

  // Route to load style.css files
  webServer.on("/style1.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/style1.css", "text/css");
  });
  webServer.on("/style2.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/style2.css", "text/css");
  });
  webServer.on("/style3.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/style3.css", "text/css");
  });

  // loginIndex is launched when we enter "c update" and press submit while on p1.html in AP mode
  webServer.on("/loginIndex", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/loginIndex.html", String(), false, processor);
    response->addHeader("Connection", "close");
    request->send(response);
  });

// don't enable this - it will negate security if you do - we send it below...
  // not able to get this working thus far - to eliminate much of the above...
  //webServer.serveStatic("/", SPIFFS, "/www/"); // upload to root directory (if you add "www" directory in "data", last param becomes "/www/")
  
  webServer.on("/update", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    if (IsLockedAlertPost(request))
      return;

    if (!bValidated)
    {
      request->send(204, "text/html", "");
      return;
    }
      
    bool shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
    delay(1000);
    ESP.restart();
    
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
//      if(!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH, ONBOARD_LED_GPIO2 , HIGH))
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
      
      if (fnlc.indexOf("spiffs") >= 0)
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
  
  // Route to set GPIO SSR_1 to HIGH
  webServer.on("/buttons", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!IsLockedAlertGet(request, WEB_PAGE_INDEX)){
      if (request->hasParam("A")){
        String buttonMode = request->getParam("A")->value();
        if (buttonMode == "0"){
          if (nvSsrMode1 != SSR_MODE_OFF){
            nvSsrMode1 = SSR_MODE_OFF;
            SetState(SSR_1, nvSsrMode1);
            m_taskMode = TASK_RELAY_A;
            m_taskTimer = TASK_TIME;
          }
        }
        else if (buttonMode == "1"){
          if (nvSsrMode1 != SSR_MODE_ON){
            nvSsrMode1 = SSR_MODE_ON;
            SetState(SSR_1, nvSsrMode1);
            m_taskMode = TASK_RELAY_A;
            m_taskTimer = TASK_TIME;
          }
        }
        else if (buttonMode == "2"){
          if (nvSsrMode1 != SSR_MODE_AUTO){
            nvSsrMode1 = SSR_MODE_AUTO;
            SetState(SSR_1, nvSsrMode1);
            m_taskMode = TASK_RELAY_A;
            m_taskTimer = TASK_TIME;
          }
        }
      }
      else if (request->hasParam("B")){
        String buttonMode = request->getParam("B")->value();
        if (buttonMode == "0"){
          if (nvSsrMode2 != SSR_MODE_OFF){
            nvSsrMode2 = SSR_MODE_OFF;
            SetState(SSR_2, nvSsrMode2);
            m_taskMode = TASK_RELAY_B;
            m_taskTimer = TASK_TIME;
          }
        }
        else if (buttonMode == "1"){
          if (nvSsrMode2 != SSR_MODE_ON){
            nvSsrMode2 = SSR_MODE_ON;
            SetState(SSR_2, nvSsrMode2);
            m_taskMode = TASK_RELAY_B;
            m_taskTimer = TASK_TIME;
          }
        }
        else if (buttonMode == "2"){
          if (nvSsrMode2 != SSR_MODE_AUTO){
            nvSsrMode2 = SSR_MODE_AUTO;
            SetState(SSR_2, nvSsrMode2);
            m_taskMode = TASK_RELAY_B;
            m_taskTimer = TASK_TIME;
          }
        }
      }
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  webServer.on("/getLIform", HTTP_GET, [] (AsyncWebServerRequest *request)
  {
    if (request->hasParam(PARAM_UDID) && request->hasParam(PARAM_UDPW))
    {
      String id = hnDecode(request->getParam(PARAM_UDID)->value());
      String pw = hnDecode(request->getParam(PARAM_UDPW)->value());
      if (id == UPLOAD_USERID && pw == UPLOAD_USERPW) // change to stored values - maybe use hostName (???)
      {
        bValidated = true;
        AsyncWebServerResponse *r = request->beginResponse(SPIFFS, WEB_PAGE_LIFORM, String(), false, processor);
        r->addHeader("Connection", "close");
        request->send(r);
      }
      else
      {
        bValidated = false;
        request->send(200, "text/html", "<script>alert('Incorrect credentials!');location.href = '" + String(WEB_PAGE_INDEX) + "'</script>");
      };
        
      id = OBFUSCATE_STR; // obfuscate memory for security
      pw = OBFUSCATE_STR;
    }
  });

  webServer.on("/getIndex", HTTP_GET, [] (AsyncWebServerRequest *request)
  {
    byte iStatus = 0; // status 0 = no send needed at end, 1 = OK response, 2 = fail response
    
    inputMessage = "";
    inputParam = "";
    
    String s = ""; // if this in not empty later, we send it as code 200 to the client
    // index.html: hostName perMax perVal perUnits
    // p1.html: (via %ISAPMODE% we send wifiName wifiPass if in AP wifi mode) phaseSlider dcASlider dcBSlider
    // p2.html: hours minutes ampm onoff delete
    
    if (request->hasParam(PARAM_STATE1))
      // this sends outlet mode and current state to javascript in our HTML web-page
      s = "Outlet1: <strong> " + ssr1State + ", " + SsrModeToString(nvSsrMode1) + "</strong>";
    else if (request->hasParam(PARAM_STATE2))
      s = "Outlet2: <strong> " + ssr2State + ", " + SsrModeToString(nvSsrMode2) + "</strong>";
    else if (request->hasParam(PARAM_HOSTNAME))
    {
      inputMessage = request->getParam(PARAM_HOSTNAME)->value();
      inputParam = PARAM_HOSTNAME;

      String cmd = hnDecode(inputMessage);
      cmd.trim();
      
      // process commands first... "c somecommand"
      if (cmd.length() > 2 && (cmd[0] == 'c' || cmd[0] == 'C') && cmd[1] == ' ')
        ProcessCommand(request, s, cmd); // all by reference!
      else if (m_lockCount == 0xff) // set WiFi hostName
        SetWiFiHostName(request, s, cmd);
      else
        s = "<script>alert('Interface is locked!');";
        
      if (s != "")
        s += "location.href = '" + String(WEB_PAGE_INDEX) + "';</script>";
    }
    else
    {
      if (IsLockedAlertGet(request, WEB_PAGE_INDEX))
        return;
        
      // locked commands...  
      if (request->hasParam(PARAM_PERMAX))
      {
        inputMessage = request->getParam(PARAM_PERMAX)->value();
        inputParam = PARAM_PERMAX;
        perMax = inputMessage.toInt();
        m_taskMode = TASK_PERMAX;
        m_taskTimer = TASK_TIME;
        iStatus = 1;
      }
      else if (request->hasParam(PARAM_PERUNITS))
      {
        inputMessage = request->getParam(PARAM_PERUNITS)->value();
        inputParam = PARAM_PERUNITS;
        perUnits = inputMessage.toInt();
        m_taskMode = TASK_PERUNITS;
        m_taskTimer = TASK_TIME;
        iStatus = 1;
      }
      else if (request->hasParam(PARAM_PERVAL))
      {
        inputMessage = request->getParam(PARAM_PERVAL)->value();
        inputParam = PARAM_PERVAL;
        perVal = inputMessage.toInt();
        m_taskMode = TASK_PERVAL;
        m_taskTimer = TASK_TIME;
        iStatus = 1;
      }
    }
    
    // 200 = OK
    // 204 = OK but No Content
    //request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
    //                                 + inputParam + ") with value: " + inputMessage +
    //                                 "<br><a href=\"/\">Return to Home Page</a>");
    if (s != "")
      request->send(200, "text/html", s.c_str());
    else if (iStatus == 1)
      request->send(204, "text/html", "");
    else if (iStatus == 2)
    {
      s = "<script>alert('Out of range (" + inputParam + ") with value: " + inputMessage + "');location.href = '" + String(WEB_PAGE_INDEX) + "';</script>";
      request->send(200, "text/html", s.c_str());
    }
    
    // Give a semaphore that we can check in the loop (just prints the inputMessege)
    xSemaphoreGiveFromISR(webInputSemaphore, NULL);
  });
  
      
  webServer.on("/getP1", HTTP_GET, [] (AsyncWebServerRequest *request)
  {
    byte iStatus = 0; // status 0 = no send needed at end, 1 = OK response
    
    inputMessage = "";
    inputParam = "";
    
    String s = ""; // if this in not empty later, we send it as code 200 to the client
    // index.html: hostName perMax perVal perUnits
    // p1.html: (via %ISAPMODE% we send wifiName wifiPass if in AP wifi mode) phaseSlider dcASlider dcBSlider
    // p2.html: hours minutes ampm onoff delete
    
    if (IsLockedAlertGet(request, WEB_PAGE_P1))
      return;
      
    if (request->hasParam(PARAM_PHASE))
    {
      inputMessage = request->getParam(PARAM_PHASE)->value();
      inputParam = PARAM_PHASE;
      phase = inputMessage.toInt();
      if (phase < PHASE_MIN)
        phase = PHASE_MIN;
      else if (phase > PHASE_MAX)
        phase = PHASE_MAX;
      m_taskMode = TASK_PHASE;
      m_taskTimer = TASK_TIME;
      iStatus = 1;
    }
    else if (request->hasParam(PARAM_DC_A))
    {
      inputMessage = request->getParam(PARAM_DC_A)->value();
      inputParam = PARAM_DC_A;
      dutyCycleA = inputMessage.toInt();
      if (dutyCycleA < DUTY_CYCLE_MIN)
        dutyCycleA = DUTY_CYCLE_MIN;
      else if (dutyCycleA > DUTY_CYCLE_MAX)
        dutyCycleA = DUTY_CYCLE_MAX;
      m_taskMode = TASK_DCA;
      m_taskTimer = TASK_TIME;
      iStatus = 1;
    }
    else if (request->hasParam(PARAM_DC_B))
    {
      inputMessage = request->getParam(PARAM_DC_B)->value();
      inputParam = PARAM_DC_B;
      dutyCycleB = inputMessage.toInt();
      if (dutyCycleB < DUTY_CYCLE_MIN)
        dutyCycleB = DUTY_CYCLE_MIN;
      else if (dutyCycleB > DUTY_CYCLE_MAX)
        dutyCycleB = DUTY_CYCLE_MAX;
      m_taskMode = TASK_DCB;
      m_taskTimer = TASK_TIME;
      iStatus = 1;
    }
    else if (request->hasParam(PARAM_BUTRST))
    {
      if (bSoftAP && request->getParam(PARAM_BUTRST)->value() == String(m_sct))
      {
        m_taskMode = TASK_RESTORE;
        m_taskTimer = TASK_TIME;
        prtln("Received command to restore WiFi credentials!");
        s = "<script>alert('SSID and password reset to previous values!');";
      }
      else
        iStatus = 1;
    }
    else if (request->hasParam(PARAM_WIFINAME) && request->hasParam(PARAM_WIFIPASS))
    {
      if (bSoftAP)
      {
        String valN = hnDecode(request->getParam(PARAM_WIFINAME)->value());
        int lenN = valN.length();
        String valP = hnDecode(request->getParam(PARAM_WIFIPASS)->value());
        int lenP = valP.length();
        if (lenN >= 1 && lenN <= 32 && lenP >= 1 && lenP <= 63)
        {
          if (valN[0] != ' ' && valN[lenN-1] != ' ' && valP[0] != ' ' && valP[lenP-1] != ' ')
          {
            if (valN != m_ssid)
            {
              PutPreferenceString(EE_SSID, valN); // save new ssid
              PutPreferenceString(EE_OLDSSID, m_ssid); // backup current ssid
              m_ssid = valN;
              //prtln("New SSID Stored: \"" + m_ssid + "\"");
            }
            String oldPwd = GetPreferenceString(EE_PWD, DEFAULT_PWD);
            if (valP != oldPwd)
            {
              PutPreferenceString(EE_OLDPWD, oldPwd); // save current pwd
              PutPreferenceString(EE_PWD, valP); // save new pwd
              //prtln("New WiFi pass Stored: \"" + valP + "\"");
            }
            oldPwd = OBFUSCATE_STR; // obfuscate memory for security
            s = "<script>alert('SSID and password set!');";
          }
          else // send error message
            s = "<script>alert('Leading or trailing spaces not allowed!);";
        }
        else
          s = "<script>alert('SSID is max 32 chars and pass is max 63 chars!');";
        // obfuscate memory for security
        valP = OBFUSCATE_STR;
        lenP = 0;
      }
      else // send error alert message
        s = "<script>alert('Must be in AP mode!');";
      if (s != "")
        s += "location.href = '" + String(WEB_PAGE_P1) + "';</script>";
    }
    
    // 200 = OK
    // 204 = OK but No Content
    //request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
    //                                 + inputParam + ") with value: " + inputMessage +
    //                                 "<br><a href=\"/\">Return to Home Page</a>");
    if (s != "")
      request->send(200, "text/html", s.c_str());
    else if (iStatus == 1)
      request->send(204, "text/html", "");
  });
  
  webServer.on("/postP2", HTTP_POST, [] (AsyncWebServerRequest *request)
  {
    int count = request->params();
    
    if (count == 0)
      return;
      
    // disallow erase-data and adding time-events from remote file if locked interface
    if (IsLockedAlertPost(request))
      return;
      
    byte iStatus = 0; // status 0 = no send needed at end, 1 = OK response, 2 = fail response

    inputMessage = "";
    inputParam = "";
    
    String s = ""; // if this in not empty later, we send it as code 200 to the client

    if (count == 1)
    {
      String sIn;
      
      if (request->multipart())
        prtln("WARNING: is multipart!");
        
      if (request->hasParam(PARAM_ERASEDATA, true)) // expecting two parms here...
      {
        sIn = request->getParam(PARAM_ERASEDATA, true)->value();
        if (sIn == "confirm")
        {
          m_taskMode = TASK_RESET_SLOTS;
          m_taskTimer = TASK_TIME;
          s = "All time-events erased!";
        }
      }
      else if (request->hasParam(PARAM_FILEDATA, true))
      {
        sIn = request->getParam(PARAM_FILEDATA, true)->value();

        //prtln("fileData=" + sIn);

        //inputParam = PARAM_FILEDATA;

        int iCount = 0;
        int len = sIn.length();
        if (len > MAX_FILE_SIZE)
          s = "file is too long!";
        else
        {
          // parse time-events from .txt file and add them. Events are seperated by \n (newline)
          // but we can handle \r\n or \n\r as well (that's handled by p2.js in javascript).
          // What we see here is the file with % as a line-seperator. We still must filter
          // comments (which begin with #) and leading whitespace or empty lines.
          // lines filtered out.
          String sOut = "";
          int ii;
          for (ii = 0; ii <= len; ii++)
          {
            char c = ii >= len ? '%' : sIn[ii]; // simulate line-terminator at end of file
            if (c == '%' && sOut.length() > 0) // line-seperator (it won't decode %0A so tried % alone...)
            {
              yield();
              if (ParseIncommingTimeEvent(sOut))
                iCount++; // added event!
              sOut = "";
            }
            else
              sOut += c;
          }
          if (iCount)
            s = String(iCount) + " events added!";
          else
            s = "no events added...";
          prtln(s);
        }
      }
    }
    
    if (s != "")
      request->send(200, "text/html", s.c_str());
     else if (iStatus == 1)
      request->send(204, "text/html", ""); // OK but no data
 });
  
  webServer.on("/getP2", HTTP_GET, [] (AsyncWebServerRequest *request)
  {
    byte iStatus = 0; // status 0 = no send needed at end, 1 = OK response, 2 = fail response
    
    inputMessage = "";
    inputParam = "";
    
    String s = ""; // if this in not empty later, we send it as code 200 to the client

    int count = request->params();

    if (count == 0)
      return;
      
    if (request->hasParam(PARAM_DATETIME))
    {
      // this is just 0 now but we could send the browser's timedate
      // and set the esp32 time/date!
      inputMessage = request->getParam(PARAM_DATETIME)->value();
      bool bSetRequestedButLocked = false;
      bool bTimeSetFailed = false;
      bool bTimeSetSuccess = false;
      
      // set our time if unlocked and it is present in input string: 2020-06-23T01:23:00
      if (inputMessage != "0" && inputMessage.length() >= 19)
      {
        if (m_lockCount == 0xff)
        {
          inputParam = PARAM_DATETIME;
          
          //prtln("setting clock to web browser's time (user pressed \"Set\" button!): \"" + inputMessage + "\"");
          
          // parse date/time
          String sT;
          sT = inputMessage.substring(0,4);
          int myYear = sT.toInt();
          sT = inputMessage.substring(5,7);
          int myMonth = sT.toInt();
          sT = inputMessage.substring(8,10);
          int myDay = sT.toInt();
          sT = inputMessage.substring(11,13);
          int myHour = sT.toInt();
          sT = inputMessage.substring(14,16);
          int myMinute = sT.toInt();
          sT = inputMessage.substring(17,19);
          int mySecond = sT.toInt();
  
          if (SetTimeManually(myYear, myMonth, myDay, myHour, myMinute, mySecond))
            bTimeSetSuccess = true;
          else
            bTimeSetFailed = true;
        }
        else
          bSetRequestedButLocked = true;        
      }
      
      // send back the time to esp32: 2020-11-31T04:32:00

      // read internal time
      struct tm timeInfo = {0};
      if (ReadInternalTime(NULL, &timeInfo) != NULL) // get current time as struct tm
      {
        s = String(timeInfo.tm_year+EPOCH_YEAR) + "-" +
          ZeroPad(timeInfo.tm_mon+1) + "-" + ZeroPad(timeInfo.tm_mday) + "T" +
          ZeroPad(timeInfo.tm_hour) + ":" + ZeroPad(timeInfo.tm_min) + ":" + ZeroPad(timeInfo.tm_sec);
        //prtln("dateTime (send): \"" + s + "\"");
      }

      // add a Tcommand to web-page p2.html
      // via the formatted time and date it's polling for each second.
      // That gives us a nice two-way communication path!
      String sCommand = "";
      if (bSetRequestedButLocked)
        sCommand += "Tlocked"; // show locked alert
      else if (bTimeSetSuccess)
        sCommand += "Tsetok"; // show "Time was set!" at web-page Javascript
      else if (bTimeSetFailed)
        sCommand += "Tnoset"; // show "Time set failed..." at web-page Javascript
      if (bTellP2WebPageToReload)
      {
        sCommand += "Treload";
        bTellP2WebPageToReload = false;
      }
        
      // add future flag-triggered commands for page p2.html here...
      s += sCommand;
    }
    else
    {
      if (IsLockedAlertGet(request, WEB_PAGE_P2))
        return;
        
      // these commands can be locked out...
  
      if (request->hasParam(PARAM_DELINDEX))
      {
        // AsyncWebParameter (file: ESPAsyncWebServer.h)
        // ->name()
        // ->value()
        // ->isFile()
        // ->isPost()
        // ->size()
        inputMessage = request->getParam(PARAM_DELINDEX)->value();
        
        inputParam = PARAM_DELINDEX;
        
        int iSlotNum = inputMessage.toInt();
        // NOTE: slot-numbers requested for deletion are not in order - there could
        // be a m_slotCount of 1 and its delete-index of 99!
        if (m_slotCount)
        {
          if (DeleteTimeSlot(iSlotNum))
            s = "Time-slot " + String(iSlotNum) + " deleted!";
          else
            s = "Failed to delete time-slot " + String(iSlotNum) + "...";
        }
        else
          iStatus = 2; // bad data!
      }
      else if (request->hasParam(PARAM_EDITINDEX)) // expecting two parms here...
      {
        int idx = request->getParam(PARAM_EDITINDEX)->value().toInt();
        //String sItem = request->getParam(PARAM_EDITITEM)->value();
        //prtln("sItem: " + sItem + " idx:" + String(idx));
  
        // Here, the user has pressed Edit and we have the edit item's index and string - 
        // we need to send back a comma-separated list of decoded vars for year, day, month, Etc.
        // We can do it two ways - parse the incomming string that represents a stored time-event,
        // or pull it out of flash where it's stored in time-slots. The latter makes more sense!
        t_event t;
        if (GetTimeSlot(idx, t)) // by ref
        {
          bool pmFlag; // by ref
          int hr12 = Make12Hour(t.timeDate.hour, pmFlag);
          s = String(t.repeatMode) + "," + String(t.deviceMode) + "," + String(t.deviceAddr) + "," + String (t.repeatCount) + "," + String(t.everyCount) + "," +
            String(t.timeDate.dayOfWeek) + "," + String(hr12) + "," + String(t.timeDate.minute) + "," + String (t.timeDate.second) + "," +
              String(t.timeDate.day) + "," + String(t.timeDate.month) + "," + String(t.timeDate.year) + "," + String(pmFlag) + "," +
                String(t.dutyCycleA) + "," + String(t.dutyCycleB) + "," + String(t.phase) + "," + String(t.perUnits) + "," +
                  String(t.perMax) + "," + String(t.bIncludeCycleTiming) + "," + String(t.bCycleTimingInRepeats) + "," + String(t.bEnable) + "," + String(idx);
        }
        else
          iStatus = 2; // bad data!
      }
      // happens when Add or Edit buttons pressed on p2.html
      // (We can differentiate by PARAM_REPLACEINDEX is -1 for  Add)
      else if (request->hasParam(PARAM_DATE)) // just check for one...
      {
        t_event t = {0};
        
        uint16_t tempHour = 0;
        bool bTempPmFlag = false;
        bool bTempIncludeCycleTiming = false; // allows us to record cycle-timing, such as phase and duty-cycle, into this event
        bool bTempCycleTimingInRepeats = false; // apply cycle-timing for repeat-events
        bool bUseOldTimeVals = false; // in a slot-replace after editing, keep old slot's timing cycle values
        int repIndex = -1; // if this is -1 we add an event rather than replace
        
        // cancel pending refresh task
        if (m_taskMode == TASK_PAGE_REFRESH_REQUEST)
        {
          m_taskTimer = 0;
          bTellP2WebPageToReload = false;
        }
      
        for (int ii = 0; ii < count; ii++)
        {
          AsyncWebParameter* pParam = request->getParam(ii);
          if (pParam == 0) continue;
          
          String sName = pParam->name();
          String sVal = pParam->value();
          int iVal = sVal.toInt();
          if (sName == PARAM_REPLACEINDEX)
            repIndex = iVal;
          else if (sName == PARAM_DATE)
          {
            // 2020-08-08
            String sYear = sVal.substring(0,4);
            t.timeDate.year = sYear.toInt();
            String sMonth = sVal.substring(5,7);
            t.timeDate.month = sMonth.toInt();
            if (t.timeDate.month > 12)
              t.timeDate.month = 1;
            String sDay = sVal.substring(8,10);
            t.timeDate.day = sDay.toInt();
            if (t.timeDate.day < 1 || t.timeDate.day > 31)
              t.timeDate.day = 1;
          }
          else if (sName == PARAM_HOUR)
            tempHour = iVal; // 12-hour format!
          else if (sName == PARAM_MINUTE)
            t.timeDate.minute = iVal;
          else if (sName == PARAM_SECOND)
            t.timeDate.second = iVal;
          else if (sName == PARAM_AMPM)
            bTempPmFlag = (iVal > 0) ? true : false; // PM if 1, AM if 0
          else if (sName == PARAM_REPEAT_MODE)
            t.repeatMode = iVal;
          else if (sName == PARAM_REPEAT_COUNT)
            t.repeatCount = iVal;
          else if (sName == PARAM_EVERY_COUNT)
            t.everyCount = iVal;
          else if (sName == PARAM_DEVICE_ADDR)
            t.deviceAddr = iVal;
          else if (sName == PARAM_DEVICE_MODE)
            t.deviceMode = iVal;
          else if (sName == PARAM_USEOLDTIMEVALS)
            bUseOldTimeVals = iVal ? true : false; // set in response to javascript "confirm" dialog in p2.js
          else if (sName == PARAM_INCLUDETIMINGCYCLE)
            // this is a checkbox - weirdly, it reports "on" if checked and no parameter is sent
            // at all if unchecked.
            bTempIncludeCycleTiming = (sVal == "on") ? true : false; // include current cycle-timing if 1
          else if (sName == PARAM_TIMINGCYCLEINREPEATS)
            bTempCycleTimingInRepeats = (sVal == "on") ? true : false; // include current cycle-timing if 1
        }
        
        // convert 12-hour am/pm to 0-23, 24-hour time
        if (bTempPmFlag)
        {
          if (tempHour != 12)
            tempHour += 12;
        }
        else if (tempHour == 12) // 1-11am is 1-11, 12 midnight is 0
          tempHour = 0;
          
        t.timeDate.hour = tempHour;
  
        t.timeDate.dayOfWeek = MyDayOfWeek(t.timeDate.day, t.timeDate.month, t.timeDate.year);
  
        t.bIncludeCycleTiming = bTempIncludeCycleTiming;
        t.bCycleTimingInRepeats = bTempCycleTimingInRepeats;
        
        if (bTempIncludeCycleTiming)
        {
          t_event t2;
          // if user has chosen to keep old cycle-values, load them
          if (bUseOldTimeVals && repIndex >= 0 && GetTimeSlot(repIndex, t2))
          {
            // move old slot's cycle-timing vals to new slot
            t.dutyCycleA = t2.dutyCycleA;
            t.dutyCycleB = t2.dutyCycleB;
            t.phase = t2.phase;
            t.perUnits = t2.perUnits;
            t.perMax = t2.perMax;
            t.perVal = t2.perVal;
          }
          else
          {
            t.dutyCycleA = dutyCycleA;
            t.dutyCycleB = dutyCycleB;
            t.phase = phase;
            t.perUnits = perUnits;
            t.perMax = perMax;
            t.perVal = perVal;
          }
        }
        else
        {
          t.dutyCycleA = 0xff;
          t.dutyCycleB = 0xff;
          t.phase = 0xff;
          t.perUnits = 0xff;
          t.perMax = 0xff;
          t.perVal = 0xffff;
        }
        
        t.bEnable = true;
  
        if (repIndex >= 0)      
        {
          if (repIndex < m_slotCount && PutTimeSlot(repIndex, t))
            s = "<script>alert('Replaced timeslot...');location.href = '" + String(WEB_PAGE_P2) + "';</script>";
          else
            s = "<script>alert('Unable to replace timeslot " + String(repIndex) + "...');location.href = '" + String(WEB_PAGE_P2) + "';</script>";
        }
        else if (m_slotCount < MAX_TIME_SLOTS)
        {
          if (AddTimeSlot(t))
            s = "<script>alert('Added timeslot...');location.href = '" + String(WEB_PAGE_P2) + "';</script>";
          else
            s = "<script>alert('Unable to add timeslot...');location.href = '" + String(WEB_PAGE_P2) + "';</script>";
        }
        else // timeslots are full
          s = "<script>alert('Timeslots are all full. Delete some! Count is: " + String(MAX_TIME_SLOTS) + "');location.href = '" + String(WEB_PAGE_P2) + "';</script>";
      }
    }
    
    
    // 200 = OK
    // 204 = OK but No Content
    //request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
    //                                 + inputParam + ") with value: " + inputMessage +
    //                                 "<br><a href=\"/\">Return to Home Page</a>");
    if (s != "")
      request->send(200, "text/html", s.c_str());
    else if (iStatus == 1)
      request->send(204, "text/html", ""); // OK but no data
    else if (iStatus == 2)
    {
      s = "<script>alert('Out of range (" + inputParam + ") with value: " + inputMessage + "');location.href = '" + String(WEB_PAGE_P2) + "';</script>";
      request->send(200, "text/html", s.c_str());
    }
    
    // Give a semaphore that we can check in the loop (just prints the inputMessege)
//    xSemaphoreGiveFromISR(webInputSemaphore, NULL);
  });
  
  // Note: don't call this between preferences.begin and end. It will cause watchdog timer resets!
  //long heap_size = ESP.getFreeHeap();
  //prtln("Heap before variable size array: " + String(heap_size)); 

  // this might take a while since we have to cycle from 0-MAX_SLOTS
  // trying to read each...
  m_slotCount = CountFullTimeSlots();
  prtln("m_slotCount = " + String(m_slotCount));
  
  // Create webInputSemaphore
  webInputSemaphore = xSemaphoreCreateBinary();

  // need this before configTime!
  WiFi.mode(WIFI_STA);
  
  // void configTzTime(const char* TIMEZONE, const char* server1, const char* server2, const char* server3);
  configTzTime(TIMEZONE, NTP_SERVER1, NTP_SERVER2);
  //configTime(gmtOffset_sec, daylightOffset_sec, NTP_SERVER1, NTP_SERVER2); // init the (S)NTP internet time system

  if (!InitTimeManually())
    prtln("Failed to set initial time!");
    
  SetupAndStartHardwareTimeInterrupt();
  
  prtln("Timer started... reducing cpu frequency to " + String(CPU_FREQ) + "MHz");
  setCpuFrequencyMhz(CPU_FREQ);
}

void ProcessCommand(AsyncWebServerRequest* &request, String &s, String &cmd)
{
  cmd = cmd.substring(2); // parse off the "C "
  cmd.trim();
  
  String subCommand = "";
  int idx = cmd.indexOf(' '); // look for space after command...

  if (idx >= 0)
  {
    subCommand = cmd.substring(idx+1);
    subCommand.trim();
    cmd = cmd.substring(0, idx);
  }
  
  cmd.toLowerCase();
  //prtln("cmd: \"" + cmd + "\"");
  
  if (cmd == COMMAND_UNLOCK || cmd == COMMAND_LOCK)
  {
    String subSubCommand = "";
    
    // parse off a possible sub-command
    bool bResetPwd = false;
    
    //prtln("subCommand: \"" + subCommand + "\"");
    
    // we require sub-command to be: word word word   or... "word word word" "word word word"
    // a trailing "" means clear-password
    int iQ1 = subCommand.indexOf('\"');
    int iQ2 = -1; 
    int iQ3 = -1; 
    int iQ4 = -1;
    if (iQ1 >= 0)
      iQ2 = subCommand.indexOf('\"', iQ1+1);
    if (iQ2 > 0)
      iQ3 = subCommand.indexOf('\"', iQ2+1);
    if (iQ3 > 0)
      iQ4 = subCommand.indexOf('\"', iQ3+1);

    // we allow: iQ1 = 0, iQ2 > 0, iQ3 > 0, iQ4 > 0
    // or iQ1 < 0. Anything else is invalid
    if (iQ1 >= 0 && iQ2 >= 0)
    {
      if (iQ3 > 0 && iQ4 > 0)
      {
        if (iQ1 == 0) // "a b c" "d e f"
        {
          if (iQ4 == iQ3+1)
            bResetPwd = true;
          else
            subSubCommand = subCommand.substring(iQ3+1, iQ4); // new pw

          subCommand = subCommand.substring(iQ1+1, iQ2); // old pw
        }
        else
          subCommand = ""; // reject command...
      }
      else if (iQ3 < 0 && iQ4 < 0)
      {
        if (iQ1 == 0) // "a b c" d e f
        {
          if (iQ2 != iQ1+1)
          {
            subSubCommand = subCommand.substring(iQ2+1); // new pw
            subSubCommand.trim(); // need this to avoid failing the check below!!!
            subCommand = subCommand.substring(iQ1+1, iQ2); // old pw
          }
          else
            subCommand = ""; // reject command...
        }
        else // a b c "d e f"
        {
          if (iQ2 == iQ1+1)
            bResetPwd = true;
          else
          {
            subSubCommand = subCommand.substring(iQ1+1, iQ2); // new pw
            subCommand = subCommand.substring(0, iQ1); // old pw
            subCommand.trim(); // need this to avoid failing the check below!!!
          }
        }
      }
    }

    int len1 = subCommand.length();
    int len2 = subSubCommand.length();
    subCommand.trim();
    subSubCommand.trim();
    if (subCommand.length() != len1 || subSubCommand.length() != len2)
    {
      subCommand = "";
      subSubCommand = "";
      bResetPwd = false;
      s = "<script>alert('Leading or trailing spaces in your password are not allowed!');";
    }
    
    //if (subCommand.length())
    //  prtln("subCommand: \"" + subCommand + "\"");

    //if (subSubCommand.length())
    //  prtln("subSubCommand: \"" + subSubCommand + "\"");
    
    //if (bResetPwd)
    //  prtln("reset password");
 
    String currentPass = GetPreferenceString(EE_LOCKPASS, LOCKPASS_INIT);
    //prtln("current pass: \"" + currentPass + "\"");
    bool bPrintUsage = false;
    
    if (cmd == COMMAND_LOCK)
    {
      if (m_lockCount == 0xff)
      {
        if (subCommand.length() == 0)
        {
          if (currentPass == LOCKPASS_INIT)
          {
            m_lockCount = 0;
            PutPreference(EE_LOCKCOUNT, m_lockCount);
            s = "<script>alert('Interface locked!');";
          }
          else
            bPrintUsage = true;
        }
        else // have a password typed-in!
        {
          if (subCommand.length() > MAX_LOCKPASS_LENGTH || subSubCommand.length() > MAX_LOCKPASS_LENGTH)
            s = "<script>alert('Lock password max length is: " + String(MAX_LOCKPASS_LENGTH) + "');";
          else if (subCommand == subSubCommand && currentPass == LOCKPASS_INIT)
          {
            // here we set password for system that's had password removed or that was never set
            PutPreferenceString(EE_LOCKPASS, subCommand);
            m_lockCount = 0;
            PutPreference(EE_LOCKCOUNT, m_lockCount);
            s = "<script>alert('Password set and interface locked!');";
          }
          else if (subCommand == currentPass)
          {
            // lock the system
            m_lockCount = 0;
            PutPreference(EE_LOCKCOUNT, m_lockCount);
            
            if (bResetPwd || subSubCommand == LOCKPASS_INIT)
            {
              // here we remove the password
              PutPreferenceString(EE_LOCKPASS, LOCKPASS_INIT);
              s = "<script>alert('Password removed and interface locked!');";
            }
            else if (subSubCommand.length() > 0)
            {
              // here we set password for system that's had password removed or that was never set
              PutPreferenceString(EE_LOCKPASS, subSubCommand);
              s = "<script>alert('Password changed and interface locked!');";
            }
            else // here we simply lock an unlocked system
              s = "<script>alert('Interface locked!');";
          }
          else
            bPrintUsage = true;
        }
      }
      else
        s = "<script>alert('Interface already locked!');";
    }
    else // c unlock
    {
      if (m_lockCount != 0xff)
      {
        if (!bResetPwd && subSubCommand.length() == 0 && (currentPass == LOCKPASS_INIT || currentPass == subCommand))
        {
          m_lockCount = 0xff;
          PutPreference(EE_LOCKCOUNT, m_lockCount);
          s = "<script>alert('Interface unlocked!');";
        }
        else
          bPrintUsage = true;
      }
      else
        s = "<script>alert('Interface already unlocked!');";
    }

    if (bPrintUsage)
      s = "<script>alert('c lock, c lock \"pass\" (lock), c lock \"pass\" \"pass\" (set pw), c lock \"pass\" \"\" (remove pw), c lock \"pass\" \"newpass\" (change pw)');";
  }
  else if (m_lockCount != 0xff)
  {
    // if locked...
    
    // this little duplication of processing allows reset of parms (and lock-flag) if WiFi connected in AP mode...
    if (bSoftAP && cmd == COMMAND_RESET && subCommand == SC_RESET_PARMS)
    {
      m_taskMode = TASK_RESET_PARMS;
      m_taskTimer = TASK_TIME;
      s = "<script>alert('Resetting parameters to defaults!');";
    }
    else
      // this far and no farther! (interface is locked)
      s = "<script>alert('Interface is locked! (" + String(VERSION_STR) + ")');";
  }
  else if (cmd == COMMAND_UPDATE)
  {
    request->send(200, "text/html", "<script>window.open('/loginIndex', '_self');</script>"); // special case to send here...
  }
  else if (cmd == COMMAND_VERSION || cmd == COMMAND_INFO)
  {
    String sInfo = "Version: " + String(VERSION_STR) + ", " + GetStringIP();
    //prtln(sInfo);        
    s = "<script>alert('" + sInfo + "');";
  }
  else if (cmd == COMMAND_RESET)
  {
    if (subCommand == SC_RESET_SLOTS)
    {
      m_taskMode = TASK_RESET_SLOTS;
      m_taskTimer = TASK_TIME;
      s = "<script>alert('Resetting slots to defaults!');";
    }
    else if (subCommand == SC_RESET_PARMS)
    {
      m_taskMode = TASK_RESET_PARMS;
      m_taskTimer = TASK_TIME;
      s = "<script>alert('Resetting parameters to defaults!');";
    }
    else
      s = "<script>alert('Invalid command!');";
  }
  else if (cmd == COMMAND_WIFI)
  {
    if (subCommand == SC_WIFI_TOGGLE)
    {
      m_taskMode = TASK_TOGGLE;
      m_taskTimer = TASK_TIME;
      s = "<script>alert('Restoring previous WiFi SSID and password!');";
    }
    else if (subCommand == SC_WIFI_RESTORE)
    {
      m_taskMode = TASK_RESTORE;
      m_taskTimer = TASK_TIME;
      s = "<script>alert('Setting factory WiFi SSID and password!');";
    }
    else if (subCommand == SC_WIFI_DISABLE)
    {
      if (bSoftAP) // only permit this if logged in as access-point!
      {
        // send confirmation first! (we are turning OFF web-access here!)
        s = "<script>alert('WiFi Disabled - Reapply power to reenable...');";
        
        // set this first, then call PollApSwitch() to disconnect nicely, then turn off WiFi
        // Have to push reset or reapply power to restart it! (might - need to store toggle switch state
        // and reenable WiFi if switch changes...)
        bWiFiDisabled = true;
        PollApSwitch();
        WiFi.mode(WIFI_OFF);
        prtln("WiFi Disabled - Reapply power to reenable...");
      }
      else
        s = "<script>alert('To disable WiFi, flip-switch, log in as " + String(DEFAULT_SOFTAP_SSID) + ", " + String(DEFAULT_SOFTAP_PWD) + ".');";
    }
    else
      s = "<script>alert('Invalid command!');";
  }
  else
    s = "<script>alert('Invalid command!');";
}

void SetWiFiHostName(AsyncWebServerRequest* &request, String &s, String &cmd)
{
  // screen off ".local"
  int len = cmd.length();
  String hn;
  for (int ii = 0; ii < len && ii <= 63; ii++)
  {
    char c = cmd[ii];
    if (c == '.')
      break;
    hn += c; 
  }
  if (hn != hostName)
  {
    len = hn.length();
    if (len >= 1 && len <= 63)
    {
      hostName = hn;
      s = "<script>alert('Changed hostname to: ";
      m_taskMode = TASK_HOSTNAME;
      m_taskTimer = TASK_TIME;
    }
    else // send "not modified"
      s = "<script>alert('Error in host-name: ";
  }
  else
    s = "<script>alert('Name already set: ";
  s += hostName + "');";
}

//    response->addHeader("Access-Control-Allow-Headers", "origin, content-type, accept, authorization");
//    response->addHeader("Content-Type", "application/json");
//    response->addHeader("Content-Type", "*");
//    response->addHeader("Access-Control-Allow-Origin", "POST, GET, OPTIONS"); // fixes it for edge but not firefox...
void SetHeaders(AsyncWebServerResponse *response)
{
  response->addHeader("Access-Control-Allow-Headers", "*");
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "*");
  response->addHeader("Cache-Control", "no-cache, no-store");
//  response->addHeader("Connection", "close"); // keep-alive is default and keeps same tcp connection
}

// set sReloadUrl to WEB_PAGE_P2 Etc.
bool IsLockedAlertGet(AsyncWebServerRequest *request, String sReloadUrl)
{
  if (m_lockCount != 0xff)
  {
    String s = "<script>alert('System is locked!');";
    if (sReloadUrl != "")
      s += "location.href='" + String(sReloadUrl) + "';";
    s += "</script>";
    request->send(200, "text/html", s);
    return true;
  }
  return false;
}
  
bool IsLockedAlertPost(AsyncWebServerRequest *request)
{
  if (m_lockCount != 0xff)
  {
    request->send(200, "text/html", "System is locked!");
    return true;
  }
  return false;
}
  
void gleanEscapes(String sIn, std::vector<uint16_t>* p)
{
  if (!p)
    return;

  p->resize(0);
  
  // input string can contain decimal numbers representing unptintable unicode chars,
  // The form is &#12345;&#67890; We need to hand parse these
  int strLen = sIn.length();
  String sEsc;
  bool bStartEsc = false;
  for (int i = 0; i < strLen; i++)
  {
    char c = sIn[i];
    if (bStartEsc)
    {
      if (c == ';')
      {
        bStartEsc = false;
        if (sEsc.length() > 0)        
          p->push_back((uint16_t)sEsc.toInt());
        else // abort
        {
          p->resize(0);
          return;
        }
      }
      else if (isDigit(c))
        sEsc += c;
      else if (c == '#')
        continue; // ignore # sign
      else // abort
      {
        p->resize(0);
        return;
      }
    }
    else if (c == '&')
    {
      bStartEsc = true;
      sEsc = "";
    }
    else
      p->push_back((uint16_t)c);
  }
}

//bool allDigits(String sIn)
//{
//  int strLen = sIn.length();
//  for (int i = 0; i < strLen; i++)
//  {
//    if (!isDigit(sIn[i])
//      return false;
//  }
//  return true;
//}

String hnDecode(String sIn)
{
  //prtln("raw input: \"" + sIn + "\"");

  std::vector<uint16_t> arr;
  gleanEscapes(sIn, &arr);
  int arrLen = arr.size();
  if (arrLen <= 0)
    return "";

  int saveSct = m_sct;
  
  char charArray[arrLen+1];
  for(int i = 0; i < arrLen; i++)
  {
    uint16_t c = (uint16_t)arr[i];
    for (int j = 0; j < m_sct; j++)
    {
      bool lsbSet = (c & 1) ? true : false;
      c >>= 1;
      if (lsbSet)
        c |= 0x8000;
    }
    charArray[i] = (char)c;
    if (--m_sct <= 0)
      m_sct = m_max;
  }
  charArray[arrLen] = '\0';
  m_sct = saveSct;
  
  //prtln("decoded input: \"" + String(charArray) + "\"");
  
  // two-digit validation prefix:
  String sOut = String(charArray);
  
  for (int i = 0; i < arrLen; i++)
    charArray[i] = i; // obfuscate memory
    
  if (!(sOut.length() > 2 && isdigit(sOut[0]) && isdigit(sOut[1]) && sOut.substring(0, 2).toInt() == m_sct))
    return ""; // no good...
    
  return sOut.substring(2);
}

void RefreshMaxSct()
{
  m_max = random(MAX_SHIFT_COUNT/2, MAX_SHIFT_COUNT);
  m_sct = random(1, m_max);
}

void IRAM_ATTR onTimer()
{
  // Increment the counter and set the time of ISR
//  portENTER_CRITICAL_ISR(&timerMux);
//  isrCounter++;
//  lastIsrAt = millis();
//  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop  
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}

// some things change a lot and so a timer
// keeps resetting - when it times out, we
// then call this and save the associated setting
// in flash memory.
void RunTasks()
{
  switch(m_taskMode)
  {
    case TASK_PERMAX:
      PutPreference(EE_PERMAX, perMax);
    break;
    
    case TASK_PERUNITS:
      PutPreference(EE_PERUNITS, perUnits);
    break;
    
    case TASK_PERVAL:
      PutPreference(EE_PERVAL, perVal);
    break;
    
    case TASK_DCA:
      PutPreference(EE_DC_A, dutyCycleA);
    break;
    
    case TASK_DCB:
      PutPreference(EE_DC_B, dutyCycleB);
    break;
    
    case TASK_DCA_DCB:
      PutPreference(EE_DC_A, dutyCycleA);
      PutPreference(EE_DC_B, dutyCycleB);
    break;
    
    case TASK_PHASE:
      PutPreference(EE_PHASE, phase);
    break;

    case TASK_RELAY_A:
      PutPreference(EE_RELAY_A, nvSsrMode1);
    break;
    
    case TASK_RELAY_B:
      PutPreference(EE_RELAY_B, nvSsrMode2);
    break;
    
    case TASK_HOSTNAME:
      PutPreferenceString(EE_HOSTNAME, hostName);

      if (bWiFiConnected)
        WiFiMonitorConnection(true); // disconnect STA mode (it will reconnect)
      if (bSoftAP)
        WiFiStartAP(true); // disconnect AP mode (it will reconnect)
        
      prtln("New hostName has been set!");
    break;
    
    case TASK_RECONNECT:

      // only reconnect if not in AP mode - generally, we HAVE to be in AP mode to
      // even set the PWD and SSID!
      if (!bSoftAP && bWiFiConnected)
        WiFiMonitorConnection(true, true); // disconnect
        
      prtln("Reconnecting by TASK_RECONNECT...");
    
    break;
    
    case TASK_RESET_PARMS:
      if (!ErasePreferences()) // ersae prefs...
        prtln("Error erasing preferences!");
      GetPreferences();
      WiFiMonitorConnection(true, true); // disconnect (and then reconnect with default preferences)
    break;
    
    case TASK_RESET_SLOTS:
      if (!EraseTimeSlots())
        prtln("Error erasing time-slots!");
      m_slotCount = 0;
    break;
    
    case TASK_TOGGLE:
      ToggelOldSsidAndPwd();
      WiFiMonitorConnection(true, true); // disconnect
    break;
    
    case TASK_RESTORE:
      RestoreDefaultSsidAndPwd();
      WiFiMonitorConnection(true, true); // disconnect
    break;
    
    case TASK_PAGE_REFRESH_REQUEST:
      // this task is set when something on p2.html has been
      // affected and the page needs to be reloaded
      bTellP2WebPageToReload = true;
      fiveSecondTimer = 0; // this will reset the flag as a "failsafe" in 5-sec.
    break;
    
    case TASK_WIFI_CONNECT:
      WiFi.begin(m_ssid.c_str(), GetPreferenceString(EE_PWD, DEFAULT_PWD).c_str());
      prtln("\nConnecting to WiFi...");
    break;
      
    default:
    break;
  };
}

// Flash-vars access
//size_t putString(const char* key, const char* value);
//size_t putString(const char* key, String value);
//size_t putBytes(const char* key, const void* value, size_t len);
//size_t getString(const char* key, char* value, size_t maxLen);
//String getString(const char* key, String defaultValue = String());
//size_t getBytesLength(const char* key);
//size_t getBytes(const char* key, void * buf, size_t maxLen);
//size_t freeEntries();
//EEPROM.write(address, value)
//EEPROM.commit()
//EEPROM.read(address)
void GetPreferences(void)
//https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/src/Preferences.cpp
{
  /* Start a namespace EE_PREFS_NAMESPACE
  in Read-Write mode: set second parameter to false 
  Note: Namespace name is limited to 15 chars */
  preferences.begin(EE_PREFS_NAMESPACE, false); // flag is the read-only flag

  // if we want to remove the TASK_RELAY_A key uncomment it
  //preferences.remove("TASK_RELAY_A");

  m_lockCount = preferences.getUChar(EE_LOCKCOUNT, LOCKCOUNT_INIT);
  
  perVal = preferences.getUChar(EE_PERVAL, PERVAL_INIT);
  prtln("period value " + String(perVal));
  perUnits = preferences.getUChar(EE_PERUNITS, PERUNITS_INIT);
  prtln("period units: " + String(perUnits));
  perMax = preferences.getUChar(EE_PERMAX, PERMAX_INIT);
  prtln("max period: " + String(perMax));
  
  savePeriod = ComputePeriod(perVal, perMax, perUnits);
  prtln("period (.5 sec units): " + String(savePeriod));
  
  nvSsrMode1 = preferences.getUChar(EE_RELAY_A, SSR1_MODE_INIT); // 0 = OFF, 1 = ON, 2 = AUTO
  SetState(SSR_1, nvSsrMode1);
#if PRINT_ON
  Serial.print("nvSsrMode1: 0x");
  Serial.println(nvSsrMode1, HEX);
#endif

  nvSsrMode2 = preferences.getUChar(EE_RELAY_B, SSR2_MODE_INIT); // 0 = OFF, 1 = ON, 2 = AUTO
  SetState(SSR_2, nvSsrMode2);
#if PRINT_ON
  Serial.print("nvSsrMode2: 0x");
  Serial.println(nvSsrMode2, HEX);
#endif
  
  dutyCycleA = preferences.getUChar(EE_DC_A, DUTY_CYCLE_A_INIT);
  if (dutyCycleA < DUTY_CYCLE_MIN)
    dutyCycleA = DUTY_CYCLE_MIN;
  else if (dutyCycleA > DUTY_CYCLE_MAX)
    dutyCycleA = DUTY_CYCLE_MAX;
  prtln("dutyCycleA (%): " + String(dutyCycleA));
  
  dutyCycleB = preferences.getUChar(EE_DC_B, DUTY_CYCLE_B_INIT);
  if (dutyCycleB < DUTY_CYCLE_MIN)
    dutyCycleB = DUTY_CYCLE_MIN;
  else if (dutyCycleB > DUTY_CYCLE_MAX)
    dutyCycleB = DUTY_CYCLE_MAX;
  prtln("dutyCycleB (%): " + String(dutyCycleB));

  phase = preferences.getUChar(EE_PHASE, PHASE_INIT);
  if (phase < PHASE_MIN)
    phase = PHASE_MIN;
  else if (phase > PHASE_MAX)
    phase = PHASE_MAX;
  prtln("phase (%): " + String(phase));

  hostName = preferences.getString(EE_HOSTNAME, DEFAULT_HOSTNAME);
  m_ssid = preferences.getString(EE_SSID, DEFAULT_SSID);
  
  // Close the Preferences
  preferences.end();
}

uint16_t ComputePeriod(uint8_t perVal, uint8_t perMax, uint8_t perUnits)
{
  // perUnits is 0=.5 sec resolution, 1=sec, 2=min, 3=hrs
  // perMax is 0=10, 1=20, 2=30... 9=100 (max limit of period slider on index.html)
  // perVal is slider's value 0-perMax
  
  // we have permax as an index representing maximums of a slider widget on p2.html
  // the user can pick 10, 20, 30, etc. up to 100. perVal is a number from 0-10, 0-20, etc
  uint16_t iPerMax = DecodePerMax(perMax);
  
  uint16_t iPerVal = (perVal == 0) ? random(1, iPerMax) : perVal;

  uint16_t iTmp;
  
  switch(perUnits)
  {
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
      iTmp = iPerVal*2*60*60;
    break;
  }

  return iTmp;
}

// perMax index from index.html:
//  <label for="perMax">max period</label>
//  <select name="perMax" id="perMax">
//  <option value="0">25</option>
//  <option value="1">50</option>
//  <option value="2">75</option>
//  <option value="3">100</option>
//  <option value="4">150</option>
//  <option value="5">200</option>
//  <option value="6">400</option>
//  <option value="7">600</option>
//  <option value="8">800</option>
//  <option value="9">1000</option>
//  </select>
uint16_t DecodePerMax(uint8_t perMax)
{
  switch(perMax)
  {
    case 0:
      return 25;
    case 1:
      return 50;
    case 2:
      return 75;
    case 3:
      return 100;
    case 4:
      return 150;
    case 5:
      return 200;
    case 6:
      return 400;
    case 7:
      return 600;
    case 8:
      return 800;
    case 9:
      return 1000;
    default:
      return 50;
  }
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
//"# (optional cycle-timing: a:40,b:50,p:20,u:0-3,m:0-9,v:0-m,\n" +
//"# i:y include cycle-timing, c:y ...in repeat events)\n";
bool ParseIncommingTimeEvent(String sIn)
{
  sIn.trim();

  int len = sIn.length();
  if (len == 0)
    return false;

  if (len > MAX_RECORD_SIZE)
  {
    prtln("Incomming file-record too long!");
    return false;
  }
  
  if (sIn[0] == '#')
    return false; // skip comments

  bool bEnable; // we set to disable if leading '*'
  // sOut can have leading * indicating "disabled"
  if (sIn[0] == '*')
  {
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
  int16_t iDcA, iDcB, iPhase, iPerUnits, iPerMax, iPerVal;

  // init these to 0 - they are "optional" in input string - but need to be 0 if not present!
  int16_t iRcount = 0;
  int16_t iEcount = 0;
  int16_t iRepeatMode = 0; // off by default
  bool bCycleTimingInRepeats = false;
  bool bIncludeCycleTiming = false;
  int iCount = 0;

  // optional - user can add any or all they want, in any order
  iDcA = iDcB = iPhase = iPerUnits  = iPerMax = iPerVal = -1; // unused on init...
  
  for (int ii = 0 ; ii <= len ; ii++) // allow ii to go past eof on purpose!
  {
    // impute a record-break if past end of file
    // to force last parameter to be processed
    char c = ii >= len ? ' ' : sIn[ii];
    
    if (c == ' ' || c == ',') // field-seperator
    {
      // skip extraneous spaces or commas
      if (ii+1 < len && (sIn[ii+1] == ' ' || sIn[ii+1] == ','))
        continue;

      int lenToken = sOut.length();
      if (lenToken > 0)
      {
        sOut.trim();
        sOut.toLowerCase();
        
        if (iCount == 0)
        {
          if (!ParseDate(sOut, iMonth, iDay, iYear))
            return false;
            
          iCount++;
        }
        else if (iCount == 1)
        {
          if (!ParseTime(sOut, iHour, iMinute, iSecond))
            return false;
          iCount++;
        }
        else if (iCount == 2)
        {
          if (!ParseDevAddressAndMode(sOut, iDevAddr, iDevMode))
            return false;
          iCount++;
        }
        else if (iCount > 2) // the following are optional and can occur in any order
        {
          int idx = sOut.indexOf(":");
          if (idx > 0)
          {
            // duty cycle A, duty cycle B, phase, units index 0-3, max slider index 0-9, value of slider 0-max
            // a:50,b:50,p:50,u:1,m:9,v:100
            char cmd = sOut[0];
            sOut = sOut.substring(idx+1);
            int iVal = sOut.toInt();
            if (cmd == 't')
              ParseRepeatMode(sOut, iRepeatMode); // by ref
            else if (cmd == 'i')
            {
              if (sOut == "y")
                bIncludeCycleTiming = true;
            }
            else if (cmd == 'c')
            {
              if (sOut == "y")
                bCycleTimingInRepeats = true;
            }
            else if (cmd == 'r')
            {
              if (sOut == "inf")
                iRcount = 0;
              else if (iVal >= 0)
                iRcount = iVal;
            }
            else if (cmd == 'e')
            {
              if (iVal >= 0)
                iEcount = iVal;
            }
            else if (cmd == 'a')
            {
              if (iVal >= 0)
                dutyCycleA = iVal;
            }
            else if (cmd == 'b')
            {
              if (iVal >= 0)
                dutyCycleB = iVal;
            }
            else if (cmd == 'p')
            {
              if (iVal >= 0)
                phase = iVal;
            }
            else if (cmd == 'u')
            {
              if (iVal >= 0)
                perUnits = iVal;
            }
            else if (cmd == 'm')
            {
              if (iVal >= 0)
                perMax = iVal;
            }
            else if (cmd == 'v')
            {
              if (iVal >= 0)
                perVal = iVal;
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
  t_event t = {0};
  t.timeDate.hour = iHour;
  t.timeDate.minute = iMinute;
  t.timeDate.second = iSecond;
  t.timeDate.year = iYear;
  t.timeDate.month = iMonth;
  t.timeDate.day = iDay;
  t.timeDate.dayOfWeek = MyDayOfWeek(iDay, iMonth, iYear);

  t.repeatMode = iRepeatMode;
  t.deviceMode = iDevMode;
  t.deviceAddr = iDevAddr;
  t.repeatCount = iRcount;
  t.everyCount = iEcount;
  
  t.bEnable = bEnable;
  t.bIncludeCycleTiming = bIncludeCycleTiming;
  t.bCycleTimingInRepeats = bCycleTimingInRepeats;

  // casting from signed to unsigned, some are -1
  t.dutyCycleA = (uint8_t)iDcA;
  t.dutyCycleB = (uint8_t)iDcB;
  t.phase = (uint8_t)iPhase;
  t.perUnits = (uint8_t)iPerUnits;
  t.perMax = (uint8_t)iPerMax;
  t.perVal = (uint16_t)iPerVal;
  
  return AddTimeSlot(t, false); // supress error messages but return status
}

bool ParseRepeatMode(String &s, int16_t &iRepeatMode)
{
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
  
bool ParseDevAddressAndMode(String &s, int16_t &iDevAddr, int16_t &iDevMode)
{
  int idx = s.indexOf(":");
  if (idx < 0)
    return false;
    
  String sTmp = s.substring(0, idx);
  sTmp.trim();
  if (sTmp == "a")
    iDevAddr = 0;
  else if (sTmp == "b")
    iDevAddr = 1;
  else if (sTmp == "ab")
    iDevAddr = 2;
  else
    iDevAddr = -1;
  
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

bool ParseTime(String &s, int16_t &iHour, int16_t &iMinute, int16_t &iSecond)
{
  bool bHaveHour = false;
  bool bHaveMinute = false;
  bool bHaveSecond = false;
  int iTemp = 0;
  String sOut = "";
  
  iHour = -1;
  iMinute = -1;
  iSecond = -1;

  int len = s.length();

  for (int ii = 0; ii < len; ii++)
  {
    if (s[ii] == ':')
    {
      if (sOut.length() > 0)
      {
        if (bHaveHour)
        {
          iTemp = sOut.toInt();
          if (iTemp >= 0 && iTemp <= 59)
          {
            iMinute = iTemp;
            bHaveMinute = true;
            sOut = "";
          }
          else
            return false;
        }
        else
        {
          iTemp = sOut.toInt();
          if (iTemp >= 1 && iTemp <= 12)
          {
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
    else
      sOut += s[ii];
  }

  if (bHaveMinute)
  {
    sOut.trim();
  
    if (sOut.length() >= 2)
    {
      String sTemp = sOut.substring(0,2); // "00"
      iTemp = sTemp.toInt();
      if (iTemp >= 0 && iTemp <= 59)
      {
        iSecond = iTemp;
        bHaveSecond = true;
      }
      else
        return false;
    }
  }
  if (bHaveSecond)
  {
    if (sOut.length() >= 4)
    {
       String sTemp = sOut.substring(2); // "    pm" or "am"
       sTemp.trim();
      if (sTemp == "am" || sTemp == "pm")
      {
        if (sTemp == "am")
        {
          if (iHour == 12)
            iHour = 0;
        }
        else // pm
        {
          if (iHour != 12)
            iHour += 12;
        }
        return true;
      }
    }
  }
  return false;
}

bool ParseDate(String &s, int16_t &iMonth, int16_t &iDay, int16_t &iYear)
{
  bool bHaveMonth = false;
  bool bHaveDay = false;
  int iTemp = 0;
  String sOut = "";

  iMonth = -1;
  iDay = -1;
  iYear = -1;

  int len = s.length();
  
  for (int ii = 0; ii < len; ii++)
  {
    if (bHaveDay && ii == len-1)
    {
      sOut += s[ii];
      iTemp = sOut.toInt();
      if (iTemp >= DEFAULT_YEAR && sOut.length() == 4) // "2020", Etc.
      {
        iYear = iTemp;
        return true;
      }
      
      return false;
    }
    else if (s[ii] == '/')
    {
      if (sOut.length() > 0)
      {
        if (bHaveMonth)
        {
          iTemp = sOut.toInt();
          if (iTemp >= 1 && iTemp <= 31)
          {
            iDay = iTemp;
            bHaveDay = true;
            sOut = "";
          }
          else
            return false;
        }
        else
        {
          iTemp = sOut.toInt();
          if (iTemp >= 1 && iTemp <= 12)
          {
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

String TimeSlotToString(t_event t)
{
  String sDevAddr;

  switch(t.deviceAddr)
  {
    case 0:
      sDevAddr = "A";
    break;
    case 1:
      sDevAddr = "B";
    break;
    case 2:
      sDevAddr = "AB";
    break;
    default:
      sDevAddr = "x";
    break;
  }
  
  String sDevMode;
  
  switch(t.deviceMode)
  {
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
      sDevMode = "x";
    break;
  }
  
  //<option value="0">off</option>
  //<option value="1">second</option>
  //<option value="2">minute</option>
  //<option value="3">hour</option>
  //<option value="4">daily</option>
  //<option value="5">weekly</option>
  //<option value="6">monthly</option>
  //<option value="7">yearly</option>
  String sTmpMode;
  switch(t.repeatMode)
  {
    default:
    case 0:
      sTmpMode = "";
    break;
    case 1:
      sTmpMode = "sec";
    break;
    case 2:
      sTmpMode = "min";
    break;
    case 3:
      sTmpMode = "hrs";
    break;
    case 4:
      sTmpMode = "day";
    break;
    case 5:
      sTmpMode = "wek";
    break;
    case 6:
      sTmpMode = "mon";
    break;
    case 7:
      sTmpMode = "yrs";
    break;
  }

  // if repeatMode not "off", then add repeat count and every count
  
  String sRptMode = "";
  if (sTmpMode != "")
  {
    if (t.repeatCount == 0)
      sRptMode = " r:inf";
    else
      sRptMode = " r:" + String(t.repeatCount);
      
    sRptMode += " e:" + String(t.everyCount);
    sRptMode += " t:" + sTmpMode;
  }
  
//  String sDayOfWeek;
//  switch(t.timeDate.dayOfWeek)
//  {
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
    String(my12Hour) + ":" + ZeroPad(t.timeDate.minute) + ":" + ZeroPad(t.timeDate.second) + sPm + ", " +
      sDevAddr + ":" + sDevMode + sRptMode;

  // here are more...
  // a:40,b:50,p:20,u:0-3,m:0-9,v:0-m, c:y, i:y
  if (t.perVal != 0xffff)
    sRet += " v:" + String(t.perVal);
  if (t.perMax != 0xff)
    sRet += " m:" + String(t.perVal);
  if (t.perUnits != 0xff)
    sRet += " u:" + String(t.perUnits);
  if (t.phase != 0xff)
    sRet += " p:" + String(t.phase);
  if (t.dutyCycleA != 0xff)
    sRet += " a:" + String(t.dutyCycleA);
  if (t.dutyCycleB != 0xff)
    sRet += " b:" + String(t.dutyCycleB);
  if (t.bIncludeCycleTiming != false)
    sRet += " i:y";
  if (t.bCycleTimingInRepeats != false)
    sRet += " c:y";

  // truncate to fit width of cell-phone
//  if (sRet.length() > 31)
//    sRet = sRet.substring(0,31) + "...";
    
  return sRet;
}

// takes hour in 24 hour and returns 12-hour
// and pmFlag by reference
int Make12Hour(int iHour, bool &pmFlag)
{
  pmFlag = (iHour >= 12) ? true : false;
  iHour %= 12; // 0-11 we get 0-11, for 12-23 we get 0-11
  if (iHour == 0) iHour = 12; // the hour '0' should be '12'
  return iHour;
}

// read time-slot info
void ProcessSecondResolutionTimeSlots()
{
  if (m_slotCount == 0 || !(bManualTimeWasSet || bWiFiTimeWasSet))
    return;
    
  int count = IV_GetCount();
  if (count == 0) return; // no non-zero second slots programmed!

  struct tm timeInfo = {0};
  if (ReadInternalTime(NULL, &timeInfo) == NULL) // get current time as both epoch-time_t and struct tm
    return;
  
  // copy current time and date to format used in each event
  t_time_date timeDate = CopyTmToTtimeDate(timeInfo);
  
  // need to check them...
  for (int ii = 0; ii < count; ii++)
  {
    // index might be any number 0 to MAX_TIME_SLOTS, even if only 1 item!
    // (this list has ONLY non-zero seconds slots!)
    int slotIndex = IV_GetIndex(ii);    

    t_event slotData;
    
    // read full slot into temp-struct slotData
    if (GetTimeSlot(slotIndex, slotData)) // by ref
      ProcessTimeSlot(slotIndex, timeDate, slotData);
    else
    {
      prtln("Error in ProcessSecondResolutionTimeSlots(). Can't read slot: " + String(slotIndex)); 
      break;
    }
    
    yield(); // let other processes run...
  }
  
  // m_prevDateTime is used to facilitate
  // the repeat modes (see p2.html in the data folder)
  // I.E. every hour, day, Etc. following a time-event
  // trigger.
  m_prevDateTime = timeDate;
}

void ProcessMinuteResolutionTimeSlots()
{
  if (m_slotCount == 0 || !(bManualTimeWasSet || bWiFiTimeWasSet))
    return;
  
  struct tm timeInfo = {0};
  if (ReadInternalTime(NULL, &timeInfo) == NULL) // get current time as both epoch-time_t and struct tm
    return;

  prt("Time: ");
  printLocalTime(timeInfo);
  
  t_time_date timeDate = CopyTmToTtimeDate(timeInfo);

  if (timeDate.second != 0)
  {
    prtln("detected drift in seconds, correcting. timeDate.second=" + String(timeDate.second));
    timeDate.second = 0;
  }
  
  int iFull = -1;
  
  for (int ii = 0; ii < m_slotCount; ii++)
  {
    iFull = FindNextFullTimeSlot(iFull+1);
    if (iFull < 0)
      break;
      
    // if this slot's in the non-zero seconds list, continue - we process it in ProcessSecondResolutionTimeSlots()
    if (IV_FindIndex(iFull) >= 0)
      continue;
      
    t_event slotData;    
    if (GetTimeSlot(iFull, slotData)) // by ref
      ProcessTimeSlot(iFull, timeDate, slotData);
    else
    {
      prtln("Error in ProcessMinuteResolutionTimeSlots(). Can't read slot: " + String(iFull)); 
      break;
    }
    yield(); // let other processes run...
  }
  
  // m_prevDateTime is used to facilitate
  // the repeat modes (see p2.html in the data folder)
  // I.E. every hour, day, Etc. following a time-event
  // trigger.
  m_prevDateTime = timeDate;
}    

// process all except timeinfo.tm_sec
// timeDate has the RTC time/date
// slotData has the data associated with slotIndex
// timeEpoch is the Epoch (# seconds since midnight 1-1-1970)
void ProcessTimeSlot(int slotIndex, t_time_date timeDate, t_event slotData)
{
  // return if time never set or if slot is not enabled (bit0 of flags)
  if (!slotData.bEnable || timeDate.year == DEFAULT_YEAR)
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
  int compareVal = CompareTimeDate(timeDate, slotData.timeDate);
  
  // timestamps match?
  if (compareVal == 0)
  {
    DoEvent(slotData.deviceAddr, slotData.deviceMode);
    prtln("Main Event! Slot index: " + String(slotIndex));

    // init the counters to 0 on first event trigger
    IR_ResetCountersBySlot(slotIndex);

    // Check for any event-specific time-cycle parameters
    // uint8_t dutyCycleA, dutyCycleB, phase, perUnits, perMax;
    // uint16_t perVal
    if (slotData.bIncludeCycleTiming)
      if (CheckEventSpecificTimeCycleParameters(&slotData))
        ResetPeriod();
  }
  else if (compareVal == 1) // event already ocurred?
  {
    // check to see if this event was out of date at InitRepeatList()
    // we don't want to start repeating on an event stored a year back,
    // so just delete it...
    if (slotData.repeatMode == RPT_OFF || IR_IsStaleBySlot(slotIndex))
    {
      if (DisableTimeSlot(slotIndex)) // disable the slot
      {
        m_taskMode = TASK_PAGE_REFRESH_REQUEST; // delay and tell P2.html to reload
        m_taskTimer = TASK_TIME;      
      }
    }
    else
    {
//      struct tm prevTimeInfo = {0};
//      if (localtime_r((time_t*)&m_prevDateTime, &prevTimeInfo) == 0)
//        return;

      // for repeating - everyCounter is set to 0 at the first DoEvent - here,
      // if t.repeatMode != RPT_OFF, we increment the counter and no longer
      // DoEvent if the counter > count. a count of 0 is intinite

      // if the unit we are repeating has changed (i.e. timeInfo.tm_hour != prevTimeInfo.tm_hour),
      // and the sub-units match our event timestamp, we re-trigger the event...
      if ((slotData.repeatMode == RPT_SECONDS && timeDate.second != m_prevDateTime.second) ||
          (slotData.repeatMode == RPT_MINUTES && timeDate.minute != m_prevDateTime.minute && SecMatch(timeDate, slotData)) ||
            (slotData.repeatMode == RPT_HOURS && timeDate.hour != m_prevDateTime.hour && MinSecMatch(timeDate, slotData)) ||
              (slotData.repeatMode == RPT_DAYS && timeDate.day != m_prevDateTime.day && MinSecHoursMatch(timeDate, slotData)) ||
                (slotData.repeatMode == RPT_WEEKS && timeDate.day != m_prevDateTime.day && MinSecHoursDayOfWeekMatch(timeDate, slotData)) ||
                  (slotData.repeatMode == RPT_MONTHLY && timeDate.month != m_prevDateTime.month && MinSecHoursDaysMatch(timeDate, slotData)) ||
                    (slotData.repeatMode == RPT_YEARS && timeDate.year > m_prevDateTime.year && MinSecHoursDaysMonthsMatch(timeDate, slotData)))
        ProcessEvent(slotIndex, slotData);
    }
  }
}

// returns true if we may want to call ResetPeriod()!;
bool CheckEventSpecificTimeCycleParameters(t_event* slotData)
{
  bool bChanged = false;;
  if (slotData->perVal != 0xffff && slotData->perVal != perVal)
  {
    perVal = slotData->perVal;
    bChanged = true;
  }
  if (slotData->dutyCycleA != 0xff && slotData->dutyCycleA != dutyCycleA)
  {
    dutyCycleA = slotData->dutyCycleA;
    bChanged = true;
  }
  if (slotData->dutyCycleB != 0xff && slotData->dutyCycleB != dutyCycleB)
  {
    dutyCycleB = slotData->dutyCycleB;
    bChanged = true;
  }
  if (slotData->phase != 0xff && slotData->phase != phase)
  {
    phase = slotData->phase;
    bChanged = true;
  }
  if (slotData->perUnits != 0xff && slotData->perUnits != perUnits)
  {
    perUnits = slotData->perUnits;
    bChanged = true;
  }
  if (slotData->perMax != 0xff && slotData->perMax != perMax)
  {
    perMax = slotData->perMax;
    bChanged = true;
  }
  return bChanged;
}

bool SecMatch(t_time_date &timeDate, t_event &slotData)
{
  return timeDate.second == slotData.timeDate.second;  
}

bool MinSecMatch(t_time_date &timeDate, t_event &slotData)
{
  return timeDate.minute == slotData.timeDate.minute && SecMatch(timeDate, slotData);  
}

bool MinSecHoursMatch(t_time_date &timeDate, t_event &slotData)
{
  return timeDate.hour == slotData.timeDate.hour && MinSecMatch(timeDate, slotData);  
}

bool MinSecHoursDaysMatch(t_time_date &timeDate, t_event &slotData)
{
  return timeDate.day == slotData.timeDate.day && MinSecHoursMatch(timeDate, slotData);  
}

bool MinSecHoursDayOfWeekMatch(t_time_date &timeDate, t_event &slotData)
{
  return timeDate.dayOfWeek == slotData.timeDate.dayOfWeek && MinSecHoursMatch(timeDate, slotData);  
}

bool MinSecHoursDaysMonthsMatch(t_time_date &timeDate, t_event &slotData)
{
  return timeDate.month == slotData.timeDate.month && MinSecHoursDaysMatch(timeDate, slotData);  
}

void ProcessEvent(int slotIndex, t_event slotData)
{
  int ir_listIdx = IR_FindIndexBySlot(slotIndex);
  if (ir_listIdx >= 0)
  {
    // 0 = off (ignore) everyCount, 0 = repeat forever for repeatCount
    if (IR_IncRepeatCounter(ir_listIdx))
    {
      DoEvent(slotData.deviceAddr, slotData.deviceMode);
      prtln("Repeat Event! Slot index: " + String(slotIndex));

      if (slotData.bIncludeCycleTiming && slotData.bCycleTimingInRepeats)
        if (CheckEventSpecificTimeCycleParameters(&slotData))
          ResetPeriod();
        
      // autodelete if not infinite repeat (0) and if finished repeating...
      if(IR_GetRptCount(ir_listIdx) != 0 && IR_GetRptCounter(ir_listIdx) >= slotData.repeatCount)
      {
        if (DisableTimeSlot(slotIndex)) // disable the slot
        {
          m_taskMode = TASK_PAGE_REFRESH_REQUEST; // delay and tell P2.html to reload
          m_taskTimer = TASK_TIME;      
        }
      }
    }
  }
}

void DoEvent(uint8_t deviceAddr, uint8_t deviceMode)
{
  // Addr 2 is "both"
  if (deviceAddr == 0 || deviceAddr == 2)
  {
    if (nvSsrMode1 != deviceMode)
    {
      nvSsrMode1 = deviceMode;
      SetState(SSR_1, nvSsrMode1);
      ResetPeriod();
    }
  }
  // Addr 2 is "both"
  if (deviceAddr == 1 || deviceAddr == 2)
  {
    if (nvSsrMode2 != deviceMode)
    {
      nvSsrMode2 = deviceMode;
      SetState(SSR_2, nvSsrMode2);
      ResetPeriod();
    }
  }
}

// Replaces placeholder %VAR% values in HTML being served from here
// (index.html and p1.html and style.css files stored in SPIFFS flash-memory) to browser "out there"
String processor(const String& var)
{
  if (var == PH_DELSTYLE)
    // here we need to hide the delete form if no items to show...
    return m_slotCount ? "display:inline" : "display:none";
    
  if (var == PH_DELETEITEMS)
  {
    // This is in p2.html and allows us to insert "option" tags representing start-stop entries we can delete
    // Ex: <option value="on">on</option>
    String sRet = "";
    if (m_slotCount > 0)
    {
      //TODO - decide how we want to store time-strings in the flash-memory - use the RTC EEPROM??? Store as web-format or ESP32 time-date format???
      // https://www.html.am/tags/html-option-tag.cfm
      for (int ii = 0; ii < MAX_TIME_SLOTS; ii++)
      {
        // get the time-item and format it however we want (it's just for display, we use the index to delete an item)

        t_event slotData = {0};
        // Get the time-slot into slotData
        if (GetTimeSlot(ii, slotData)) // by-reference
        {
          // option accepts: value, selected, label, disabled, id - I'm trying style
          sRet += "<option value=" + String(ii) + ">" + TimeSlotToString(slotData) + "</option>";

          // NO SUCCESS GETTING COLOR TO WORK (FireFox)
          //String sColor = slotData.bEnable ? "black" : "gray";
          // option accepts: value, selected, label, disabled, id - I'm trying style
          //sRet += "<option value='" + String(ii) + "' style='color:" + String(sColor) + "'>" + TimeSlotToString(slotData) + "</option>";

          //String sLabel = slotData.bEnable ? "" : "disabled";
          // option accepts: value, selected, label, disabled, id - I'm trying style
          //sRet += "<option value='" + String(ii) + "' label='" + String(sLabel) + "'>" + TimeSlotToString(slotData) + "</option>";
          
          //sRet += "<option value='" + String(ii) + "'" + (slotData.bEnable ? "" : " disabled") + ">" + TimeSlotToString(slotData) + "</option>";
          
          //prtln("sending value:" + String(ii) + ", \"" + sTime + "\"");
        }
      }
    }
    return sRet;
  }
  
  if (var == PH_ISAPMODE)
  {
    // NOTE tried inserting {% if %ISAPMODE% } {% endif %} into webpage but it would not handle it...
    // so - this just substitures the var with ALL the HTML and javascript needed to enter SSID etc.!
    // Both input and label fields are aligned in the CSS file
    String sRet;
    String sDummyPass = ""; // empty string so user knows they MUST enter PW!
    if (bSoftAP)
    {
      String sTemp;
      if (m_ssid.length() != 0)
          sTemp = m_ssid;
      else
          sTemp = "(not set)";
      RefreshMaxSct();
      sRet = "<form action='/getP1' method='get' name='fName' id='fName'>"
             "<input type='hidden' name='hidName' id='hidName'>WiFi Name:<br>"
             "<select name='wifiName' id='wifiName'>" + WiFiScan(sTemp) + "</select><br>"             
             "<input type='hidden' name='hidPass' id='hidPass'>Password:<br>"
             "<input type='password' name='wifiPass' id='wifiPass' value='" + sDummyPass + "' maxlength='64'>"
             "<input type='submit' value='Submit'></form><br>"
             "<a href='/getP1?butRst=" + String(m_sct) + "'><button>Restore</button></a>"
             "<script>"
                "var varMaxSct = '" + String(m_sct) + "," + String(m_max) + "';"
                "$('#fName').submit(function(){"
                  "var wfn = document.getElementById('wifiName');"
                  "document.getElementById('hidName').value = hnEncode(wfn.options[wfn.selectedIndex].text);"
                  "wfn.value = '';"
                  "var wfp = document.getElementById('wifiPass');"
                  "document.getElementById('hidPass').value = hnEncode(wfp.value);"
                  "wfp.value = '';"
                "});"
             "</script>"
             "<script src='sct.js' type='text/javascript'></script>";
    }
    else
      sRet = ""; // replace %ISAPMODE% with nothing if in STA WiFi mode!
    return sRet;
  }
  
  if (var == PH_PERUNITS)
    return String(perUnits);
    
  if (var == PH_PERMAX)
    return String(perMax);
    
  if (var == PH_PERVAL)
    return String(perVal);
    
  if (var == PH_PHASE)
    return String(phase);
    
  if (var == PH_DC_A)
    return String(dutyCycleA);
    
  if (var == PH_DC_B)
    return String(dutyCycleB);
    
  if (var == PH_HOSTNAME)
    return hostName+".local";
    
  if (var == PH_MAXSCT)
  {
    RefreshMaxSct();
    return String(m_sct) + "," + String(m_max);
  }
  
  if (var == PH_STATE1)
  {
    if(digitalRead(SSR_1))
      ssr1State = "ON";
    else
      ssr1State = "OFF";
    prtln("SSR1 State:" + ssr1State);
    return ssr1State;
  }
      
  if (var == PH_STATE2)
  {
    if(digitalRead(SSR_2))
      ssr2State = "ON";
    else
      ssr2State = "OFF";
    prtln("SSR2 State:" + ssr2State);
    return ssr2State;
  }
    
  if (var == PH_MODE1)
  {
    String s = SsrModeToString(nvSsrMode1);
    prtln("SSR1 Mode:" + s);
    return s;
  }
    
  if (var == PH_MODE2)
  {
    String s = SsrModeToString(nvSsrMode2);
    prtln("SSR2 Mode:" + s);
    return s;
  }
    
  prtln("Unknown:" + var);
  return String();
}

void WiFiMonitorConnection(bool bDisconnect, bool bEraseOldCredentials)
{
  if (bSoftAP)
    return;
    
  //WiFi.status() codes
  //WL_NO_SHIELD   255
  //WL_IDLE_STATUS  0
  //WL_NO_SSID_AVAIL  1
  //WL_SCAN_COMPLETED   2
  //WL_CONNECTED  3
  //WL_CONNECT_FAILED   4
  //WL_CONNECTION_LOST  5
  //WL_DISCONNECTED   6
  int WiFiStatus = WiFi.status();
  
  if (bWiFiConnected)
  {
    if (bDisconnect || WiFiStatus == WL_CONNECT_FAILED || WiFiStatus == WL_CONNECTION_LOST || WiFiStatus == WL_DISCONNECTED)
    {
      if (bDisconnect)
        prtln("STA Mode: Stopping mDNS, web-server and WiFi...");
      else
        prtln("Connection failed, lost or disconnected...");
        
      dnsAndServerStart(true); // stop dns and webserver
      
      WiFi.disconnect(bEraseOldCredentials); // disconnect wifi, erase internal stored info
      bWiFiConnected = false;
      fiveSecondTimer = FIVE_SECOND_TIME-2; // restart in 2-3 seconds
    }
//    else if (!(bManualTimeWasSet || bWiFiTimeWasSet) && bResetOrPowerLoss)
//    {
//    }
  }
  else if (!bDisconnect)
  {
    if (WiFiStatus == WL_CONNECTED)
    {
      prtln(GetStringIP());
      
      dnsAndServerStart();
      
      bWiFiConnected = true;
      bWiFiConnecting = false;
    }
    else if (bWiFiConnecting)
    {
      //prt(".");
    }
    else if (m_ssid.length() != 0) // connect to router
    {
      WiFi.mode(WIFI_STA);
      m_taskMode = TASK_WIFI_CONNECT; // fetch pw from flash and connect
      m_taskTimer = TASK_TIME;      
      bWiFiConnecting = true;
    }
  }
}

void WiFiStartAP(bool bDisconnect, bool bEraseOldCredentials)
{
  if (bDisconnect)
  {
    if (bSoftAP)
    {
      prtln("AP Mode: Stopping mDNS, web-server and WiFi...");
      dnsAndServerStart(true); // stop dns and webserver
      WiFi.softAPdisconnect(bEraseOldCredentials);
      
      // do this to avoid pot thinking knob was turned as we start
      // reading it again when bSoftAP goes false!
      oldPot1Value = pot1Value;
      
      bSoftAP = false;
      digitalWrite(ONBOARD_LED_GPIO2, LOW);
      fiveSecondTimer = FIVE_SECOND_TIME-2;
    }
  }
  else if (!bSoftAP)
  {
    prtln("Starting WiFi AP mode");
    
    WiFi.mode(WIFI_AP);
    //WiFi.mode(WIFI_AP_STA);
    //delay(1000);
    const char* apSSID     = DEFAULT_SOFTAP_SSID;
    const char* apPassword = DEFAULT_SOFTAP_PWD;
    IPAddress apIP(192,168,IP_MIDDLE,DEFAULT_SOFTAP_IP);
    IPAddress gateway(192,168,IP_MIDDLE,DEFAULT_SOFTAP_IP);
    IPAddress subnet(255,255,255,0);
 
    //SSID (defined earlier): maximum of 63 characters;
    //password(defined earlier): minimum of 8 characters; set to NULL if you want the access point to be open
    //channel: Wi-Fi channel number (1-13)
    //ssid_hidden: (0 = broadcast SSID, 1 = hide SSID)
    //max_connection: maximum simultaneous connected clients (1-4)
    //WiFi.softAP(const char* ssid, const char* password, int channel, int ssid_hidden, int max_connection)
    WiFi.softAP(apSSID, apPassword, AP_CHANNEL, 0, MAX_AP_CLIENTS); // additional parms: int channel (1-13), int ssid_hidden (0 = broadcast SSID, 1 = hide SSID), int max_connection (1-4))
    delay(100);
    WiFi.softAPConfig(apIP, gateway, subnet);
    prtln("AP mode IP address:\t" + WiFi.softAPIP());
    
    dnsAndServerStart();

    prtln("Reconnected WiFi as access-point...");
    prtln("IP: 192.168." + String(IP_MIDDLE) + "." + String(DEFAULT_SOFTAP_IP));
    prtln(String(DEFAULT_SOFTAP_SSID));
    prtln(String(DEFAULT_SOFTAP_PWD));
    
    digitalWrite(ONBOARD_LED_GPIO2, HIGH);
    fiveSecondTimer = FIVE_SECOND_TIME-2;
    bSoftAP = true;
  }
}

void dnsAndServerStart(bool bDisconnect)
{
  if (bDisconnect)
  {
    MDNS.end();
    webServer.end();
  }
  else // start
  {
    // Set up mDNS responder:
    // - first argument is the domain name, in this example
    //   the fully-qualified domain name is "esp8266.local"
    // - second argument is the IP address to advertise
    //   we send our IP address on the WiFi network
    if (!MDNS.begin(hostName.c_str()))
      prtln("Error setting up MDNS responder!");
    else
      prtln("mDNS responder started");    

    webServer.begin();
    prtln("TCP server started");
    
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);    
    
    prtln("Host Name: " + hostName);
  }
}

void PollApSwitch()
{
#if FORCE_AP_ON
  bool bApSwitchOn = true;
#else
  bool bApSwitchOn = (digitalRead(SW_SOFT_AP) == HIGH) ? true : false;
#endif

  //prtln("bSoftAP:" + String(bSoftAP) + ", bApSwitchOn:" + String(bApSwitchOn) + ", bWiFiDisabled:" + String(bWiFiDisabled) + ", bWiFiConnected:" + String(bWiFiConnected) );
  
  if (bWiFiConnected)
  {
    if (bWiFiDisabled || bApSwitchOn) 
      WiFiMonitorConnection(true); // disconnect from router
  }
  else // not connected to router
  {
    if (!bWiFiDisabled && !bApSwitchOn) 
      WiFiMonitorConnection(false); // connect to router in STA mode
  }
  
  if (bSoftAP)
  {
    if (bWiFiDisabled || !bApSwitchOn)
      WiFiStartAP(true); // disconnect AP WiFi Mode
  }
  else
  {
    if (!bWiFiDisabled && bApSwitchOn)
      WiFiStartAP(false); // connect in AP mode
  }
}

// esp_deep_sleep_start();
// esp_light_sleep_start();
// esp_sleep_enable_timer_wakeup(); // enable RTC timer's wakeup
// https://github.com/espressif/esp-idf/tree/7d75213/examples/system/deep_sleep
// https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

  switch(wakeupReason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : prtln("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : prtln("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : prtln("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : prtln("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : prtln("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeupReason); break;
  }
}

//-------------------------------------------------------------

void loop()
{
  // return 1/4 sec. timer has not fired
  if (xSemaphoreTake(timerSemaphore, 0) != pdTRUE)
    return;
    
//  uint32_t isrCount = 0, isrTime = 0;
    // Read the interrupt count and time
//  portENTER_CRITICAL(&timerMux);
//  isrCount = isrCounter;
//  isrTime = lastIsrAt;
//  portEXIT_CRITICAL(&timerMux);
    
  // -------- do stuff every .25 sec here
  
  if (m_taskTimer && --m_taskTimer == 0)
    RunTasks();

  // read potentiometer and associated mode-switch every 1/4 second unless locked
  if (m_lockCount == 0xff)
  {
    ReadModeSwitch(); // read center-off SPST 3-Mode switch
    ReadPot1();
  }
  
  if (++quarterSecondTimer < 2)
    return;
  
  quarterSecondTimer = 0; // reset
  
  //-------- do stuff every .5 sec here

  if (ledFlashTimer && --ledFlashTimer == 0)
    digitalWrite(ONBOARD_LED_GPIO2, LOW);
    
  // If button is pressed
  //if (digitalRead(BTN_STOP_ALARM_GPIO0) == LOW)
  //{
  //  // If timer is still running
  //  if (timer)    if (IsLockedAlert(request))
  //    return;
  //  {
  //    // Stop and free timer
  //    timerEnd(timer);
  //    timer = NULL;
  //  }
  //}

  // Algorithm: We set periodTimer from potentiometer and also reset
  // phase and dutyCycle timers to 0, and turn on device A if it's
  // in AUTO mode. This happens each timer-timeout of period-timer.
  // At period-timer timeout, start phase-timer and start the duty-cycle
  // timer for device A.
  // When duty-cycle timer for device A times out, turn off the device.
  // When phase-timer times out, turn on device B if mode is AUTO and
  // start duty-cycle timer B.

  // check duty-cycle before period and set any pending "off" event
  // if duty-cycle is >= 98 we stay on all the time
  if (dutyCycleTimerA && --dutyCycleTimerA == 0)
    if (nvSsrMode1 == SSR_MODE_AUTO && dutyCycleA <= 98)
      SetState(SSR_1, "OFF");
    
  // check duty-cycle before period and set any pending "off" event
  // if duty-cycle is >= 98 we stay on all the time
  if (dutyCycleTimerB && --dutyCycleTimerB == 0)
    if (nvSsrMode2 == SSR_MODE_AUTO && dutyCycleB <= 98)
      SetState(SSR_2, "OFF");

  if (periodTimer && --periodTimer == 0)
  {
    // do this before setting duty-cycle and phase timers!
    periodTimer = ComputePeriod(perVal, perMax, perUnits);
    
    // random mode is > 98
    if (phase >= 98)
      phaseTimer = random(PHASE_MIN, PHASE_MAX)*(float)periodTimer/100.0;
    else
      phaseTimer = phase*(float)periodTimer/100.0;
      
    // phaseTimer can compute to 0 - turn on both at same time
    if (phaseTimer == 0)  
      SSR2On(periodTimer);
      
    SSR1On(periodTimer);
      
    savePeriod = periodTimer; // save for use computing duty-cycles
  }
    
  if (phaseTimer && --phaseTimer == 0)
    SSR2On(savePeriod);
    
  //----------------------------------------------
  // Read RTC every .5 seconds of the hardware-timer - if it's changed
  // since the last sample, an RTC second's elapsed...
  //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html
  // Start synchronization via sntp:
  //sntp_setoperatingmode(SNTP_OPMODE_POLL);
  //sntp_setservername(0, "pool.ntp.org");
  //sntp_init();
  time_t now = time(0);
  if (now == m_prevNow)
    return;
  m_prevNow = now;
    
  if (now % 60 == 0)
  {
    // -------- do stuff every 1 minute here

    // we have a brand new minute here - which means seconds == 00 on the RTC - but
    // we need to sync seconds for the hw timer still, and reset the software timers
    if (bRequestManualTimeSync || bRequestWiFiTimeSync)
    {
      DoTimeSyncOneSecondStuff();
      
      // set "time was set" flag before initializeing lists (the init routines check for time-sync!)
      if (bRequestManualTimeSync)
      {
        prtln("Time set manually... enabling time-events!");
        bRequestManualTimeSync = false;
        bManualTimeWasSet = true;
      }
      if (bRequestWiFiTimeSync)
      {
        prtln("Time set via internet... enabling time-events!");
        bRequestWiFiTimeSync = false;
        bWiFiTimeWasSet = true;
      }
    
      int iNZScount = InitSecondsList(m_slotCount); // call this AFTER CountFullTimeSlots()!
      prtln("Number of items with nonzero seconds: " + String(iNZScount));
      // InitRepeatList will delete expired slots that repeat
      int iRPTcount = InitRepeatList(m_slotCount);
      prtln("Number of items with repeat count: " + String(iRPTcount));  
    }
  
    // do stuff every minute here
    ProcessMinuteResolutionTimeSlots();
  }

  // -------- do stuff every 1 sec here
  
  if (clockSetDebounceTimer) // prevent multiple manual clock-set requests
    clockSetDebounceTimer--;

  ProcessSecondResolutionTimeSlots();

  if (++fiveSecondTimer >= FIVE_SECOND_TIME)
  {
    // -------- do stuff every 5 sec here

    // this is clumsy bit all I could think of for now - when the p2.html page is displaying time-slots
    // and then back here in the code we auto-delete an event - we need to update the list. So I'm
    // doing that by setting this flag, then if the flag is set, the 1 second time-request from the
    // user's web-browser (from p2.html) will get an extra "Treload" tacked on to the time/date.
    // we clear this flag there, but it can get "stuck on" if the user closes the page randomly...
    // so here, we clear the flag.
    if (bTellP2WebPageToReload)
      bTellP2WebPageToReload = false;
    
    // do stuff every 5 sec here
    PollApSwitch();
  
    // Time from router:
    // I get the date and time from my router using client.print(“GET / HTTP/1.1\r\n Host: 192.168.1.254 \r\nConnection: close\r\n\r\n’ | nc 192.168.1.254 80” );
    // which returns the local time.
    // https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
    // https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/NTP-TZ-DST/NTP-TZ-DST.ino
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html
    // https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/ (gets time from the web)
    // https://en.cppreference.com/w/c/chrono/localtime
  
    // return from loop if we re-synced!
    if (CheckForWiFiTimeSync())
      return;
      
    WiFiMonitorConnection();
    
    // if user input data
    if(xSemaphoreTake(webInputSemaphore, 0) == pdTRUE)
    {
      if (inputMessage != "" && inputMessage != "No message sent" && inputParam != "" && inputParam != "none")
      {
        prtln("inputParam: " + inputParam);
        prtln("inputMessage: " + inputMessage);
      }
    }

    fiveSecondTimer = 0; // reset
  }
}

// timerEnd();
// bool timerStarted(#); bool timerAlarmEnabled(#);
// timerStart(#); timerRestart(#); timerStop(#); timerAlarmDisable(#);
// C:\Users\Scott\Documents\Arduino\hardware\espressif\esp32\cores\esp32\esp32-hal-timer.h
void SetupAndStartHardwareTimeInterrupt()
{
  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();

  // Use 1st timer of 4 (counted from zero).
  // The frequency of the base signal used by the ESP32 counters is 80 MHz. If we divide this value
  // by 80 (using 80 as the prescaler value), we will get a signal with a 1 MHz frequency that will
  // increment the timer counter 1,000,000 times per second.
  // hw_timer_t * timerBegin(uint8_t num, uint16_t divider, bool countUp);
  m_timer = timerBegin(TIMER_0, 80, true); // Set 80 divider for prescaler, count up

  // Attach onTimer function to our timer.
  timerAttachInterrupt(m_timer, &onTimer, true);

  // Set alarm to call onTimer function every 1/4 second (value in microseconds).
  // Repeat the alarm (third parameter)
  m_prevNow = DoTimeSyncOneSecondStuff(time(0));

  // Start timer-interrupt
  timerAlarmEnable(m_timer);
}

time_t DoTimeSyncOneSecondStuff(time_t now)
{
  // synchronize to RTC seconds
  // NOTE: we sync to minutes also but after this, in loop()
  while (time(0) == now)
    yield();
    
  DoTimeSyncOneSecondStuff();
  
  return now+1;
}

void DoTimeSyncOneSecondStuff(void)
{
  prtln("Synchronizing RTC to internal timer...");
  
  // this one is important to make "now" - it rolls over at 2...
  quarterSecondTimer = 2;

  // this one's not used for sync and IS used as a "debounce"
  // for web-input, so leave it 0
  fiveSecondTimer = 0;
  
  ledFlashTimer = 0;
  //clockSetDebounceTimer = 0; DON'T DO THIS HERE! this negates the debounce!
  
  ResetPeriod();
  
  HardwareTimerRestart(m_timer);
}

// call this when the internal clock is changed or set
// FYI: C:\Users\Scott\Documents\Arduino\hardware\espressif\esp32\cores\esp32\esp32-hal-timer.h
void HardwareTimerRestart(hw_timer_t * timer)
{
  prtln("Restarting hardware timer...");
  
  // Set alarm to call onTimer function every 1/4 second (value in microseconds).
  // Repeat the alarm (third parameter)
  // void timerAlarmWrite(hw_timer_t *timer, uint64_t alarm_value, bool autoreload);
  //  timerAlarmWrite(timer, 1000000, true); // 1 second
  //  timerAlarmWrite(timer, 1000000, false); // disable (may then need yeild() before re-enabling!)
  timerAlarmWrite(timer, 1000000/4, true); // .25 sec
}

void SSR1On(int iPeriod)
{
  if (nvSsrMode1 == SSR_MODE_AUTO)
    SetState(SSR_1, "ON");
    
  // random mode is 0
  if (dutyCycleA == 0)
    dutyCycleTimerA = random(MIN_RAND_PERCENT_DUTY_CYCLE, DUTY_CYCLE_MAX);
  else
    dutyCycleTimerA = dutyCycleA;
  dutyCycleTimerA = (float)(dutyCycleTimerA*iPeriod)/100.0;
}

void SSR2On(int iPeriod)
{
  if (nvSsrMode2 == SSR_MODE_AUTO)
    SetState(SSR_2, "ON");

  // random mode is 0
  if (dutyCycleB == 0)
    dutyCycleTimerB = random(MIN_RAND_PERCENT_DUTY_CYCLE, DUTY_CYCLE_MAX);
  else
    dutyCycleTimerB = dutyCycleB;
  dutyCycleTimerB = (float)(dutyCycleTimerB*iPeriod)/100.0;
}

// return true if timers reset (need to return from loop())
bool CheckForWiFiTimeSync()
{
  // don't check unless wifi and time's not yet been set or if 
  // in the process of synchronizing minutes...
  if (!bWiFiConnected || bManualTimeWasSet || bWiFiTimeWasSet || bRequestWiFiTimeSync || bRequestManualTimeSync)
    return false;

  struct tm timeInfo = {0};
  if (ReadInternalTime(NULL, &timeInfo) == NULL) // get current time as struct tm
    return false;

  int myYear = timeInfo.tm_year + EPOCH_YEAR;

  if (myYear > DEFAULT_YEAR)
  {
    prtln("Requesting WiFi time-sync...");
    bRequestWiFiTimeSync = true;
    return true;
  }
  //else if (myYear < DEFAULT_YEAR)
  //  prtln("Y2038 Issue??? CheckForTimeSync() year: " + String(myYear) + " less than DEFAULT_YEAR: " + String(DEFAULT_YEAR));
  return false;
}
