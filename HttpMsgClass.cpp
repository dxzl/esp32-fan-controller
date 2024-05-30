// this file HttpMsgClass.cpp
#include "FanController.h"

HttpMsgClass HMC;

// This class pertains to internal messaging from one ESP32 to others on the same local WiFi network.

// Here we want to encode the last two octets of the mac address plus relay status and
// any changed parameters for transmission to other units and save the strings in IML.arr[].sSend
// to be sent to other networked ESP32s with the same service-name.
// We save a lowercase id (below) followed by the base 10 encoding of the parameter and string them all
// together.
// Keeping the numbers base 10 allows us to easily merge new changed parameters into the old strings
// if they have not yet been sent.
// Before sending, encode the base 10 numbers to base 36 (0-9, A-Z) and then hnEncode() the entire
// transmit string as base 64.
// A random transmit "token" is also included in the string (CMtoken) and stored in txToken. A receiving unit will
// store it in rxToken and use the token to decode the next received data. The next transmit will use txToken to encode.
// Set bSendDefaultTxToken true to store DEFAULT_TX_TOKEN for each ip or to false (the default) to store a
// random tx token for each ip.
// The resulting encoded string is stored in IML.arr for each remote unit's ip address.
// Encoded transmit strings for each IP are saved in a t_ip_time struct (see MdnsClass.h) 
// Received data is decoded using the rxToken for the remote IP. The CMtoken token in the receive data is stored in the
// rxToken mDNS slot.
// Transmitted data is encoded using the txToken for the remote IP. A CMtoken token is randomly generated and sent
// in the transmit string. It is also stored in in the txToken slot.
//
// 12/29/2023 - A limitation up until now has been that all "data" following a "command character" must be ASCII digits.
// Only numerical data has been permitted. I'm changing things today to allow any character(s) to follow a command character.
// This will be accomplished by encoding all data as base 64 using the default base64 table with a token of 0. Routines in this file will be
// modified to retuen a string rather than an integer. 
void HttpMsgClass::EncodeChangedParametersForAllIPs(){

  String sNew, sTemp;

  // solid-state relay state (on is 1, off is 0)
  //prtln("ssr1 and 2: " + String(g_bSsr1On) + "," + String(g_bSsr2On));
  if (g_bOldSsr1On != g_bSsr1On){
    sTemp = (g_bSsr1On ? "1" : "0");
    AddTableCommand(CMstatA, sTemp, sNew);
    g_bOldSsr1On = g_bSsr1On;
  }
  if (g_bOldSsr2On != g_bSsr2On){
    sTemp = (g_bSsr2On ? "1" : "0");
    AddTableCommand(CMstatB, sTemp, sNew);
    g_bOldSsr2On = g_bSsr2On;
  }
  if (g_perVals.perVal != g_oldPerVals.perVal){
    AddTableCommand(CMper, g_perVals.perVal, sNew);
    g_oldPerVals.perVal = g_perVals.perVal;
  }
  if (g_perVals.perMax != g_oldPerVals.perMax){
    AddTableCommand(CMmaxPer, g_perVals.perMax, sNew);
    g_oldPerVals.perMax = g_perVals.perMax;
  }
  if (g_perVals.perUnits != g_oldPerVals.perUnits){
    AddTableCommand(CMunits, g_perVals.perUnits, sNew);
    g_oldPerVals.perUnits = g_perVals.perUnits;
  }
  if (g_perVals.phase != g_oldPerVals.phase){
    AddTableCommand(CMphase, g_perVals.phase, sNew);
    g_oldPerVals.phase = g_perVals.phase;
  }
  if (g_perVals.dutyCycleA != g_oldPerVals.dutyCycleA){
    AddTableCommand(CMdcA, g_perVals.dutyCycleA, sNew);
    g_oldPerVals.dutyCycleA = g_perVals.dutyCycleA;
  }
  if (g_perVals.dutyCycleB != g_oldPerVals.dutyCycleB){
    AddTableCommand(CMdcB, g_perVals.dutyCycleB, sNew);
    g_oldPerVals.dutyCycleB = g_perVals.dutyCycleB;
  }

  if (g_bSyncMaster){
    if (g_defToken != g_oldDefToken){
      AddTableCommand(CMtoken, g_defToken, sNew);
      g_oldDefToken = g_defToken;
    }
    // send current remaining g32_periodTimer value so remote units can sync to us if we are the master
    if (g_bSyncCycle && g32_periodTimer > MIN_PERIOD_TIMER)
      AddRangeCommand(CMremPerMin, CMremPerMax, g32_periodTimer, sNew);
  }

  uint16_t macLastTwo = IML.GetOurDeviceMacLastTwoOctets();

  if (macLastTwo != g16_oldMacLastTwo){
    // encode last two octets of MAC
    AddTableCommand(CMmac, macLastTwo, sNew);
    g16_oldMacLastTwo = macLastTwo;
  }
  
  AddHttpStringAll(sNew);
}

// Returns existing sSend from mDNS IP array with the CMtoken txToken and CMcs checksum added.
// !!NOTE: never store a string containing CMtoken or CMcs back into IML.arr[].sSend!!
// (We need to keep that string base 64 and only containing changed parameters we can easily merge)
// idx is the IP array index. set bSendDefaultToken to force g_defToken to be added in CMtoken
String HttpMsgClass::EncodeTxTokenAndChecksum(int idx, bool bUseDefaultToken){
  return EncodeTxTokenAndChecksum(idx, IML.GetSendStr(idx), bUseDefaultToken);
}

// NOTE: Computes txNextToken and encodes it into the send-string
String HttpMsgClass::EncodeTxTokenAndChecksum(int idx, String sSend, bool bUseDefaultToken){
  
//prtln("debug: HttpMsgClass::EncodeTxTokenAndChecksum() sSend = \"" + sSend + "\"");
  if (sSend.isEmpty())
    return "";

  // if sSend contains CMsetToken, we want to update its associated down-counter...
  String sParam = DecodeAndStripBase64Command(CMsetToken, sSend); // strip out old...

  if (!sParam.isEmpty()){
    AddTableCommand(CMsetToken, g16_tokenSyncTimer, sSend); // replace with updated...
    prtln("Sending g16_tokenSyncTimer of " + String(g16_tokenSyncTimer) + " to " + IML.GetIP(idx).toString());   
  }

  int txNextToken = bUseDefaultToken ? g_defToken : random(0, MAX_TOKEN+1);
  IML.SetTxNextToken(idx, txNextToken); // set new txToken for next transmit

  // token command is a random number from a range in the ASCII 0-31 range
  AddRangeCommand(CMtokenMin, CMtokenMax, txNextToken, sSend);
//prtln("debug: HttpMsgClass::EncodeTxTokenAndChecksum() sSend after adding CMtokenMinMax random = \"" + sSend + "\"");

  // pad out the command-string with random dummy commands for security
  while (sSend.length() < MINSENDSTRLENGTH)
    AddRangeCommand(CMdummyMin, CMdummyMax, random(1, 255+1), sSend);
//prtln("debug: HttpMsgClass::EncodeTxTokenAndChecksum() sSend after adding CMdummyMinMax random = \"" + sSend + "\"");
  
  AddTableCommand(CMcs, (~Sum(sSend)+1)&0xffff, sSend);

//prtln("debug: HttpMsgClass::EncodeTxTokenAndChecksum() sSend after adding CMcs = \"" + sSend + "\"");

  return sSend;    
}

// Here we want to decode one of the strings encoded by EncodeChangedParametersForAllIPs()
// HMC.DecodeParameters() is used to decode an incomming, fully encoded string.
// First we decode from base 64 shifted to base 36 unshifted, then from base 36 to base 10
// compatible with sSend unencoded strings.
// returns sInOut by-reference with all commands and their decoded data-fields
// returns negative error code if failed, the last two octets of macAddress if it was decoded or 0 if success
int HttpMsgClass::DecodeParameters(String& sInOut, int rxIdx, bool &bPendingTokenWasSet, bool bDecodeUsingDefaultToken){

  if (rxIdx < 0 || rxIdx >= IML.GetCount()){
    prtln("DecodeParameters(): rxIdx out of range!");
    return -1; // return -1 for errors that don't require drastic action...
  }
  
  int rxToken = bDecodeUsingDefaultToken ? g_defToken : IML.GetRxToken(rxIdx);
  
  if (rxToken == NO_TOKEN){
    prtln("DecodeParameters(): rxToken == NO_TOKEN!");
    return -2;
  }
  
  bPendingTokenWasSet = false; // init the by-reference flag false

//prtln("DEBUG: HttpMsgClass::DecodeParameters(): rxToken: " + String(rxToken)); 
//prtln("DEBUG: HttpMsgClass::DecodeParameters() before MyDecode: sInOut: " + sInOut); 

  sInOut = MyDecodeStr(sInOut, HTTP_TABLE1, rxToken, CIPH_CONTEXT_FOREGROUND); // returns base 64 command string
  if (sInOut.isEmpty()){
    prtln("DecodeParameters(): MyDecodeStr() returned empty (decode fail) for rxToken=" + String(rxToken));
    return -3;
  }
    
//prtln("DEBUG: HttpMsgClass::DecodeParameters() after MyDecodeStr(): sInOut: " + sInOut); 

  // strip out the checksum, returning it as int and returning the new string by reference
  String sCs = DecodeAndStripBase64Command(CMcs, sInOut);

  if (!alldigits(sCs))
    return -4;
    
  int cs = sCs.toInt();
//prtln("debug: DecodeParameters(): checksum after StripParam(): \"" + String(cs) +"\"");
//prtln("debug: DecodeParameters(): sInOut after StripParam(): \"" + sInOut +"\"");

  if (cs < 0)
    return -5;

  // 16-bit sum of chars in sInOut plus the value that was in CMcs should add to 0!
//prtln("debug: Sum(sInOut): " + String(Sum(sInOut)));
  cs += Sum(sInOut);
//prtln("DecodeParameters(): cs after Sum(): " + String(cs));
  cs &= 0xffff;
//prtln("DecodeParameters(): cs after & 0xffff: " + String(cs));
  if (cs != 0){
    prtln("DecodeParameters(): bad checksum!");    
    return -6;
  }
  
  int len = sInOut.length();

  if (len == 0)
    return -7;

  prtln("Good Rx command string! \"" + sInOut + "\" decoded with rxToken=" + String(rxToken));

  int newRxToken = -1;
  int macLastTwo = -1;
  String sParse, sOut;
  for(int ii = len-1; ii >= 0; ii--){
    char c = sInOut[ii];
    if (IsHttpCommand(c)){
      if (!sParse.isEmpty()){
        sParse = B64C.hnDecodeStr(Flip(sParse), HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
        sOut += String(c) + sParse;
        int iVal = 0;
        bool bAllDigits = alldigits(sParse) ? true:false;
        if (bAllDigits)
          iVal = sParse.toInt();
        if (c >= CMdummyMin && c <= CMdummyMax){
          // do nothing
        }
        else if (bAllDigits){
          
          // NOTE: iVal can have either negative or positive data!
          // (make sure alldigits() in FCUtils.cpp allows a leading +/- sign!)
          
          if (iVal >= 0){
            if (c >= CMtokenMin && c <= CMtokenMax)
              newRxToken = iVal; // rxToken the receiving station will use when DecodeParameters() is called
            else if (c >= CMremPerMin && c <= CMremPerMax){
              if (!g_bSyncMaster && g_bSyncCycle)
                g32_periodTimer = (uint16_t)iVal;
            }
            else if (c == _HttpCommandTable[CMsetToken]){
              if (g_pendingDefToken != NO_TOKEN)
                g16_tokenSyncTimer = (uint16_t)iVal;
            }
            else if (c == _HttpCommandTable[CMmac])
              macLastTwo = iVal; // last two octets of remote MAC address used to elect a "master"
            else if (c == _HttpCommandTable[CMtoken]){
              g_pendingDefToken = iVal; // default token remotely changed
              bPendingTokenWasSet = true; // set the by-reference flag true
            }
            else if (!ParamProcess(c, iVal)){
              prtln("DecodeParameters(): ParamProcess() returned false for command: " + String((int)c));    
              return -8;
            }
          }
        }
        else {
          // process commands that have non-numerical data here...
//          prtln("DecodeParameters(): Received Non-Numerical data, command=" + String((int)c) + ", data=\"" + sParse + "\"");
          if (c == _HttpCommandTable[CMtxt]){
            int len = sParse.length();
            if (len > 0 && len < MAXTXTLEN){
              IML.SetRxTxtStr(rxIdx, sParse);
              prtln("DecodeParameters(): CMtxt from: " + IML.GetIP(rxIdx).toString() + ", \"" + sParse + "\"");
            }
          }
        }
        sParse = "";
      }
      // these incomming commands have no data...
      else if (c >= CMreqMacMin && c <= CMreqMacMax){ // MAC requested?
        uint16_t uMac = IML.GetOurDeviceMacLastTwoOctets();
        AddTableCommand(CMmac, uMac, rxIdx); // Queue MAC last-two-octets into sSend for the remote IP
        prtln("DecodeParameters(): added CMmac of " + String(uMac) + " for " + IML.GetIP(rxIdx).toString() + ": \"" + IML.GetSendStr(rxIdx) + "\"");
      } 
    }
    else
      sParse += c;
  }
  
  if (newRxToken >= 0){
    IML.XferRxTokens(rxIdx); // move rxToken to rxPrevToken
    IML.SetRxToken(rxIdx, newRxToken);
//prtln("DEBUG: HttpMsgClass::DecodeParameters(): SET NEWLY RECEIVED RXTOKEN: " + String(newRxToken)); 
  }

  // return decoded string in sInOut
  if (!sOut.isEmpty())
    sInOut = sOut;
    
  return (macLastTwo >= 0) ? macLastTwo : 0;
}

bool HttpMsgClass::ParamProcess(char cmd, int iVal){

  if (iVal < 0)
    return false;
    
  if (cmd == _HttpCommandTable[CMmac]){
    // iVal = decimal last two octets of remote MAC address used to elect a "master"
    // this should be handled before calling ParamProcess()!
  }
  else if (cmd == _HttpCommandTable[CMstatA]){
    // remote A channel 1=on, 0 =off
  }
  else if (cmd == _HttpCommandTable[CMstatB]){
    // remote B channel 1=on, 0 =off
  }
  else if (g_bSyncRx){
    if (cmd == _HttpCommandTable[CMmaxPer]){
      QueueTask(TASK_PARMS, SUBTASK_PERMAX, iVal);
      g_oldPerVals.perMax = g_perVals.perMax;
    }
    else if (cmd == _HttpCommandTable[CMunits]){
      QueueTask(TASK_PARMS, SUBTASK_PERUNITS, iVal);
      g_oldPerVals.perUnits = g_perVals.perUnits;
    }
    else if (cmd == _HttpCommandTable[CMper]){
      QueueTask(TASK_PARMS, SUBTASK_PERVAL, iVal);
      g_oldPerVals.perVal = g_perVals.perVal;
    }
    else if (cmd == _HttpCommandTable[CMphase]){
      QueueTask(TASK_PARMS, SUBTASK_PHASE, iVal);
      g_oldPerVals.phase = g_perVals.phase;
    }
    else if (cmd == _HttpCommandTable[CMdcA]){
      QueueTask(TASK_PARMS, SUBTASK_DCA, iVal);
      g_oldPerVals.dutyCycleA = g_perVals.dutyCycleA;
    }
    else if (cmd == _HttpCommandTable[CMdcB]){
      QueueTask(TASK_PARMS, SUBTASK_DCB, iVal);
      g_oldPerVals.dutyCycleB = g_perVals.dutyCycleB;
    }
    else
      return false;
  }
  return true;
}

String HttpMsgClass::DecodeAllParams(int mdnsIdx){
  return DecodeAllParams(IML.GetSendStr(mdnsIdx));
}

String HttpMsgClass::DecodeAllParams(String sSend){
  String sParse, sOut;
  int len = sSend.length();
  for(int ii = len-1; ii >= 0; ii--){
    char c = sSend[ii];
    if (IsHttpCommand(c)){
      sOut += c;
      if (!sParse.isEmpty()){
        sOut += B64C.hnDecodeStr(Flip(sParse), HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
        sParse = "";
      }
    }
    else
      sParse += c;
  }
  return sOut;
}

// strips one of the "range" commands...
void HttpMsgClass::StripParam(char cmdMin, char cmdMax, String& sData){

  int len = sData.length();
  if (len == 0)
    return;

  String sParse, sNew;

  for(int ii = len-1; ii >= 0; ii--){
    char c = sData[ii];
    if (IsHttpCommand(c)){
      if (!sParse.isEmpty())
        sParse = Flip(sParse);
      if (!(c >= cmdMin && c <= cmdMax))
        sNew += c + sParse; // keep valid commands and their data
      sParse = "";
    }
    else
      sParse += c;
  }
  sData = sNew; // return by reference
}

// wrappers for GetParam that set bStrip true...
// sData must contain only base-64 data characters like: J7-p, YUs3, R2- at default table and token=0
// cmdIdx must contain a a command index:
//  CMmac
//  CMstatA
//  CMstatB
//  CMper
//  CMmaxPer
//  CMunits
//  CMphase
//  CMdcA
//  CMdcB
//  CMtoken
//  CMsetToken
//  CMtxt
//  CMcs
String HttpMsgClass::DecodeAndStripBase64Command(int cmdIdx, String& sData){
  return DecodeBase64Command(cmdIdx, sData, true);
}

// Returns cmd plus decoded data string from sData
// sData must contain only base-64 data characters like: J7-p, YUs3, R2- at default table and token=0
// cmdIdx must contain a a command index:
//  CMmac
//  CMstatA
//  CMstatB
//  CMper
//  CMmaxPer
//  CMunits
//  CMphase
//  CMdcA
//  CMdcB
//  CMtoken
//  CMsetToken
//  CMtxt
//  CMcs
String HttpMsgClass::DecodeBase64Command(int cmdIdx, String& sData, bool bStrip){
  return B64C.hnDecodeStr(GetBase64Param(cmdIdx, sData, bStrip), HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
}

// Returns the base 64 data associated with a command character or empty string if error
// sData must contain only base-64 data characters like: J7-p, YUs3, R2- at default table and token=0
// cmdIdx must contain a a command index:
//  CMmac
//  CMstatA
//  CMstatB
//  CMper
//  CMmaxPer
//  CMunits
//  CMphase
//  CMdcA
//  CMdcB
//  CMtoken
//  CMsetToken
//  CMtxt
//  CMcs
// set bStrip true to remove the command and data from sInOut. bStrip defaults false.
// NOTE: if a command has no data we return only the command
String HttpMsgClass::GetBase64Param(int cmdIdx, String& sData, bool bStrip){

  char cmd = GetTableCommand(cmdIdx);
  
  if (cmd == '?')
    return "";
   
  int len = sData.length();

  if (len == 0)
    return "";

  String sParse, sNew, sOut;

  for(int ii = len-1; ii >= 0; ii--){
    char c = sData[ii];
    if (IsHttpCommand(c)){
      if (!sParse.isEmpty())
        sParse = Flip(sParse);
      if (c == cmd){
        if (!bStrip)
          sNew += c;
        if (!sParse.isEmpty()){
          sOut = sParse;
          if (!bStrip)
            sNew += sParse;
        }
        else
          sOut = c;
      }
      else
        sNew += c + sParse;
      sParse = "";
    }
    else
      sParse += c;
  }
  sData = sNew; // return by reference
  return sOut;
}

int HttpMsgClass::AddTableCommandAll(int cmdIdx){
  return AddHttpStringAll(String(GetTableCommand(cmdIdx)));
}

//int HttpMsgClass::AddRangeCommandAll(int cmdMin, int cmdMax){
//  return AddHttpStringAll(String(char(random(cmdMin, cmdMax+1))));
//}

// Note: val is positive integer only: 143, 5324, Etc.
int HttpMsgClass::AddTableCommandAll(int cmdIdx, int val){
  return AddHttpStringAll(FormTableCommand(cmdIdx, val));
}

// Note: val is positive integer only: 143, 5324, Etc.
//int HttpMsgClass::AddRangeCommandAll(int cmdMin, int cmdMax, int val){
//  return AddHttpStringAll(FormRangeCommand(cmdMin, cmdMax, val));
//}

//void HttpMsgClass::AddRangeCommand(int cmdMin, int cmdMax, String &sInOut){
//  sInOut += String(char(random(cmdMin, cmdMax+1)));
//}

//void HttpMsgClass::AddRangeCommand(int cmdMin, int cmdMax, String sVal, String &sInOut){
//  sInOut += FormRangeCommand(cmdMin, cmdMax, sVal);
//}

void HttpMsgClass::AddRangeCommand(int cmdMin, int cmdMax, int val, String &sInOut){
  sInOut += FormRangeCommand(cmdMin, cmdMax, val);
}

String HttpMsgClass::FormRangeCommand(int cmdMin, int cmdMax, int val){
  return String(char(random(cmdMin, cmdMax+1))) + B64C.hnEncodeStr(String(val), HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
}

String HttpMsgClass::FormRangeCommand(int cmdMin, int cmdMax, String sData){
  return String(char(random(cmdMin, cmdMax+1))) + B64C.hnEncodeStr(sData, HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
}

//int HttpMsgClass::AddRangeCommandAll(int cmdMin, int cmdMax, String sVal){
//  return AddHttpStringAll(FormRangeCommand(cmdMin, cmdMax, sVal));
//}

void HttpMsgClass::AddTableCommand(int cmdIdx, int val, int mdnsIdx){
  AddHttpString(FormTableCommand(cmdIdx, val), mdnsIdx);
}

void HttpMsgClass::AddTableCommand(int cmdIdx, String sData, int mdnsIdx){
  AddHttpString(FormTableCommand(cmdIdx, sData), mdnsIdx);
}

void HttpMsgClass::AddTableCommand(int cmdIdx, int mdnsIdx){
  AddHttpString(String(GetTableCommand(cmdIdx)), mdnsIdx);
}

//void HttpMsgClass::AddTableCommand(int cmdIdx, IPAddress ip){
//  AddHttpString(String(GetTableCommand(cmdIdx)), ip);
//}

//void HttpMsgClass::AddTableCommand(int cmdIdx, int val, IPAddress ip){
//  AddHttpString(FormTableCommand(cmdIdx, val), ip);
//}

void HttpMsgClass::AddTableCommand(int cmdIdx, String &sInOut){
  sInOut += GetTableCommand(cmdIdx);
}

void HttpMsgClass::AddTableCommand(int cmdIdx, String sData, String &sInOut){
  sInOut += FormTableCommand(cmdIdx, sData);
}

void HttpMsgClass::AddTableCommand(int cmdIdx, int val, String &sInOut){
  sInOut += FormTableCommand(cmdIdx, val);
}

// Note: sVal MUST be numeric, non-negative, base-10 such as "452", "37"...
int HttpMsgClass::AddTableCommandAll(int cmdIdx, String sData){
  return AddHttpStringAll(FormTableCommand(cmdIdx, sData));
}

String HttpMsgClass::FormTableCommand(int cmdIdx, int val){
  return GetTableCommand(cmdIdx) + B64C.hnEncodeStr(String(val), HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
}

String HttpMsgClass::FormTableCommand(int cmdIdx, String sData){
  return GetTableCommand(cmdIdx) + B64C.hnEncodeStr(sData, HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
}

// for each ESP32 in mDNS table, merge new parameters with those already waiting to be sent
// returns mDNS count
int HttpMsgClass::AddHttpStringAll(String sNew){
  int count = IML.GetCount();
  for (int ii=0; ii<count; ii++)
    AddHttpString(sNew, ii);
  return count;
}

//void HttpMsgClass::AddHttpString(String sNew, IPAddress ip){
//  int idx = IML.FindMdnsIp(ip);
//  if (idx >= 0)
//    AddHttpString(sNew, idx);
//}

// Merge new base 64 data strings in sNew with pre-existing strings
// in the sSend string of a particular mDNS entry
// idx is mDNS table index
void HttpMsgClass::AddHttpString(String sNew, int idx){
  String sSend = IML.GetSendStr(idx);
//prtln("before MergeBase64Params(): \"" + sSend + "\"");
  sSend = MergeBase64Params(sNew, sSend);
//prtln("after MergeBase64Params(): \"" + sSend + "\"");
  IML.SetSendStr(idx, sSend);
}

// this method will keep all base 64 data strings in sNew and add any from
// sOld that are not in sNew and return the merged result.
// Do not pass a string that has CMtoken, txToken or CMcs, checksum! 
String HttpMsgClass::MergeBase64Params(String sNew, String sOld){

  int lenOld = sOld.length();
  int lenNew = sNew.length();

  if (lenOld == 0)
    return sNew;

  // merge standard commands that are in _HttpCommandTable[]
  for (int ii = 0; ii < HTTP_COMMAND_TABLE_SIZE; ii++){
    char c = _HttpCommandTable[ii];
    int idx = sNew.indexOf(c);
    if (idx >= 0)
      continue; // command is in new string, keep the new...
    idx = sOld.indexOf(c);
    if (idx >= 0){ // command is in old string but not in new, add it to new
      sNew += c;
      String sParam = GetBase64ParamAtIndex(sOld, idx);
      if (!sParam.isEmpty())
        sNew += sParam;
    }
  }

  // merge the commands that are a random-range (random range-commands increase security!)
  MergeSpecial(CMreqMacMin, CMreqMacMax, sNew, sOld);
  MergeSpecial(CMtokenMin, CMtokenMax, sNew, sOld);
  MergeSpecial(CMremPerMin, CMremPerMax, sNew, sOld);
  MergeSpecial(CMdummyMin, CMdummyMax, sNew, sOld);

  return sNew;
}

// If range-command is not already in the new string, then if it exists in the old string,
// then add it to the new string
void HttpMsgClass::MergeSpecial(int cmdMin, int cmdMax, String& sNew, String sOld){
  int idx = FindRangeCommand(cmdMin, cmdMax, sNew);
  if (idx < 0){
    idx = FindRangeCommand(cmdMin, cmdMax, sOld);
    if (idx >= 0){
      sNew += sOld[idx];
      String sParam = GetBase64ParamAtIndex(sOld, idx);
      if (!sParam.isEmpty())
        sNew += sParam;
    }
  }
}

// return index of special range-command in sIn or -1 if not found
// (You can call this after calling MyDecodeStr())
int HttpMsgClass::FindRangeCommand(int cmdMin, int cmdMax, String sIn){
  int len = sIn.length();
  for (int ii = 0; ii < len; ii++){
    int c = (int)sIn[ii];
    if (c >= cmdMin && c <= cmdMax)
      return ii;
  }
  return -1;
}

// pass command string in sIn and 0-based idx should index the
// command char associated with the data to be gleaned.
// The return string is base 64 (encoded with HTTP_MSG_TABLE, HTTP_MSG_TOKEN)
String HttpMsgClass::GetBase64ParamAtIndex(String sIn, int idx){
  String sData;
  int lenIn = sIn.length();
  for (int ii = idx+1; ii < lenIn; ii++){
    char c = sIn[ii];
    if (IsHttpCommand(c))
      break;
    sData += c;
  }
  return sData;
}

// pass in a table-index and get back the character from the table
// DO NOT PASS IN A RANGE-COMMAND CHARACTER!!!
char HttpMsgClass::GetTableCommand(int cmdIdx){
  if (cmdIdx < HTTP_COMMAND_TABLE_SIZE)
    return _HttpCommandTable[cmdIdx];
  return '?';
}

// NOTE: A command-string's data is encoded as base-64. Only non-base-64 chars are used for commands. A command
// is a single ANSI char. Some commands are from HTTP_COMMAND_STRING and others are in the range 1-31 (0 not allowed in a String!)
// cmd is an actual char received in a string, it's not an index!
bool HttpMsgClass::IsHttpCommand(char cmd){

  // commands 1-31 are broken up into four ranges: CMdummyMin-CMdummyMax, CMreqMacMin-CMreqMacMax
  // CMtokenMin-CMtokenMax, CMremPerMin-CMremPerMax
  if (cmd > 0 && cmd <= CMSPECIALRANGESMAX)
    return true;

  // is cmd in our commands-table?
  for (int ii = 0; ii < HTTP_COMMAND_TABLE_SIZE; ii++)
    if (cmd == _HttpCommandTable[ii])
      return true;
  return false;
}

uint16_t HttpMsgClass::Sum(String sIn){
  int len = sIn.length();
  uint16_t sum = 0;
  for (int ii = 0; ii < len; ii++)
    sum += sIn[ii];
  return sum;
}

// mirror-image "flip" the string...
String HttpMsgClass::Flip(String sIn){
  int len = sIn.length();
  if (len == 0)
    return "";
  String sOut;
  for (int ii = len-1; ii >= 0; ii--)
    sOut += sIn[ii];
  return sOut;
}
