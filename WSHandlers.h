#ifndef WSHandlersH
#define WSHandlersH

#include <Arduino.h>

// NOTE: there are many const strings at the top of WSHandlers.cpp that should be changed if you are
// refreshing "security" overall... so of them need to also be changed in their respective web-page
// HTML or JavaScript files. You can do it fast with Notepad++ - just open all files and search for a
// string then do a global-replace. Don't forget to re-obfuscate the .js files at https://obfuscator.io/

// These you can't change unless you search for the strings in the respective javascript
// files in \Documents\Arduino\projects\ESP32\Gpc\origdata and change them there, then upload the .js file to
// https://obfuscator.io/ and put the obfuscated file into the \Documents\Arduino\projects\ESP32\Gpc\data folder, then rebuild
// fc.spiffs.bin by double-clicking fcspiffs.bat. Then upload the bin file using
// Arduino IDE 1.x Tools->ESP32 Sketch Data Upload. NOTE: the menu item won't appear until you
// unzip ESP32FS.zip into \Documents\Arduino\tools\. It has the python data-file uploader tool.
#define ERASE_DATA_CONFIRM "K7w5V" // p2.js
#define PARAM_STATE1_VALUE "F2jB" // p0.js stateTxtA
#define PARAM_STATE2_VALUE "iV2m" // p0.js stateTxtB
#define PARAM_STATE3_VALUE "HwOI" // p0.js stateTxtC
#define PARAM_STATE4_VALUE "mPsE" // p0.js stateTxtD
#define PARAM_TEXT_VALUE   "u7Za" // p0.js

#define TEXT_PREAMBLE "TXT:" // sent to a web-browser to differentiate a "heartbeat" normal data-response from that of a CMtxt message

// The WiFi credentials entry boxes and reset button only appear when you connect
// in AP mode (one of the choices in the 3-position WiFi switch on the Gpc)
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
#define LOGIN_FILENAME    "/loginIndex.html"
#define INDEX_FILENAME    "/index.html"
#define SERVERINDEX_FILENAME  "/serverIndex.html"

#define JS_DIRECTORY "/js"
#define P0JS_FILENAME     "/p0.js"
#define P1JS_FILENAME     "/p1.js"
#define P2JS_FILENAME     "/p2.js"
#define JQUERY_FILENAME   "/jquery.min.js"
#define SCT_FILENAME      "/sct.js"

#define CSS_DIRECTORY "/css"
#define STYLE1_FILENAME   "/style1.css"
#define STYLE2_FILENAME   "/style2.css"
#define STYLE3_FILENAME   "/style3.css"
#define STYLELED_FILENAME "/led.css"

// web-page entry points (if you change these, they must also be changed in the html/javascript files in "data")
#define EP_POST_UPDATE     "/update"
#define EP_POST_P2         "/postP2"
#define EP_POST_P2FORM     "/p2Form"
#define EP_GET_P1          "/getP1"
#define EP_ALT_P1          "/altP1"
#define EP_GET_P2          "/getP2"
#define EP_GET_LIFORM      "/getLIform"
#define EP_GET_BUTTONS     "/getBut"
#define EP_GET_HEART       "/getHeart"
#define EP_GET_INDEX       "/getIndex"

// Web-page placeholders we fill-in dynamically as the html file is "served"
// index.html
const char PH_HOSTNAME[] = "HOSTNAME";
const char PH_MAXSCT[] = "MAXSCT";
const char PH_PERVARS[] = "PERVARS";
const char PH_LABEL_A[] = "LABELA";
const char PH_LABEL_B[] = "LABELB";
const char PH_LABEL_C[] = "LABELC";
const char PH_LABEL_D[] = "LABELD";

// p1.html
const char PH_P1APMODE[] = "P1APMODE";
const char PH_P1VARS[] = "P1VARS"; // combination script tag with vars for g8_midiChan, g8_midiNoteA, g8_midiNoteB, g8_midiNoteC, g8_midiNoteD

// p2.html
const char PH_P2DELSTYLE[] = "P2DELSTYLE";
const char PH_P2DELITEMS[] = "P2DELITEMS";

// ------------------ web-server handlers -------------------

// NOTE: These all appear both here and in the .js files for the web-pages served out of SPIFFS file-system
// by the AsyncWebServer library. You can best change via Notepad++ by opening all .html files and .js (in "origdata" folder)
// files as well as this file and WSHandlers.h and use Find/Replace "in all opened documents". Then upload the changed .js files
// to https://obfuscator.io/ and save obfuscated files in the "data" folder.

// sct.js heartbeat /getHeart
const char PARAM_HEARTBEAT[]   = "neihs";

// index.html
const char PARAM_HOSTNAME[]    = "hddsi"; // hnEnc
const char PARAM_PERMAX[]      = "geuge"; // perMax
const char PARAM_PERUNITS[]    = "djeuc"; // perUnits
const char PARAM_PERVAL[]      = "rrksv"; // perVal
const char PARAM_STATE1[]      = "mwial"; // stateTxtA
const char PARAM_STATE2[]      = "uvohh"; // stateTxtB
const char PARAM_STATE3[]      = "bseor"; // stateTxtC
const char PARAM_STATE4[]      = "lewap"; // stateTxtD
const char PARAM_LABEL_A[]     = "wjdte"; // labelTxtA
const char PARAM_LABEL_B[]     = "meufw"; // labelTxtB
const char PARAM_LABEL_C[]     = "keinj"; // labelTxtC
const char PARAM_LABEL_D[]     = "jyolx"; // labelTxtD
const char PARAM_TEXT[]        = "ifhrv"; // text-message

// loginIndex.html
const char PARAM_UDID[]   = "bwihl"; // used in firmware update
const char PARAM_UDPW[]   = "pdklt";

// p1.html

// (PARAM_BUTRST, PARAM_WIFINAME and PARAM_WIFIPASS can be changed without reprocessing the spiffs web-data)
const char PARAM_BUTRST[]     = "neuqj"; // restore button press
const char PARAM_WIFINAME[]   = "fgdje"; // set by hidden field on p1.html - variable %P1APMODE% is replaced by two edit-input fields
const char PARAM_WIFIPASS[]   = "rsgyb"; // data submitted is handled in in ReplaceHtmlPercentSign().

// changing these requires serching for the strings in p1.html/p1.js and changeing there too!
const char PARAM_PHASE_B[]    = "jeita"; // phase B slider p1.js
const char PARAM_PHASE_C[]    = "aimrs"; // phase C slider p1.js
const char PARAM_PHASE_D[]    = "kepuw"; // phase D slider p1.js
const char PARAM_DC_A[]       = "neufb"; // duty-cycle A slider p1.js
const char PARAM_DC_B[]       = "xbmey"; // duty-cycle B slider p1.js
const char PARAM_DC_C[]       = "wovrj"; // duty-cycle C slider p1.js
const char PARAM_DC_D[]       = "lrone"; // duty-cycle D slider p1.js
const char PARAM_MIDICHAN[]   = "ahejn";
const char PARAM_MIDINOTE_A[] = "ehwdo";
const char PARAM_MIDINOTE_B[] = "fjezm";
const char PARAM_MIDINOTE_C[] = "yrpma";
const char PARAM_MIDINOTE_D[] = "keiwh";

// p2.html
// received from p2.html when Add button pressed
const char PARAM_MINUTE[] = "hriqn";
const char PARAM_HOUR[] = "nwuds";
const char PARAM_SECOND[] = "bbeis";
const char PARAM_AMPM[] = "heidc";
const char PARAM_DEVICE_MODE[] = "whrro";
const char PARAM_DEVICE_ADDR[] = "ajedd";
const char PARAM_REPEAT_MODE[] = "nriwq";
const char PARAM_REPEAT_COUNT[] = "jegsi";
const char PARAM_EVERY_COUNT[] = "ehitm";
const char PARAM_DATE[] = "wjyrc";
// when Edit button pressed on p2.html
const char PARAM_EDITINDEX[] = "ejddo";
const char PARAM_DELINDEX[] = "heifq";
// received from p2.html
const char PARAM_REPLACEINDEX[] = "jfrej";
const char PARAM_INCLUDETIMINGCYCLE[] = "whsin";
const char PARAM_TIMINGCYCLEINREPEATS[] = "ydjyz";
const char PARAM_USEOLDTIMEVALS[] = "vjenw";
const char PARAM_DATETIME[] = "gejcc";
const char PARAM_FILEDATA[] = "wqjun";
const char PARAM_ERASEDATA[] = "lkeism";

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

//extern const char PARAM_HEARTBEAT[], PARAM_HOSTNAME[], PARAM_PERMAX[], PARAM_PERUNITS[];
//extern const char PARAM_PERVAL[], PARAM_STATE1[], PARAM_STATE2[], PARAM_STATE3[], PARAM_STATE4[], PARAM_LABEL_A[], PARAM_LABEL_B[], PARAM_LABEL_C[], PARAM_LABEL_D[], PARAM_TEXT[];
//extern const char PARAM_UDID[], PARAM_UDPW[], PARAM_BUTRST[], PARAM_WIFINAME[], PARAM_WIFIPASS[], PARAM_PHASE_B[], PARAM_PHASE_C[], PARAM_PHASE_D[];
//extern const char PARAM_DC_A[], PARAM_DC_B[], PARAM_DC_C[], PARAM_DC_D[], PARAM_MIDICHAN[], PARAM_MIDINOTE_A[], PARAM_MIDINOTE_B[], PARAM_MIDINOTE_C[], PARAM_MIDINOTE_D[];
//extern const char PARAM_MINUTE[], PARAM_HOUR[], PARAM_SECOND[], PARAM_AMPM[], PARAM_DEVICE_MODE[];
//extern const char PARAM_DEVICE_ADDR[], PARAM_REPEAT_MODE[], PARAM_REPEAT_COUNT[], PARAM_EVERY_COUNT[];
//extern const char PARAM_DATE[], PARAM_EDITINDEX[], PARAM_DELINDEX[], PARAM_REPLACEINDEX[], PARAM_INCLUDETIMINGCYCLE[];
//extern const char PARAM_TIMINGCYCLEINREPEATS[], PARAM_USEOLDTIMEVALS[], PARAM_DATETIME[], PARAM_FILEDATA[], PARAM_ERASEDATA[];
//
//extern const char PH_HOSTNAME[], PH_MAXSCT[], PH_PERVARS[], PH_LABEL_A[], PH_LABEL_B[], PH_LABEL_C[], PH_LABEL_D[];
//extern const char PH_P1APMODE[], PH_P1VARS[], PH_P2DELSTYLE[], PH_P2DELITEMS[];
