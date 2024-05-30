#ifndef WSHandlersH
#define WSHandlersH

#include <Arduino.h>

// These you can't change unless you search for the strings in the respective javascript
// files in \Documents\Arduino\projects\ESP32\FanController\origdata and change them there, then upload the .js file to
// https://obfuscator.io/ and put the obfuscated file into the \Documents\Arduino\projects\ESP32\FanController\data folder, then rebuild
// fc.spiffs.bin by double-clicking fcspiffs.bat. Then upload the bin file using
// Arduino IDE 1.x Tools->ESP32 Sketch Data Upload. NOTE: the menu item won't appear until you
// unzip ESP32FS.zip into \Documents\Arduino\tools\. It has the python data-file uploader tool.
#define ERASE_DATA_CONFIRM "K7w5V" // p2.js
#define PARAM_STATE1_VALUE "F2jB" // p0.js stateTxtA
#define PARAM_STATE2_VALUE "iV2m" // p0.js stateTxtB
#define PARAM_TEXT_VALUE   "u7Za" // p0.js

#define TEXT_PREAMBLE "TXT:" // sent to a web-browser to differentiate a "heartbeat" normal data-response from that of a CMtxt message

// The WiFi credentials entry boxes and reset button only appear when you connect
// in AP mode (one of the choices in the 3-position WiFi switch on the fan-controller)
// and go to P1.html on any web-browser.
// You can change PARAM_BUTRST_VALUE without re-building and uploading
// the SPIFFS web-site data. This one is injected into P1.html via %PH_P1APMODE%
// when the web-page is invoked by a remote browser when connected to us in AP
// WiFi mode (as opposed to connection via a router). The web-server hook is
// invoked via EP_GET_P1 (/getP1) Since none of the HTML or javascript is in
// any file in our "data" folder.
#define PARAM_BUTRST_VALUE "gWeFi"

// web-page files in SPIFFS flash-memory
#define HELP1_FILENAME    "/help1.html"
#define HELP2_FILENAME    "/help2.html"
#define P1_FILENAME       "/p1.html"
#define P2_FILENAME       "/p2.html"
#define P0JS_FILENAME     "/p0.js"
#define P1JS_FILENAME     "/p1.js"
#define PLJS_FILENAME     "/pl.js"
#define P2JS_FILENAME     "/p2.js"
#define JQUERY_FILENAME   "/jquery.min.js"
#define SCT_FILENAME      "/sct.js"
#define STYLE1_FILENAME   "/style1.css"
#define STYLE2_FILENAME   "/style2.css"
#define STYLE3_FILENAME   "/style3.css"
#define STYLELED_FILENAME "/led.css"
#define LOGIN_FILENAME    "/loginIndex.html"
#define INDEX_FILENAME    "/index.html"
#define SERVERINDEX_FILENAME  "/serverIndex.html"

// web-page entry points
#define EP_POST_UPDATE     "/update"
#define EP_POST_P2         "/postP2"
#define EP_POST_P2FORM     "/p2Form"
#define EP_GET_P1          "/getP1"
#define EP_GET_P2          "/getP2"
#define EP_GET_LIFORM      "/getLIform"
#define EP_GET_BUTTONS     "/buttons"
#define EP_GET_HEART       "/getHeart"
#define EP_GET_INDEX       "/getIndex"

// web-server handlers
void HandleHeartbeatReq(AsyncWebServerRequest *request);
void HandleHttpAsyncCanRxReq(AsyncWebServerRequest *request);
void HandleHttpAsyncTextReq(AsyncWebServerRequest *request);
void HandleHttpAsyncReq(AsyncWebServerRequest *request);
void HandleButtonsReq(AsyncWebServerRequest *request);
void HandleIndexReq(AsyncWebServerRequest *request);
void HandleGetP2Req(AsyncWebServerRequest *request);
void HandleAltP1Req(AsyncWebServerRequest *request);
void HandleGetP1Req(AsyncWebServerRequest *request);
void HandleP2FormReq(AsyncWebServerRequest *request);
void HandlePostP2Req(AsyncWebServerRequest *request);
void SendHttpClientMacResponse(AsyncWebServerRequest *request, String sResp, int code);
void SendHttpClientResponse(AsyncWebServerRequest *request, String sResp, int code);
String ReplaceHtmlPercentSign(const String& var);

#endif

extern const char PARAM_HEARTBEAT[], PARAM_HOSTNAME[], PARAM_PERMAX[], PARAM_PERUNITS[];
extern const char PARAM_PERVAL[], PARAM_STATE1[], PARAM_STATE2[], PARAM_LABEL_A[], PARAM_LABEL_B[], PARAM_TEXT[];
extern const char PARAM_UDID[], PARAM_UDPW[], PARAM_BUTRST[], PARAM_WIFINAME[], PARAM_WIFIPASS[], PARAM_PHASE[];
extern const char PARAM_DC_A[], PARAM_DC_B[], PARAM_MIDICHAN[], PARAM_MIDINOTE_A[], PARAM_MIDINOTE_B[];
extern const char PARAM_MINUTE[], PARAM_HOUR[], PARAM_SECOND[], PARAM_AMPM[], PARAM_DEVICE_MODE[];
extern const char PARAM_DEVICE_ADDR[], PARAM_REPEAT_MODE[], PARAM_REPEAT_COUNT[], PARAM_EVERY_COUNT[];
extern const char PARAM_DATE[], PARAM_EDITINDEX[], PARAM_DELINDEX[], PARAM_REPLACEINDEX[], PARAM_INCLUDETIMINGCYCLE[];
extern const char PARAM_TIMINGCYCLEINREPEATS[], PARAM_USEOLDTIMEVALS[], PARAM_DATETIME[], PARAM_FILEDATA[], PARAM_ERASEDATA[];

extern const char PH_HOSTNAME[], PH_MAXSCT[], PH_PERVARS[], PH_LABEL_A[], PH_LABEL_B[];
extern const char PH_P1APMODE[], PH_P1VARS[], PH_P2DELSTYLE[], PH_P2DELITEMS[];
