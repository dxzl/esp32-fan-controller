#ifndef IndexMdnsListH
#define IndexMdnsListH

#include <Arduino.h>
//#include <esp_wifi.h>
//#include <esp_wifi_types.h>
#include <ESPmDNS.h>
#include <WiFi.h>

// FYI: writing search predicate to find element in a struct in array of objects
// https://stackoverflow.com/questions/589985/vectors-structs-and-stdfind
//struct find_id : std::unary_function<monster, bool> {
//    DWORD id;
//    find_id(DWORD id):id(id) { }
//    bool operator()(monster const& m) const {
//        return m.id == id;
//    }
//};
//auto it = std::find_if(bot.monsters.begin(), bot.monsters.end(), 
//         find_id(currentMonster));
#define NO_TOKEN 255 // token is stored as int8_t - 0-63 is valid range for a base-64 table index... 255 is "no token has been set" 
         
#define MDNS_ASYNC_SEARCH_TIMEOUT 1000
#define MDNS_ASYNC_SEARCH_MAX_COUNT 7

#define MDNS_FLAG_CHANGE_OK 0
#define MDNS_FLAG_LINK_OK 1
#define MDNS_FLAG_REQUEST_DEFAULT_TOKEN_CHANGE 2
#define MDNS_FLAG_CANRX_MAIN_DECODE_FAIL 3
#define MDNS_FLAG_CANRX_CALLBACK_DECODE_FAIL 4
#define MDNS_FLAG_SEND_PROCESSING 5
#define FLAG_COUNT_MDNS 6 // set 1 higher than last flag's index!

#define MDNS_TOKEN_RX 0
#define MDNS_TOKEN_RXPREV 1
#define MDNS_TOKEN_RXNEXT 2
#define MDNS_TOKEN_TX 3
#define MDNS_TOKEN_TXPREV 4
#define MDNS_TOKEN_TXNEXT 5
#define MDNS_TOKEN_CANRX_RX 6
#define MDNS_TOKEN_CANRX_TX 7
#define MDNS_TOKEN_SAVE 8
#define TOKEN_COUNT_MDNS 9 // set 1 higher than last token's index!

#define MDNS_STRING_KEY 0
#define MDNS_STRING_RXTXT 1
#define MDNS_STRING_SEND_ALL 2
#define MDNS_STRING_SEND_SPECIFIC 3
#define MDNS_STRING_RXPROCCODE 4
#define STRING_COUNT_MDNS 5 // set 1 higher than last string's index!

class MdnsListClass{
  private:

    typedef struct{
      IPAddress ip;
      uint16_t mac_last_two_octets;
      int iMin, devStatus, sendCount;
      int tokens[TOKEN_COUNT_MDNS];
      bool flags[FLAG_COUNT_MDNS];
      String strings[STRING_COUNT_MDNS];
    } t_indexMdns;
    
    std::vector<t_indexMdns> arr;

    bool SetSize(int newSize);
    int DeleteOldestNoLinkMdnsEntry();

  public:
    
    bool IsGoodIndex(int idx);
    
    void PrintMdnsInfo();
    uint16_t GetMdnsMAClastTwo(int idx);
    void SetMdnsMAClastTwo(int idx, uint16_t val);
    
    String GetStr(int idx, int iString);
    void SetStr(int idx, int iString, String val);
    void InitStrings(int idx);
    
    int GetToken(int idx, int iToken);
    void SetToken(int idx, int iToken, int val);
    void InitTokens(int idx);

    bool GetFlag(int idx, int iFlag);
    void SetFlag(int idx, int iFlag, bool val);
    void SetFlag(IPAddress ipFind, int iFlag, bool val);
    bool IsFlagSetForAnyIP(int iFlag);
    bool IsFlagSetForAllIP(int iFlag);
    void SetFlagForAllIP(int iFlag, bool val);
    void InitFlags(int idx);
    
    int GetSendCount(int idx);
    void SetSendCount(int idx, int val);
    int GetStatus(int idx);
    void SetStatus(int idx, int val);
    int GetMinute(int idx);
    void SetMinute(int idx, int val);
    IPAddress GetIP(int idx);
    void SetIP(int idx, IPAddress val);

    int GetCount(void);
    void Clear(void);
    void XferRxTokens(int idx);
    void XferTxTokens(int idx);
    void RestorePrevTokens(int idx);
    
    int AddMdnsIp(IPAddress ipNew);
    int FindMdnsIp(uint32_t iIp);
    int AddMdnsIp(String sIP);
    int FindMdnsIp(String sIp);
    int FindMdnsIp(IPAddress ipFind);
    void DeleteExpiredMdnsIps();

    bool DelMdnsIp(IPAddress ipDel);
    bool DelMdnsIp(int idx);
    
    bool ClearMdnsSendInfo(IPAddress ipClear);
    bool ClearMdnsSendInfo(int idx);
    
    void QueryMdnsServiceAsync(const char* host_name);
    //void QueryHostServiceAsync(const char* host_name);
    bool CheckMdnsSearchResult();

    mdns_search_once_t* g_pMdnsSearch = NULL;
};
#endif

extern MdnsListClass IML;
