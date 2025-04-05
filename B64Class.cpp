// this file B64Class.cpp
#include "Gpc.h"

B64Class B64C;

// Source for parts of this code:
// https://stackoverflow.com/questions/6213227/fastest-way-to-convert-a-number-to-radix-64-in-javascript
// https://stackoverflow.com/questions/57292210/what-are-the-exact-base64-url-decoding-rules-and-implementation-using-openssl-an/57314480#57314480
//
// Public Methods:
//

// don't think the constructor is being called since new operator is not used, must call init() from setup()!!!!!
B64Class::B64Class(){
  init();
}

void B64Class::init() {
  SetB64Table(-1, 0);
  GenB64ExternalTable();
  _busy = false;
}

// NOTE: if a function is being used by another task and the table must be changed for this task,
// we wait for the _busy flag to be set false... delay() calls yield().
// token defaults to 0
void B64Class::SetTableAndBusy(int tableIdx, int token){
  uint16_t ii = 0;
  while(_busy && ii++ < BUSY_WAIT_COUNT)
    delay(2);
  _busy = true;
  SetB64Table(tableIdx, token);
}

// NOTE: the term External vs. Internal - External is encoding that must interact with Javascript web-pages
// During AP WiFi mode, we use a randomized set derrived from ENCODE[0] that's stored in internal array _b64ExternalTable[].
// For internal messaging between ESP32s (using HTTPAsyncClient), we use the "internal" tables, ENCODE[1] or ENCODE[2]
void B64Class::SetB64Table(int tableIdx, int token) {
  _b64T = tableIdx < 0 ? _b64ExternalTable : (char*)ENCODE[tableIdx];
  _base = strlen(_b64T);
  _token = token;
}

void B64Class::SetBusy(bool bBusy){_busy = bBusy;}
bool B64Class::GetBusy(){return _busy;}
char* B64Class::GetTable(){return _b64T;}
int B64Class::GetBase(){return _base;}
int B64Class::GetToken(){return _token;}

// generates random _b64InternalTable from _b64DefaultTable
void B64Class::GenB64ExternalTable() {
  char buf[B64_TABLE_SIZE];
  for (int ii=0; ii<B64_TABLE_SIZE; ii++)
    buf[ii] = ENCODE[0][ii];
  for (int len=B64_TABLE_SIZE; len>0; len--) {
    int idx = random(len);
    _b64ExternalTable[len-1] = buf[idx];
    // delete char we just used from buf
    for (int jj=idx; jj+1<len; jj++)
      buf[jj] = buf[jj+1];
  }
  _b64ExternalTable[B64_TABLE_SIZE] = '\0';
//  prtln("_b64ExternalTable: \"" + String(_b64ExternalTable) + "\"");
}

// Swaps "count" random values in selected table. Less processing than using GenB64ExternalTable()!
void B64Class::ShuffleB64ExternalTable(int count){
  for (int ii=0; ii<count; ii++){
    int r1 = random(0,B64_TABLE_SIZE);
    int r2 = random(0,B64_TABLE_SIZE);
    int iTemp = _b64ExternalTable[r1];
    _b64ExternalTable[r1] = _b64ExternalTable[r2];
    _b64ExternalTable[r2] = iTemp;
  }
}

// call to encode a number and also hnShiftEncode() it
// returns empty string if error
String B64Class::hnEncNum(int iIn, int tableIdx, int token){
  String sEnc = hnEncNumOnly(iIn, tableIdx, token);
  if (sEnc.isEmpty())
    return "";
  return hnShiftEncode(sEnc, tableIdx, token);
}

// call to encode a number without hnShiftEncode() of it
// returns empty string if error
String B64Class::hnEncNumOnly(int iIn, int tableIdx, int token){
  SetTableAndBusy(tableIdx, token);
  String sEnc = B64Enc(iIn);
  _busy = false;
  return sEnc;
}

// this method adds the shift-count, min shift-count and max shift-count to beginning of the output string,
// converts the UTF-8 input string to wide (wchar_t 16-bit chars), then applies shifting to 18-bit values,
// encodes each 18-bit integer via B64Inc() (18-bits fit perfectly into 3 base64 "digits"), then escapes the
// entire string as base 64 using B64StrInc().
String B64Class::hnShiftEncode(String sIn, int tableIdx, int token){

  SetTableAndBusy(tableIdx, token);

  int minSct, maxSct;
  int sct = GetSct(minSct, maxSct); // get by reference

  String sOut = B64Enc(sct) + ',' + B64Enc(minSct) + ',' + B64Enc(maxSct) + ',';
  uint32_t cs = sct+minSct+maxSct;

  std::wstring wIn = _cvt.from_bytes(sIn.c_str());

  int wLen = wIn.length();

  for (int ii=0; ii<wLen; ii++){
    uint32_t u = (uint32_t)wIn[ii];
    cs += u;

    for (int jj = 0; jj < sct; jj++){
      bool msbSet = (u & 0x20000) ? true : false;
      u <<= 1;
      if (msbSet)
        u |= 1; // rotate overflow bit back in
    }

    if (--sct < minSct)
      sct = maxSct;

    sOut += B64Enc(u) + ',';
  }

  sOut += B64Enc((~cs+1) & 0x3ffff); // add two's compliment checksum, unshifted

  sOut = B64EncStr(sOut);
  
  _busy = false;
  return sOut;
}

// call this if expecting a number that's been encoded to string via hnEncNum()
// returns negative if error
int B64Class::hnDecNum(String sIn, int tableIdx, int token){
  String sOut = hnShiftDecode(sIn, tableIdx, token);
  if (sOut.isEmpty())
    return -2;
  return hnDecNumOnly(sOut, tableIdx, token);
}

// call this if expecting an hnEncNum() number that's already had hnDecode() called...
// returns negative if error
int B64Class::hnDecNumOnly(String sIn, int tableIdx, int token){
  SetTableAndBusy(tableIdx, token);
  int iRet = B64Dec(sIn);
  _busy = false;
  return iRet;
}

// returns empty string if error
String B64Class::hnShiftDecode(String sIn, int tableIdx, int token){
  String sOut;
  int errorCode = hnShiftDecode(sIn, sOut, tableIdx, token);
  if (errorCode < 0)
    prtln("B64Class::hnShiftDecode() error: " + String(errorCode));
  return sOut;
}

// returns errorCode -2 if empty input string, -3 if string too short (must have prefix of comma-separated
// B64Enc() of sct, minSct, maxSct then at least one data char = 7), -4 if bad validation prefix, -5 if bad checksum.
//std::string narrow = converter.to_bytes(wide_utf16_source_string);
int B64Class::hnShiftDecode(String sIn, String &sOut, int tableIdx, int token){

  SetTableAndBusy(tableIdx, token);

  int errorCode, strLen, arrSize, idx, iTmp;
  int sct, minSct, maxSct; // these need to be able to represent negative error...
  uint32_t cs;
  std::wstring wstr;
  std::string u8str;
  
  errorCode = 0; // assume no error
  sOut = "";
  
  sIn = B64DecStr(sIn);

  strLen = sIn.length();

  // a,b,c,d (seven chars minimum expected...)
  if (strLen < 7){
    prtln("B64Class::hnShiftDecode() error: decoded string is too short! strLen=" + String(strLen));
    errorCode = -2;
    goto finally; // no good...
  }

  arrSize = 1;
  for(int ii=0; ii<strLen; ii++) {
    if (sIn[ii] == ',')
      arrSize++;
  }

  if (arrSize < 4){
    prtln("B64Class::hnShiftDecode(): array has wrong data! arrSize=" + String(arrSize));
    errorCode = -3;
    goto finally;
  }

  idx = 0;
  iTmp = B64getNext(sIn, idx, strLen);
  if (iTmp < 0){
    prtln("B64Class::hnDecode(): sct B64getNext() error: " + String(iTmp));
    errorCode = -4;
    goto finally;
  }
  sct = iTmp;
//prtln("SCT: " + String(sct));
  iTmp = B64getNext(sIn, idx, strLen);
  if (iTmp < 0){
    prtln("B64Class::hnDecode(): minSct B64getNext() error: " + String(iTmp));
    errorCode = -5;
    goto finally;
  }
  minSct = iTmp;
//prtln("MINSCT: " + String(minSct));
  iTmp = B64getNext(sIn, idx, strLen);
  if (iTmp < 0){
    prtln("B64Class::hnDecode(): maxSct B64getNext() error: " + String(iTmp));
    errorCode = -6;
    goto finally;
  }
  maxSct = iTmp;
//prtln("MAXSCT: " + String(maxSct));

  if (sct < minSct || sct > maxSct){
    prtln("B64Class::hnDecode(): validation prefix is out of range!");
    errorCode = -7;
    goto finally; // no good...
  }

  cs = sct+minSct+maxSct; // checksum

  wstr = L"";
  
//  for (JsonVariant v: arr){
//    uint16_t n = v.as<uint16_t>();
//    prtln(String(n));
//  }
  for (int ii = 3; ii < arrSize-1; ii++){
    iTmp = B64getNext(sIn, idx, strLen);    
    if (iTmp < 0){
      prtln("B64Class::hnDecode(): B64getNext() error: " + String(iTmp));
      errorCode = -8;
      goto finally;
    }

    uint32_t u = (uint32_t)iTmp;
    
    for (int jj = 0; jj < sct; jj++){
      bool lsbSet = (u & 1) ? true : false;
      u >>= 1;
      if (lsbSet)
        u |= 0x20000;
    }

    if (--sct < minSct)
      sct = maxSct;

    wstr += (wchar_t)u;
    cs += u; // this adds to 0 since encoder sets last char to two's compliment of the sum
  }

  // last char, unshifted, is the 2's compliment checksum (18-bit number!)
  iTmp = B64getNext(sIn, idx, strLen); 
  if (iTmp < 0){
    prtln("B64Class::hnShiftDecode(): cs B64getNext() error: " + String(iTmp));
    errorCode = -9;
    goto finally;
  }
  cs += iTmp;

  if ((cs&0x3ffff) != 0){
//!!!!!!!!!!!!!!!!!!!!!    
//u8str = _cvt.to_bytes(wstr);
//sOut = String(u8str.c_str());
//prtln("BAD STRING: " + sOut);
//!!!!!!!!!!!!!!!!!!!!!    
    prtln("B64Class::hnShiftDecode(): bad checksum on encoded string!");
    errorCode = -10;
    goto finally; // no good...
  }

  u8str = _cvt.to_bytes(wstr);

  sOut = String(u8str.c_str());

finally:
  _busy = false;
  return errorCode;
}

// Need to make sure to call SetTableAndBusy()!
int B64Class::B64getNext(String& sIn, int& idx, int strLen){
  if (idx < 0 || idx >= strLen)
    return -2;
    
  String subStr;
  while(idx < strLen){
    char c = sIn[idx++];
    if (c == ',')
      break;
    subStr += c;
  }

  if (subStr.isEmpty()){
    idx = strLen;
    return -2;
  }

  return B64Dec(subStr);
}

String B64Class::hnEncodeStr(String sIn, int tableIdx, int token){
  if (sIn.isEmpty())
    return "";
  SetTableAndBusy(tableIdx, token);
  sIn = B64EncStr(sIn);
  _busy = false;
  return sIn;
}

// Modified algorithm from solutions given on StackOverflow:
// Base64 translates 24 bits into 4 ASCII characters at a time. First,
// 3 8-bit bytes are treated as 4 6-bit groups. Those 4 groups are
// translated into ASCII characters. That is, each 6-bit number is treated
// as an index into the ASCII character array.
// If the final set of bits is less 8 or 16 instead of 24, traditional base64
// would add a padding character. However, if the length of the data is
// known, then padding can be eliminated.
// One difference between the "standard" Base64 is two characters are different.
// See RFC 4648 for details.
// This is how we end up with the Base64 URL encoding.

// When encoding/decoding for the web pages, the javascript btoa() and atob() routines use the same base-64 alphabet as is in _b64DefaultTable,
// so we want to use that for base64_encode() and base64_decode(). But for b64Enc() and b64Dec() we want to use _b64ExternalTable.
// But when encoding/decoding for internal messages, we want to use one table throughout the network, _b64InternalTable. _b64T will point to
// _b64InternalTable for internal messages and to _b64ExternalTable for web-pages. _b36InternalTable is used for internal message encoding of
// individual numbers such as 1234567.

String B64Class::B64EncStr(String& sIn){
  if (sIn.isEmpty())
    return "";
  String sOut;
  int val=0, valb=-6;
  size_t len = sIn.length();
  char* p = (_b64T == _b64ExternalTable) ? (char*)ENCODE[0] : _b64T;

  // make a shifted table using _token as an offset
  char tokTable[_base+1];
  tokTable[_base] = '\0';
  int tokIdx = _token;
  for (int i=0; i<_base; i++){
    if (tokIdx >= _base)
      tokIdx = 0;
    tokTable[tokIdx++] = p[i];
  }
  p = &tokTable[0];
  
  for (int i=0; i<len; i++){
    unsigned char c = sIn[i];
    val = (val<<8) + c;
    valb += 8;
    while (valb >= 0){
      sOut += p[(val>>valb)&0x3F];
      valb -= 6;
    }
  }
  if (valb > -6)
    sOut += p[((val<<8)>>(valb+8))&0x3F];
  return sOut;
}

String B64Class::hnDecodeStr(String sIn, int tableIdx, int token){
  if (sIn.isEmpty())
    return "";
  SetTableAndBusy(tableIdx, token);
  sIn = B64DecStr(sIn);
  _busy = false;
  return sIn;
}

String B64Class::B64DecStr(String& sIn) {
  if (sIn.isEmpty())
    return "";
  String sOut;
  std::vector<int> T(256, -1);
  char* p = (_b64T == _b64ExternalTable) ? (char*)ENCODE[0] : _b64T;

  // make a shifted table using _token as an offset
  char tokTable[_base+1];
  tokTable[_base] = '\0';
  int tokIdx = _token;
  for (int i=0; i<_base; i++){
    if (tokIdx >= _base)
      tokIdx = 0;
    tokTable[tokIdx++] = p[i];
  }
  p = &tokTable[0];

  for (int i=0; i<_base; i++)
    T[p[i]] = i;

  int val = 0, valb = -8;
  int len = sIn.length();
  for (int i=0; i<len; i++) {
    unsigned char c = sIn[i];
    if (T[c] == -1) break;
    val = (val<<6) + T[c];
    valb += 6;
    if (valb >= 0) {
      sOut += char((val>>valb)&0xFF);
      valb -= 8;
    }
  }
  return sOut;
}

// handles full int32_t range of positive and negative numbers
String B64Class::B64Enc(int32_t n){
  uint32_t residual = n;
  String sOut;
  for(;;) {
    sOut = _b64T[(residual+_token)%_base] + sOut;
    residual /= _base;
    if (residual == 0)
      break;
  }
  return sOut;
}

// handles full int32_t range of positive and negative numbers
int32_t B64Class::B64Dec(String& sIn){
  int32_t result = 0;
  int len = sIn.length();
  for (int i=0; i<len; i++)
    result = (result*_base) + B64getTableIndex(sIn[i]);
  return result;
}

int B64Class::B64getTableIndex(char& c){
  for (int i=0; i<_base; i++)
    if (c == _b64T[(i+_token)%_base])
      return i;
  prtln("ERROR: B64Class::B64getTableIndex() returning -1!!!");
  return -1;
}

int B64Class::GetSct(int &minSct, int &maxSct){
  return ::GetSct(minSct, maxSct);
}

// print wrappers
void B64Class::prtln(String s){
  ::prtln(s);
}
