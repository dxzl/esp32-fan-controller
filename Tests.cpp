// this file Tests.cpp
#include "FanController.h"

// put any tests here then comment them out...
  
void Tests(){
  //---------------------------------------------------------------
  // test CipherClass.h
//    String sKey, sData, sDec, sEnc;
//    
//    prtln("test 1 Cipher class...");
//    CIP.saveCiphKey(CIPH_CONTEXT_FOREGROUND);
//
//    sKey = "iflcdkeonnrtahximxtrheivlhyftuaa";
//    CIP.setCiphKey(sKey);
//    for (int ii=1; ii<64;ii++){
//      sData = "ESP32 AES256bit Encryption test. Hello world THIS IS A 1234567890 Test!!!!";
//      
//      // test encryptString() decryptString()
////      sEnc = CIP.encryptString(sData, ii, CIPH_CONTEXT_FOREGROUND);
////      prtln(sEnc);
////      prtln(CIP.decryptString(sEnc, ii, CIPH_CONTEXT_FOREGROUND));
//
//      // test MyEncodeNum() MyDecodeNum()
//      sEnc = MyEncodeNum(ii*1000, HTTP_TABLE3, ii, CIPH_CONTEXT_FOREGROUND);
//      prtln(String(MyDecodeNum(sEnc, HTTP_TABLE3, ii, CIPH_CONTEXT_FOREGROUND)));
//      
//      // test MyEncodeStr() MyDecodeStr()
//      sEnc = MyEncodeStr(sData, HTTP_TABLE1, ii, CIPH_CONTEXT_FOREGROUND);
//      prtln(MyDecodeStr(sEnc, HTTP_TABLE1, ii, CIPH_CONTEXT_FOREGROUND));
//      
////      prtln("shifted key: \"" + String((char*)CIP.getShiftArrayPtr(CIPH_CONTEXT_FOREGROUND)) + "\"");
//    }
//    CIP.restoreCiphKey(CIPH_CONTEXT_FOREGROUND);
//    prtln("restored key is: \"" + CIP.getCiphKey() + "\"");
//---------------------------------------------------------------------------    
//    prtln("test 2 Cipher class...");
//    CIP.saveCiphKey(CIPH_CONTEXT_BACKGROUND);
//    sKey = "kbcpozcwajduwpceltnnesxowunxhdus";
//    CIP.setCiphKey(sKey);
//    sData = "ESP32 AES256bit Encryption test. Hello world THIS IS A 1234567890 Test!!!!\x1f\x01\x02\x03\x04\x05";
//    for (int ii=1; ii<64;ii++){
//      sEnc = CIP.encryptString(sData, ii, CIPH_CONTEXT_BACKGROUND);
//      sDec = CIP.decryptString(sEnc, ii, CIPH_CONTEXT_BACKGROUND);
//      String sSub1 = sDec.substring(0,74);
//      prtln(sSub1);
//      String sSub2 = sDec.substring(74);
//      for(int ii=0;ii<sSub2.length();ii++)
//        Serial.println(sSub2[ii], HEX);
////      prtln("shifted key: \"" + String((char*)CIP.getShiftArrayPtr(CIPH_CONTEXT_BACKGROUND)) + "\"");
//    }
//    CIP.restoreCiphKey(CIPH_CONTEXT_BACKGROUND);
//    prtln("restored key is: \"" + CIP.getCiphKey() + "\"");
    //---------------------------------------------------------------
//    String sTest = B64C.hnEncNumOnly(40584, HTTP_TABLE1, 18); // table, token
//    prtln("TEST1: sTest=\"" + sTest + "\"");
//    int iResult = B64C.hnDecNumOnly(sTest, HTTP_TABLE1, 18);
//    prtln("TEST1: " + String(iResult));
//    sTest = B64C.hnEncNumOnly(3460, HTTP_TABLE3, 4); // table, token
//    prtln("TEST2: sTest=\"" + sTest + "\"");
//    iResult = B64C.hnDecNumOnly(sTest, HTTP_TABLE3, 4);
//    prtln("TEST2: " + String(iResult));
//
//    sTest = MyEncodeNum(40584, HTTP_TABLE2, MAX_TOKEN, CIPH_CONTEXT_FOREGROUND); // table, token
//    prtln("TEST1: sTest=\"" + sTest + "\"");
//    iResult = MyDecodeNum(sTest, HTTP_TABLE2, MAX_TOKEN, CIPH_CONTEXT_FOREGROUND);
//    prtln("TEST1: " + String(iResult));
//    sTest = MyEncodeNum(3460, HTTP_TABLE3, 4, CIPH_CONTEXT_FOREGROUND); // table, token
//    prtln("TEST2: sTest=\"" + sTest + "\"");
//    iResult = MyDecodeNum(sTest, HTTP_TABLE3, 4, CIPH_CONTEXT_FOREGROUND);
//    prtln("TEST2: " + String(iResult));

    //---------------------------------------------------------------
    // ---- encode ----
//    prtln("Test of HMC.EncodeChangedParametersForAllIPs() and HMC.DecodeParameters():");
//    IML.AddMdnsIp("192.168.1.2");
//    g_oldPerVals.phase = 0xff; // cause a few parameters to appear changed
//    g_oldPerVals.dutyCycleA = 0x55;
//    g_oldPerVals.dutyCycleB = 0xaa;
//    HMC.EncodeChangedParametersForAllIPs(); // add new changed parameters to existing MDC.arr[].sSend
//    IPAddress ipFind;
//    ipFind.fromString("192.168.1.2");
//    int idx = IML.FindMdnsIp(ipFind);
//    if (idx >= 0){
//      String sSend = IML.GetSendStr(idx);
//      HMC.AddTableCommand(CMtxt, "[this is my cool TEXT 12345!](Ok!)", sSend); // add a text command
//      prtln("sSend initially: \"" + sSend + "\"");
//      sSend = HMC.EncodeTxTokenAndChecksum(idx, sSend, true); // add default token and checksum
//      IML.XferTxTokens(idx); // shift g_defToken from nextTxToken to txToken 
//      prtln("sSend after EncodeTxTokenAndChecksum: \"" + sSend + "\"");
//      prtln ("HMC.DecodeAllParams(): \"" + HMC.DecodeAllParams(sSend) + "\""); // used just to print base64 decoded data fields for show...
//      //RefreshSct(); // set g_sct, g_minSct and g_maxSct!
//      sSend = MyEncodeStr(sSend, HTTP_TABLE1, g_defToken); // (encode using table 1, default token)
//      prtln("sSend after MyEncodeStr(): \"" + sSend + "\"");
//      // ---- decode ----
//      bool bPendingTokenWasSet; // set by-reference in DecodeParameters()
//      int macLstTwo = HMC.DecodeParameters(sSend, idx, bPendingTokenWasSet, true); // decode using default token
//      prtln("sSend after HMC.DecodeParameters: \"" + sSend + "\"");
//      prtln("macLastTwo: " + String(macLstTwo));
//    }
//    else
//      prtln("can't find ip address: " + ipFind.toString());
  //---------------------------------------------------------------
//      g_bSyncEncrypt = true;
//      String sEnc = MyEncodeNum(143, 2, 4);
//      prtln("MyEncodeNum()/MyDecodeNum(): " + String(MyDecodeNum(sEnc, 2, 4)));
//      sEnc = MyEncodeStr("hello world", 1, 34);
//      prtln("MyEncodeStr()/MyDecodeStr(): " + MyDecodeStr(sEnc, 1, 34));
//      g_bSyncEncrypt = false;
//      sEnc = MyEncodeNum(143, 2, 4);
//      prtln("MyEncodeNum()/MyDecodeNum(): " + String(MyDecodeNum(sEnc, 2, 4)));
//      sEnc = MyEncodeStr("hello world", 1, 34);
//      prtln("MyEncodeStr()/MyDecodeStr(): " + MyDecodeStr(sEnc, 1, 34));
  //---------------------------------------------------------------
  // String sB64 = B64C.hnEncNum(12345678);
  // prtln(String(B64C.hnDecNum(sB64)));
  //---------------------------------------------------------------
  // String sB64 = B64C.hnEncode("testing 123!");
  // int errx;
  // prtln(B64C.hnDecode(sB64, errx));
  // if (errx < -1)
  //   prtln("error errx=" + String(errx));
  //---------------------------------------------------------------
}
