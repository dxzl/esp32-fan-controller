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

class MdnsListClass{
  private:

    typedef struct{
      IPAddress ip;
      uint16_t mac_last_two_octets;
      int iMin, sendCount; // we store current minute and delete this entry if older than 10 min.
      int rxToken, rxPrevToken, txToken, txPrevToken, txNextToken, saveToken; // NO_TOKEN is 255
      bool bSendTime, bSendOk, bTokOk, bLinkOk, bCanRxInProgress;
      String sSend, sUtil, sRxTxt;
    } t_indexMdns;
    
    std::vector<t_indexMdns> arr;

    uint16_t GetHighestMac();
    bool SetSize(int newSize);
    int DeleteOldestNoLinkMdnsEntry();

  public:
    
    void PrintInfo();
    int GetRxToken(int idx);
    int GetRxPrevToken(int idx);
    void SetRxToken(int idx, int val);
    void SetRxPrevToken(int idx, int val);
    void SetSaveToken(int idx, int val);
    int GetTxToken(int idx);
    int GetTxPrevToken(int idx);
    int GetTxNextToken(int idx);
    int GetSaveToken(int idx);
    void SetTxToken(int idx, int val);
    void SetTxPrevToken(int idx, int val);
    void SetTxNextToken(int idx, int val);
    void SetAllTokens(int idx, int val);
    uint16_t GetMdnsMAClastTwo(int idx);
    void SetMdnsMAClastTwo(int idx, uint16_t val);
    String GetSendStr(int idx);
    void SetSendStr(int idx, String val);
    String GetUtilStr(int idx);
    void SetUtilStr(int idx, String val);
    String GetRxTxtStr(int idx);
    void SetRxTxtStr(int idx, String val);
    bool GetSendTimeFlag(int idx);
    void SetSendTimeFlag(int idx, bool val);
    bool GetCanRxInProgFlag(int idx);
    void SetCanRxInProgFlag(int idx, bool val);
    bool GetSendOkFlag(int idx);
    void SetSendOkFlag(int idx, bool val);
    void SetSendOkFlag(IPAddress ipFind, bool val);
    bool GetTokOkFlag(int idx);
    void SetTokOkFlag(int idx, bool val);
    void SetTokOkFlag(IPAddress ipFind, bool val);
    bool GetLinkOkFlag(int idx);
    void SetLinkOkFlag(int idx, bool val);
    void SetLinkOkFlag(IPAddress ipFind, bool val);
    int GetSendCount(int idx);
    void SetSendCount(int idx, int val);
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
    int AddMdnsIp(String sIP);
    int FindMdnsIp(String sIp);
    int FindMdnsIp(IPAddress ipFind);
    void ClearLinkOkFlagsForExpiredMdnsIps();
    uint16_t GetOurDeviceMacLastTwoOctets();

    bool DelMdnsIp(IPAddress ipDel);
    bool DelMdnsIp(int idx);
    
    bool ClearMdnsSendInfo(IPAddress ipClear);
    bool ClearMdnsSendInfo(int idx);
    
    bool AreWeMaster();
    bool IsAnyMacZero();
    bool AreAllTokOkFlagsSet();
    void ClearAllTokOkFlags();
    void ClearAllLinkOkFlags();
    void SetAllSendTimeFlags();
    void QueryMdnsServiceAsync(const char* host_name);
    //void QueryHostServiceAsync(const char* host_name);
    bool CheckMdnsSearchResult();

    mdns_search_once_t* g_pMdnsSearch = NULL;
};
#endif

extern MdnsListClass IML;
