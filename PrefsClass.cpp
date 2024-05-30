// this file PrefsClass.cpp
#include "FanController.h"

PrefsClass PC;

// for EE_SLOTS_NAMESPACE
// these are 15 chars max!
const char EE_SLOT_PREFIX[]    = "EE_SLOT_"; // will become EE_SLOT_000

// for EE_PREFS_NAMESPACE
// these are 15 chars max!
const char EE_PERMAX[]         = "EE_PERMAX";
const char EE_PERUNITS[]       = "EE_PERUNITS";
const char EE_PERVAL[]         = "EE_PERVAL";
const char EE_DC_A[]           = "EE_DC_A";
const char EE_DC_B[]           = "EE_DC_B";
const char EE_PHASE[]          = "EE_PHASE";
const char EE_RELAY_A[]        = "EE_RELAY_A";
const char EE_RELAY_B[]        = "EE_RELAY_B";
const char EE_MIDICHAN[]       = "EE_MIDICHAN";
const char EE_MIDINOTE_A[]     = "EE_MIDINOTE_A";
const char EE_MIDINOTE_B[]     = "EE_MIDINOTE_B";
const char EE_SYNC[]           = "EE_SYNC";
const char EE_WIFI_DIS[]       = "EE_WIFI_DIS";
const char EE_LABEL_A[]        = "EE_LABEL_A";
const char EE_LABEL_B[]        = "EE_LABEL_B";
const char EE_TOKEN[]          = "EE_TOKEN";
const char EE_CIPKEY[]         = "EE_CIPKEY";
const char EE_PULSE_OFF_MODE_A[] = "EE_PULSEOFF_A";
const char EE_PULSE_MINWID_A[] = "EE_MINWID_A";
const char EE_PULSE_MAXWID_A[] = "EE_MAXWID_A";
const char EE_PULSE_MINPER_A[] = "EE_MINPER_A";
const char EE_PULSE_MAXPER_A[] = "EE_MAXPER_A";
const char EE_PULSE_MINWID_B[] = "EE_MINWID_B";
const char EE_PULSE_OFF_MODE_B[] = "EE_PULSEOFF_B";
const char EE_PULSE_MAXWID_B[] = "EE_MAXWID_B";
const char EE_PULSE_MINPER_B[] = "EE_MINPER_B";
const char EE_PULSE_MAXPER_B[] = "EE_MAXPER_B";
const char EE_MAX_POWER[]      = "EE_MAX_POWER";

// for EE_WIFI_NAMESPACE
// these are 15 chars max!
const char EE_HOSTNAME[]       = "EE_HOSTNAME";
const char EE_SSID[]           = "EE_SSID";
const char EE_OLDSSID[]        = "EE_OLDSSID";
const char EE_PWD[]            = "EE_PWD";
const char EE_OLDPWD[]         = "EE_OLDPWD";
const char EE_APSSID[]         = "EE_APSSID";
const char EE_OLDAPSSID[]      = "EE_OLDAPSSID";
const char EE_APPWD[]          = "EE_APPWD";
const char EE_OLDAPPWD[]       = "EE_OLDAPPWD";
const char EE_MAC[]            = "EE_MAC";
const char EE_LOCKCOUNT[]      = "EE_LOCKCOUNT"; // 0xff=unlocked, 0x00=locked
const char EE_LOCKPASS[]       = "EE_LOCKPASS";

// NOT in PrefsClass but related!!!!!!!!!!!!!!
// Flash-vars access
//size_t putString(const char* key, const char* value);
//size_t putString(const char* key, String value);
//size_t putBytes(const char* key, const void* value, size_t len);
//size_t getString(const char* key, char* value, size_t maxLen);
//String getString(const char* key, String defaultValue = String());
//size_t getBytesLength(const char* key);
//size_t getBytes(const char* key, void * buf, size_t maxLen);
//size_t freeEntries();
//EEPROM.write(address, value)
//EEPROM.commit()
//EEPROM.read(address)
void GetPreferences(){
  ReadEEprefs();
  if (g_perVals.perVal < PERIOD_MIN)
    g_perVals.perVal = PERIOD_MIN;
  else if (g_perVals.perVal > PERIOD_MAX)
    g_perVals.perVal = PERIOD_MAX;
  if (g_perVals.dutyCycleA < DUTY_CYCLE_MIN)
    g_perVals.dutyCycleA = DUTY_CYCLE_MIN;
  else if (g_perVals.dutyCycleA > DUTY_CYCLE_MAX)
    g_perVals.dutyCycleA = DUTY_CYCLE_MAX;
  if (g_perVals.dutyCycleB < DUTY_CYCLE_MIN)
    g_perVals.dutyCycleB = DUTY_CYCLE_MIN;
  else if (g_perVals.dutyCycleB > DUTY_CYCLE_MAX)
    g_perVals.dutyCycleB = DUTY_CYCLE_MAX;
  if (g_perVals.phase < PHASE_MIN)
    g_perVals.phase = PHASE_MIN;
  else if (g_perVals.phase > PHASE_MAX)
    g_perVals.phase = PHASE_MAX;
  g32_savePeriod = ComputePeriod(g_perVals.perVal, g_perVals.perMax, g_perVals.perUnits);
  g32_nextPhase = ComputePhase();
  SetSSRMode(GPIO32_SSR_1, g8_nvSsrMode1);
  SetSSRMode(GPIO23_SSR_2, g8_nvSsrMode2);
  g_oldDefToken = g_defToken; // don't want send to remotes!
  CIP.setCiphKey(g_sKey);
}

void ReadEEprefs(){
//https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/src/PF.cpp
  /* Start a namespace EE_PREFS_NAMESPACE
  in Read-Write mode: set second parameter to false
  Note: Namespace name is limited to 15 chars */
  PF.begin(EE_PREFS_NAMESPACE, true); // flag is the read-only flag

  // to remove...
  //PF.remove("TASK_RELAY_A");

  g8_pulseModeA =  PF.getUChar(EE_PULSE_OFF_MODE_A, PULSE_OFF_MODE_A_INIT);
  g8_pulseMinWidthA = PF.getUChar(EE_PULSE_MINWID_A, MINWID_A_INIT);
  g8_pulseMaxWidthA = PF.getUChar(EE_PULSE_MAXWID_A, MAXWID_A_INIT);
  g16_pulseMinPeriodA = PF.getUShort(EE_PULSE_MINPER_A, MINPER_A_INIT); // minPer 0 we use maxPer as the period with no variation
  g16_pulseMaxPeriodA = PF.getUShort(EE_PULSE_MAXPER_A, MAXPER_A_INIT); // maxPer 0 pulse feature is off
  g8_pulseModeB =  PF.getUChar(EE_PULSE_OFF_MODE_B, PULSE_OFF_MODE_B_INIT);
  g8_pulseMinWidthB = PF.getUChar(EE_PULSE_MINWID_B, MINWID_B_INIT);
  g8_pulseMaxWidthB = PF.getUChar(EE_PULSE_MAXWID_B, MAXWID_B_INIT);
  g16_pulseMinPeriodB = PF.getUShort(EE_PULSE_MINPER_B, MINPER_B_INIT); // minPer 0 we use maxPer as the period with no variation
  g16_pulseMaxPeriodB = PF.getUShort(EE_PULSE_MAXPER_B, MAXPER_B_INIT); // maxPer 0 pulse feature is off

  g_perVals.perVal = PF.getUChar(EE_PERVAL, PERIOD_INIT);
  g_perVals.perUnits = PF.getUChar(EE_PERUNITS, PERUNITS_INIT);
  g_perVals.perMax = PF.getUShort(EE_PERMAX, PERMAX_INIT);

  g_perVals.dutyCycleA = PF.getUChar(EE_DC_A, DUTY_CYCLE_A_INIT);
  g_perVals.dutyCycleB = PF.getUChar(EE_DC_B, DUTY_CYCLE_B_INIT);
  g_perVals.phase = PF.getUChar(EE_PHASE, PHASE_INIT);

  byte temp = PF.getUChar(EE_SYNC, SYNC_INIT);
  g_bSyncRx = temp & EE_SYNC_MASK_RX;  
  g_bSyncTx = temp & EE_SYNC_MASK_TX;
  g_bSyncCycle = temp & EE_SYNC_MASK_CYCLE;  
  g_bSyncToken = temp & EE_SYNC_MASK_TOKEN;  
  g_bSyncTime = temp & EE_SYNC_MASK_TIME;  
  g_bSyncEncrypt = temp & EE_SYNC_MASK_ENCRYPT;

  g8_nvSsrMode1 = PF.getUChar(EE_RELAY_A, SSR1_MODE_INIT); // 0 = OFF, 1 = ON, 2 = AUTO
  g8_nvSsrMode2 = PF.getUChar(EE_RELAY_B, SSR2_MODE_INIT); // 0 = OFF, 1 = ON, 2 = AUTO

  g8_midiChan = PF.getUChar(EE_MIDICHAN, MIDICHAN_INIT);

  g8_midiNoteA = PF.getUChar(EE_MIDINOTE_A, MIDINOTE_A_INIT);
  g8_midiNoteB = PF.getUChar(EE_MIDINOTE_B, MIDINOTE_B_INIT);

  g8_maxPower = PF.getUChar(EE_MAX_POWER, WIFI_MAX_POWER_INIT);

#if FORCE_DEF_CIPHER_KEY
  g_defToken = TOKEN_INIT;
#else
  g_defToken = (int)PF.getUChar(EE_TOKEN, TOKEN_INIT);
#endif

  g_bWiFiDisabled = PF.getUChar(EE_WIFI_DIS, 0) == 1 ? true : false;

  g_sLabelA = PF.getString(EE_LABEL_A, LABEL_A_INIT);
  g_sLabelB = PF.getString(EE_LABEL_B, LABEL_B_INIT);

#if FORCE_DEF_CIPHER_KEY
  g_sKey = HTTP_CIPKEY_INIT;
  prtln("WARNING!!! FORCE_DEF_CIPHER_KEY is set true!!!");
#else
  g_sKey = PF.getString(EE_CIPKEY, HTTP_CIPKEY_INIT);
#endif
  // Close the Preferences
  PF.end();
}

void GetWiFiPrefs(){
  PF.begin(EE_WIFI_NAMESPACE, true); // flag is the read-only flag
    g8_lockCount = PF.getUChar(EE_LOCKCOUNT, LOCKCOUNT_INIT);
    g_sHostName = PF.getString(EE_HOSTNAME, DEF_HOSTNAME);
    g_sSSID = PF.getString(EE_SSID, DEF_SSID);
    g_sApSSID = PF.getString(EE_APSSID, DEF_AP_SSID);
    g_sMac = PF.getString(EE_MAC, DEF_MAC);
  PF.end();
}

// called via QueueTask(TASK_WRITE_PULSE_EE_VALUES) in Cmd.cpp when user types a
// command such as "c pulse a per min 30" or "c pulse b mode 3"
// Usage: c pulse a|b per|pw <time in .25sec units> OR c pulse a|b mode <0=off,1=pulse-off,2=pulse-on,3=both>
void TaskWritePulseFeaturePreferences(){
  PF.begin(EE_PREFS_NAMESPACE);
  if (g8_pulseModeA != PF.getUChar(EE_PULSE_OFF_MODE_A, PULSE_OFF_MODE_A_INIT)){
    PF.putUChar(EE_PULSE_OFF_MODE_A, g8_pulseModeA);
    prtln("pulse-off mode A: " + String(g8_pulseModeA));
  }      
  if (g8_pulseMinWidthA != PF.getUChar(EE_PULSE_MINWID_A, MINWID_A_INIT)){
    PF.putUChar(EE_PULSE_MINWID_A, g8_pulseMinWidthA);
    prtln("pulse-off min width A: " + String(g8_pulseMinWidthA));
  }
  if (g8_pulseMaxWidthA != PF.getUChar(EE_PULSE_MAXWID_A, MAXWID_A_INIT)){
    PF.putUChar(EE_PULSE_MAXWID_A, g8_pulseMaxWidthA);
    prtln("pulse-off max width A: " + String(g8_pulseMaxWidthA));
  }
  if (g16_pulseMinPeriodA != PF.getUShort(EE_PULSE_MINPER_A, MINPER_A_INIT)){
    PF.putUShort(EE_PULSE_MINPER_A, g16_pulseMinPeriodA);
    prtln("pulse-off min period A: " + String(g16_pulseMinPeriodA));
  }
  if (g16_pulseMaxPeriodA != PF.getUShort(EE_PULSE_MAXPER_A, MAXPER_A_INIT)){
    PF.putUShort(EE_PULSE_MAXPER_A, g16_pulseMaxPeriodA);
    prtln("pulse-off max period A: " + String(g16_pulseMaxPeriodA));
  }
  
  if (g8_pulseModeB != PF.getUChar(EE_PULSE_OFF_MODE_B, PULSE_OFF_MODE_B_INIT)){
    PF.putUChar(EE_PULSE_OFF_MODE_B, g8_pulseModeB);
    prtln("pulse-off mode B: " + String(g8_pulseModeB));
  }      
  if (g8_pulseMinWidthB != PF.getUChar(EE_PULSE_MINWID_B, MINWID_B_INIT)){
    PF.putUChar(EE_PULSE_MINWID_B, g8_pulseMinWidthB);
    prtln("pulse-off min width B: " + String(g8_pulseMinWidthB));
  }
  if (g8_pulseMaxWidthB != PF.getUChar(EE_PULSE_MAXWID_B, MAXWID_B_INIT)){
    PF.putUChar(EE_PULSE_MAXWID_B, g8_pulseMaxWidthB);
    prtln("pulse-off max width B: " + String(g8_pulseMaxWidthB));
  }
  if (g16_pulseMinPeriodB != PF.getUShort(EE_PULSE_MINPER_B, MINPER_B_INIT)){
    PF.putUShort(EE_PULSE_MINPER_B, g16_pulseMinPeriodB);
    prtln("pulse-off min period B: " + String(g16_pulseMinPeriodB));
  }
  if (g16_pulseMaxPeriodB != PF.getUShort(EE_PULSE_MAXPER_B, MAXPER_B_INIT)){
    PF.putUShort(EE_PULSE_MAXPER_B, g16_pulseMaxPeriodB);
    prtln("pulse-off max period B: " + String(g16_pulseMaxPeriodB));
  }
  PF.end();
}

bool PrefsClass::EraseWiFiPrefs(){
  bool bRet = false;
  prtln("Erasing wifi PF...");

  // clear prefs namespace
  PF.begin(EE_WIFI_NAMESPACE);
  bRet = PF.clear();
  PF.end();
  return bRet;
}

bool PrefsClass::ErasePreferences(){
  bool bRet = false;
  prtln("Erasing PF...");

  // clear prefs namespace
  PF.begin(EE_PREFS_NAMESPACE);
  bRet = PF.clear();
  PF.end();

  return bRet;
}

void PrefsClass::PutWiFiPrefByte(const char* s, byte val){
  PutEEbyte(EE_WIFI_NAMESPACE, s, val);
}

void PrefsClass::PutPrefByte(const char* s, byte val){
  PutEEbyte(EE_PREFS_NAMESPACE, s, val);
}

void PrefsClass::PutEEbyte(const char* ns, const char* s, byte val){
  PF.begin(ns);
  PF.putUChar(s, val);
  PF.end();

  prtln(String(s) + ":" + String(val));
  yield();
}

uint8_t PrefsClass::GetPrefUChar(const char* s, const char eeDefault){
  PF.begin(EE_PREFS_NAMESPACE, true);
  uint8_t tmp = PF.getUChar(s, eeDefault);
  PF.end();

  yield();
  return tmp;
}

String PrefsClass::GetWiFiPrefString(const char* s, const char* eeDefault){
  return GetEEstring(EE_WIFI_NAMESPACE, s, eeDefault);
}

String PrefsClass::GetPrefString(const char* s, const char* eeDefault){
  return GetEEstring(EE_PREFS_NAMESPACE, s, eeDefault);
}

String PrefsClass::GetEEstring(const char* ns, const char* s, const char* eeDefault){
  PF.begin(ns, true);
  String sTemp = PF.getString(s, eeDefault);
  PF.end();

  yield();
  return sTemp;
}

void PrefsClass::PutWiFiPrefString(const char* s, String val){
  PutEEstring(EE_WIFI_NAMESPACE, s, val);
}

void PrefsClass::PutPrefString(const char* s, String val){
  PutEEstring(EE_PREFS_NAMESPACE, s, val);
}

void PrefsClass::PutEEstring(const char* ns, const char* s, String val){
  PF.begin(ns);
  PF.putString(s, val);
  PF.end();

  prtln(String(s) + ":" + String(val));
  yield();
}

void PrefsClass::PutPrefU16(const char* s, uint16_t val){
  PF.begin(EE_PREFS_NAMESPACE);
  PF.putUShort(s, val);
  PF.end();

  prtln(String(s) + ":" + String(val));
  yield();
}

void PrefsClass::ToggleOldSsidAndPwd(){
  PF.begin(EE_WIFI_NAMESPACE, true); // flag is the read-only flag
  String sOld = PF.getString(EE_OLDPWD, DEF_PWD);
  String sNew = PF.getString(EE_PWD, DEF_PWD);
  PF.putString(EE_OLDPWD, sNew);
  PF.putString(EE_PWD, sOld);
  sOld = PF.getString(EE_OLDSSID, DEF_SSID);
  sNew = PF.getString(EE_SSID, DEF_SSID);
  PF.putString(EE_OLDSSID, sNew);
  PF.putString(EE_SSID, sOld);
  PF.end();
  g_sSSID = sOld;
}

void PrefsClass::RestoreDefaultSsidAndPwd(){
  PF.begin(EE_WIFI_NAMESPACE, true); // flag is the read-only flag
  PF.putString(EE_OLDPWD, DEF_PWD);
  PF.putString(EE_PWD, DEF_PWD);
  PF.putString(EE_OLDSSID, DEF_SSID);
  PF.putString(EE_SSID, DEF_SSID);
  PF.end();
  g_sSSID = DEF_SSID;
}

void PrefsClass::RestoreDefaultApSsidAndPwd(){
  PF.begin(EE_WIFI_NAMESPACE, true); // flag is the read-only flag
  PF.putString(EE_OLDAPPWD, DEF_AP_PWD);
  PF.putString(EE_APPWD, DEF_AP_PWD);
  PF.putString(EE_OLDAPSSID, DEF_AP_SSID);
  PF.putString(EE_APSSID, DEF_AP_SSID);
  PF.end();
  g_sApSSID = DEF_AP_SSID;
}
