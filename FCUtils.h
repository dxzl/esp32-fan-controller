#ifndef FCUtilsH
#define FCUtilsH

#include <Arduino.h>
#include <esp_wifi.h>
#include <esp_mac.h>
//#include <esp_cpu.h>
#include <esp_app_desc.h>
//#include <esp_chip_info.h>
#include <esp_wifi_types.h>
#include <ESPmDNS.h>

// returns ESP_OK
// esp_err_t rc = esp_efuse_read_block(EFUSE_BLK3, &fuse3, 0, sizeof(fuse3)*8); // size in bits = sizeof(fuse3) * 8
// read separately as:
//uint8_t mac[6];
//esp_efuse_read_field_blob(ESP_EFUSE_MAC_CUSTOM, mac, 48);
//uint8_t efuse_crc;
//esp_efuse_read_field_blob(ESP_EFUSE_MAC_CUSTOM_CRC, &efuse_crc, 8);
//uint8_t version;
//esp_efuse_read_field_blob(ESP_EFUSE_MAC_CUSTOM_VER, &version, 8);

//Write key to efuse block starting at the offset and the required size.
//esp_err_t esp_efuse_write_block(esp_efuse_block_t blk, const void *src_key, size_t offset_in_bits, size_t size_bits)
//Return
//ESP_OK: The operation was successfully completed.
//ESP_ERR_INVALID_ARG: Error in the passed arguments.
//ESP_ERR_CODING: Error range of data does not match the coding scheme.
//ESP_ERR_EFUSE_REPEATED_PROG: Error repeated programming of programmed bits
//Parameters
//[in] blk: Block number of eFuse.
//[in] src_key: A pointer to array that contains the key for writing.
//[in] offset_in_bits: Start bit in block.
//[in] size_bits: The number of bits required to write.

// one-time programmable PROM for storing custom MAC and version
//typedef struct{
//uint8_t crc;
//uint8_t macAddr[6];
//uint8_t reserved[16];
//uint8_t version;
//} ESP_FUSE3;

//uint8_t mac[6];
//esp_efuse_read_field_blob(ESP_EFUSE_MAC_CUSTOM, mac, 48);
// esp_err_t esp_efuse_write_field_blob(const esp_efuse_desc_t *field[], const void *src, size_t src_size_bits)
// void esp_efuse_burn_new_values(void)

//template <class T,class U> U* Int2Hex(T lnumber, U* buffer){
//    const char* ref = "0123456789ABCDEF";
//    T hNibbles = (lnumber >> 4);
//
//    unsigned char* b_lNibbles = (unsigned char*)&lnumber;
//    unsigned char* b_hNibbles = (unsigned char*)&hNibbles;
//
//    U* pointer = buffer + (sizeof(lnumber) << 1); // need 2 chars per decimal digit, so mult. by 2
//
//    *pointer = 0;
//    do {
//        *--pointer = ref[(*b_lNibbles++) & 0xF];
//        *--pointer = ref[(*b_hNibbles++) & 0xF];
//    } while (pointer > buffer);
//
//    return buffer;
//}

String CommandStrToPrintable(String sIn);
int StringToPerVals(String sPerVals, PerVals& perVals);
String PerValsToString(PerVals perVals);
String GetEmbeddedVersionString();

String PrintCharsWithEscapes(String sIn);
String Unescape(String sIn);
int hexCharToInt(char c);
bool alldigits(String &sIn);
//String convertUnicode(String unicodeStr);
bool isHex(char c);
String genRandMessage(int iMin, int iMax);
//String genRandPositioning(String sIn, int iMin, int iMax);
//bool isIp(String sIn);
bool isHexOrDigit(char c);

String UIntToIp(uint32_t ip_uint);
uint32_t IpToUInt(IPAddress ip);
uint32_t IpToUInt(String sIp);

//String urlencode(String str);
//String urldecode(String str);

String ZeroPad(byte val);
String GetStringInfo();
void IpToArray(uint16_t ipLastOctet);
String GetPerUnitsString(int perUnitsIndex);
String GetPhaseString(int phase);
String GetPerDCString(int iVal);
String PercentOnToString(uint32_t totalDCon, uint32_t totalTime);
String SsrModeToString(uint8_t ssrMode);

//String translateEncryptionType(wifi_auth_mode_t encryptionType);
void twiddle(String& s);

void PrintCycleTiming();
void PrintSpiffs(String sFile);
void PrintMidiChan();
void PrintMidiNote(uint8_t note);
void PrintSsrMode(uint8_t ssrMode);
void PrintPreferences();
void PrintPulseFeaturePreferences();

void prtln(String s);
void prt(String s);

bool WeAreMaster();
bool IsAnyMacZero();
uint16_t GetOurDeviceMacLastTwoOctets();
uint16_t GetHighestMac(int& iIdx);
void RefreshGlobalMasterFlagAndIp();

#if ESP32_S3
void ReadSsrSwitches();
#endif

void ReadPot1();
void ReadPotModeSwitch();
void ReadWiFiSwitch();

void SetSSR(uint8_t gpout, bool bSetSsrOn);
void SetSSRMode(uint8_t gpout, uint8_t ssrMode);

uint8_t* MacStringToByteArray(const char* pMac, uint8_t* pbyAddress);
String MacArrayToString(uint8_t* pMacArray);

void ResetPeriod();
void LimitPeriod();
//void WiFiApInfo();
String WiFiScan(String sInit);
int GetWiFiChan();

bool MasterStartRandomDefaultTokenChange();
bool MasterStartSynchronizedChange(int iChangeCmd, int iChangeFlags, String sChangeData);
bool QueueSynchronizedChange(int iChangeCmd, int iChangeFlags, String sChangeData);

void RefreshSct();
int GetSct(int &minSct, int &maxSct);
int GetRandToken();

int ComputeTimeToOnOrOffA();
int ComputeTimeToOnOrOffB();

bool IsLockedAlertGetPlain(AsyncWebServerRequest *request, bool bAllowInAP=false);
bool IsLockedAlertGet(AsyncWebServerRequest *request, String sReloadUrl, bool bAllowInAP=false);
bool IsLockedAlertPost(AsyncWebServerRequest *request, bool bAllowInAP=false);
bool IsLocked();

String SyncFlagStatus();

void FlashSequencerInit(uint8_t postFlashMode);
void FlashSequencerStop();
void FlashSequencer();
void FlashLED();

//String AddThreeDigitBase10Checksum(String sIn);
String AddTwoDigitBase16Checksum(String sIn);
//String SubtractThreeDigitBase10Checksum(String sIn);
String SubtractTwoDigitBase16Checksum(String sIn);

int MyEncodeStr(String& sInOut, int table, int token, int context);
int MyDecodeStr(String& sInOut, int table, int token, int context);
String MyEncodeNum(int iIn, int table, int token, int context);
int MyDecodeNum(int& iOut, String s, int table, int token, int context);

void InitMAC();

void TaskProcessPulseOffFeatureTiming();
void TaskSetPulseOffFeatureVars();
void TaskStatisticsMonitor();

int SendText(String sIp, String sText);
int SendText(String sText);
int SendText(int idx, String sText);

#endif
