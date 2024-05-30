#ifndef Encode64H
#define Encode64H

//https://gist.github.com/darelf/0f96e1d313e1d0da5051e1a6eff8d329
#include <Arduino.h>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
//#include <iostream>

// don't change these
#define HTTP_TABLE1      1
#define HTTP_TABLE2      2
#define HTTP_TABLE3      3

// DON'T CHANGE TABLE0!!! (you CAN change the order of chars in TABLE1-TABLE3 but not the character-set. Each char may appear only once!!!)
// ENCODE[0], the default table, must be the same as is used in btoa() and atob() in JavaScript (except for last two chars). Don't change it.
// used in the outer wrapper encoding/decoding of web-page messages. This is the industry-standatd base-64 encoding table and allows
// communication via a standard HTTP web-URL WITHOUT percent-encoding!
#define ENCODE_TABLE0 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" // 64 chars A-Z, a-z, 0-9, -, _
// set ENCODE[1] to random sequence of A-Z, a-z, 0-9, _, - for internal messaging outer wrapper.
// use my Windows utility program Password Generator Sharp https://github.com/dxzl/password-generator-sharp
// (if changed, must reflash all units!!!)
#define ENCODE_TABLE1 "wrq53xbRA_v8SPCJmDZ2Npt0O1BknjiQ4TgscUIGWFoYyMXKdLuE6-79zlfHheaV" // 64 chars 
// set ENCODE[2] to random sequence of A-Z, a-z, 0-9, _, - for internal messaging individual components such as *8Sh or &J2d
// use my Windows utility program Password Generator Sharp https://github.com/dxzl/password-generator-sharp
// (if changed, must reflash all units!!!)
#define ENCODE_TABLE2 "17oNf53VTmIsxudGRFyYzPZSQK-DtwhXOk8L4cpCq6jvUJ2lgrbA_ieMHBn9aWE0" // 64 chars
#define ENCODE_TABLE3 "kejRvTQXNxH13I8taKYpAsgJSEu6mfOyqoBZWl24FD9cr-_0Uhn57dCwzMiPVLbG" // 64 chars

//#define B36_TABLE_SIZE 36
#define B64_TABLE_SIZE 64

// wait for _busy flag to clear...
#define BUSY_WAIT_COUNT 500 // 1 second in 2ms units

class B64Class{
  private:
    String B64EncStr(String& sIn);
    String B64DecStr(String& sIn);
    int B64getTableIndex(char& c);
    int B64getNext(String& sIn, int& idx, int strLen);
    void prtln(String s);
    int GetSct(int& minSct, int& maxSct);

    // This table is set by GenRandTable()
    char _b64ExternalTable[B64_TABLE_SIZE+1]; // web-page messaging individual components, populated by GenRandTable()

    char* _b64T;
    int _token, _base;
    bool _busy;
    
    std::wstring_convert<std::codecvt_utf8<wchar_t>> _cvt;
    
    // indexed by tableIdx. tableIdx defaults to -1 which means "use the _b64ExternalTable[] array" 
    const char* ENCODE[4] = {
      // ENCODE[0], the default table, must be the same as is used in btoa() and atob() in JavaScript (except for last two chars). Don't change it.
      // used in the outer wrapper encoding/decoding of web-page messages
      ENCODE_TABLE0, // 0 _b64DefaultTable
      // set ENCODE[1] to fixed random sequence for internal messaging outer wrapper
      // (if changed, must reflash all units!!!)
      ENCODE_TABLE1, // 1 _b64InternalTable
      // set ENCODE[2] to fixed random sequence for internal messaging individual components such as *8Sh or &J2d
      // (if changed, must reflash all units!!!)
      ENCODE_TABLE2, // 2 _b64InternalTable2
      ENCODE_TABLE3
    };

  public:
    B64Class();
    void init();
    void SetBusy(bool bBusy);
    bool GetBusy();
    int GetBase();
    int GetToken();
    char* GetTable();
    void SetTableAndBusy(int tableIdx, int token);
    void SetB64Table(int tableIdx, int token=0);
    void ShuffleB64ExternalTable(int count=10);
    void GenB64ExternalTable();

    String hnEncNum(int iIn, int tableIdx=-1, int token=0);
    int hnDecNum(String sIn, int tableIdx=-1, int token=0);

    String hnEncNumOnly(int iIn, int tableIdx=-1, int token=0);
    int hnDecNumOnly(String sIn, int tableIdx=-1, int token=0);

    String hnEncode(String sIn, int tableIdx=-1, int token=0);
    int hnDecode(String sIn, String &sOut, int tableIdx=-1, int token=0);
    String hnDecode(String sIn, int tableIdx=-1, int token=0);

    String hnEncodeStr(String sIn, int tableIdx, int token);
    String hnDecodeStr(String sIn, int tableIdx, int token);

    int B64Dec(String& sIn);
    String B64Enc(int n);
};
#endif

extern B64Class B64C;
