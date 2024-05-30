#ifndef PrefsClassH
#define PrefsClassH

#include <Arduino.h>

#define EE_PREFS_NAMESPACE "dts-fc01"
#define EE_SLOTS_NAMESPACE "dts-fc02"
#define EE_WIFI_NAMESPACE  "dts-fc03"

// tokens are uint8_t (0-255) 255 = NO_TOKEN
// uint8_t, range MIN_TOKEN to MAX_TOKEN (should be under the base64 table-length...)
// you can change TOKEN_INIT but I'd advise no greater than 18 or so because the higher the number the greater the processing time to do shifts...
#define TOKEN_INIT 3 // NOTE: to set token use command "c token <token>"
#define MIN_TOKEN  0
#define MAX_TOKEN  (B64_TABLE_SIZE-1) // 63

// 40 = 10dBm, 82 = 20dBm,  0.25dBm steps [40..82]
#define WIFI_MAX_POWER_INIT 40
#define MAX_POWER_MIN 40
#define MAX_POWER_MAX 82

// Flag masks for HTTP communication between other units through a router
// (flag masks 64, 128 available...)(stored in flash as a uint8_t EE_SYNC/SYNC_INIT)
// g_bSyncRx, g_bSyncTx, g_bSyncCycle, g_bSyncToken, g_bSyncTime, g_bSyncEncrypt
#define SYNC_INIT               0 // EE_SYNC_MASK_RX|EE_SYNC_MASK_TX|EE_SYNC_MASK_CYCLE|EE_SYNC_MASK_TOKEN|EE_SYNC_MASK_TIME|EE_SYNC_MASK_ENCRYPT
#define EE_SYNC_MASK_RX         1
#define EE_SYNC_MASK_TX         2
#define EE_SYNC_MASK_CYCLE      4
#define EE_SYNC_MASK_TOKEN      8
#define EE_SYNC_MASK_TIME       16
#define EE_SYNC_MASK_ENCRYPT    32

// .25 sec units (4 ticks per second)
#define PULSE_OFF_MODE_A_INIT 0 // 0=off, 1=on-to-off, 2=off-to-on, 3= both
#define MINWID_A_INIT   (1*4) // when 0, we use only pulse-maxWidth with no variation
#define MAXWID_A_INIT   (4*4) 
#define MINPER_A_INIT   (10*4) // 10 sec
#define MAXPER_A_INIT   (14*4)  // 14 sec

// .25 sec units (4 ticks per second)
#define PULSE_OFF_MODE_B_INIT 0 // 0=off, 1=on-to-off, 2=off-to-on, 3= both
#define MINWID_B_INIT   (1*4) // when 0, we use only pulse-maxWidth with no variation
#define MAXWID_B_INIT   (4*4) 
#define MINPER_B_INIT   (10*4) // 10 sec
#define MAXPER_B_INIT   (14*4)  // 14 sec

#define LABEL_A_INIT            "Outlet A"
#define LABEL_B_INIT            "Outlet B"
#define LABEL_MAXLENGTH         32 // max length of index.html labelTxtA and labelTxtB contenteditable HTML5 fields

#define PERUNITS_INIT           1 // 0= 1/2 sec, 1=sec, 2=min, 3=hrs
#define PERUNITS_MIN            0 // index represents .5 second units for perMax
#define PERUNITS_MAX            3 // index represents 1 hour units for perMax

#define PERMAX_INIT             60 // 60 (sec, min, hrs, Etc.)
#define PERMAX_MIN              1
#define PERMAX_MAX              65535

#define PERIOD_INIT             50 // percent offset of period (0-100)
#define PERIOD_MIN              0 // %
#define PERIOD_MAX              100 // 0 = random mode

#define PHASE_INIT              50 // percent offset of period (0-100)
#define PHASE_MIN               0 // %
#define PHASE_MAX               100 // 0 = random mode

#define DUTY_CYCLE_A_INIT       50 // percent on (0-100)
#define DUTY_CYCLE_B_INIT       50 // percent on (0-100)
#define DUTY_CYCLE_MIN          0 // %
#define DUTY_CYCLE_MAX          100 // 0 = random mode
#define MIN_RAND_PERCENT_DUTY_CYCLE 20 // smallest time-on is 20% of period when in random mode!

#define MIDICHAN_INIT           MIDICHAN_OFF // off
#define MIDINOTE_A_INIT         60 // middle C
#define MIDINOTE_B_INIT         62 // middle D
#define MIDINOTE_ALL            128 // all notes
#define MIDICHAN_ALL            0 // all channels
#define MIDICHAN_OFF            255 // no channels

#define LOCKCOUNT_INIT          -1 // unlocked
#define LOCKPASS_INIT           "****"

#define SSR1_MODE_INIT          SSR_MODE_AUTO // 0 = OFF, 1 = ON, 2 = AUTO
#define SSR2_MODE_INIT          SSR_MODE_AUTO // 0 = OFF, 1 = ON, 2 = AUTO

class PrefsClass{
  private:
    void PutEEbyte(const char* ns, const char* s, byte val);
    void PutEEstring(const char* ns, const char* s, String val);
    String GetEEstring(const char* ns, const char* s, const char* eeDefault);

  public:

    void RestoreDefaultSsidAndPwd();
    void RestoreDefaultApSsidAndPwd();
    void ToggleOldSsidAndPwd();
    bool ErasePreferences();
    bool EraseWiFiPrefs();
    void PutWiFiPrefByte(const char* s, byte val);
    void PutWiFiPrefString(const char* s, String val);
    String GetWiFiPrefString(const char* s, const char* eeDefault);
    void PutPrefByte(const char* s, byte val);
    void PutPrefString(const char* s, String val);
    String GetPrefString(const char* s, const char* eeDefault);
    void PutPrefU16(const char* s, uint16_t val);
    uint8_t GetPrefUChar(const char* s, const char eeDefault);
    
};

void GetPreferences();
void ReadEEprefs();
void GetWiFiPrefs();
void TaskWritePulseFeaturePreferences();

#endif

extern PrefsClass PC;
