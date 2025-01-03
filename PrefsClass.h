#ifndef PrefsClassH
#define PrefsClassH

#include <Arduino.h>

// NOTE: by placing "const char" strings in the header files, we don't then need to put in
// an "extern" definition for them (see below). The compiler only sees the definition once because
// of the #ifndef" above.

// spaces not allowed (to keep parsing simple for command interpreter for "c read/write")
const char EE_PREFS_NAMESPACE[] = "dts-fc01";
const char EE_SLOTS_NAMESPACE[] = "dts-fc02";
const char EE_WIFI_NAMESPACE[]  = "dts-fc03";

// for EE_SLOTS_NAMESPACE
// these are 15 chars max!
const char EE_SLOT_PREFIX[]      = "EE_SLOT_"; // will become EE_SLOT_000

// for EE_PREFS_NAMESPACE
// these are 15 chars max! (do NOT use the CM_SEP character in a key!)
#define EE_PERMAX           0
#define EE_PERUNITS         1
#define EE_PERVAL           2
#define EE_DC_A             3
#define EE_DC_B             4
#define EE_PHASE            5
#define EE_RELAY_A          6
#define EE_RELAY_B          7
#define EE_MIDICHAN         8
#define EE_MIDINOTE_A       9
#define EE_MIDINOTE_B       10
#define EE_SYNC             11
#define EE_WIFI_DIS         12
#define EE_LABEL_A          13
#define EE_LABEL_B          14
#define EE_TOKEN            15
#define EE_CIPKEY           16
#define EE_PULSE_OFF_MODE_A 17
#define EE_PULSE_MINWID_A   18
#define EE_PULSE_MAXWID_A   19
#define EE_PULSE_MINPER_A   20
#define EE_PULSE_MAXPER_A   21
#define EE_PULSE_MINWID_B   22
#define EE_PULSE_OFF_MODE_B 23
#define EE_PULSE_MAXWID_B   24
#define EE_PULSE_MINPER_B   25
#define EE_PULSE_MAXPER_B   26
#define EE_MAX_POWER        27
#define EE_SNTP_TZ          28
#define EE_SNTP_INT         29
#define TOTAL_PREFS         30

// for EE_WIFI_NAMESPACE
// these are 15 chars max! (do NOT use the CM_SEP character in a key!)
#define EE_HOSTNAME         0
#define EE_SSID             1
#define EE_OLDSSID          2
#define EE_PWD              3
#define EE_OLDPWD           4
#define EE_APSSID           5
#define EE_OLDAPSSID        6
#define EE_APPWD            7
#define EE_OLDAPPWD         8
#define EE_MAC              9
#define EE_LOCKCOUNT        10 // 0xff=unlocked, 0x00=locked
#define EE_LOCKPASS         11
#define TOTAL_WIFI          12

class PrefsClass{
  private:
    void ReadEEprefs();

    // U8
    uint8_t GetPrefU8(int idx, const char eeDefault);
    uint8_t getPrefU8(int idx, const char eeDefault);

    uint8_t GetWiFiU8(int idx, const char eeDefault);
    uint8_t getWiFiU8(int idx, const char eeDefault);

    uint8_t GetU8(const char* ns, int idx, const char eeDefault);
    uint8_t getU8(const char* ns, int idx, const char eeDefault);

//    void PutPrefU8(int idx, uint8_t val);
    void putPrefU8(int idx, uint8_t val);

//    void PutWiFiU8(int idx, uint8_t val);
    void putWiFiU8(int idx, uint8_t val);

    void PutU8(const char* ns, int idx, uint8_t val);
    void putU8(const char* ns, int idx, uint8_t val);

    // U16
    uint16_t GetPrefU16(int idx, const char eeDefault);
    uint16_t getPrefU16(int idx, const char eeDefault);

    uint16_t GetWiFiU16(int idx, const char eeDefault);
    uint16_t getWiFiU16(int idx, const char eeDefault);

    uint16_t GetU16(const char* ns, int idx, const char eeDefault);
    uint16_t getU16(const char* ns, int idx, const char eeDefault);

//    void PutPrefU16(int idx, uint16_t val);
    void putPrefU16(int idx, uint16_t val);

    void PutWiFiU16(int idx, uint16_t val);
    void putWiFiU16(int idx, uint16_t val);

    void PutU16(const char* ns, int idx, uint16_t val);
    void putU16(const char* ns, int idx, uint16_t val);

    // string
    String GetPrefString(int idx, const char* eeDefault);
    String getPrefString(int idx, const char* eeDefault);

//    String GetWiFiString(int idx, const char* eeDefault);
    String getWiFiString(int idx, const char* eeDefault);
    
    String GetEEString(const char* ns, int idx, const char* eeDefault);
    String getEEString(const char* ns, int idx, const char* eeDefault);

//    void PutPrefString(int idx, String val);
    void putPrefString(int idx, String val);
    
//    void PutWiFiString(int idx, String val);
    void putWiFiString(int idx, String val);
    
    void PutEEString(const char* ns, int idx, String val);
    void putEEString(const char* ns, int idx, String val);
    
    // table look-up
    const char* getPref(int idx);
    const char* getWiFi(int idx);

    String LookupKey(String sKeyIdx, String sNamespace);
    
    const char* _PrefsTable[TOTAL_PREFS] =
      {"^Js","Dp#","7M@","Lf$","a(n",".@d","72*","Bwo","P@3","t7x",
       "P!g","8d2","K%h","jw0","f#v","9e&","so2","oP^","yc*","RF4",
       "9J$","p4F","@aD","Mt_","Eg+","59F","#V4","J02","O&l","beN"};
      
    const char* _WiFiTable[TOTAL_WIFI] =
      {"EE_HOSTNAME","EE_SSID","EE_OLDSSID","EE_PWD","EE_OLDPWD","EE_APSSID",
       "EE_OLDAPSSID","EE_APPWD","EE_OLDAPPWD","EE_MAC","EE_LOCKCOUNT","EE_LOCKPASS"};
      
  public:
    String GetWiFiString(int idx, const char* eeDefault);
    void PutPrefString(int idx, String val);
    void PutWiFiString(int idx, String val);
    void PutPrefU8(int idx, uint8_t val);
    void PutWiFiU8(int idx, uint8_t val);
    void PutPrefU16(int idx, uint16_t val);
  
    void RestoreDefaultSsidAndPwd();
    void RestoreDefaultApSsidAndPwd();
    void ToggleOldSsidAndPwd();
    bool ErasePreferences();
    bool EraseWiFiPrefs();
    bool SetPrefFromString(String sPref);
    String GetPrefAsString(String sPref);
    void GetPreferences();
    void GetWiFiPrefs();
    void WritePulseFeaturePreferences();
};

#endif

extern PrefsClass PC;
