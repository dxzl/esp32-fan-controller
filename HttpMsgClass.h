#ifndef HttpMsgClassH
#define HttpMsgClassH

#include <Arduino.h>

#define HTTP_MSG_TABLE   HTTP_TABLE2 // can also be HTTP_TABLE1 or HTTP_TABLE3
#define HTTP_MSG_TOKEN   0 // Used to encode/decode the data field for individual commands packed into sSend of t_indexMdns within MdnsListClass

// NOTE: '?' is reserved for "unknown command"
// currently 13 characters/commands (NOTE: Use escape backslash for \\ and \")
// NOTE: can be any char from (CMSPECIALRANGESMAX+1) to 255 except: A-Z, a-z, 0-9, - or _ or ? (CANNOT BE A CHARACTER USED IN ENCODE_TABLE0 (B64Class.h)!)
// NOTE: If you change the order of the letters below you will need to reflash the firmware for ALL Esp32s on the network!
// you can change these...
#define HTTP_COMMAND_STRING "/:+>\\@'.*\"^$!"

// Commands sent via the AsyncHTTPRequest_Generic client library. NOTE: all of these single-character commands are expected to have a base 10
// numeric data string following it (NOTE: you can also have a single-letter command with no data following it). You can't follow it by a string at this time, only numbers!
// indexes into _HttpCommandTable
#define CMmac          0 // mac address last two octets uint16_t
#define CMstatA        1 // local A channel status, 1 = on
#define CMstatB        2 // local B channel status, 1 = on
#define CMper          3 // uint8_t percent period
#define CMmaxPer       4 // uint16_t max period
#define CMunits        5 // uint8_t units index
#define CMphase        6 // uint8_t percent phase
#define CMdcA          7 // uint8_t percent duty-cycle A
#define CMdcB          8 // uint8_t percent duty-cycle B
#define CMtoken        9  // sets the remote pending default token, g_pendingDefToken
#define CMsetToken     10 // sets g16_tokenSyncTimer - after down-count we move g_pendingDefToken to g_defToken and store into flash-memory
#define CMtxt          11 // send text string (for test purposes only at present!)
#define CMcs           12 // checksum uint16_t (eep this as last one, it's used below to set HTTP_COMMAND_TABLE_SIZE)
// NOTE: if you add commands, for example CMnewParam, be sure to increase HTTP_COMMAND_TABLE_SIZE and add the new char(s)
// to _HttpCommandTable[] below!
#define HTTP_COMMAND_TABLE_SIZE (CMcs+1) // currently 13

// the special-command ranges are used to select a token-command, remaining-cycle command or
// dummy command randomly to insert as the command followed by its base 10 numeric data (which will
// be base-64 encoded by hnEncNum()). Dummy commands are used to pad out a short command string to make
// it harder to decode if intercepted.  
#define CMSPECIALRANGESMAX 31 // the first 32 chars of the ASCII table, 0-31

// arbitrary... longer it is the more chars that need transmitting/receiving though...
// too short and it defeats the purpose (which is: obfuscation!)
#define MINSENDSTRLENGTH   17

// The CMdummyMin to CMdummyMax range of command-bytes is - purely and simply - used
// to pad-out a short HTTP command-string for the puropse of improving overall security.
#define CMdummyMin     1 // don't use 0 - it's a null char in a string
#define CMdummyMax     10

// Remaining period is used when g_bSyncCycle is set and is used to synchronize the main timing
// cycle of the g_bSyncMaster unit to other mDNS-discovered units. Below is the command-range.
// We could use just a one-character command as other commands (above) but it's spread over a
// range to improve overall security...
#define CMremPerMin    11
#define CMremPerMax    17

// The CMreqMacMin to CMreqMacMax range of is for requesting a new mDNs IP to send back the last two
// octets of its MAC address so that we can determine if we are the master or not.
#define CMreqMacMin     18
#define CMreqMacMax     24

// The next-access token is a uint16_t. Its command is randomly chosen from the range below, CMtokenMin to CMtokenMax.
// We could use just a one-character command as other commands (above) but it's spread over a
// range to improve overall security...
#define CMtokenMin    25
#define CMtokenMax    CMSPECIALRANGESMAX

class HttpMsgClass{
  private:
    // feel free to move any of these to "public:" if needed...

    String FormRangeCommand(int cmdMin, int cmdMax, int val);
    String FormRangeCommand(int cmdMin, int cmdMax, String sData);
    String FormTableCommand(int cmdIdx, int val);
    String FormTableCommand(int cmdIdx, String sData);

    void AddHttpString(String sNew, int mdnsIdx); // merges sNew into sSend for mDNS entry at idx (call FormTableCommand() to make sNew!)
//    void AddHttpString(String sNew, IPAddress ip);

//    void AddTableCommand(int cmdIdx, int val, IPAddress ip);
//    void AddTableCommand(int cmdIdx, String sData, IPAddress ip);
//    void AddTableCommand(int cmdIdx, IPAddress ip);

    void AddTableCommand(int cmdIdx, String &sInOut);
    void AddTableCommand(int cmdIdx, int val, String &sInOut);

//    int AddRangeCommandAll(int cmdMin, int cmdMax);
//    int AddRangeCommandAll(int cmdMin, int cmdMax, int val);
//    int AddRangeCommandAll(int cmdMin, int cmdMax, String sVal);
//    void AddRangeCommand(int cmdMin, int cmdMax, String &sInOut);
//    void AddRangeCommand(int cmdMin, int cmdMax, String sVal, String &sInOut);
    void AddRangeCommand(int cmdMin, int cmdMax, int val, String &sInOut);

    int AddHttpStringAll(String sNew);

    int FindRangeCommand(int cmdMin, int cmdMax, String sIn);
    void MergeSpecial(int cmdMin, int cmdMax, String& sNew, String sOld);
    String Flip(String sIn);
    uint16_t Sum(String sIn);
    bool ParamProcess(char cmd, int iVal);
    bool IsHttpCommand(char cmd);
    String DecodeBase64Command(int cmdIdx, String& sData, bool bStrip=false);
    String GetBase64ParamAtIndex(String sIn, int idx);
    String GetBase64Param(int cmdIdx, String& sData, bool bStrip=false);

    // NOTE: '?' is reserved for "unknown command"
    // currently 12 characters/commands (two with escape backslash!)
    // NOTE: can be any (CMSPECIALRANGESMAX+1)-255 char except: A-Z, a-z, 0-9, - or _ or ?
    const char _HttpCommandTable[HTTP_COMMAND_TABLE_SIZE+1] = HTTP_COMMAND_STRING;

  public:
    char GetTableCommand(int cmdIdx);

    void AddTableCommand(int cmdIdx, String sData, String &sInOut); // add command+data to sInOut

    void AddTableCommand(int cmdIdx, int mdnsIdx); // add command only
    int AddTableCommandAll(int cmdIdx);

    void AddTableCommand(int cmdIdx, int val, int mdnsIdx); // add command + int data
    int AddTableCommandAll(int cmdIdx, int val);
    
    void AddTableCommand(int cmdIdx, String sData, int mdnsIdx); // add command + string data
    int AddTableCommandAll(int cmdIdx, String sData);

    String DecodeAndStripBase64Command(int cmdIdx, String& sData);
    void StripParam(char cmdMin, char cmdMax, String& sData);
    String MergeBase64Params(String sNew, String sOld);
    void EncodeChangedParametersForAllIPs();
    String EncodeTxTokenAndChecksum(int idx, String sSend, bool bUseDefaultToken=false);
    String EncodeTxTokenAndChecksum(int idx, bool bUseDefaultToken=false);
    int DecodeParameters(String& sInOut, int rxIdx, bool& bPendingTokenWasSet, bool bDecodeUsingDefaultRxToken);
    String DecodeAllParams(int mdnsIdx);
    String DecodeAllParams(String sSend);
};

#endif

extern HttpMsgClass HMC;
