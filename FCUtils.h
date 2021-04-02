#ifndef FCUtilsH
#define FCUtilsH

#include <Arduino.h>
#include "esp_wifi.h"

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
//typedef struct {
//uint8_t crc;
//uint8_t macAddr[6];
//uint8_t reserved[16];
//uint8_t version;
//} ESP_FUSE3;

//uint8_t mac[6];
//esp_efuse_read_field_blob(ESP_EFUSE_MAC_CUSTOM, mac, 48);
// esp_err_t esp_efuse_write_field_blob(const esp_efuse_desc_t *field[], const void *src, size_t src_size_bits)
// void esp_efuse_burn_new_values(void)

typedef struct
{
  uint8_t dayOfWeek; // 0=Sunday, 1=Monday... 6=Saturday
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t day;
  uint8_t month;
  uint16_t year;
} t_time_date;

typedef struct
{
  // off=0,sec,min,hrs,day,wek,mon,yrs; OFF, ON, AUTO; A, B, AB
  uint8_t repeatMode, deviceMode, deviceAddr; // 0xff = unset
  // dutyCycleA, dutyCycleB, phase are 0-100%
  // 0xff = unset (perUnits and perMax are indicies to html select widget!)
  uint8_t dutyCycleA, dutyCycleB, phase, perUnits, perMax;
  uint16_t repeatCount, everyCount, perVal; // 0xffff = unset
  t_time_date timeDate;
  bool bEnable, bIncludeCycleTiming, bCycleTimingInRepeats; // true if enabled
} t_event;

uint8_t* MacStringToByteArray(const char *pMac, uint8_t* pbyAddress);

//String urlencode(String str);
//String urldecode(String str);
unsigned char h2int(char c);
String ZeroPad(byte val);
String GetStringIP();
void IpToArray(uint16_t ipLastOctet);

void ReadPot1();
void ReadModeSwitch();

void RestoreDefaultSsidAndPwd();
void ToggelOldSsidAndPwd();

String SsrModeToString(byte ssrMode);
void SetState(byte val, String s);
void SetState(byte val, byte ssrMode);

String hnDecode(String sIn);
String hnDecode(String sIn, int &errorCode);

//String convertUnicode(String unicodeStr);
//bool alldigits(String sIn);

bool EraseTimeSlots();
int CountFullTimeSlots();
String GetSlotNumAsString(int val);
bool DisableTimeSlot(int slotIndex);
bool EnableTimeSlot(int slotIndex, bool bEnable=true);
bool AddTimeSlot(t_event &slotData, bool bVerbose=true);
bool DeleteTimeSlot(int slotIndex);
bool GetTimeSlot(int slotIndex, t_event &t);
bool PutTimeSlot(int slotIndex, t_event &t);
int FindFirstEmptyTimeSlot();
int FindNextFullTimeSlot(int iStart);

bool ErasePreferences();
void PutPreferenceU16(const char* s, uint16_t val);
void PutPreferenceString(const char* s, String val);
uint8_t GetPreferenceUChar(const char* s, const char eeDefault);
String GetPreferenceString(const char* s, const char* eeDefault);
void PutPreference(const char* s, byte val);

bool SetTimeManually(int myYear, int myMonth, int myDay, int myHour, int myMinute, int mySecond);
bool InitTimeManually();
struct tm* ReadInternalTime(time_t* pEepochSeconds, struct tm* pTm);
int Make12Hour(int iHour, bool &pmFlag);
void printLocalTime(struct tm &timeInfo);
int CompareTimeDate(t_time_date &timeDate, t_time_date &slotTimeDate);
t_time_date CopyTmToTtimeDate(struct tm &tm);
void ResetPeriod();
//void WiFiApInfo();
String WiFiScan(String sInit);
int GetWiFiChan();
String translateEncryptionType(wifi_auth_mode_t encryptionType);

int InitSecondsList(int slotCount);
int InitRepeatList(int slotCount);

int MyDayOfWeek(int d, int m, int y);

void prtln(String s);
void prt(String s);

#endif
