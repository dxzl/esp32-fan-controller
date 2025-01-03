#ifndef HttpMsgClassH
#define HttpMsgClassH

#include <Arduino.h>

// seperates the int string of command+flags from the data-string in the CMchangeReq/CMchangeData data field
// general-purpose seperator... also used for the CMcmdReq/CMcmdRsp commands, used in CMto/CMfrom as well...
#define CM_SEP ':'

// below defines are for CMcmdReq/CMcmdRSp used for projecting the Cmd.h commands to remote units
#define CM_BAD_ACK  "leia"
#define CM_GOOD_ACK "keig"

#define HTTP_MSG_TABLE   HTTP_TABLE1 // can also be HTTP_TABLE1 or HTTP_TABLE3
#define HTTP_MSG_TOKEN   5 // Used to encode/decode the data field for individual commands packed into sSendAll of t_indexMdns within MdnsListClass

#define STRIP_NONE   0
#define STRIP_ALL    1
#define STRIP_FIRST  2

// Commands sent via the AsyncHTTPRequest_Generic client library. NOTE: all of these multi-character commands are expected
// to have a base 64 data string following it (NOTE: a command must have at least one data-character following it!)
#define CMmac          0 // mac address last two octets uint16_t
#define CMstat         1 // channel status bits, bit0=channelA, bit1=channelB, 1=on, 0=off
#define CMper          2 // uint8_t percent period
#define CMmaxPer       3 // uint16_t max period
#define CMunits        4 // uint8_t units index
#define CMphase        5 // uint8_t percent phase
#define CMdcA          6 // uint8_t percent duty-cycle A
#define CMdcB          7 // uint8_t percent duty-cycle B
#define CMtxt          8 // send text string (for test purposes only at present!)
#define CMtime         9 // send time
#define CMto           10 // send group of base64 encoded commands to a specific unit via the master
#define CMfrom         11 // repackaged version of CMto with prepended destination IP int-string replaced by the source IP
#define CMchangeData   12 // sets the remote pending default token or mac to g_sChangeData
#define CMchangeSet    13 // sets g16_changeSyncTimer - after down-count we move g_sChangeData to g_defToken (but not saved in flash-memory!)
#define CMchangeReq    14 // sent from non-master to master to start a system-wide change for QueueChange(). Includes a uint32 with the command and flags and a String field separated by CM_SEP
#define CMcmdReq       15 // send "cmd:data" to request CM_ command
#define CMcmdRsp       16 // reply to CMreqCmd "data:key:optional namespace" CMreadEE response
#define CMrestart      17 // reboot with key that matches key sent in CMrebootAck
#define CMcs           18 // checksum uint16_t
#define TOTAL_HTTP_COMMANDS 19

// CMcmdReq request/response each with a base-64 encoded data-field:

// Request: "EEkey:optional namespace" to request an EE_* parameter from a remote unit
// Response: "data:EEkey:optional namespace" the EE_* data requested
#define CM_CMD_READ    0
// Request: "data:EEkey:optional namespace" to write an EE_* parameter to a remote unit
// Response: CM_BAD_ACK/CM_GOOD_ACK
#define CM_CMD_WRITE   1
// Request: "magic string" to request restart (uses genRandMessage(minLength, maxLength))
// Response: "randomly generated key" (next you should manually send CMreboot to each remote with its individual key)
#define CM_CMD_RESTART 2
#define CM_CMD_RESTART_MAGIC "hekksif"
#define CM_CMD_INFO    3

// the special-command ranges are used to select a token-command, remaining-cycle command or
// dummy command randomly to insert as the command followed by its base 10 numeric data (which will
// be base-64 encoded by hnEncNum()). Dummy commands are used to pad out a short command string to make
// it harder to decode if intercepted.  
#define CMSPECIALRANGESMAX 31 // the first 32 chars of the ASCII table, 0-31

// arbitrary... longer it is the more chars that need transmitting/receiving though...
// too short and it defeats the purpose (which is: obfuscation!)
//#define MINSENDSTRLENGTH   17 // getting core 0 crashes...
#define MINSENDSTRLENGTH   10 // try...

// The CMdummyMin to CMdummyMax range of command-bytes is - purely and simply - used
// to pad-out a short HTTP command-string for the puropse of improving overall security.
#define CMdummyMin     1 // don't use 0 - it's a null char in a string
#define CMdummyMax     10

// Remaining period is used when g_bSyncCycle is set and is used to synchronize the main timing
// cycle of the g_bMaster unit to other mDNS-discovered units. Below is the command-range.
// We could use just a one-character command as other commands (above) but it's spread over a
// range to improve overall security...
#define CMremPerMin    11
#define CMremPerMax    17

// The CMreqMacMin to CMreqMacMax range of is for requesting a new mDNs IP to send back the last two
// octets of its MAC address so that we can determine if we are the master or not.
#define CMreqMacMin   18
#define CMreqMacMax   24

// The next-access token is a uint16_t. Its command is randomly chosen from the range below, CMtokenMin to CMtokenMax.
// We could use just a one-character command as other commands (above) but it's spread over a
// range to improve overall security...
#define CMtokenMin    25
#define CMtokenMax    CMSPECIALRANGESMAX

class HttpMsgClass{
  private:
    // feel free to move any of these to "public:" if needed...

    String EncodeChangeData(int iChangeCmd, int iChangeFlags, String sChangeData);
    
    String FormRangeCommand(int cmdMin, int cmdMax, int val);
    String FormRangeCommand(int cmdMin, int cmdMax, String sData);
    String FormCommand(int iCmd, int val);
    String FormCommand(int iCmd, String sData);

    // merges sNew into sSendAll for mDNS entry at idx (call FormCommand() to make sNew!)
    // set iStrIdx to MDNS_STRING_SEND_SPECIFIC into sSendSpecific
    // set iStrIdx to MDNS_STRING_SEND_ALL into sSendAll
    bool AddHttpString(String sNew, int mdnsIdx, int iStrIdx);
    
//    bool AddHttpString(String sNew, IPAddress ip, int iStrIdx);

//    bool AddCommand(int iCmd, int val, IPAddress ip, int iStrIdx);
//    bool AddCommand(int iCmd, String sData, IPAddress ip, int iStrIdx);
//    bool AddCommand(int iCmd, IPAddress ip, int iStrIdx);

//    int AddRangeCommandAll(int cmdMin, int cmdMax);
//    int AddRangeCommandAll(int cmdMin, int cmdMax, int val);
//    int AddRangeCommandAll(int cmdMin, int cmdMax, String sVal);
//    void AddRangeCommand(int cmdMin, int cmdMax, String &sInOut);
//    void AddRangeCommand(int cmdMin, int cmdMax, String sVal, String &sInOut);

//    bool IsHttpCommand(String sCmd);
//    bool IsRangeCmdChar(char c);
//    String ExtractAll(int iCmd, String& sInOut);
//    int FindAndStripCommand(String sCmd, String& sInOut);
//    String GetDataForCommandAtIndex(int idx, String& sIn);
//    String GetDataForCommand(String sCmd, String& sIn);

    int AddHttpStringAll(String sNew);

    uint16_t Sum(String sIn);
    String MergeBase64Params(String sNew, String sOld);

    bool IsBase64Char(char c); // all data is base-64
    bool ParseCommand(int& idx, String& sIn, String& sCmd, String& sData);
    String GetRandData();

    String StripCommandAtIndex(int iIdx, String& sInOut);
    int ProcessMsgCommand(int rxIdx, String& sCmd, String& sData, bool& bCMchangeDataWasReceived);
    void MergeSpecial(int cmdMin, int cmdMax, String& sNew, String& sOld);

    int GetCommandIndex(String& sCmd); // returns the CMxx code given a command-string
    String GetCommandString(int iCmd); // returns the command-string given a CMxx code
    
    // NOTE: '?' is reserved for "unknown command"
    // currently 19 commands
    // NOTE: can be any (CMSPECIALRANGESMAX+1)-255 char except: A-Z, a-z, 0-9, - or _ or ?
    //
    // WARNING: don't use indexOf() to search for commands!!!! for example, if we have two
    //   commands "@" and "@+" - indexOf will find BOTH!!!!
    //
    const char* _HttpCommandTable[TOTAL_HTTP_COMMANDS] =
      {"@/","(&","#!","'.","$:","^*","\"%",">\\","@","(","#","'","$","^","\"",">","/","&","!"};

  public:
  
    int AddCMchangeDataAll(int iChangeCmd, int iChangeFlags, String sChangeData);
    bool AddCMchangeReq(int iChangeCmd, int iChangeFlags, String sChangeData, int mdnsIdx, int iStrIdx);

    void AddCommand(int iCmd, String &sInOut);
    void AddCommand(int iCmd, int val, String &sInOut);
    void AddCommand(int iCmd, String sData, String &sInOut); // add command+data to sInOut

    bool AddCommand(int iCmd, int mdnsIdx, int iStrIdx); // add command only
    int AddCommandAll(int iCmd);

    bool AddCommand(int iCmd, int val, int mdnsIdx, int iStrIdx); // add command + int data
    int AddCommandAll(int iCmd, int val);
    
    bool AddCommand(int iCmd, String sData, int mdnsIdx, int iStrIdx); // add command + string data
    int AddCommandAll(int iCmd, String sData);

    void AddRangeCommand(int cmdMin, int cmdMax, int val, String &sInOut);
    String StripRangeCommand(int cmdMin, int cmdMax, String& sInOut, int iStrip=STRIP_ALL);
    int FindRangeCommand(int cmdMin, int cmdMax, String& sIn);

// these should work ok - not tested - no need for them yet!
//    int FindCommand(String sCmd, String& sIn, String& sDataOut);
//    int FindCommand(String sCmd, String& sIn);
//    int FindCommand(int iCmd, String& sIn, String& sDataOut);
//    int FindCommand(int iCmd, String& sIn);
    
    String StripCommand(int iCmd, String& sInOut, int iStrip=STRIP_ALL);
    String StripCommand(String sCmd, String& sInOut, int iStrip=STRIP_ALL);

    String GetDataForCommandAtIndex(int iIdx, String& sInOut, bool bStrip=false);

    void EncodeChangedParametersForAllIPs();
    int EncodeTxTokenAndChecksum(int idx, String& sInOut, bool bUseDefaultToken=false);
    int DecodeCommands(String& sInOut, int rxIdx);
    int ProcessMsgCommands(String& sInOut, int rxIdx, bool& bCMchangeDataWasReceived);
    String DecodeBase64Commands(int mdnsIdx, int iStrIdx);
    String DecodeBase64Commands(String sIn);
};

#endif

extern HttpMsgClass HMC;
