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
    for (int ii=1; ii<B64_TABLE_SIZE;ii++){
      
      // test encryptString() decryptString()
      String sEnc1 = CIP.encryptString(sData1, ii, CIPH_CONTEXT_FOREGROUND);
      prtln(CIP.decryptString(sEnc1, ii, CIPH_CONTEXT_FOREGROUND));

      // test MyEncodeNum() MyDecodeNum()
      sEnc1 = MyEncodeNum(ii*1000, HTTP_TABLE3, ii, CIPH_CONTEXT_FOREGROUND);
      int iOut = 0;
      int iErr = MyDecodeNum(iOut, sEnc1, HTTP_TABLE3, ii, CIPH_CONTEXT_FOREGROUND)));
      if (iErr)
        prtln("error decodeNum=" + String(iErr));
      prtln("iOut=" + String(iOut) + ", iErr=" + String(iErr));
      
      // test MyEncodeStr() MyDecodeStr()
      iErr = MyEncodeStr(sData1, HTTP_TABLE1, ii, CIPH_CONTEXT_FOREGROUND);
      if (iErr)
        prtln("error encodeStr=" + String(iErr));
      iErr = MyDecodeStr(sData1, HTTP_TABLE1, ii, CIPH_CONTEXT_FOREGROUND);
      if (iErr)
        prtln("error decodeStr=" + String(iErr));
      prtln(sData1);
      
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
    for (int ii=1; ii<B64_TABLE_SIZE;ii++){
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
//    String sTest3 = B64C.hnEncNumOnly(40584, HTTP_TABLE1, 18); // table, token
//    prtln("TEST3: sTest=\"" + sTest3 + "\"");
//    int iResult3 = B64C.hnDecNumOnly(sTest3, HTTP_TABLE1, 18);
//    prtln("TEST3: " + String(iResult3));
//    sTest3 = B64C.hnEncNumOnly(3460, HTTP_TABLE3, 4); // table, token
//    prtln("TEST3: sTest=\"" + sTest3 + "\"");
//    iResult3 = B64C.hnDecNumOnly(sTest3, HTTP_TABLE3, 4);
//    prtln("TEST3: " + String(iResult3));
//
//    sTest3 = MyEncodeNum(40584, HTTP_TABLE2, MAX_TOKEN, CIPH_CONTEXT_FOREGROUND); // table, token
//    prtln("TEST3: sTest=\"" + sTest3 + "\"");
//    iResult3 = MyDecodeNum(sTest3, HTTP_TABLE2, MAX_TOKEN, CIPH_CONTEXT_FOREGROUND);
//    prtln("TEST3: " + String(iResult3));
//    sTest3 = MyEncodeNum(3460, HTTP_TABLE3, 4, CIPH_CONTEXT_FOREGROUND); // table, token
//    prtln("TEST3: sTest=\"" + sTest3 + "\"");
//    iResult3 = MyDecodeNum(sTest3, HTTP_TABLE3, 4, CIPH_CONTEXT_FOREGROUND);
//    prtln("TEST3: " + String(iResult3));

    for (int ii7=MIN_TOKEN; ii7<=MAX_TOKEN; ii7++){
      String s7 = MyEncodeNum(0, HTTP_TABLE2, ii7, CIPH_CONTEXT_FOREGROUND); // table, token
      int iOut = 0;
      int iErr = MyDecodeNum(iOut, s7, HTTP_TABLE2, ii7, CIPH_CONTEXT_FOREGROUND)
      if (iErr)
        prtln("iErr=" + String(iErr));
      else
        prtln("TEST3 \"" + String(ii7) + "\": " + String(iOut));
    }

    for (int ii7=MIN_TOKEN; ii7<=MAX_TOKEN; ii7++){
      String s7 = MyEncodeNum(40584, HTTP_TABLE2, ii7, CIPH_CONTEXT_FOREGROUND); // table, token
      int iOut = 0;
      int iErr = MyDecodeNum(iOut, s7, HTTP_TABLE2, ii7, CIPH_CONTEXT_FOREGROUND)
      if (iErr)
        prtln("iErr=" + String(iErr));
      else
        prtln("TEST3 \"" + String(ii7) + "\": " + String(iOut));
    }

    for (int ii7=MIN_TOKEN; ii7<=MAX_TOKEN; ii7++){
      String s7 = MyEncodeNum(-1, HTTP_TABLE2, ii7, CIPH_CONTEXT_FOREGROUND); // table, token
      int iOut = 0;
      int iErr = MyDecodeNum(iOut, s7, HTTP_TABLE2, ii7, CIPH_CONTEXT_FOREGROUND)
      if (iErr)
        prtln("iErr=" + String(iErr));
      else
        prtln("TEST3 \"" + String(ii7) + "\": " + String(iOut));
    }

    for (int ii7=MIN_TOKEN; ii7<=MAX_TOKEN; ii7++){
      String s7 = MyEncodeNum(2147483647, HTTP_TABLE2, ii7, CIPH_CONTEXT_FOREGROUND); // table, token
      int iOut = 0;
      int iErr = MyDecodeNum(iOut, s7, HTTP_TABLE2, ii7, CIPH_CONTEXT_FOREGROUND)
      if (iErr)
        prtln("iErr=" + String(iErr));
      else
        prtln("TEST3 \"" + String(ii7) + "\": " + String(iOut));
    }

#endif // TEST_3_ON
    //---------------------------------------------------------------
#if TEST_4_ON
    // ---- encode ----
    prtln("Test of HMC.EncodeChangedParametersForAllIPs(), HMC.DecodeCommands() and HMC.ProcessMsgCommands():");
    IPAddress ip4 = IPAddress("192.168.1.3");
    IML.AddMdnsIp(String("192.168.1.2"));
    IML.AddMdnsIp(ip4);
    IML.AddMdnsIp(String("192.168.1.4"));
    int mdnsCount = IML.GetCount();
    if (mdnsCount != 3){
      prtln("BAD Mdns COUNT!");
      return;
    }
    g_oldPerVals.phase = 0xff; // cause a few parameters to appear changed
    g_oldPerVals.dutyCycleA = 0x55;
    g_oldPerVals.dutyCycleB = 0xaa;
    HMC.EncodeChangedParametersForAllIPs(); // add new changed parameters to existing MDC.arr[].sSend
    int idx4 = IML.FindMdnsIp(ip4);
    if (idx4 < 0){
      prtln("can't find ip address: " + ip4.toString());
      return;
    }
    prtln("ip4 mdns index should be 1, is " + String(idx4));

    // add mac last-two
    IML.SetMdnsMAClastTwo(0, 0x0002); // slave
    IML.SetMdnsMAClastTwo(1, 0x0003); // slave
    IML.SetMdnsMAClastTwo(2, 0xffff); // make us a slave, make 2 the master
    g_bMaster = true; // setup RefreshGlobalMasterFlagAndIp() to clear g_IpMaster
//    IML.SetMdnsMAClastTwo(2, 0x0001); // make us the master, make 2 a slave
//    g_bMaster = false; // setup RefreshGlobalMasterFlagAndIp() to set g_IpMaster
    
    RefreshGlobalMasterFlagAndIp(); // set/clear g_bMaster flag and g_IpMaster
    
    if (WeAreMaster())
      prtln("WE ARE MASTER!");
    else
      prtln("WE ARE NOT MASTER!");

    prtln("Master's IP is: " + g_IpMaster.toString());
    
    int idxOfHighest = -1;
    int iHighestMacLT = GetHighestMac(idxOfHighest);
    prtln("idxOfHighestMac=" + String(idxOfHighest));
    prtln("iHighestMacLT=" + String(iHighestMacLT));
    prtln("OurDeviceMacLT=" + String(GetOurDeviceMacLastTwoOctets()));
    
    if (g_bMaster){
      HMC.AddCMchangeDataAll(CD_CMD_TOKEN, CD_FLAG_SAVE_IN_FLASH, "22"); // add CMchangeData to sSendSpecific for master
      HMC.AddCMchangeDataAll(CD_CMD_RECONNECT, 0, "");
    }
    else{
      idx4 = IML.FindMdnsIp(g_IpMaster);
      if (idx4 < 0)
        prtln("Cant't find master's IP!!!!");
      else{
        HMC.AddCMchangeReq(CD_CMD_TOKEN, CD_FLAG_SAVE_IN_FLASH, "11", idx4, MDNS_STRING_SEND_SPECIFIC);
        HMC.AddCMchangeReq(CD_CMD_RECONNECT, 0, "", idx4, MDNS_STRING_SEND_SPECIFIC);
      }
    }
    
    prtln("sSendSpecific: \"" + IML.GetStr(idx4, MDNS_STRING_SEND_SPECIFIC) + "\"");
    prtln("sSendAll: \"" + IML.GetStr(idx4, MDNS_STRING_SEND_ALL) + "\"");

    String sSendAll4 = IML.GetStr(idx4, MDNS_STRING_SEND_ALL) + IML.GetStr(idx4, MDNS_STRING_SEND_SPECIFIC);
    HMC.AddRangeCommand(CMremPerMin, CMremPerMax, 54321, sSendAll4);
    HMC.AddCommand(CMtxt, "[this is my cool TEXT 12345!](Ok!)", sSendAll4); // add a text command
    prtln("sSendAll4 combined: \"" + sSendAll4 + "\"");

    int iErr4 = HMC.EncodeTxTokenAndChecksum(idx4, sSendAll4, true); // add default token and checksum
    if (iErr4){
      prtln("HMC.EncodeTxTokenAndChecksum() iErr=" + String(iErr4));
      return;
    }
    prtln("sSend after EncodeTxTokenAndChecksum: \"" + sSendAll4 + "\"");

    // here we test MyEncodeStr()/MyDecodeStr() on sSendAll4
    IML.XferTxTokens(idx4); // shift g_defToken from nextTxToken to txToken 
    prtln ("HMC.DecodeBase64Commands(): \"" + HMC.DecodeBase64Commands(sSendAll4) + "\""); // used just to print base64 decoded data fields for show...
    //RefreshSct(); // set g_sct, g_minSct and g_maxSct!
    iErr4 = MyEncodeStr(sSendAll4, HTTP_TABLE1, g_defToken, CIPH_CONTEXT_FOREGROUND); // (encode using table 1, default token)
    if (iErr4){
      prtln("MyEncodeStr iErr=" + String(iErr4));
      return;
    }
    prtln("sSend after MyEncodeStr(): \"" + sSendAll4 + "\"");
    // ---- decode ----
    iErr4 = MyDecodeStr(sSendAll4, HTTP_TABLE1, g_defToken, CIPH_CONTEXT_FOREGROUND); // (encode using table 1, default token)
    if (iErr4){
      prtln("MyDecodeStr iErr=" + String(iErr4));
      return;
    }
    prtln("sSend after MyDecodeStr(): \"" + sSendAll4 + "\"");
    
    int iRangeCmd = HMC.FindRangeCommand(CMremPerMin, CMremPerMax, sSendAll4);
    if (iRangeCmd >= 0){
      String sData = HMC.GetDataForCommandAtIndex(iRangeCmd, sSendAll4);
      prtln("CMremPerMin/Max range-command encoded data: " + sData);
      sData = B64C.hnDecodeStr(sData, HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
      prtln("CMremPerMin/Max range-command decoded data: " + sData); // print 54321
    }
// NOTE: uncomment to test StripRangeCommand() - BUT - checksum will fail below!!!!
//    HMC.StripRangeCommand(CMremPerMin, CMremPerMax, sSendAll4);
//    iRangeCmd = HMC.FindRangeCommand(CMremPerMin, CMremPerMax, sSendAll4);
//    if (iRangeCmd < 0)
//      prtln("CMremPerMin/Max range-command stripped ok!");

    IML.SetToken(idx4, MDNS_TOKEN_RX, g_defToken);
    g_bMaster = !g_bMaster; // for testing we have to be both master and slave! flip here to process commands
    iErr4 = HMC.DecodeCommands(sSendAll4, idx4); // decode using default token
    if (iErr4){
      prtln("HMC.DecodeCommands iErr=" + String(iErr4));
      return;
    }
    prtln("sSend after HMC.DecodeCommands: \"" + sSendAll4 + "\"");
    
    bool bCMchangeDataWasReceived4 = false; // set by-reference in ProcessMsgCommands()
    iErr4 = HMC.ProcessMsgCommands(sSendAll4, idx4, bCMchangeDataWasReceived4); // decode using default token
    if (iErr4 < 0)
      prtln("error from ProcessMsgCommands() iErr=" + String(iErr4));
    else if (bCMchangeDataWasReceived4)
      prtln("bCMchangeDataWasReceived4 is set!");
#endif // TEST_4_ON
    //---------------------------------------------------------------
#if TEST_5_ON

    g_bSyncEncrypt = true;
    String sEnc = MyEncodeNum(143, 2, 4, CIPH_CONTEXT_BACKGROUND);
    int iOut = 0;
    int iErr = MyDecodeNum(iOut, sEnc, 2, 4, CIPH_CONTEXT_BACKGROUND)
    if (iErr)
      prtln("decodeNum iErr=" + String(iErr));
    else
      prtln("MyEncodeNum()/MyDecodeNum(): " + String(iOut));
    sEnc = "hello world";
    iErr = MyEncodeStr(sEnc, 1, 34, CIPH_CONTEXT_BACKGROUND);
    if (iErr)
      prtln("encodeStr iErr=" + String(iErr));
    iErr = MyDecodeStr(sEnc, 1, 34, CIPH_CONTEXT_BACKGROUND);
    if (iErr)
      prtln("decodeStr iErr=" + String(iErr));
    else
      prtln("MyEncodeStr()/MyDecodeStr(): " + sEnc);
    
    g_bSyncEncrypt = false;
    sEnc = MyEncodeNum(143, 2, 4, CIPH_CONTEXT_BACKGROUND);
    int iOut = 0;
    iErr = MyDecodeNum(iOut, sEnc, 2, 4, CIPH_CONTEXT_BACKGROUND)
    if (iErr)
      prtln("decodeNum iErr=" + String(iErr));
    else
      prtln("MyEncodeNum()/MyDecodeNum(): " + String(iOut));

    // failing hnShiftEncode()/hnShiftDecode() -10 bad checksum    
    sEnc = "hello world";
    iErr = MyEncodeStr(sEnc, 1, 34, CIPH_CONTEXT_BACKGROUND);
    if (iErr)
      prtln("encodeStr iErr=" + String(iErr));
    iErr = MyDecodeStr(sEnc, 1, 34, CIPH_CONTEXT_BACKGROUND);
    if (iErr)
      prtln("decodeStr iErr=" + String(iErr));
    else
      prtln("MyEncodeStr()/MyDecodeStr(): " + sEnc);
#endif // TEST_5_ON
    //---------------------------------------------------------------
#if TEST_6_ON
    int N = 12345678;
    String sB64_6 = B64C.hnEncNumOnly(N);
    prtln(String(N) + " \"" + sB64_6 + "\" = " + String(B64C.hnDecNumOnly(sB64_6)));
    
    N = 0;
    sB64_6 = B64C.hnEncNumOnly(N);
    prtln(String(N) + " \"" + sB64_6 + "\" = " + String(B64C.hnDecNumOnly(sB64_6)));

    N = 0x7fffffff;
    sB64_6 = B64C.hnEncNumOnly(N);
    prtln(String(N) + " \"" + sB64_6 + "\" = " + String(B64C.hnDecNumOnly(sB64_6)));

    N = -1;
    sB64_6 = B64C.hnEncNumOnly(N);
    prtln(String(N) + " \"" + sB64_6 + "\" = " + String(B64C.hnDecNumOnly(sB64_6)));

    N = -1879048193;
    sB64_6 = B64C.hnEncNumOnly(N);
    prtln(String(N) + " \"" + sB64_6 + "\" = " + String(B64C.hnDecNumOnly(sB64_6)));
    
    N = -57384326;
    sB64_6 = B64C.hnEncNumOnly(N);
    prtln(String(N) + " \"" + sB64_6 + "\" = " + String(B64C.hnDecNumOnly(sB64_6)));
    
    N = -728491325;
    sB64_6 = B64C.hnEncNumOnly(N);
    prtln(String(N) + " \"" + sB64_6 + "\" = " + String(B64C.hnDecNumOnly(sB64_6)));
#endif // TEST_6_ON
    //---------------------------------------------------------------
#if TEST_7_ON
    prtln("Test7 B64C.hnEncNumOnly()/B64C.hnDecNumOnly()... please wait...");
    int N7 = 1;
    for(;;){
      for(int ii = 0; ii < 1024; ii++){
        String sB64_7 = B64C.hnEncNumOnly(N7+ii);
        if (B64C.hnDecNumOnly(sB64_7) != N7+ii)
          prtln("Error: " + String(N7));
      }
      prtln("N7: " + String(N7));
      if (!(N7 <<= 1))
        break;
    }
    prtln("Test7 - complete!");    
#endif // TEST_7_ON
    //---------------------------------------------------------------
#if TEST_8_ON
    B64C.GenB64ExternalTable();
    B64C.ShuffleB64ExternalTable();
    String sB64_7 = B64C.hnShiftEncode("testing 123!");
    prtln(B64C.hnShiftDecode(sB64_7));
    if (sB64_7.isEmpty())
      prtln("can't decode!!!! B64C.hnShiftDecode()");
#endif // TEST_8_ON
    //---------------------------------------------------------------
#if TEST_9_ON
    prtln("Test of mDNS token, string and flag read/write:");
    IML.AddMdnsIp(String("192.168.1.0"));
    IML.AddMdnsIp(String("192.168.1.1"));
    IML.AddMdnsIp(String("192.168.1.2"));
    IML.AddMdnsIp(String("192.168.1.3"));

    int Count9 = IML.GetCount();
    if (Count9 != 4)
      prtln("Error in MDNS count!");
    else{
      IML.SetStr(2, MDNS_STRING_SEND_ALL, "this is my send-string!");
      IML.SetToken(2, MDNS_TOKEN_RX, MIN_TOKEN);
      IML.SetToken(2, MDNS_TOKEN_TX, MAX_TOKEN);
      IML.SetFlag(2, MDNS_FLAG_REQUEST_DEFAULT_TOKEN_CHANGE, true);
  
      IPAddress ipFind9;
      ipFind9.fromString("192.168.1.2");
      int idx9 = IML.FindMdnsIp(ipFind9);
      if (idx9 >= 0){
        prtln("sSend TEST = \"" + IML.GetStr(idx9, MDNS_STRING_SEND_ALL) + "\"");
        prtln("TokenRx TEST = " + String(IML.GetToken(idx9, MDNS_TOKEN_RX)));
        prtln("TokenTx TEST = " + String(IML.GetToken(idx9, MDNS_TOKEN_TX)));
        bool bFlag = IML.GetFlag(2, MDNS_FLAG_REQUEST_DEFAULT_TOKEN_CHANGE);
        if (bFlag)
          prtln("Flag TEST = true");
        else
          prtln("Flag TEST = false");
      }
      else
        prtln("can't find ip address: " + ipFind9.toString());
    }
#endif // TEST_9_ON
#if TEST_10_ON
   String s10 = PerValsToString(g_perVals);
   PerVals pv = {0};
   int iErr10 = StringToPerVals(s10, pv);
   prtln("Original:");
   PrintCycleTiming();
   g_perVals = pv;
   prtln("After PerValsToString()/StringToPerVals():");
   PrintCycleTiming();
#endif // TEST_10_ON
    //---------------------------------------------------------------
}
