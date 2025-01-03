// this file PrefsClass.cpp
#include "FanController.h"

PrefsClass PC;

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
void PrefsClass::GetPreferences(){
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
  SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
  SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
  CIP.setCiphKey(g_sKey);
}

void PrefsClass::ReadEEprefs(){
  // https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/src/PF.cpp
  // Start a namespace EE_PREFS_NAMESPACE
  // in Read-Write mode: set second parameter to false
  // Note: Namespace name is limited to 15 chars
  // PF.begin(EE_PREFS_NAMESPACE, true); // read only
  //
  // Note: If the key has never been written, its default value is returned!
  // 
  // to remove...
  //PF.remove("TASK_RELAY_A");
  
  PF.begin(EE_PREFS_NAMESPACE);
  
  g8_pulseModeA = getPrefU8(EE_PULSE_OFF_MODE_A, PULSE_OFF_MODE_A_INIT);
  g8_pulseMinWidthA = getPrefU8(EE_PULSE_MINWID_A, MINWID_A_INIT);
  g8_pulseMaxWidthA = getPrefU8(EE_PULSE_MAXWID_A, MAXWID_A_INIT);
  g16_pulseMinPeriodA = getPrefU16(EE_PULSE_MINPER_A, MINPER_A_INIT); // minPer 0 we use maxPer as the period with no variation
  g16_pulseMaxPeriodA = getPrefU16(EE_PULSE_MAXPER_A, MAXPER_A_INIT); // maxPer 0 pulse feature is off
  g8_pulseModeB =  getPrefU8(EE_PULSE_OFF_MODE_B, PULSE_OFF_MODE_B_INIT);
  g8_pulseMinWidthB = getPrefU8(EE_PULSE_MINWID_B, MINWID_B_INIT);
  g8_pulseMaxWidthB = getPrefU8(EE_PULSE_MAXWID_B, MAXWID_B_INIT);
  g16_pulseMinPeriodB = getPrefU16(EE_PULSE_MINPER_B, MINPER_B_INIT); // minPer 0 we use maxPer as the period with no variation
  g16_pulseMaxPeriodB = getPrefU16(EE_PULSE_MAXPER_B, MAXPER_B_INIT); // maxPer 0 pulse feature is off

  g_perVals.perVal = getPrefU8(EE_PERVAL, PERIOD_INIT);
  g_perVals.perUnits = getPrefU8(EE_PERUNITS, PERUNITS_INIT);
  g_perVals.perMax = getPrefU16(EE_PERMAX, PERMAX_INIT);

  g_perVals.dutyCycleA = getPrefU8(EE_DC_A, DUTY_CYCLE_A_INIT);
  g_perVals.dutyCycleB = getPrefU8(EE_DC_B, DUTY_CYCLE_B_INIT);
  g_perVals.phase = getPrefU8(EE_PHASE, PHASE_INIT);

  byte temp = getPrefU8(EE_SYNC, SYNC_INIT);
  g_bSyncRx = temp & EE_SYNC_MASK_RX;  
  g_bSyncTx = temp & EE_SYNC_MASK_TX;
  g_bSyncCycle = temp & EE_SYNC_MASK_CYCLE;
  g_bSyncTime = temp & EE_SYNC_MASK_TIME;  
  g_bSyncEncrypt = temp & EE_SYNC_MASK_ENCRYPT;

  g8_ssr1ModeFromWeb = getPrefU8(EE_RELAY_A, SSR1_MODE_INIT); // 0 = OFF, 1 = ON, 2 = AUTO
  g8_ssr2ModeFromWeb = getPrefU8(EE_RELAY_B, SSR2_MODE_INIT); // 0 = OFF, 1 = ON, 2 = AUTO

  g8_midiChan = getPrefU8(EE_MIDICHAN, MIDICHAN_INIT);

  g8_midiNoteA = getPrefU8(EE_MIDINOTE_A, MIDINOTE_A_INIT);
  g8_midiNoteB = getPrefU8(EE_MIDINOTE_B, MIDINOTE_B_INIT);

  g8_maxPower = getPrefU8(EE_MAX_POWER, WIFI_MAX_POWER_INIT);

  g_bWiFiDisabled = getPrefU8(EE_WIFI_DIS, 0) == 1 ? true : false;

  g_sLabelA = getPrefString(EE_LABEL_A, LABEL_A_INIT);
  g_sLabelB = getPrefString(EE_LABEL_B, LABEL_B_INIT);

  g_sTimezone = getPrefString(EE_SNTP_TZ, SNTP_TZ_INIT);
  g16_SNTPinterval = getPrefU16(EE_SNTP_INT, SNTP_INT_INIT);

#if FORCE_DEF_CIPHER_KEY
  g_defToken = TOKEN_INIT;
  g_sKey = HTTP_CIPKEY_INIT;
  prtln("WARNING!!! FORCE_DEF_CIPHER_KEY is set true!!!");
#else
  g_defToken = (int)getPrefU8(EE_TOKEN, TOKEN_INIT);
  g_sKey = getPrefString(EE_CIPKEY, HTTP_CIPKEY_INIT);
#endif
  g_origDefToken = g_defToken;
  
  // Close the Preferences
  PF.end();
}

void PrefsClass::GetWiFiPrefs(){
  PF.begin(EE_WIFI_NAMESPACE, true); // flag is the read-only flag
    g8_lockCount = getWiFiU8(EE_LOCKCOUNT, LOCKCOUNT_INIT);
    g_sHostName = getWiFiString(EE_HOSTNAME, DEF_HOSTNAME);
    g_sSSID = getWiFiString(EE_SSID, DEF_SSID);
    g_sApSSID = getWiFiString(EE_APSSID, DEF_AP_SSID);
    g_sMac = getWiFiString(EE_MAC, DEF_MAC);
  PF.end();
}

// called via TSK.QueueTask(TASK_WRITE_PULSE_EE_VALUES) in Cmd.cpp when user types a
// command such as "c pulse a per min 30" or "c pulse b mode 3"
// Usage: c pulse a|b per|pw <time in .25sec units> OR c pulse a|b mode <0=off,1=pulse-off,2=pulse-on,3=both>
void PrefsClass::WritePulseFeaturePreferences(){
  PF.begin(EE_PREFS_NAMESPACE);
  if (g8_pulseModeA != getPrefU8(EE_PULSE_OFF_MODE_A, PULSE_OFF_MODE_A_INIT)){
    putPrefU8(EE_PULSE_OFF_MODE_A, g8_pulseModeA);
    prtln("pulse-off mode A: " + String(g8_pulseModeA));
  }      
  if (g8_pulseMinWidthA != getPrefU8(EE_PULSE_MINWID_A, MINWID_A_INIT)){
    putPrefU8(EE_PULSE_MINWID_A, g8_pulseMinWidthA);
    prtln("pulse-off min width A: " + String(g8_pulseMinWidthA));
  }
  if (g8_pulseMaxWidthA != getPrefU8(EE_PULSE_MAXWID_A, MAXWID_A_INIT)){
    putPrefU8(EE_PULSE_MAXWID_A, g8_pulseMaxWidthA);
    prtln("pulse-off max width A: " + String(g8_pulseMaxWidthA));
  }
  if (g16_pulseMinPeriodA != getPrefU16(EE_PULSE_MINPER_A, MINPER_A_INIT)){
    putPrefU16(EE_PULSE_MINPER_A, g16_pulseMinPeriodA);
    prtln("pulse-off min period A: " + String(g16_pulseMinPeriodA));
  }
  if (g16_pulseMaxPeriodA != getPrefU16(EE_PULSE_MAXPER_A, MAXPER_A_INIT)){
    putPrefU16(EE_PULSE_MAXPER_A, g16_pulseMaxPeriodA);
    prtln("pulse-off max period A: " + String(g16_pulseMaxPeriodA));
  }
  
  if (g8_pulseModeB != getPrefU8(EE_PULSE_OFF_MODE_B, PULSE_OFF_MODE_B_INIT)){
    putPrefU8(EE_PULSE_OFF_MODE_B, g8_pulseModeB);
    prtln("pulse-off mode B: " + String(g8_pulseModeB));
  }      
  if (g8_pulseMinWidthB != getPrefU8(EE_PULSE_MINWID_B, MINWID_B_INIT)){
    putPrefU8(EE_PULSE_MINWID_B, g8_pulseMinWidthB);
    prtln("pulse-off min width B: " + String(g8_pulseMinWidthB));
  }
  if (g8_pulseMaxWidthB != getPrefU8(EE_PULSE_MAXWID_B, MAXWID_B_INIT)){
    putPrefU8(EE_PULSE_MAXWID_B, g8_pulseMaxWidthB);
    prtln("pulse-off max width B: " + String(g8_pulseMaxWidthB));
  }
  if (g16_pulseMinPeriodB != getPrefU16(EE_PULSE_MINPER_B, MINPER_B_INIT)){
    putPrefU16(EE_PULSE_MINPER_B, g16_pulseMinPeriodB);
    prtln("pulse-off min period B: " + String(g16_pulseMinPeriodB));
  }
  if (g16_pulseMaxPeriodB != getPrefU16(EE_PULSE_MAXPER_B, MAXPER_B_INIT)){
    putPrefU16(EE_PULSE_MAXPER_B, g16_pulseMaxPeriodB);
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

// U8
uint8_t PrefsClass::GetPrefU8(int idx, const char eeDefault){
  return GetU8(EE_PREFS_NAMESPACE, idx, eeDefault);
}

uint8_t PrefsClass::getPrefU8(int idx, const char eeDefault){
  return getU8(EE_PREFS_NAMESPACE, idx, eeDefault);
}

uint8_t PrefsClass::GetWiFiU8(int idx, const char eeDefault){
  return GetU8(EE_WIFI_NAMESPACE, idx, eeDefault);
}

uint8_t PrefsClass::getWiFiU8(int idx, const char eeDefault){
  return getU8(EE_WIFI_NAMESPACE, idx, eeDefault);
}

uint8_t PrefsClass::GetU8(const char* ns, int idx, const char eeDefault){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  uint8_t tmp;
  if (s){
    PF.begin(ns, true);
    tmp = PF.getUChar(s, eeDefault);
    PF.end();
  }
  else
    tmp = 0;

  return tmp;
}

uint8_t PrefsClass::getU8(const char* ns, int idx, const char eeDefault){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  if (s)
    return PF.getUChar(s, eeDefault);
  return 0;
}

void PrefsClass::PutPrefU8(int idx, uint8_t val){
  PutU8(EE_PREFS_NAMESPACE, idx, val);
}

void PrefsClass::putPrefU8(int idx, uint8_t val){
  putU8(EE_PREFS_NAMESPACE, idx, val);
}

void PrefsClass::PutWiFiU8(int idx, uint8_t val){
  PutU8(EE_WIFI_NAMESPACE, idx, val);
}

void PrefsClass::putWiFiU8(int idx, uint8_t val){
  putU8(EE_WIFI_NAMESPACE, idx, val);
}

void PrefsClass::PutU8(const char* ns, int idx, uint8_t val){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  if (s){
    PF.begin(ns);
    PF.putUChar(s, val);
    PF.end();
    prtln(String(ns) + ':' + String(s) + '=' + String(val));
  }
}

void PrefsClass::putU8(const char* ns, int idx, uint8_t val){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  if (s)
    PF.putUChar(s, val);
}

// U16
uint16_t PrefsClass::GetPrefU16(int idx, const char eeDefault){
  return GetU16(EE_PREFS_NAMESPACE, idx, eeDefault);
}

uint16_t PrefsClass::getPrefU16(int idx, const char eeDefault){
  return getU16(EE_PREFS_NAMESPACE, idx, eeDefault);
}

uint16_t PrefsClass::GetWiFiU16(int idx, const char eeDefault){
  return GetU16(EE_WIFI_NAMESPACE, idx, eeDefault);
}

uint16_t PrefsClass::getWiFiU16(int idx, const char eeDefault){
  return getU16(EE_WIFI_NAMESPACE, idx, eeDefault);
}

uint16_t PrefsClass::GetU16(const char* ns, int idx, const char eeDefault){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  uint16_t tmp;
  if (s){
    PF.begin(ns, true);
    tmp = PF.getUShort(s, eeDefault);
    PF.end();
  }
  else
    tmp = 0;

  return tmp;
}

uint16_t PrefsClass::getU16(const char* ns, int idx, const char eeDefault){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  if (s)
    return PF.getUShort(s, eeDefault);
  return 0;
}

void PrefsClass::PutPrefU16(int idx, uint16_t val){
  PutU16(EE_PREFS_NAMESPACE, idx, val);
}

void PrefsClass::putPrefU16(int idx, uint16_t val){
  putU16(EE_PREFS_NAMESPACE, idx, val);
}

void PrefsClass::PutWiFiU16(int idx, uint16_t val){
  PutU16(EE_WIFI_NAMESPACE, idx, val);
}

void PrefsClass::putWiFiU16(int idx, uint16_t val){
  putU16(EE_WIFI_NAMESPACE, idx, val);
}

void PrefsClass::PutU16(const char* ns, int idx, uint16_t val){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  if (s){
    PF.begin(ns);
    PF.putUShort(s, val);
    PF.end();
    prtln(String(ns) + ':' + String(s) + '=' + String(val));
  }
}

void PrefsClass::putU16(const char* ns, int idx, uint16_t val){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  if (s)
    PF.putUShort(s, val);
}

// String
String PrefsClass::GetPrefString(int idx, const char* eeDefault){
  return GetEEString(EE_PREFS_NAMESPACE, idx, eeDefault);
}

String PrefsClass::getPrefString(int idx, const char* eeDefault){
  return getEEString(EE_PREFS_NAMESPACE, idx, eeDefault);
}

String PrefsClass::GetWiFiString(int idx, const char* eeDefault){
  return GetEEString(EE_WIFI_NAMESPACE, idx, eeDefault);
}

String PrefsClass::getWiFiString(int idx, const char* eeDefault){
  return getEEString(EE_WIFI_NAMESPACE, idx, eeDefault);
}

String PrefsClass::GetEEString(const char* ns, int idx, const char* eeDefault){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  String tmp;
  if (s){
    PF.begin(ns, true);
    tmp = PF.getString(s, eeDefault);
    PF.end();
  }
  else
    tmp = "";

  return tmp;
}

String PrefsClass::getEEString(const char* ns, int idx, const char* eeDefault){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  if (s)
    return PF.getString(s, eeDefault);
  return "";
}

void PrefsClass::PutPrefString(int idx, String val){
  PutEEString(EE_PREFS_NAMESPACE, idx, val);
}

void PrefsClass::putPrefString(int idx, String val){
  putEEString(EE_PREFS_NAMESPACE, idx, val);
}

void PrefsClass::PutWiFiString(int idx, String val){
  PutEEString(EE_WIFI_NAMESPACE, idx, val);
}

void PrefsClass::putWiFiString(int idx, String val){
  putEEString(EE_WIFI_NAMESPACE, idx, val);
}

void PrefsClass::PutEEString(const char* ns, int idx, String val){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  if (s){
    PF.begin(ns);
    PF.putString(s, val);
    PF.end();
    prtln(String(ns) + ':' + String(s) + '=' + String(val));
  }
}

void PrefsClass::putEEString(const char* ns, int idx, String val){
  const char* s = (ns == EE_WIFI_NAMESPACE) ? getWiFi(idx) : getPref(idx);
  if (s)
    PF.putString(s, val);
}

// low-level
const char* PrefsClass::getPref(int idx){
  if (idx >= 0 && idx < TOTAL_PREFS)
    return _PrefsTable[idx];
  return NULL;
}

const char* PrefsClass::getWiFi(int idx){
  if (idx >= 0 && idx < TOTAL_WIFI)
    return _WiFiTable[idx];
  return NULL;
}

// sPref = "data:key:optional namespace" Note: data can be "" with just the leading ':'
// and data can be all spaces.
bool PrefsClass::SetPrefFromString(String sPref){
  String sData, sNamespace, sKeyIdx;
  int idx = sPref.indexOf(CM_SEP);
  if (idx >= 0){
    if (idx > 0)
      sData = B64C.hnDecodeStr(sPref.substring(0, idx));
    sPref = sPref.substring(idx+1); // key:ns
    idx = sPref.indexOf(CM_SEP);
    if (idx >= 1){
      sKeyIdx = sPref.substring(0, idx);
      sNamespace = sPref.substring(idx+1);
    }
    else{
      sKeyIdx = sPref;
      sNamespace = String(EE_PREFS_NAMESPACE);
    }
  }
  else
    return false;
  
  bool bRet = false;
  
  // NOTE: this is invoked via the example command:
  //   "c IpAddress write 42:dts-fc02 11/4/2024,12:59:59pm,AB:auto,r=inf,e=1,t=min" (trigger then repeat every minute forever)
  // If 0-index based slot 42 is NOT empty, its data will be replaced with the new data. Using the slot index
  // MAX_TIME_SLOTS (or greater) will cause a new timeslot to be added at the first empty slot.
  //
  // Example:
  //  "c IpAddress write 42:dts-fc02 1/12/2027,23:00:00,A:off" (replaces timeslot index 42)
  //  "c IpAddress write add:dts-fc02 1/12/2027,3:00:00am,A:off" (adds a new timeslot to a remote)
  //  "c IpAddress write 0:dts-fc02 delete" (deletes timeslot 0-99)
  //  "c IpAddress write all:dts-fc02 delete" (deletes all timeslots)
  if (sNamespace == String(EE_SLOTS_NAMESPACE)){
    int iKey = -1;
    if (alldigits(sKeyIdx))
      iKey = sKeyIdx.toInt();
    if (sData.length() == 6){
      sData.toLowerCase();
      if (sData == "delete"){
        if (iKey == -1){
          sKeyIdx.toLowerCase();
          if (sKeyIdx == "all")
            if (TSC.EraseTimeSlots())
              bRet = true;
        }
        else if (iKey >= 0 && iKey < MAX_TIME_SLOTS)
          if (TSC.DeleteTimeSlot(iKey))
            bRet = true;
      }
    }
    else{
      t_event t = {0};
      if (TSC.StringToTimeSlot(sData, t)){
        if (iKey >= 0){
          if (iKey < g_slotCount)
            if (TSC.PutTimeSlot(iKey, t)) // replace existing slot
              bRet = true;
        }
        else{
          sKeyIdx.toLowerCase();
          if (sKeyIdx == "add")
            if (TSC.AddTimeSlot(t))
              bRet = true;
        }
      }
    }
  }
  else{
    String sKey = LookupKey(sKeyIdx, sNamespace);
  
    if (sKey.isEmpty())
      return false;
  
    PF.begin(sNamespace.c_str());
    if (PF.isKey(sKey.c_str())){
      PreferenceType pType = PF.getType(sKey.c_str());
      switch(pType){
        case PT_U8:
          if (alldigits(sData)){
            uint8_t val = (uint8_t)sData.toInt();
            PF.putUChar(sKey.c_str(), val);
            bRet = true;
          }
        break;
        case PT_U16:
          if (alldigits(sData)){
            uint16_t val = (uint16_t)sData.toInt();
            PF.putUShort(sKey.c_str(), val);
            bRet = true;
          }
        break;
        case PT_STR:
          PF.putString(sKey.c_str(), sData);
          bRet = true;
        break;
        default:
        break;
      };
    }
    PF.end();
  }
  return bRet;
}

//typedef enum {
//  PT_I8,
//  PT_U8,
//  PT_I16,
//  PT_U16,
//  PT_I32,
//  PT_U32,
//  PT_I64,
//  PT_U64,
//  PT_STR,
//  PT_BLOB,
//  PT_INVALID
//} PreferenceType;
// sPref = "keyIdx:optional namespace"
String PrefsClass::GetPrefAsString(String sPref){
  String sNamespace, sKeyIdx;
  int idx = sPref.indexOf(CM_SEP);
  if (idx >= 1){
    sKeyIdx = sPref.substring(0, idx);
    sNamespace = sPref.substring(idx+1);
  }
  else{
    sKeyIdx = sPref;
    sNamespace = String(EE_PREFS_NAMESPACE);
  }
  
  String sOut;
  if (sNamespace == String(EE_SLOTS_NAMESPACE)){
    t_event t = {0};
    if (alldigits(sKeyIdx)){
      int iKey = sKeyIdx.toInt();
      if (iKey >= 0 && iKey < g_slotCount)
        if (TSC.GetTimeSlot(iKey, t))
          sOut = TSC.TimeSlotToSpaceSepString(t);
    }
    if (sOut.isEmpty()){
      if (!g_slotCount)
        sOut = "no slots to read!";
      else
        sOut = "can't read timeslot " + sKeyIdx + ". Range is 0-" + String(g_slotCount-1) + ".";
    }
  }
  else{
    String sKey = LookupKey(sKeyIdx, sNamespace);
  
    if (sKey.isEmpty())
      return "bad key!";
  
    PF.begin(sNamespace.c_str(), true); // read-only
    if (PF.isKey(sKey.c_str())){
      PreferenceType pType = PF.getType(sKey.c_str());
      switch(pType){
        case PT_U8:
          sOut = String(PF.getUChar(sKey.c_str()));
        break;
        case PT_U16:
          sOut = String(PF.getUShort(sKey.c_str()));
        break;
        case PT_STR:
          sOut = PF.getString(sKey.c_str());
        break;
        default:
          sOut = "unimplimented preference type!";
        break;
      };
    }
    PF.end();
    if (sOut.isEmpty())
      sOut = "requested key has never been written - see the default value... (not known here)";
  }
  return sOut;
}

String PrefsClass::LookupKey(String sKeyIdx, String sNamespace){
  String sKey;
  if (alldigits(sKeyIdx) && !sNamespace.isEmpty()){
    int iKey = sKeyIdx.toInt();
    if (iKey >= 0){
      if (sNamespace == String(EE_PREFS_NAMESPACE)){
        if (iKey < TOTAL_PREFS)
          sKey = String(getPref(iKey));
      }
      else if (sNamespace == String(EE_WIFI_NAMESPACE)){
        if (iKey < TOTAL_WIFI)
          sKey = String(getWiFi(iKey));
      }
      else if (sNamespace == String(EE_SLOTS_NAMESPACE)){
        if (iKey < g_slotCount)  
          sKey = TSC.GetSlotNumAsString(iKey);
      }
    }
  }
  return sKey;
}

void PrefsClass::ToggleOldSsidAndPwd(){
  PF.begin(EE_WIFI_NAMESPACE);
  String sOld = getWiFiString(EE_OLDPWD, DEF_PWD);
  String sNew = getWiFiString(EE_PWD, DEF_PWD);
  putWiFiString(EE_OLDPWD, sNew);
  putWiFiString(EE_PWD, sOld);
  sOld = getWiFiString(EE_OLDSSID, DEF_SSID);
  sNew = getWiFiString(EE_SSID, DEF_SSID);
  putWiFiString(EE_OLDSSID, sNew);
  putWiFiString(EE_SSID, sOld);
  PF.end();
  g_sSSID = sOld;
}

void PrefsClass::RestoreDefaultSsidAndPwd(){
  PF.begin(EE_WIFI_NAMESPACE);
  putWiFiString(EE_OLDPWD, DEF_PWD);
  putWiFiString(EE_PWD, DEF_PWD);
  putWiFiString(EE_OLDSSID, DEF_SSID);
  putWiFiString(EE_SSID, DEF_SSID);
  PF.end();
  g_sSSID = DEF_SSID;
}

void PrefsClass::RestoreDefaultApSsidAndPwd(){
  PF.begin(EE_WIFI_NAMESPACE);
  putWiFiString(EE_OLDAPPWD, DEF_AP_PWD);
  putWiFiString(EE_APPWD, DEF_AP_PWD);
  putWiFiString(EE_OLDAPSSID, DEF_AP_SSID);
  putWiFiString(EE_APSSID, DEF_AP_SSID);
  PF.end();
  g_sApSSID = DEF_AP_SSID;
}
