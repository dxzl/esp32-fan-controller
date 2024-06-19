// this file Tests.cpp
#include "FanController.h"

// put any tests here then comment them out...
  
void Tests(){
  //---------------------------------------------------------------
  // test CipherClass.h
#if TEST_1_ON
    prtln("test 1 Cipher class...");
    CIP.saveCiphKey(CIPH_CONTEXT_FOREGROUND);

    String sKey1 = "iflcdkeonnrtahximxtrheivlhyftuaa";
    CIP.setCiphKey(sKey1);
    String sData1 = "ESP32 AES256bit Encryption test. Hello world THIS IS A 1234567890 Test!!!!";
    for (int ii=1; ii<64;ii++){
      
      // test encryptString() decryptString()
      String sEnc1 = CIP.encryptString(sData1, ii, CIPH_CONTEXT_FOREGROUND);
      prtln(CIP.decryptString(sEnc1, ii, CIPH_CONTEXT_FOREGROUND));

      // test MyEncodeNum() MyDecodeNum()
      sEnc1 = MyEncodeNum(ii*1000, HTTP_TABLE3, ii, CIPH_CONTEXT_FOREGROUND);
      prtln(String(MyDecodeNum(sEnc1, HTTP_TABLE3, ii, CIPH_CONTEXT_FOREGROUND)));
      
      // test MyEncodeStr() MyDecodeStr()
      sEnc1 = MyEncodeStr(sData1, HTTP_TABLE1, ii, CIPH_CONTEXT_FOREGROUND);
      prtln(MyDecodeStr(sEnc1, HTTP_TABLE1, ii, CIPH_CONTEXT_FOREGROUND));
      
//      prtln("shifted key: \"" + String((char*)CIP.getShiftArrayPtr(CIPH_CONTEXT_FOREGROUND)) + "\"");
    }
    CIP.restoreCiphKey(CIPH_CONTEXT_FOREGROUND);
    prtln("restored key is: \"" + CIP.getCiphKey() + "\"");
#endif // TEST_1_ON
  //---------------------------------------------------------------------------    
#if TEST_2_ON
    prtln("test 2 Cipher class...");
    CIP.saveCiphKey(CIPH_CONTEXT_BACKGROUND);
    String sKey2 = "kbcpozcwajduwpceltnnesxowunxhdus";
    CIP.setCiphKey(sKey2);
    String sData2 = "ESP32 AES256bit Encryption test. Hello world THIS IS A 1234567890 Test!!!!\x1f\x01\x02\x03\x04\x05";
    String sEnc2, sDec2;
    for (int ii=1; ii<64;ii++){
      sEnc2 = CIP.encryptString(sData2, ii, CIPH_CONTEXT_FOREGROUND);
      sDec2 = CIP.decryptString(sEnc2, ii, CIPH_CONTEXT_FOREGROUND);
      String sSubA2 = sDec2.substring(0,74);
      prtln(sSubA2);
      String sSubB2 = sDec2.substring(74);
      String s2;
      for(int ii=0;ii<sSubB2.length();ii++)
        s2 += String(sSubB2[ii], HEX) + ',';
      Serial.println(s2);
//      prtln("shifted key: \"" + String((char*)CIP.getShiftArrayPtr(CIPH_CONTEXT_BACKGROUND)) + "\"");
    }
    CIP.restoreCiphKey(CIPH_CONTEXT_BACKGROUND);
    prtln("restored key is: \"" + CIP.getCiphKey() + "\"");
#endif // TEST_2_ON
  //---------------------------------------------------------------
#if TEST_3_ON
    String sTest3 = B64C.hnEncNumOnly(40584, HTTP_TABLE1, 18); // table, token
    prtln("TEST3: sTest=\"" + sTest3 + "\"");
    int iResult3 = B64C.hnDecNumOnly(sTest3, HTTP_TABLE1, 18);
    prtln("TEST3: " + String(iResult3));
    sTest3 = B64C.hnEncNumOnly(3460, HTTP_TABLE3, 4); // table, token
    prtln("TEST3: sTest=\"" + sTest3 + "\"");
    iResult3 = B64C.hnDecNumOnly(sTest3, HTTP_TABLE3, 4);
    prtln("TEST3: " + String(iResult3));

    sTest3 = MyEncodeNum(40584, HTTP_TABLE2, MAX_TOKEN, CIPH_CONTEXT_FOREGROUND); // table, token
    prtln("TEST3: sTest=\"" + sTest3 + "\"");
    iResult3 = MyDecodeNum(sTest3, HTTP_TABLE2, MAX_TOKEN, CIPH_CONTEXT_FOREGROUND);
    prtln("TEST3: " + String(iResult3));
    sTest3 = MyEncodeNum(3460, HTTP_TABLE3, 4, CIPH_CONTEXT_FOREGROUND); // table, token
    prtln("TEST3: sTest=\"" + sTest3 + "\"");
    iResult3 = MyDecodeNum(sTest3, HTTP_TABLE3, 4, CIPH_CONTEXT_FOREGROUND);
    prtln("TEST3: " + String(iResult3));
#endif // TEST_3_ON
    //---------------------------------------------------------------
#if TEST_4_ON
    // ---- encode ----
    prtln("Test of HMC.EncodeChangedParametersForAllIPs() and HMC.DecodeParameters():");
    IML.AddMdnsIp(String("192.168.1.2"));
    g_oldPerVals.phase = 0xff; // cause a few parameters to appear changed
    g_oldPerVals.dutyCycleA = 0x55;
    g_oldPerVals.dutyCycleB = 0xaa;
    HMC.EncodeChangedParametersForAllIPs(); // add new changed parameters to existing MDC.arr[].sSend
    IPAddress ipFind4;
    ipFind4.fromString("192.168.1.2");
    int idx4 = IML.FindMdnsIp(ipFind4);
    if (idx4 >= 0){
      String sSend4 = IML.GetSendStr(idx4);
      HMC.AddTableCommand(CMtxt, "[this is my cool TEXT 12345!](Ok!)", sSend4); // add a text command
      prtln("sSend initially: \"" + sSend4 + "\"");
      sSend4 = HMC.EncodeTxTokenAndChecksum(idx4, sSend4, true); // add default token and checksum
      IML.XferTxTokens(idx4); // shift g_defToken from nextTxToken to txToken 
      prtln("sSend after EncodeTxTokenAndChecksum: \"" + sSend4 + "\"");
      prtln ("HMC.DecodeAllParams(): \"" + HMC.DecodeAllParams(sSend4) + "\""); // used just to print base64 decoded data fields for show...
      //RefreshSct(); // set g_sct, g_minSct and g_maxSct!
      sSend4 = MyEncodeStr(sSend4, HTTP_TABLE1, g_defToken, CIPH_CONTEXT_FOREGROUND); // (encode using table 1, default token)
      prtln("sSend after MyEncodeStr(): \"" + sSend4 + "\"");
      // ---- decode ----
      bool bPendingTokenWasSet4; // set by-reference in DecodeParameters()
      int macLstTwo4 = HMC.DecodeParameters(sSend4, idx4, bPendingTokenWasSet4, true); // decode using default token
      prtln("sSend after HMC.DecodeParameters: \"" + sSend4 + "\"");
      prtln("macLastTwo: " + String(macLstTwo4));
    }
    else
      prtln("can't find ip address: " + ipFind4.toString());
#endif // TEST_4_ON
    //---------------------------------------------------------------
#if TEST_5_ON
    g_bSyncEncrypt = true;
    String sEnc = MyEncodeNum(143, 2, 4, CIPH_CONTEXT_BACKGROUND);
    prtln("MyEncodeNum()/MyDecodeNum(): " + String(MyDecodeNum(sEnc, 2, 4, CIPH_CONTEXT_BACKGROUND)));
    sEnc = MyEncodeStr("hello world", 1, 34, CIPH_CONTEXT_BACKGROUND);
    prtln("MyEncodeStr()/MyDecodeStr(): " + MyDecodeStr(sEnc, 1, 34, CIPH_CONTEXT_BACKGROUND));
    g_bSyncEncrypt = false;
    sEnc = MyEncodeNum(143, 2, 4, CIPH_CONTEXT_BACKGROUND);
    prtln("MyEncodeNum()/MyDecodeNum(): " + String(MyDecodeNum(sEnc, 2, 4, CIPH_CONTEXT_BACKGROUND)));
    sEnc = MyEncodeStr("hello world", 1, 34, CIPH_CONTEXT_BACKGROUND);
    prtln("MyEncodeStr()/MyDecodeStr(): " + MyDecodeStr(sEnc, 1, 34, CIPH_CONTEXT_BACKGROUND));
#endif // TEST_5_ON
    //---------------------------------------------------------------
#if TEST_6_ON
    String sB64_6 = B64C.hnEncNum(12345678);
    prtln(String(B64C.hnDecNum(sB64_6)));
#endif // TEST_6_ON
    //---------------------------------------------------------------
#if TEST_7_ON
    B64C.GenB64ExternalTable();
    B64C.ShuffleB64ExternalTable();
    String sB64_7 = B64C.hnShiftEncode("testing 123!");
    prtln(B64C.hnShiftDecode(sB64_7));
    if (sB64_7.isEmpty())
      prtln("can't decode!!!! B64C.hnShiftDecode()");
#endif // TEST_7_ON
    //---------------------------------------------------------------
}
