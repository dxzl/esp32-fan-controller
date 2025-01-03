// this file HttpMsgClass.cpp
#include "FanController.h"

HttpMsgClass HMC;

// This class pertains to internal messaging from one ESP32 to others on the same local WiFi network.

// Here we want to encode the last two octets of the mac address plus relay status and
// any changed parameters for transmission to other units and save the strings in IML.arr[].sSendAll
// to be sent to other networked ESP32s with the same service-name.
// We save a lowercase id (below) followed by the base 10 encoding of the parameter and string them all
// together.
// Keeping the data base 64 allows us to easily merge new changed parameters into the old strings
// if they have not yet been sent.
// Before sending, we either shift-encode or encrypt the base 64 sSendAll string and then encode the final string as base 64.
// A random transmit "token" is also included in the string (CMtokenMin/Max) and stored in txToken. A receiving unit will
// store it in rxToken and use the token to decode the next received data. The next transmit will use txToken to encode.
// If the txToken = NO_TOKEN on a send-attempt, a "CanRx?" handshake task is queued-up that will set initial Tx/Rx tokens
// at either end of the link.
// Encoded transmit strings for each IP are saved in sSendAll of a t_indexMdns struct (see MdnsClass.h). Each string can contain
// multiple commands which can be added asynchronously. The string is cleard after successful transmission. Data for each
// command is encoded as base 64. A "command" consists of one or more non-base64 ANSI characters (the character must NOT be in the
// ENCODE_TABLE0 string! (see B64Class.h)).
// Received data is decoded using the rxToken for the remote IP. The CMtokenMin/Max in the receive data is stored in the
// rxToken mDNS slot.
// Transmitted data is encoded using the txToken for the remote IP. A CMtokenMin/Max is randomly generated and sent
// in the transmit string. It is also stored in in the txToken slot.
// All commands must be followed by a base64 encoded data string. Any character can be encoded as data. +NNNN/-NNNN numerical
// data is alowed. Data is encoded using the default base64 table with a token of 0.

void HttpMsgClass::EncodeChangedParametersForAllIPs(){

  String sNew, sTemp;

  if (g_oldDevStatus != g_devStatus){
    AddCommand(CMstat, g_devStatus, sNew); // send on/off status of up to 32 relays/devices
    g_oldDevStatus = g_devStatus;
  }
  if (g_perVals.perVal != g_oldPerVals.perVal){
    AddCommand(CMper, g_perVals.perVal, sNew);
    g_oldPerVals.perVal = g_perVals.perVal;
  }
  if (g_perVals.perMax != g_oldPerVals.perMax){
    AddCommand(CMmaxPer, g_perVals.perMax, sNew);
    g_oldPerVals.perMax = g_perVals.perMax;
  }
  if (g_perVals.perUnits != g_oldPerVals.perUnits){
    AddCommand(CMunits, g_perVals.perUnits, sNew);
    g_oldPerVals.perUnits = g_perVals.perUnits;
  }
  if (g_perVals.phase != g_oldPerVals.phase){
    AddCommand(CMphase, g_perVals.phase, sNew);
    g_oldPerVals.phase = g_perVals.phase;
  }
  if (g_perVals.dutyCycleA != g_oldPerVals.dutyCycleA){
    AddCommand(CMdcA, g_perVals.dutyCycleA, sNew);
    g_oldPerVals.dutyCycleA = g_perVals.dutyCycleA;
  }
  if (g_perVals.dutyCycleB != g_oldPerVals.dutyCycleB){
    AddCommand(CMdcB, g_perVals.dutyCycleB, sNew);
    g_oldPerVals.dutyCycleB = g_perVals.dutyCycleB;
  }

  if (g_bMaster){
    // send current remaining g32_periodTimer value so remote units can sync to us if we are the master
    if (g_bSyncCycle && g32_periodTimer > MIN_PERIOD_TIMER)
      AddRangeCommand(CMremPerMin, CMremPerMax, g32_periodTimer, sNew);
  }

  AddHttpStringAll(sNew);
}

// Returns the sInOut base64 command-string with the CMtokenMin/Max txToken and CMcs checksum added.
// !!NOTE: never store a string containing CMtoken or CMcs back into IML.arr[].sSendAll!!
// (We need to keep that string base 64 and only containing changed parameters we can easily merge)
// idx is the IP array index. Set bUseDefaultToken to set txNextToken to g_defToken rather than using a
// random token.
// NOTE: Computes txNextToken and encodes it into the send-string as a CMtokenMin/Max command
//
// returns negative if error, 0 if ok
// result returned by-reference in sInOut
int HttpMsgClass::EncodeTxTokenAndChecksum(int idx, String& sInOut, bool bUseDefaultToken){
  
//prtln("debug: HttpMsgClass::EncodeTxTokenAndChecksum() sInOut = \"" + sInOut + "\"");
  if (sInOut.isEmpty())
    return -2;

  // if sInOut contains CMchangeSet, we want to update its associated down-counter...
  String sParam = StripCommand(CMchangeSet, sInOut); // strip out old...
  sParam = B64C.hnDecodeStr(sParam, HTTP_MSG_TABLE, HTTP_MSG_TOKEN);

  if (!sParam.isEmpty()){
    AddCommand(CMchangeSet, g16_changeSyncTimer, sInOut); // replace with updated...
    prtln("Sending g16_changeSyncTimer of " + String(g16_changeSyncTimer) + " to " + IML.GetIP(idx).toString());   
  }

  int txNextToken = bUseDefaultToken ? g_defToken : GetRandToken();
  IML.SetToken(idx, MDNS_TOKEN_TXNEXT, txNextToken); // set new txToken for next transmit

  // token command is a random number from a range in the ASCII 0-31 range
  AddRangeCommand(CMtokenMin, CMtokenMax, txNextToken, sInOut);
//prtln("debug: HttpMsgClass::EncodeTxTokenAndChecksum() sInOut after adding CMtokenMinMax random = \"" + sInOut + "\"");

  // pad out the command-string with random dummy commands for security
  while (sInOut.length() < MINSENDSTRLENGTH)
    AddRangeCommand(CMdummyMin, CMdummyMax, random(1, 255+1), sInOut);
//prtln("debug: HttpMsgClass::EncodeTxTokenAndChecksum() sInOut after adding CMdummyMinMax random = \"" + sInOut + "\"");
  
  AddCommand(CMcs, (~Sum(sInOut)+1)&0xffff, sInOut);

  if (sInOut.isEmpty())
    return -3;
    
//prtln("debug: HttpMsgClass::EncodeTxTokenAndChecksum() sInOut after adding CMcs = \"" + sInOut + "\"");

  return 0;
}

// Here we want to decode one of the strings encoded by EncodeChangedParametersForAllIPs()
// HMC.DecodeCommands() is used to decode an incomming, fully encoded string.
// returns sInOut by-reference with all commands and their base64 data-fields
// returns negative error code if failed, the last two octets of macAddress if it was decoded or 0 if success
int HttpMsgClass::DecodeCommands(String& sInOut, int rxIdx){

  int iCount = IML.GetCount();
  
  if (rxIdx < 0 || rxIdx >= iCount){
    prtln("DecodeCommands() rxIdx out of range!");
    return -1; // return -1 for errors that don't require drastic action...
  }
  
  String sIp = IML.GetIP(rxIdx).toString();
  prtln("DecodeCommands() for: " + sIp);

  int rxToken = IML.GetToken(rxIdx, MDNS_TOKEN_RX);
  
  if (rxToken == NO_TOKEN){
    prtln("DecodeCommands() rxToken == NO_TOKEN!");
    return -2;
  }

  if (sInOut.isEmpty()){
    prtln("DecodeCommands() sInOut is empty");
    return -3;
  }
    
  // strip out the checksum, returning it as int and returning the new string by reference
  String sCs = StripCommand(CMcs, sInOut); // strip out old...
  sCs = B64C.hnDecodeStr(sCs, HTTP_MSG_TABLE, HTTP_MSG_TOKEN);

  if (!alldigits(sCs))
    return -4;
    
  int cs = sCs.toInt();

  if (cs < 0)
    return -5;

  // 16-bit sum of chars in sInOut plus the value that was in CMcs should add to 0!
  cs += Sum(sInOut);
  cs &= 0xffff;
  if (cs != 0){
    prtln("DecodeCommands() bad checksum!");    
    return -6;
  }
  
  if (sInOut.isEmpty())
    return -7;

  return 0;
}

// call ProcessMsgCommands() after DecodeCommands() - it parses out the individual command-strings and
// decodes the base64 data field that must follow any command.
//
// A unique kind of command, CMto/CMfrom is a a wrapper command for commands that need to be forwarded
// by a master to a non-master. CMto is sent from a non-master to a master with the target IP address
// prepended as a base-10 int. A master then repackages the commands in a received CMto into a CMfrom
// prepended by the IP of the non-master we got it from, and forwards it to the specified non-master.
// CMto/CMfrom can invoke ProcessMsgCommands() to process the embedded commands via
// TSK.QueueTask(TASK_PROCESS_COMMANDS, sData, sIp);
//
// returns negative if error, 0 if not
// returns string of decoded commands that were correctly handled, by reference, in sInOut
int HttpMsgClass::ProcessMsgCommands(String& sInOut, int rxIdx, bool& bCMchangeDataWasReceived){
        
  bCMchangeDataWasReceived = false; // init the by-reference flag false

  int len = sInOut.length();

  prtln("processing commands from: " + IML.GetIP(rxIdx).toString());
  
  String sCmd, sData, sOut;
  int idx = 0;
  for(;;){
    if (!ParseCommand(idx, sInOut, sCmd, sData))
      break;
    sData = B64C.hnDecodeStr(sData, HTTP_MSG_TABLE, HTTP_MSG_TOKEN); // decode from base-64
    if (sData.isEmpty())
      return -3; // error decoding base64 data
    if (sData.length() > CD_MAX_DATA_STRING_LENGTH)
      return -4; // error decoding base64 data
    int iErr = ProcessMsgCommand(rxIdx, sCmd, sData, bCMchangeDataWasReceived);
    if (iErr < 0){
      prtln("HttpMsgClass::ProcessMsgCommands() error " + String(iErr) + " for command \"" + sCmd+sData + "\"");
      if (iErr < -1)
        return -5; // error processing command
    }
    else
      sOut += sCmd+sData;
  }

  // return fully decoded command-string in sInOut
  if (!sOut.isEmpty())
    sInOut = sOut;
    
  return 0;
}

// returns 0 if no error, negative if error, -1 if command not found (sData has already been base-64 decoded!)
int HttpMsgClass::ProcessMsgCommand(int rxIdx, String& sCmd, String& sData, bool& bCMchangeDataWasReceived){
  if (sCmd.isEmpty() || sData.isEmpty())
    return -2;
  // Note: make sure alldigits() in FCUtils.cpp allows a leading +/- sign!
  bool bAllDigits = alldigits(sData);
  int iVal = bAllDigits ? sData.toInt() : 0;
  int iCmd = GetCommandIndex(sCmd);
  if (iCmd < 0){
    if (sCmd.length() != 1)
      return -3;
    iCmd = (int)sCmd[0];
    // command not in table - is it a range-command?  
    if (iCmd >= CMdummyMin && iCmd <= CMdummyMax){
      // do nothing
    }
    else if (bAllDigits){
      if (iCmd >= CMtokenMin && iCmd <= CMtokenMax){
        if (iVal >= MIN_TOKEN && iVal <= MAX_TOKEN){
          IML.SetToken(rxIdx, MDNS_TOKEN_RXNEXT, iVal); // new token will take effect when XferRxTokens() is called
          prtln("ParamProcess() received CMtokenMin/Max, set MDNS_TOKEN_RXNEXT: " + String(iVal)); 
        }
      }
      else if (iCmd >= CMremPerMin && iCmd <= CMremPerMax){
        if (!g_bMaster && g_bSyncCycle)
          g32_periodTimer = (uint16_t)iVal;
      }
    }
  }
  else if (bAllDigits){
    if (iCmd == CMstat)
      // remote channel status bits bit0=A, bit1=B, 1=on, 0=off
      IML.SetStatus(rxIdx, iVal);
    else if (iCmd == CMchangeSet){
      if (g_bMaster)
        prtln("master should not get a CMchangeSet command!");
      else if (CNG.GetCount() && !g16_changeSyncTimer) // previously have received CMchangeCmd as first-step...
        g16_changeSyncTimer = (uint16_t)iVal; // start countdown to synchronously process change-commands saved in ChangeClass.cpp
    }
    else if (iCmd == CMmac){
      // last two octets of remote MAC address used to elect a "master"
      if (iVal > 0){
        IML.SetMdnsMAClastTwo(rxIdx, iVal);
        RefreshGlobalMasterFlagAndIp();
        prtln("ProcessCommand() received CMmac and set macLastTwo=" + String(iVal));
      }
    }
    else if (g_bSyncRx){
      if (iCmd == CMmaxPer){
        TSK.QueueTask(TASK_PARMS, SUBTASK_PERMAX, iVal);
        g_oldPerVals.perMax = g_perVals.perMax; // avoid re-transimtting changes in "feedback" loop!
      }
      else if (iCmd == CMunits){
        TSK.QueueTask(TASK_PARMS, SUBTASK_PERUNITS, iVal);
        g_oldPerVals.perUnits = g_perVals.perUnits;
      }
      else if (iCmd == CMper){
        TSK.QueueTask(TASK_PARMS, SUBTASK_PERVAL, iVal);
        g_oldPerVals.perVal = g_perVals.perVal;
      }
      else if (iCmd == CMphase){
        TSK.QueueTask(TASK_PARMS, SUBTASK_PHASE, iVal);
        g_oldPerVals.phase = g_perVals.phase;
      }
      else if (iCmd == CMdcA){
        TSK.QueueTask(TASK_PARMS, SUBTASK_DCA, iVal);
        g_oldPerVals.dutyCycleA = g_perVals.dutyCycleA;
      }
      else if (iCmd == CMdcB){
        TSK.QueueTask(TASK_PARMS, SUBTASK_DCB, iVal);
        g_oldPerVals.dutyCycleB = g_perVals.dutyCycleB;
      }
      else
        return -1;
    }
  }
  // process commands that have alphanumeric data here...
  else if (iCmd == CMchangeReq || iCmd == CMchangeData){
    // we expect sData to contain two base64 encoded uint32_t separated by CM_SEP
    int idx = sData.indexOf(CM_SEP);
    if (idx >= 1){
      uint32_t uCmd = (uint32_t)B64C.hnDecNumOnly(sData.substring(0, idx));
      uint32_t uFlags = uCmd & ((1<<FLAG_COUNT_CD)-1);
      uCmd >>= FLAG_COUNT_CD;
      if (uCmd != CD_CMD_EMPTY){
        sData = sData.substring(idx+1);
        if (iCmd == CMchangeReq){
          if (!g_bMaster)
            prtln("ERROR: ProcessMsgCommand(): non-master should not get a CMchangeReq command!");
          else{
            if (MasterStartSynchronizedChange(uCmd, uFlags, sData))
              prtln("master is initiating requested synchronized change: " + String(uCmd));
            else
              prtln("master is unable to initiate requested synchronized change!");
          }
        }
        else{ // CMchangeData
          if (g_bMaster)
            prtln("ERROR: ProcessMsgCommand(): master should not get a CMchangeData command!");
          else{
            CNG.QueueChange(uCmd, uFlags, sData); // uCmd in the process of being changed...
            bCMchangeDataWasReceived = true; // set the by-reference flag true
          }
        }
      }
    }
  }
  else if (iCmd == CMrestart){
    String sIp = IML.GetIP(rxIdx).toString();
    prt("Received CMrestart from " + sIp + ". ");
    if (!sData.isEmpty()){ // sData will have the verification key
      if (!g_sRestartKey.isEmpty()){
        if (g_sRestartKey == sData && g_sRestartIp == sIp){
          TSK.QueueTask(TASK_RESTART);
          prtln("Good key! This unit is retarting...");
        }
        else
          prtln("But either key or IP is wrong!");
      }
      else
        prtln("But key has expired.");
    }
    else
      prtln("But the key is empty!");
  }
  else if (iCmd == CMcmdReq){
    String sCmd;
    iCmd = sData.indexOf(CM_SEP);
    if (iCmd < 0){
      sCmd = sData;
      sData = "";
    }
    else{
      sCmd = sData.substring(0, iCmd);
      sData = sData.substring(iCmd+1);
    }
    if (!alldigits(sCmd))
      return 0;
    iCmd = sCmd.toInt();
    String sIp = IML.GetIP(rxIdx).toString();
    if (iCmd == CM_CMD_READ){
      prt("CMcmdRsp CM_CMD_READ: Unit " + sIp + " ");
      if (!sData.isEmpty()){ // sData will have eeKey or eeKey:eeNamespace (optional namespace, default is EE_PREFS_NAMESPACE)
        String sKeyNs = sData;
        String sRespData = PC.GetPrefAsString(sKeyNs); // read flash-memory SPIFFS
        prt("requested EE key:namespace \"" + sKeyNs + "\" ");
        if (!sRespData.isEmpty()){
          // send back cmd:((data):key:ns)
          sData = B64C.hnEncodeStr(sRespData)+CM_SEP+sKeyNs;
          String sSend = sCmd+CM_SEP+sData;
          HMC.AddCommand(CMcmdRsp, sSend, rxIdx, MDNS_STRING_SEND_SPECIFIC);
          prtln("= \"" + sRespData + "\"");
        }
        else
          prtln("bad data!");
      }
      else
        prtln("empty data!");
    }
    else if (iCmd == CM_CMD_WRITE){
      prt("CMcmdRsp CM_CMD_WRITE: Unit " + sIp + " ");
      // sData will have (data):key:optional namespace
      if (!sData.isEmpty()){
        String sAck;
        if (PC.SetPrefFromString(sData)){ // set flash-memory SPIFFS
          sAck = CM_GOOD_ACK;
          prtln("replying CM_GOOD_ACK, SPIFFS data was set: \"" + sData + "\"");
        }
        else{
          sAck = CM_BAD_ACK;
          prtln("replying CM_BAD_ACK, bad data \"" + sData + "\"");
        }
        String sSend = sCmd+CM_SEP+sAck;
        HMC.AddCommand(CMcmdRsp, sSend, rxIdx, MDNS_STRING_SEND_SPECIFIC);
      }
      else
        prtln("empty data!");
    }
    else if (iCmd == CM_CMD_RESTART){
      if (!sData.isEmpty()){
        prt("CMcmdRsp CM_CMD_RESTART: Unit " + sIp + " ");
        if (sData == CM_CMD_RESTART_MAGIC){
          g_sRestartKey = genRandMessage(RESTART_KEY_MIN_LENGTH, RESTART_KEY_MAX_LENGTH);
          g16_restartExpirationTimer = RESTART_EXPIRATION_TIME; 
          g_sRestartIp = sIp;
          String sSend = sCmd+CM_SEP+g_sRestartKey;
          HMC.AddCommand(CMcmdRsp, sSend, rxIdx, MDNS_STRING_SEND_SPECIFIC);
          prtln("Sending CMcmdAck CM_CMD_RESTART, key = \"" + g_sRestartKey + "\"");
        }
        else
          prtln("bad data!");
      }
      else
        prtln("empty data!");
    }
    else if (iCmd == CM_CMD_INFO){
      sData = GetStringInfo();
      String sSend = sCmd+CM_SEP+sData;
      HMC.AddCommand(CMcmdRsp, sSend, rxIdx, MDNS_STRING_SEND_SPECIFIC);
      prtln("Sending CMcmdAck CM_CMD_INFO, key = \"" + sData + "\"");
    }
  }
  else if (iCmd == CMcmdRsp){
    iCmd = sData.indexOf(CM_SEP);
    if (iCmd < 0){
      sCmd = sData;
      sData = "";
    }
    else{
      sCmd = sData.substring(0, iCmd);
      sData = sData.substring(iCmd+1);
    }
    if (!alldigits(sCmd))
      return 0;
    iCmd = sCmd.toInt();
    String sIp = IML.GetIP(rxIdx).toString();
    if (iCmd == CM_CMD_READ){
      prt("CMcmdRsp CM_CMD_READ: Unit " + sIp + " ");
      // sData at this point has (data):key:optional namespace
      if (!sData.isEmpty()){
        int idx = sData.indexOf(CM_SEP);
        if (idx >= 1){
          String sEEdata = B64C.hnDecodeStr(sData.substring(0, idx));
          sData = sData.substring(idx+1);
          idx = sData.indexOf(CM_SEP);
          if (idx >= 1){ // namespace at end of string
            String sEEkey = sData.substring(0, idx);
            sData = sData.substring(idx+1);
            prtln("received: EE key: \"" + sEEkey + "\", EE namespace: \"" + sData +
                                                    "\", EE data: \"" + sEEdata + "\"");
          }
          else{ // no namespace, use prefs namespace
            prtln("received: EE key: \"" + sData + "\", EE namespace: \"" + String(EE_PREFS_NAMESPACE) +
                                                    "\", EE data: \"" + sEEdata + "\"");
          }
        }
        else
          prtln("bad data format!");
      }
      else
        prtln("empty data!");
    }
    else if (iCmd == CM_CMD_WRITE){
      prt("CMcmdRsp CM_CMD_WRITE: Unit " + sIp + " ");
      if (!sData.isEmpty()){
        if (sData == CM_GOOD_ACK)
          prtln("Good Acknowledge!");
        else if (sData == CM_BAD_ACK)
          prtln("Bad Acknowledge!");
        else
          prtln("Bogus Acknowledge!");
      }
      else
        prtln("Bogus Acknowledge");
    }
    else if (iCmd == CM_CMD_RESTART){
      prt("CMcmdRsp CM_CMD_RESTART: Unit " + sIp + " ");
      if (!sData.isEmpty()) // sData will have the verification key
        prtln("reports that it is armed for restart! Type \"c restart " + sIp + " " + sData);
      else
        prtln("failed to send a restart key!");
    }
    else if (iCmd == CM_CMD_INFO){
      prt("CMcmdRsp CM_CMD_INFO: Unit " + sIp + " ");
      if (!sData.isEmpty())
        prtln("INFO: \"" + sData + "\"");
      else
        prtln("empty data!");
    }
  }
  else if (iCmd == CMtxt){
    if (sData.length() <= MAXTXTLEN){
      sData = IML.GetIP(rxIdx).toString() + ':' + sData; // prepend "who it's from" IP address
      IML.SetStr(rxIdx, MDNS_STRING_RXTXT, sData);
      prtln("ProcessCommand() got CMtxt \"" + sData + "\"");
    }
  }
  else if (iCmd == CMfrom){
    // here, we expect that we are a non-master receiving this from the master
    // sData contains one or more commands/data from either another non-master that have been
    // forwarded to us through the master - OR the bundled commands CAN be from the master!
    prtln("got CMfrom...");
    if (g_bMaster)
      prtln("master should not get a CMfrom command!");
    else{
      // get original sender's prepended ip address
      int idx = sData.indexOf(CM_SEP);
      if (idx >= 1){ // base10number:
        String sSourceIp = sData.substring(0, idx);
prtln("ProcessCommand() CMfrom: original sender's sSourceIp: \"" + sSourceIp + "\"");
        sData = sData.substring(idx+1);
prtln("ProcessCommand() CMfrom: commands sData: \"" + sData + "\"");
        if (!sData.isEmpty() && !sSourceIp.isEmpty()){
          uint32_t uSourceIp = B64C.hnDecNumOnly(sSourceIp);
prtln("ProcessCommand() CMfrom: uSourceIp: " + String(uSourceIp));
          if (uSourceIp > 0){
            idx = IML.FindMdnsIp(uSourceIp);
            if (idx >= 0){
              // here, we have verified that the other non-master unit that sent us some commands actually exists
              String sFromIp = IML.GetIP(idx).toString();
              TSK.QueueTask(TASK_PROCESS_COMMANDS, sData, sFromIp);
prtln("ProcessCommand() CMfrom: this non-master queued TASK_PROCESS_COMMANDS, sFromIp: " + sFromIp);
            }
          }
        }
      }
    }
  }
  else if (iCmd == CMto){
    // here, we expect that we are a master receiving this from a non-master
    prtln("got CMto...");
    if (!g_bMaster)
      prtln("non-master should not get a CMto command!");
    else{
      // get prepended ultimate destination ip address
      int idx = sData.indexOf(CM_SEP);
      if (idx >= 1){
        String sDestIp = sData.substring(0, idx);
prtln("ProcessCommand() CMto: ultimate destination sDestIp: " + sDestIp);
        sData = sData.substring(idx+1);
prtln("ProcessCommand() CMto: sData: \"" + sData + "\"");
        if (!sData.isEmpty() && !sDestIp.isEmpty()){
          IPAddress ipFrom = IML.GetIP(rxIdx);
          String sFromIp = ipFrom.toString();
          uint32_t uFromIp = (uint32_t)ipFrom;
          uint32_t uDestIp = B64C.hnDecNumOnly(sDestIp);
          String sFromData = B64C.hnEncNumOnly(uFromIp) + CM_SEP + sData; // repackage CMto as CMfrom prepending sending non-master's uint32_t IP
          if (uDestIp == 0){ // addressed to all units?
            TSK.QueueTask(TASK_PROCESS_COMMANDS, sData, sFromIp); // it's addressed to "all" which includes us!
prtln("ProcessCommand() CMto (all): this master queued TASK_PROCESS_COMMANDS, sFromIp: " + sFromIp);
            int iCount = IML.GetCount();
            if (iCount){
              // add CMfrom to sSendSpecific
prtln("ProcessCommand() CMto: Sending repackaged CMto as CMfrom to all units: \"" + sFromData + "\", sFromIp: " + sFromIp);
              for (int ii=0; ii < iCount; ii++)
                if (ii != rxIdx) // don't send back to unit we got it from!
                  HMC.AddCommand(CMfrom, sFromData, ii, MDNS_STRING_SEND_SPECIFIC);
            }
          }
          else{ // uDestIp > 0
            idx = IML.FindMdnsIp(uDestIp);
            if (idx >= 0){
              // here, we have verified that the destination non-master unit actually exists
prtln("ProcessCommand() CMto: Sending repackaged CMto as CMfrom to specific unit: \"" + sFromData +
                                                 "\", sFromIp: " + sFromIp + ", sDestIp: " + sDestIp);
              HMC.AddCommand(CMfrom, sFromData, idx, MDNS_STRING_SEND_SPECIFIC); // prepend sending non-master's IP and add to sSendSpecific
            }
            else if (g_bWiFiConnected && uDestIp == (uint32_t)GetLocalIp()){ // is it addressed to us, the master???
              TSK.QueueTask(TASK_PROCESS_COMMANDS, sData, sFromIp);
prtln("ProcessCommand() CMto: this master queued TASK_PROCESS_COMMANDS, sFromIp: " + sFromIp);
            }
          }
        }
      }
    }
  }
  else if (iCmd == CMtime){
    if (sData.length() == MAXTIMELEN){
      if (g_bSyncTime){
        TSK.QueueTask(TASK_SET_TIMEDATE, sData);
        prtln("ProcessCommand() queue task TASK_SET_TIMEDATE, got CMtime \"" + sData + "\"");
      }
      else
        prtln("ProcessCommand() got CMtime but sync-time is off! \"" + sData + "\"");
    }
    else
      prtln("ProcessCommand() got CMtime but bad length \"" + sData + "\"");
  }
  else
    return -1;
  return 0;
}

// set iStrIdx to MDNS_STRING_SEND_SPECIFIC or MDNS_STRING_SEND_ALL
// returns a command string that has base-64 encoded data fields - with data-fields decoded
String HttpMsgClass::DecodeBase64Commands(int mdnsIdx, int iStrIdx){
  return DecodeBase64Commands(IML.GetStr(mdnsIdx, iStrIdx));
}

String HttpMsgClass::DecodeBase64Commands(String sIn){
  String sCmd, sData, sOut;
  int idx = 0;
  for(;;){
    if (!ParseCommand(idx, sIn, sCmd, sData))
      break;
    sOut += sCmd + B64C.hnDecodeStr(sData, HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
  }
  return sOut;
}


// this routine can't just use indexOf() because two different commands
// such as "@" and "@#" would both be returned!
//int HttpMsgClass::FindCommand(int iCmd, String& sIn){
//  String sDataOut;
//  return FindCommand(GetCommandString(iCmd), sIn, sDataOut);
//}
//// returns >= 0 if iCmd (CMxx) was found. the base64 data is returned by-reference in sDataOut
//int HttpMsgClass::FindCommand(int iCmd, String& sIn, String& sDataOut){
//  return FindCommand(GetCommandString(iCmd), sIn, sDataOut);
//}
//int HttpMsgClass::FindCommand(String sCmd, String& sIn){
//  String sDataOut;
//  return FindCommand(sCmd, sIn, sDataOut);
//}
//// returns >= 0 if sCmd was found. the base64 data is returned by-reference in sDataOut
//int HttpMsgClass::FindCommand(String sCmd, String& sIn, String& sDataOut){
//  String sCmdTemp, sData;
//  int idx = 0;
//  int iCmd = 0;
//  for(;;){
//    iCmd = idx;
//    if (!ParseCommand(idx, sIn, sCmdTemp, sData))
//      break;
//    if (sCmdTemp == sCmd){
//      sDataOut = sData;
//      return iCmd;
//    }
//  }
//  sDataOut = "";
//  return -1;
//}

// return index of special range-command in sIn or -1 if not found
// (You can call this after calling MyDecodeStr())
int HttpMsgClass::FindRangeCommand(int cmdMin, int cmdMax, String& sIn){
  int len = sIn.length();
  for (int ii=0; ii < len; ii++){
    char c = sIn[ii];
    if (c <= CMSPECIALRANGESMAX && c >= cmdMin && c <= cmdMax)
      return ii;
  }
  return -1;
}

// strips a single-character range command. set iStrip to STRIP_ONE to just strip the first one.
// returns the base64 data for the first one stripped
// returns empty if nothing stripped or if no data was associated with the command(s) we stripped
// set iStrip to either STRIP_ALL or STRIP_FIRST (defaults to STRIP_ALL)
String HttpMsgClass::StripRangeCommand(int cmdMin, int cmdMax, String& sInOut, int iStrip){
  String sCmd, sData, sNew, sOut;  
  bool bOneStripped = false;
  int idx = 0;
  for(;;){
    if (!ParseCommand(idx, sInOut, sCmd, sData))
      break;
    char c = sCmd[0];
    bool bIsRangeCommand = c <= CMSPECIALRANGESMAX && sCmd.length() == 1 && c >= cmdMin && c <= cmdMax; 
    if (!bIsRangeCommand || (bOneStripped && iStrip == STRIP_FIRST))
      sNew += sCmd + sData;
    else if (!bOneStripped){
      sOut = sData;
      bOneStripped = true;
    }
  }
  
  sInOut = sNew; // return command-string, now stripped,  by reference
  return sOut;
}

// strips a command from sInOut. set iStrip to STRIP_FIRST to just strip the first one.
// returns the base64 data for the first one stripped
// returns empty if command not found
// set iStrip to either STRIP_ALL or STRIP_FIRST (defaults to STRIP_ALL)
String HttpMsgClass::StripCommand(int iCmd, String& sInOut, int iStrip){
  return StripCommand(GetCommandString(iCmd), sInOut, iStrip);
}
String HttpMsgClass::StripCommand(String sCmd, String& sInOut, int iStrip){
  String sCmdTemp, sData, sNew, sOut;
  bool bOneStripped = false;
  int idx = 0;
  for(;;){
    if (!ParseCommand(idx, sInOut, sCmdTemp, sData))
      break;
    if (sCmdTemp != sCmd || (bOneStripped && iStrip == STRIP_FIRST))
      sNew += sCmdTemp + sData;
    else if (!bOneStripped){
      sOut = sData;
      bOneStripped = true;
    }
  }
  
  sInOut = sNew; // return command-string, now stripped,  by reference
  return sOut;
}

// iIdx must point to the first character of a command string.
// strips command/data at iIdx from sInOut if bStrip = true
// returns the base64 data or empty-string if not found or error
String HttpMsgClass::StripCommandAtIndex(int iIdx, String& sInOut){
  return GetDataForCommandAtIndex(iIdx, sInOut, true);
}
// iIdx should point to the first char of the command for which we return its data
String HttpMsgClass::GetDataForCommandAtIndex(int iIdx, String& sInOut, bool bStrip){
  String sCmdTemp, sData;  
  int iCmd = iIdx;
  if (!ParseCommand(iCmd, sInOut, sCmdTemp, sData))
    return "";
  if (bStrip){
    String sFirst = sInOut.substring(0, iIdx);
    String sLast;
    if (iCmd < sInOut.length())
      sLast = sInOut.substring(iCmd);
    sInOut = sFirst + sLast;
  }
  // return data
  return sData;
}

// Gets the next sCmd/sData from sIn at idx. Data in sIn must be base-64. sData will be returned as base-64
// Note: commands consist of one or more non-base-64 characters. Data following a command
// is in base-64 characters and there must be at least one character.
// returns negative error-code or 0 if ok.
// returns sCmd/sData by-reference (returns idx by-reference which will point to the next command
// or be sIn.length() if finished). Does not alter sIn.
bool HttpMsgClass::ParseCommand(int& idx, String& sIn, String& sCmd, String& sData){
  int len = sIn.length();
  if (len < 2 || idx < 0 || idx >= len-2 || IsBase64Char(sIn[idx])) // bad index or no command-char
    return false;
    
  sCmd = "";
  sData = "";
  
  bool bBuildingCommand = true;
  for(; idx < len; idx++){
    char c = sIn[idx];
    if (IsBase64Char(c)){
      if (bBuildingCommand){
        if (sCmd.isEmpty())
          return false; // data before command
        bBuildingCommand = false;
      }
      sData += c;
    }
    else if (bBuildingCommand)
      sCmd += c;
    else // we have our command/data
      break;
  }
  if (bBuildingCommand) // no data for last command
    return false;
  if (sCmd.isEmpty() || sData.isEmpty())
    return false;
  return true;
}

// wrappers for GetBase64Param() that set bStrip true...

// this method will keep all base 64 data strings in sNew and add any from
// sOld that are not in sNew (except CMfrom/CMto/CMchangeReq/CMchangeData) and return the merged result.
// Do not pass a string that has CMtokenMin/Max or CMcs! 
String HttpMsgClass::MergeBase64Params(String sNew, String sOld){

  if (sOld.isEmpty())
    return sNew;

  // loop through commands in sNew and see if they are in sOld - if so delete them from sOld
  // and return sNew + sOld (keep old CMto/CMfrom unless exact duplicates)
  String sCmd, sData;
  int idx = 0;
  for(;;){
    if (!ParseCommand(idx, sNew, sCmd, sData))
      break;
    if (sCmd == GetCommandString(CMchangeData) || sCmd == GetCommandString(CMchangeReq) || sCmd == GetCommandString(CMto) || sCmd == GetCommandString(CMfrom)){
      int iDup = sOld.indexOf(sCmd+sData);
      if (iDup >= 0)
        StripCommandAtIndex(iDup, sOld);
    }
    else
      StripCommand(sCmd, sOld);
  }

  // remove old random-range commands for which we have a newer version (random range-commands increase security!)
  MergeSpecial(CMreqMacMin, CMreqMacMax, sNew, sOld);
  MergeSpecial(CMtokenMin, CMtokenMax, sNew, sOld);
  MergeSpecial(CMremPerMin, CMremPerMax, sNew, sOld);
  MergeSpecial(CMdummyMin, CMdummyMax, sNew, sOld);
  
  return sNew+sOld;
}

// if range-command exists in new string, search for it in old string and
// remove it from old string
void HttpMsgClass::MergeSpecial(int cmdMin, int cmdMax, String& sNew, String& sOld){
  int idx = FindRangeCommand(cmdMin, cmdMax, sNew);
  if (idx < 0)
    return;
  for(;;){
    idx = FindRangeCommand(cmdMin, cmdMax, sOld);
    if (idx < 0)
      break;
    StripCommandAtIndex(idx, sOld); // strip command/data from sOld
  }
}

bool HttpMsgClass::IsBase64Char(char c){
  return c == '-' || c == '_' || (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

// returns the command-string given a CMxx code
// NOTE: DO NOT PASS IN A RANGE COMMAND
String HttpMsgClass::GetCommandString(int iCmd){
  if (iCmd >= 0 && iCmd < TOTAL_HTTP_COMMANDS)
    return _HttpCommandTable[iCmd];
  return "";
}

// returns the CMxx code given a command-string
int HttpMsgClass::GetCommandIndex(String& sCmd){
  for (int ii=0; ii < TOTAL_HTTP_COMMANDS; ii++)
    if (sCmd == _HttpCommandTable[ii])
      return ii;
  return -1;
}

int HttpMsgClass::AddCommandAll(int iCmd){
  return AddHttpStringAll(FormCommand(iCmd, GetRandData()));
}

// Note: val is positive integer only: 143, 5324, Etc.
int HttpMsgClass::AddCommandAll(int iCmd, int val){
  return AddHttpStringAll(FormCommand(iCmd, val));
}

void HttpMsgClass::AddRangeCommand(int cmdMin, int cmdMax, int val, String &sInOut){
  sInOut += FormRangeCommand(cmdMin, cmdMax, val);
}

String HttpMsgClass::FormRangeCommand(int cmdMin, int cmdMax, int val){
  return String(char(random(cmdMin, cmdMax+1))) + B64C.hnEncodeStr(String(val), HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
}

String HttpMsgClass::FormRangeCommand(int cmdMin, int cmdMax, String sData){
  return String(char(random(cmdMin, cmdMax+1))) + B64C.hnEncodeStr(sData, HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
}

bool HttpMsgClass::AddCommand(int iCmd, int val, int mdnsIdx, int iStrIdx){
  return AddHttpString(FormCommand(iCmd, val), mdnsIdx, iStrIdx);
}

bool HttpMsgClass::AddCommand(int iCmd, String sData, int mdnsIdx, int iStrIdx){
  return AddHttpString(FormCommand(iCmd, sData), mdnsIdx, iStrIdx);
}

bool HttpMsgClass::AddCommand(int iCmd, int mdnsIdx, int iStrIdx){
  return AddHttpString(FormCommand(iCmd, GetRandData()), mdnsIdx, iStrIdx);
}

void HttpMsgClass::AddCommand(int iCmd, String &sInOut){
  sInOut += FormCommand(iCmd, GetRandData());
}

void HttpMsgClass::AddCommand(int iCmd, String sData, String &sInOut){
  sInOut += FormCommand(iCmd, sData);
}

void HttpMsgClass::AddCommand(int iCmd, int val, String &sInOut){
  sInOut += FormCommand(iCmd, val);
}

// called by masters to send CMchangeData to non-masters
// iChangeCmd is CD_CMD_TOKEN or CD_CMD_RECONNECT shifted left 4 and ored with
// an optional flag-mask CD_FLAG_SAVE_IN_FLASH
// returns count of remote units not including us...
int HttpMsgClass::AddCMchangeDataAll(int iChangeCmd, int iChangeFlags, String sChangeData){
  return AddCommandAll(CMchangeData, EncodeChangeData(iChangeCmd, iChangeFlags, sChangeData));
}

// called by non-masters to send CMchangeReq to master
bool HttpMsgClass::AddCMchangeReq(int iChangeCmd, int iChangeFlags, String sChangeData, int mdnsIdx, int iStrIdx){
  return AddHttpString(FormCommand(CMchangeReq, EncodeChangeData(iChangeCmd, iChangeFlags, sChangeData)), mdnsIdx, iStrIdx);
}

String HttpMsgClass::EncodeChangeData(int iChangeCmd, int iChangeFlags, String sChangeData){
  iChangeCmd <<= FLAG_COUNT_CD;
  iChangeFlags &= (1<<FLAG_COUNT_CD)-1;
  iChangeCmd |= iChangeFlags;
  if (sChangeData.isEmpty())
    sChangeData = CD_DATA_PLACE_HOLDER; // don't want empty field...
  return B64C.hnEncNumOnly(iChangeCmd) + CM_SEP + sChangeData;
}

// Note: sVal MUST be numeric, non-negative, base-10 such as "452", "37"...
// returns count of remote units not including us...
int HttpMsgClass::AddCommandAll(int iCmd, String sData){
  return AddHttpStringAll(FormCommand(iCmd, sData));
}

String HttpMsgClass::FormCommand(int iCmd, int val){
  return GetCommandString(iCmd) + B64C.hnEncodeStr(String(val), HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
}

String HttpMsgClass::FormCommand(int iCmd, String sData){
  return GetCommandString(iCmd) + B64C.hnEncodeStr(sData, HTTP_MSG_TABLE, HTTP_MSG_TOKEN);
}

// for each ESP32 in mDNS table, merge new parameters with those already waiting to be sent
// returns count of remote units not including us...
int HttpMsgClass::AddHttpStringAll(String sNew){
  int count = IML.GetCount();
  for (int ii=0; ii<count; ii++)
    AddHttpString(sNew, ii, MDNS_STRING_SEND_ALL);
  return count;
}

// Merge new base 64 data strings in sNew with pre-existing strings
// in the sSend string of a particular mDNS entry
// idx is mDNS table index
bool HttpMsgClass::AddHttpString(String sNew, int idx, int iStrIdx){
  int count = IML.GetCount();
  if (!count || idx < 0 || idx >= count) return false;
  
  String sSend = IML.GetStr(idx, iStrIdx);
//prtln("before MergeBase64Params(): \"" + sSend + "\"");
  sSend = MergeBase64Params(sNew, sSend);
//prtln("after MergeBase64Params(): \"" + sSend + "\"");
  IML.SetStr(idx, iStrIdx, sSend);
  return true;
}

String HttpMsgClass::GetRandData(){
  int idx = random(0, B64C.GetBase());
  char* p = B64C.GetTable();
  if (!p){
    prtln("HttpMsgClass::GetRandData() error!!!");
    return "0";
  }
  return String(p[idx]);
}

uint16_t HttpMsgClass::Sum(String sIn){
  int len = sIn.length();
  uint16_t sum = 0;
  for (int ii = 0; ii < len; ii++)
    sum += sIn[ii];
  return sum;
}

//int HttpMsgClass::AddRangeCommandAll(int cmdMin, int cmdMax){
//  return AddHttpStringAll(String(char(random(cmdMin, cmdMax+1))));
//}

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

//int HttpMsgClass::AddRangeCommandAll(int cmdMin, int cmdMax, String sVal){
//  return AddHttpStringAll(FormRangeCommand(cmdMin, cmdMax, sVal));
//}

//bool HttpMsgClass::AddCommand(int iCmd, IPAddress ip, int iStrIdx){
//  return AddHttpString(FormCommand(iCmd, GetRandData()), ip, iStrIdx);
//}

//bool HttpMsgClass::AddCommand(int iCmd, int val, IPAddress ip, int iStrIdx){
//  return AddHttpString(FormCommand(iCmd, val), ip, iStrIdx);
//}

//void HttpMsgClass::AddHttpString(String sNew, IPAddress ip, int iStrIdx){
//  int idx = IML.FindMdnsIp(ip);
//  if (idx >= 0)
//    AddHttpString(sNew, idx, iStrIdx);
//}

//int HttpMsgClass::FindAndStripCommand(String sCmd, String& sInOut){
//  int iOldIdx = FindCommand(sCmd, sInOut);
//  if (iOldIdx >= 0)
//    StripCommandAtIndex(iOldIdx, sInOut);
//  return iOldIdx;
//}

// removes all iCmd command+data from sInOut and returns a new string of only iCmd command+data
// and removing exact duplicates! (useful for extracting multiple CMto/CMfrom commands)
//String HttpMsgClass::ExtractAll(int iCmd, String& sInOut){
//
//  String sCmd = GetCommandString(iCmd);
//  if (sCmd.isEmpty())
//    return "";
//    
//  String sCmdTemp, sData, sNew, sOut;
//  int idx = 0;
//  for(;;){
//    if (!ParseCommand(idx, sInOut, sCmdTemp, sData))
//      break;
//    if (sCmdTemp = sCmd)
//      sOut += sCmdTemp + sData;
//    else
//      sNew += sCmdTemp + sData;
//  }
//
//  sInOut = sNew;
//  return sOut;
//}

// pass command string in sIn and 0-based idx should index the
// first command char associated with the data to be gleaned.
// The return string is base 64 (encoded with HTTP_MSG_TABLE, HTTP_MSG_TOKEN)
//String HttpMsgClass::GetDataForCommandAtIndex(int idx, String& sIn){
//  String sData;
//  int lenIn = sIn.length();
//  // get past the command-chars
//  int ii = idx+1;
//  for (; ii < lenIn; ii++)
//    if (IsBase64Char(sIn[ii]))
//      break;
//  // add the data chars to sData
//  for (; ii < lenIn; ii++){
//    char c = sIn[ii];
//    if (!IsBase64Char(c))
//      break;
//    sData += c;
//  }
//  return sData;
//}

// NOTE: A command-string's data is encoded as base-64. Only non-base-64 chars are used for commands. Commands
// are strings of non base-64 chars. Some commands are strings and others are in the range 1-31 (0 not allowed in a String!)
//bool HttpMsgClass::IsHttpCommand(String sCmd){
//
//  // commands 1-31 are broken up into four ranges: CMdummyMin-CMdummyMax, CMreqMacMin-CMreqMacMax
//  // CMtokenMin-CMtokenMax, CMremPerMin-CMremPerMax
//  int len = sCmd.length();
//
//  if (!len)
//    return false;
//  
//  if (len == 1 && IsRangeCmdChar(sCmd[0]))
//    return true;
//
//  for (int ii=0; ii < len; ii++)
//    if (IsBase64Char(sCmd[ii]))
//      return false;
//    
//  return true; // if no base-64 chars - it's a command!
//}

// commands 1-31 are broken up into four ranges: CMdummyMin-CMdummyMax, CMreqMacMin-CMreqMacMax
// CMtokenMin-CMtokenMax, CMremPerMin-CMremPerMax
//bool HttpMsgClass::IsRangeCmdChar(char c){
//  if (c > 0 && c <= CMSPECIALRANGESMAX)
//    return true;
//  return false;
//}
