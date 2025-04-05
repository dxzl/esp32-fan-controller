#ifndef CmdH
#define CmdH

#include <Arduino.h>

// ------------------ USB command-line processing -------------------

// NOTE: by placing "const char" strings in the header files, we don't then need to put in
// an "extern" definition for them (see below). The compiler only sees the definition once because
// of the #ifndef" above.

const char VERSION_STR[] = GPC_VERSION;
const char BOARD_STR[] = GPC_BOARD;

// spaces not allowed in a command/subcommand (to keep parsing simple for command interpreter!)
// spaces ARE allowed in the data-string that follows the command/subcommand. COMMAND_LOCK[] is
// unique in that it allows two data strings that are delineated with quotes (c lock "old passphrase" "new passphrase").

// "c sntp interval 0" turn off SNTP,
// "c sntp tz CST6CDT" central standard time, central daylight savings time
// (leaving off CDT turns off daylight savings time)
const char COMMAND_SNTP[]     = "sntp";
const char SC_SNTP_INTERVAL[] = "interval";
const char SC_SNTP_TIMEZONE[] = "timezone";

// "c mac random, c mac reset, c mac 0xe6, cd:43:a3:6e:b5
const char COMMAND_MAC[]     = "mac";
const char SC_MAC_RANDOM[]   = "rand";
const char SC_MAC_RESET[]    = "reset";

// type just "c relay 1" to print the relay mode and state
const char COMMAND_RELAY[]   = "relay";
const char SC_RELAY_1[]      = "1";
const char SC_RELAY_2[]      = "2";
const char SC_RELAY_3[]      = "3";
const char SC_RELAY_4[]      = "4";
const char SSC_RELAY_ON[]    = "on";
const char SSC_RELAY_OFF[]   = "off";
const char SSC_RELAY_AUTO[]  = "auto";

// set "on" this unit will broadcast messages of random length and content to all other of our devices
const char COMMAND_TEST[]    = "test";
const char SC_TEST_ON[]      = "on";
const char SC_TEST_OFF[]     = "off";

// type just "c label 1" to print the label
const char COMMAND_LABEL[]   = "label";
const char SC_LABEL_1[]      = "1";
const char SC_LABEL_2[]      = "2";
const char SC_LABEL_3[]      = "3";
const char SC_LABEL_4[]      = "4";

// you can type these commands into the HOSTNAME web-edit field and submit the command!

// erase a flash-memory segment...
const char COMMAND_RESET[]   = "reset";
const char SC_RESET_PREFS[]  = "prefs";
const char SC_RESET_SLOTS[]  = "slots";
const char SC_RESET_WIFI[]   = "wifi";

// these represent feature on/off bits related to the custom, secured communication between ESP32
// units via sending commands from the AsyncHTTPRequest_Generic.h library to the remote unit's ESPAsyncWebServer.h
// library
const char COMMAND_SYNC[]    = "sync";
const char SC_SYNC_RX[]      = "rx";
const char SC_SYNC_TX[]      = "tx";
const char SC_SYNC_CYCLE[]   = "cycle";
const char SC_SYNC_TIME[]    = "time";
const char SC_SYNC_ENCRYPT[] = "encrypt";
const char SSC_SYNC_ON[]     = "on";
const char SSC_SYNC_OFF[]    = "off";

// cycle pulse-off/on
// independant timing cycle with period and pulse-width that can vary between min and max values.
// mode for each of channel A and B of OFF, On-to-Off, Off-to-On and Both. Timing units are .25Sec/tick
// Example commands: 
// c pulse a per 100 (sets pulse-period for channel A to 100/4 = 25sec)
// c pulse b pw 20 (sets pulse-width for channel B to 12/4 = 3sec)
// c pulse a mode 0 (turn off pulse-feature for channel A)
const char COMMAND_PULSE[]   = "pulse";
const char SC_PULSE_A[]      = "a";
const char SC_PULSE_B[]      = "b";
const char SC_PULSE_C[]      = "c";
const char SC_PULSE_D[]      = "d";
const char SSC_PULSE_PER[]   = "per";
const char SSC_PULSE_PW[]    = "pw";
const char SSC_PULSE_MODE[]  = "mode";
const char SSSC_PULSE_MIN[]  = "min";
const char SSSC_PULSE_MAX[]  = "max";

// NOTE: timeslot # <new slot> replaces a slot!
const char COMMAND_TIMESLOT[]   = "timeslot";
const char SC_TIMESLOT_ADD[]    = "add";
const char SC_TIMESLOT_DEL[]    = "del";
const char SC_TIMESLOT_DELALL[] = "delall";
const char SC_TIMESLOT_LIST[]   = "list";

const char COMMAND_WIFI[]    = "wifi";
const char SC_WIFI_TOGGLE[]  = "toggle";
const char SC_WIFI_RESTORE[] = "restore";
const char SC_WIFI_AP_RESTORE[] = "aprestore";
const char SC_WIFI_ON[]      = "on";
const char SC_WIFI_OFF[]     = "off";
const char SC_WIFI_SSID[]    = "ssid";
const char SC_WIFI_PWD[]    = "pass";
const char SC_WIFI_APSSID[]  = "apssid";
const char SC_WIFI_APPASS[]  = "appass";
const char SC_WIFI_HOST[]    = "host";

const char COMMAND_DCA[]      = "dca";
const char COMMAND_DCB[]      = "dcb";
const char COMMAND_DCC[]      = "dcc";
const char COMMAND_DCD[]      = "dcd";
const char COMMAND_HELP[]     = "help";
const char COMMAND_INFO[]     = "info";
const char COMMAND_LOCK[]     = "lock";
const char COMMAND_PERIOD[]   = "period";
const char COMMAND_PERMAX[]   = "permax";
const char COMMAND_PERUNITS[] = "perunits";
const char COMMAND_PHASEB[]   = "phaseb";
const char COMMAND_PHASEC[]   = "phasec";
const char COMMAND_PHASED[]   = "phased";
const char COMMAND_TEXT[]     = "text";
const char COMMAND_TIMEDATE[] = "timedate";
const char COMMAND_TOKEN[]    = "token";
const char COMMAND_CIPKEY[]   = "key";
const char COMMAND_UPDATE[]   = "update";
const char COMMAND_UPLOAD[]   = "upload";
const char COMMAND_UNLOCK[]   = "unlock";
const char COMMAND_VERSION[]  = "version";
const char COMMAND_PREFS[]    = "prefs";
const char COMMAND_MAX_POWER[] = "maxpower";
const char COMMAND_READ[]     = "read"; // c read idx|ip (0 is this unit, "all" is all units) key:ns 
const char COMMAND_WRITE[]    = "write"; // c write idx|ip (0 is this unit, "all" is all units) key:ns data to write
const char COMMAND_RESTART[]  = "restart"; // c restart idx|ip (0 is this unit, "all" is all units) <optional key> 
const char COMMAND_REBOOT[]   = "reboot"; // alias for "restart"
const char COMMAND_SYNCHRONIZE[] = "synchronize";

// command-line handlers
void ProcessSerialCommand(String cmd);
String ProcessCommand(AsyncWebServerRequest *request, String &cmd);
String GetSubCommand(String &remCommand);
String ProcessWifiSsidPass(String &valN, String &valP);
//String ProcessApWifiSsidPass(String &valN, String &valP);
int ProcessWifiSsid(String &valN);
int ProcessWifiPass(String &valP);
int ProcessApWifiSsid(String &valN);
int ProcessApWifiPass(String &valP);
String ProcessWiFiHostName(String sName);
void ProcessRandMAC(wifi_interface_t macMode);
void ProcessMAC(wifi_interface_t macMode);
int GetMdnsIdxFromSubCommand(String sToken);
String SetRelayStateFromCommandString(String& sCmd);
String SetWiFiStateFromCommandString(String& sCmd);
String SetTimeSlotFromCommandString(String& sCmd);
String SetSyncStateFromCommandString(String& sCmd);
String SetPulseStateFromCommandString(String& sCmd);
String SetLockUnlockFromCommandString(String& sMainCmd, String& sCmd);
String ReadEEFromCommandString(String& sIp, String& remCommand, int& iMdnsIdx, int& iMdnsCount);
String WriteEEFromCommandString(String& sIp, String& remCommand, int& iMdnsIdx, int& iMdnsCount);
String RestartFromCommandString(String& sIp, String& remCommand, int& iMdnsIdx, int& iMdnsCount);
String GetInfoFromCommandString(String& sIp, String& remCommand, int& iMdnsIdx, int& iMdnsCount);
String SendTextFromCommandString(String& sIp, String& remCommand, int& iMdnsIdx, int& iMdnsCount);
String SetMacFromCommandString(String& remCommand);
//String RemoveQuotes(String sIn);
#endif
