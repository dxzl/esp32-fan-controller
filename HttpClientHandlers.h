#ifndef HttpClientHandlersH
#define HttpClientHandlersH

#define DEF_HTTP_TIMEOUT      3 // seconds
#define MAX_HTTP_CLIENT_SENDS 3
//#define HTTP_TEXT_MAXCHARS 80
#define MAX_HEADER_LENGTH 200
#define MINUTES_AFTER_DEF_TOKEN_FAIL_TO_INITIATE_RESET 2 // 0 is OFF

#define MIN_VALUE_LENGTH 1
#define MIN_NAME_LENGTH 1
#define MAX_VALUE_LENGTH 100
#define MAX_NAME_LENGTH 20

#define CANRX_PARAMETER_COUNT 2 // fixed number of parameters in CanRx HTTP request
#define MAINRX_PARAM_COUNT 2

// 24 => 32 bit int - 3 bits in canrx half-token + 3 bits in rxtx half-token and 1 bit as a right-justified "always 1" marker
// used for "unshifting" - we want to steer clear of bit 31 (the int's sign-bit) so 32-(3+3+1+1)=24
#define MAX_CANRX_TOKEN_SHIFT 12 // 1-24 (24 in theory but better stick with 12 or under for less processing and length...)

// 7/17/2024 - we'll invoke the "Can Rx?" HTTP query to set the Rx/Tx tokens when we try to send and find they are NO_TOKEN
// The "CanRx" query token is initially set randomly and it will fail... but the fail-process then sets a CanRx
// token using g_defToken to encode/decode that new token - and two successive failures causes g_origDefToken
// to be used in encoding/decoding the new CanRx token - which should succeed on the next attempt...
//
// OK - all of the below is out the window (it's implimented in version 2.37 but never tested)... new way...
// We send "Can Rx?" query. Receiver either can or can't decode it. We add
// the last two octets of the receiver's MAC as a header (going both ways) and encoded with ****. On a good decode, the remaining bits
// of a new defToken is sent in the string-field with HTTPCODE_CANRX_OK. If the receiver's MAC is greater than the sender's, then
// the receiver sends back its defToken and HTTPCODE_CANRX_DECODE_FAIL. If not, we send HTTPCODE_CANRX_FAIL and do nothing.
//
// [If we add a new ESP32 unit to a network of other units that has been up for some time and has
// collectively changed the "default token" (saved in flash-settings) - there's a problem. The tokens the
// new unit sets as default for Rx/Tx will not work. When an exixting unit sends to us, we can't decode it
// and send back a fail-response code and a new token to use - but that's itself encoded with g_defToken and
// HttpClientCallback() won't be able to decode it. So - what will happen is that HttpClientCallback() queues the
// TASK_SEND_FAILSAFE_REQ task (processed in RunTasks()). We specially encode the HTTP_FAILSAFE_COMMAND and send it
// to the new unit which decodes it in HandleHttpAsyncFailsafeReq() and sets its Rx/Tx tokens for our IP to
// FAILSAFE_DEF_TOKEN2 and sends a response back to HttpClientCallback() that's encoded with FAILSAFE_DEF_TOKEN1.
// Upon correct decode we set our Rx/Tx tokens for the remote IP to FAILSAFE_DEF_TOKEN2 as well... and also
// set up to initiate a system-wide default token change in 3 minutes. Voila!]

// new strategy 7/10/2023 - default token state = NO_TOKEN. on send, if no-token, instead send a new HTTP command
// "are you enabled". Rx unit checks its Rx-enabled flag and replies with "use this token" and embeds token in bits
// 9-14 if enabled. On callback reply, if we get token, set our tx token for that IP to it. if we instead get
// HTTPCODE_RXDISABLED, set our tx token to NO_TOKEN. If we have a tx token, send normally.

// int16_t code. we use bits 0-8 for codes 0-511,
// bit 15 is sign bit for negative error-codes.
// (bits 9-14 can be used for data)
// pertains to WsHandlers() HandleHttpAsyncReq()

// range 200-255
#define HTTPCODE_OK           200      // standard HTTP success code, don't change!
#define HTTPRESP_OK           "OK"     // (used in OTA programming [so don't change!])
#define HTTPCODE_CANRX_OK     244      // response data has other 3-bits of new Rx/Tx token pair shifted left 5
#define HTTPCODE_PROCESSING_OK 255
#define HTTPRESP_PROCESSING_OK "MeIse" // reply to sender's callback from HandleHttpAsyncReq() in wsHandlers.cpp

// range 400-511
// codes used by HandleHttpAsyncReq()
#define HTTPCODE_FAIL           400         // standard HTTP fail code, don't change!
#define HTTPRESP_FAIL           "FAIL"      // (used in OTA programming [so don't change!])
#define HTTPCODE_NOIP_FAIL      501
#define HTTPRESP_NOIP_FAIL      "VesAm"
#define HTTPCODE_RXTOKEN_FAIL   502
#define HTTPRESP_RXTOKEN_FAIL   "esdIb"
#define HTTPCODE_RXDISABLED     503 // sent to HttpClientCallback if "c sync rx off" command (g_bSyncRx = false)
#define HTTPRESP_RXDISABLED     "eucjt"
#define HTTPCODE_PARAM_TOOSHORT 504
#define HTTPRESP_PARAM_TOOSHORT "KDheV"
#define HTTPCODE_PARAM_TOOLONG  505
#define HTTPRESP_PARAM_TOOLONG  "Odesf"

// codes sent by HandleHttpAsyncCanRxReq()
// unencoded:
//HTTPCODE_RXDISABLED, HTTPRESP_RXDISABLED (above)
//HTTPCODE_ADDIP_FAIL, HTTPRESP_ADDIP_FAIL
// encoded:
//HTTPCODE_CANRX_FAIL, HTTPRESP_CANRX_FAIL
//HTTPCODE_CANRX_DECODE_FAIL, HTTPRESP_CANRX_FAIL
//HTTPCODE_CANRX_OK, with low 3-bits of both the next canrx token and next tx token
#define HTTPCODE_ADDIP_FAIL         510
#define HTTPRESP_ADDIP_FAIL         "mejDp"
#define HTTPCODE_CANRX_FAIL         511
#define HTTPRESP_CANRX_FAIL         "KEidG"
#define HTTPCODE_CANRX_DECODE_FAIL  513 // response data has new default token to set (can be 255 = NO_TOKEN)

#define MAX_PROC_CODE_LENGTH 3 // (0-255) max decoded sProcCode string-length for all digits allowing for +/- sign (if any)
// codes below will be left-shifted 4 and bits 0-3 made random
#define PROCESSED_CODE_CHANGE_OK            2 // these codes should all be positive, greater than 0, up to 16
#define PROCESSED_CODE_PARAM_OK             3
#define PROCESSED_CODE_NOPARMS              4
#define PROCESSED_CODE_PARAM_FAIL           5
#define PROCESSED_CODE_DECODE_FAIL          6
#define PROCESSED_CODE_COMMAND_DECODE_FAIL  7
#define PROCESSED_CODE_NORXTOKEN            8
#define PROCESSED_CODE_NOLINK               9
#define PROCESSED_CODE_BADIP                10
#define PROCESSED_CODE_DECPREV_FAIL         11

// after resetting wifi-parameters flash-memory via "c reset wifi", this device will try
// to connect to a router called MyRouter with password MyRouterPass. you can use the USB command-interface
// with "c wifi ssid NEWSSID" and "c wifi pass NEWWIFIPASS" to set different values.
// NOTE: tie GPIO18 low (it has an internal pulldown!) and GPIO19 high for WiFi station-mode (router-mode)
// this is the mDNS service name we search the LAN (local router network) for to find other
// connected ESP32s like us! we can then get their IP addresses via mDNS search and add them to a list.
// we then can exchange data with other units such as (when "c sync on") cycle timing parameter changes.
// data-exchange is done via the AsyncHTTPRequest library, SendHttpReq(), and the HttpClientCallback() method.
// NOTE: tie GPIO18 high and GPIO19 low (it has an internal pulldown!) for WiFi AP-mode (access-point-mode)

// ------------------ web-client handlers -------------------
// These pertain to http get requests from our client-library (HttpClientHandlers.cpp)
// the strings can be freely changed for security... but if you change them, ALL devices must be updated!

//#define HTTP_SERVER_IP_HEADER_NAME  "evdo" // receiving unit's IP added to response via custom header 
#define HTTP_CLIENT_MAC_HEADER_NAME "WyoGkw" // last two octets of MAC address added to client-send via custom header
#define HTTP_SERVER_MAC_HEADER_NAME "Ldris" // last two octets of MAC address added to response via custom header

// Commands sent in the HTTP_ASYNCREQ_CANRX_PARAM_COMMAND field
#define HTTP_COMMAND_CANRX          "legou"

const char HTTP_ASYNCREQ_CANRX[] = "/lnrgj";
const char HTTP_ASYNCREQ_CANRX_PARAM_COMMAND[] = "abon";
const char HTTP_ASYNCREQ_CANRX_PARAM_TOK3BITS[] = "kjewy";

const char HTTP_ASYNCREQ[] = "/wridt";
const char HTTP_ASYNCREQ_PARAM_COMMAND[] = "LFvte";
const char HTTP_ASYNCREQ_PARAM_PROCCODE[] = "odkle";

void ClearLinkOk(String sIp, int code);
void InitHttpClientHandlers();
bool SendHttpCanRxReq(String sIp);
bool SendHttpReq(String sIp);
String GetHttpCodeString(int httpCode);

#endif

//extern const char HTTP_ASYNCREQ[], HTTP_ASYNCREQ_PARAM_COMMAND[], HTTP_ASYNCREQ_PARAM_PROCCODE[];
//extern const char HTTP_ASYNCREQ_CANRX[], HTTP_ASYNCREQ_CANRX_PARAM_COMMAND[], HTTP_ASYNCREQ_CANRX_PARAM_TOK3BITS[];
