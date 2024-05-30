#ifndef HttpClientHandlersH
#define HttpClientHandlersH

#define DEF_HTTP_TIMEOUT      3 // seconds
#define MAX_HTTP_CLIENT_SENDS 3
//#define HTTP_TEXT_MAXCHARS 80
#define MAX_HEADER_LENGTH 200
#define MINUTES_AFTER_DEF_TOKEN_FAIL_TO_INITIATE_RESET 2 // 0 is OFF

#define CANRX_TOKEN_SHIFT 4 // 1-5
#define HTTP_SERVER_IP_HEADER_NAME         "SmOw" // receiving unit's IP added to response via custom header 
#define HTTP_CLIENT_MAC_HEADER_NAME        "EgLg" // last two octets of MAC address added to client-send via custom header
#define HTTP_SERVER_MAC_HEADER_NAME        "jeUc" // last two octets of MAC address added to response via custom header

// HTTP commands sent in the HTTP_CANRX_PARAM_COMMAND field
#define HTTP_COMMAND_CANRX                 "Ofew"

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
#define HTTPRESP_OK           "OK"     // (used in OTA programming!)
#define HTTPCODE_PARAM_OK     211
#define HTTPRESP_PARAM_OK     "ZqTi"
#define HTTPCODE_TOK_OK       222
#define HTTPRESP_TOK_OK       "rHYY"
//#define HTTPCODE_TXT_OK       233
//#define HTTPRESP_TXT_OK       "RqeW"
#define HTTPCODE_CANRX_OK     244      // response data has other 3-bits of new Rx/Tx token pair shifted left 5

// range 400-511
#define HTTPCODE_FAIL           400         // standard HTTP fail code, don't change!
#define HTTPRESP_FAIL           "FAIL"      // (used in OTA programming!)
#define HTTPCODE_PARAM_FAIL     501         // good remote decode but error in remote mDNS command string processing
#define HTTPRESP_PARAM_FAIL     "cdxy"
#define HTTPCODE_DECODE_FAIL    502         // bad remote decode, remote has set rxToken NO_TOKEN
#define HTTPRESP_DECODE_FAIL    "zomc"
#define HTTPCODE_DECPREV_FAIL   503         // bad remote decode, remote has moved rxPrevToken to rxToken
#define HTTPRESP_DECPREV_FAIL   "AFhO"
#define HTTPCODE_ADDIP_FAIL     504
#define HTTPRESP_ADDIP_FAIL     "kbFU"
#define HTTPCODE_TOK_FAIL       505
#define HTTPRESP_TOK_FAIL       "Iuqi"
//#define HTTPCODE_TXT_FAIL       506
//#define HTTPRESP_TXT_FAIL       "KltL"
#define HTTPCODE_CANRX_FAIL     507
#define HTTPRESP_CANRX_FAIL     "jRiW"
#define HTTPCODE_CANRX_DECODE_FAIL 508      // response data has new default token to set (can be 255 = NO_TOKEN)
#define HTTPCODE_CANRX_NOMAC_FAIL 509       // response data has NO_TOKEN because we don't yet have MAC and don't know who's "more master"
#define HTTPCODE_RXDISABLED     510 // sent to HttpClientCallback if "c sync rx off" command (g_bSyncRx = false)
#define HTTPRESP_RXDISABLED     "tyDN"
#define HTTPCODE_TIMESET_FAIL   511
#define HTTPRESP_RXTOKEN_FAIL   "pkrK"
#define HTTPCODE_RXTOKEN_FAIL   512

// after resetting wifi-parameters flash-memory via "c reset wifi", this device will try
// to connect to a router called MyRouter with password MyRouterPass. you can use the USB command-interface
// with "c wifi ssid NEWSSID" and "c wifi pass NEWWIFIPASS" to set different values.
// NOTE: tie GPIO18 low (it has an internal pulldown!) and GPIO19 high for WiFi station-mode (router-mode)
// this is the mDNS service name we search the LAN (local router network) for to find other
// connected ESP32s like us! we can then get their IP addresses via mDNS search and add them to a list.
// we then can exchange data with other units such as (when "c sync on") cycle timing parameter changes.
// data-exchange is done via the AsyncHTTPRequest library, SendHttpReq(), and the HttpClientCallback() method.
// NOTE: tie GPIO18 high and GPIO19 low (it has an internal pulldown!) for WiFi AP-mode (access-point-mode)

void ClearLinkOk(int code);
void InitHttpClientHandlers();
//bool SendHttpFailsafeReq(int idx);
//int SendHttpText(int idx, String sText);
//int SendHttpText(String sIP, String sText);
bool SendHttpCanRxReq(int idx);
bool SendHttpReq(IPAddress ip);
String GetHttpCodeString(int httpCode);

#endif

extern const char HTTP_ASYNCREQ[], HTTP_PARAM_COMMAND[],  HTTP_ASYNCREQ_PARAM_TIMESET[];
//extern const char HTTP_ASYNCTEXTREQ[], HTTP_ASYNCTEXTREQ_PARAM_TEXT[];
extern const char HTTP_ASYNCCANRXREQ[], HTTP_ASYNCCANRXREQ_PARAM_TOK3BITS[];
