// this file Cmd.cpp
#include "FanController.h"

// ------------------ USB command-line processing -------------------

// returns leading token in remCommand with leading/trailing spaces trimmed
// remCommand is returned by reference with leading token and following space removed
String GetSubCommand(String &remCommand){
  int len = remCommand.length();
  if (!len)
    return "";
  String subCommand, sRem;
  int ii=0;
  // bypass leading spaces
  for (; ii<len; ii++)
    if (remCommand[ii] != ' ')
      break;
  // extract desired subSubCommand
  for (; ii<len; ii++){
    char c = remCommand[ii];
    if (c == ' ')
      break;
    subCommand += c;
  }
  
  // return remaining portion by reference in remCommand for next call to this function...
  remCommand = remCommand.substring(ii+1);
  
  return subCommand;
}

// Expects a 1-based mDNS index or 0 if "us" or "all" if intended for all mDNS IPs
// returns -1 if sToken is presumed to be a command
// returns -2 if sToken is presumed to be mDNS index or IP address but is invalid
// returns a 1-based mDNS index, 0 if for us, mDNS count+1 if for all units
int GetMdnsIdxFromSubCommand(String sToken){
  int iIdx = -1; // presume sToken is a command (even if empty)
  int count = IML.GetCount();
  if (!sToken.isEmpty()){
    sToken.toLowerCase();
    if (sToken == "all"){
      if (count)
        iIdx = count+1;
      else
        iIdx = -2; // invalid
    }
    else if (isdigit(sToken[0])){
      if (alldigits(sToken)){
        int i = sToken.toInt();
        if (i == 0) // "us"
          iIdx = 0;
        else if (i > 0 && i < count)
          iIdx = i+1;
        else
          iIdx = -2; // invalid
      }
      else if (sToken.indexOf('.')){
        int i = IML.FindMdnsIp(sToken);
        if (i >= 0)
          iIdx = i+1;
        else
          iIdx = -2; // invalid
      }
    }
  }
  return iIdx;
}

// pass in: <sp><sp><sp>"<sp>hello<sp><sp><sp>world<sp><sp><sp>"<sp><sp><sp>
// get out:              <sp>hello<sp><sp><sp>world<sp><sp><sp>
//String RemoveQuotes(String sIn){
//  sIn.trim();
//  int len = sIn.length();
//  if (len >= 2){
//    if (sIn[0] == '"' && sIn[len-1] == '"'){
//      if (len == 2)
//        return "";
//      sIn = sIn.substring(1, len-1);
//    }
//  }
//  return sOut;
//}

String ProcessCommand(AsyncWebServerRequest *request, String &cmd){

  cmd = cmd.substring(2); // parse off the "C "

  String sToken = GetSubCommand(cmd);

  int iMdnsIdx = GetMdnsIdxFromSubCommand(sToken);

  if (iMdnsIdx == -2)
    return "invalid ip address or mDNS index!";
  
  int iMdnsCount = IML.GetCount();
  
  String sOut, sIp;
  
  String remCommand = cmd;
  if (iMdnsIdx == -1){ // sToken is not an index or ip-address, we presume it's a command...
    iMdnsIdx = 0; // a command for our local unit only!
    cmd = sToken;
  }
  else{ // sToken was a valid ip address or index (0=us, 1-iMdnsCount, iMdnsCount+1=all) so we must still get the command (next token!)
    if (iMdnsIdx > 0 && iMdnsIdx <= iMdnsCount)
      sIp = IML.GetIP(iMdnsIdx-1).toString(); // useful below...
    cmd = GetSubCommand(remCommand);
  }
  
  cmd.toLowerCase();
  //prtln("cmd: \"" + cmd + "\"");

  // Note: at this point, iMdnsIdx will have 0 if the command is for us only, a 1-based mDNS index or
  // mDNS count+1 if for "all" units!
  
  if (cmd == COMMAND_UNLOCK || cmd == COMMAND_LOCK){
    sOut = SetLockUnlockFromCommandString(cmd, remCommand);
  }
  else if (cmd == COMMAND_HELP){
    // here we implement the c help command for the serial port only...
    if (!request){
      PrintSpiffs(HELP2_FILENAME);
      PrintSpiffs(HELP1_FILENAME);
    }
  }
  else if (cmd == COMMAND_VERSION){
    sOut = String(VERSION_STR) + ", Unlocked " + String(g16_unlockCounter) + " times since power applied!";
  }
  else if (IsLocked()){
    // if locked...
    // allow update while unlocked if in AP mode... successful update in AP mode resets parms...
    if (g_bSoftAP && (cmd == COMMAND_UPDATE || cmd == COMMAND_UPLOAD)){
      if (request){
        // special case to send...
        // (NOTE: sOut will be returned empty which tells calling method NOT to send...)
        String s = "{\"a\":\"\", \"l\":\"\", \"o\":\"" + String(LOGIN_FILENAME) + "\"}";
        request->send(HTTPCODE_OK, "text/html", B64C.hnShiftEncode(s));
//        request->send(HTTPCODE_OK, "text/html", "<script>window.open('" + String(LOGIN_FILENAME) + "', '_self');</script>");
      }
    }
    else
      // this far and no farther! (interface is locked)
      sOut = "Interface is locked! (" + String(VERSION_STR) + ")";
  }
  // c update (select binary file for "over the air" update, fc.bin or fc.spiffs.bin)
  // (NOTE: Only useful when you set "ESP32_S3 true" in FanController.h - this program
  // no longer fits in the old 4MB ESP32 without eliminating OTA (Over-the-air) updates!)
  else if (cmd == COMMAND_UPDATE || cmd == COMMAND_UPLOAD){
    if (request){
      // special case to send...
      // (NOTE: sOut will be returned empty which tells calling method NOT to send...)
      String s = "{\"a\":\"\", \"l\":\"\", \"o\":\"" + String(LOGIN_FILENAME) + "\"}";
      request->send(HTTPCODE_OK, "text/html", B64C.hnShiftEncode(s));
      //request->send(HTTPCODE_OK, "text/html", "<script>window.open('" + String(LOGIN_FILENAME) + "', '_self');</script>");
    }
  }
  else if (cmd == COMMAND_PREFS){
    TSK.QueueTask(TASK_PRINT_PREFERENCES);
  }
  // c reset prefs (restore parameter defaults)
  // c reset slots (erase timeslots)
  // c reset wifi (restore wifi defaults)
  else if (cmd == COMMAND_RESET){
    if (remCommand == SC_RESET_SLOTS){
      TSK.QueueTask(TASK_RESET_SLOTS);
      sOut = "Resetting slots to defaults!";
    }
    else if (remCommand == SC_RESET_PREFS){
      TSK.QueueTask(TASK_RESET_PREFS);
      sOut = "Resetting main preferences to defaults!";
    }
    else if (remCommand == SC_RESET_WIFI){
      TSK.QueueTask(TASK_RESET_WIFI);
      sOut = "Resetting all wifi hostname, SSID and passwords to defaults!";
    }
    else
      sOut = "Invalid command!";
  }
  // c token [1-63] internal HTTP-sync security token - must be set the same on all units!
  else if (cmd == COMMAND_TOKEN){
    sOut = "Range: " + String(MIN_TOKEN) + "-" + String(MAX_TOKEN) + ". Current value: " + String(g_defToken);
    if (alldigits(remCommand)){
      int iVal = remCommand.toInt();
      if (iVal >= MIN_TOKEN && iVal <= MAX_TOKEN){
        if (QueueSynchronizedChange(CD_CMD_TOKEN, CD_FLAG_SAVE_IN_FLASH, String(iVal)))
          sOut = "Starting system-wide default token change to " + remCommand;
        else if (iVal != g_defToken){ // local token-change at this unit only!
          g_defToken = iVal;
          TSK.QueueTask(TASK_SETTOKEN); // set it in Preferences flash-memory
          sOut = "Setting local token to " + remCommand;
        }
        else
          sOut = remCommand + " was already set locally!";
      }
    }
  }
  // c maxpower [40-82] max WiFi power, 40=10dBm, 82=20dBm, 0.25dBm steps
  else if (cmd == COMMAND_MAX_POWER){
    sOut = "Range: " + String(MAX_POWER_MIN) + "-" + String(MAX_POWER_MAX) + ". Current value: " + String(g8_maxPower);
    if (alldigits(remCommand)){
      int iVal = remCommand.toInt();
      if (iVal >= MAX_POWER_MIN && iVal <= MAX_POWER_MAX){
        if (iVal != g8_maxPower){
          TSK.QueueTask(TASK_MAX_POWER, iVal);
          sOut = "Setting max power to " + remCommand;
        }
        else
          sOut = String(iVal) + " was already set!";
      }
    }
  }
  // c permax [1-65535] cycle-length (in perunits) when period is set to 100%
  else if (cmd == COMMAND_PERMAX){
    sOut = "Range: " + String(PERMAX_MIN) + "-" + String(PERMAX_MAX) + ". Current value: " + String(g_perVals.perMax);
    if (alldigits(remCommand)){
      int iVal = remCommand.toInt();
      if (iVal >= PERMAX_MIN && iVal <= PERMAX_MAX){
        if (iVal != g_perVals.perMax){
          TSK.QueueTask(TASK_PARMS, SUBTASK_PERMAX, iVal);
          sOut = "Setting permax to " + remCommand;
        }
        else
          sOut = String(iVal) + " was already set!";
      }
    }
  }
  // c perunits [0-3] units for permax, 0=1/2 sec, 1=sec, 2=min, 3=hrs
  else if (cmd == COMMAND_PERUNITS){
    sOut = "Range: " + String(PERUNITS_MIN) + "-" + String(PERUNITS_MAX) + ". Current value: " + String(g_perVals.perUnits);
    if (alldigits(remCommand)){
      int iVal = remCommand.toInt();
      if (iVal >= PERUNITS_MIN && iVal <= PERUNITS_MAX){
        if (iVal != g_perVals.perUnits){
          TSK.QueueTask(TASK_PARMS, SUBTASK_PERUNITS, iVal);
          sOut = "Setting perunits to " + GetPerUnitsString(iVal);
        }
        else
          sOut = String(iVal) + " was already set!";
      }
    }
  }
  // c period [0-100%] timing-cycle period, 0=random, channel A goes on at start of cycle
  else if (cmd == COMMAND_PERIOD){
    sOut = "Range: " + String(PERIOD_MIN) + "-" + String(PERIOD_MAX) + ". Current value: " + String(g_perVals.perVal);
    if (alldigits(remCommand)){
      int iVal = remCommand.toInt();
      if (iVal >= PERIOD_MIN && iVal <= PERIOD_MAX){
        if (iVal != g_perVals.perVal){
          TSK.QueueTask(TASK_PARMS, SUBTASK_PERVAL, iVal);
          sOut = "Setting period to " + GetPerDCString(iVal);
        }
        else
          sOut = String(iVal) + " was already set!";
      }
    }
  }
  // c dca [0-100] percent duty-cycle channel A, 0=random
  else if (cmd == COMMAND_DCA){
    sOut = "Range: " + String(DUTY_CYCLE_MIN) + "-" + String(DUTY_CYCLE_MAX) + ". Current value: " + String(g_perVals.dutyCycleA);
    if (alldigits(remCommand)){
      int iVal = remCommand.toInt();
      if (iVal >= DUTY_CYCLE_MIN && iVal <= DUTY_CYCLE_MAX){
        if (iVal != g_perVals.dutyCycleA){
          TSK.QueueTask(TASK_PARMS, SUBTASK_DCA, iVal);
          sOut = "Setting duty-cycle A to " + GetPerDCString(iVal);
        }
        else
          sOut = String(iVal) + " was already set!";
      }
    }
  }
  // c dcb [0-100] percent duty-cycle channel B, 0=random
  else if (cmd == COMMAND_DCB){
    sOut = "Range: " + String(DUTY_CYCLE_MIN) + "-" + String(DUTY_CYCLE_MAX) + ". Current value: " + String(g_perVals.dutyCycleB);
    if (alldigits(remCommand)){
      int iVal = remCommand.toInt();
      if (iVal >= DUTY_CYCLE_MIN && iVal <= DUTY_CYCLE_MAX){
        if (iVal != g_perVals.dutyCycleB){
          TSK.QueueTask(TASK_PARMS, SUBTASK_DCB, iVal);
          sOut = "Setting duty-cycle B to " + GetPerDCString(iVal);
        }
        else
          sOut = String(iVal) + " was already set!";
      }
    }
  }
  // c phase [0-100] percent phase, 100=random, channel B goes on at this point in cycle
  else if (cmd == COMMAND_PHASE){
    sOut = "Range: " + String(PHASE_MIN) + "-" + String(PHASE_MAX) + ". Current value: " + String(g_perVals.phase);
    if (alldigits(remCommand)){
      int iVal = remCommand.toInt();
      if (iVal >= PHASE_MIN && iVal <= PHASE_MAX){
        if (iVal != g_perVals.phase){
          TSK.QueueTask(TASK_PARMS, SUBTASK_PHASE, iVal);
          sOut = "Setting phase " + GetPhaseString(iVal);
        }
        else
          sOut = String(iVal) + " was already set!";
      }
    }
  }
  else if (cmd == COMMAND_SYNCHRONIZE){
    if (remCommand.isEmpty()){
      if (g_bSyncCycle){
        if (QueueSynchronizedChange(CD_CMD_CYCLE_PARMS, 0, PerValsToString(g_perVals))){
          g_oldPerVals = g_perVals; // don't send via CMdcA, CMphase, Etc.! We're sending all as a string!
          sOut = "Sending this unit's cycle-timing values to all units... this may take some time!";
        }
        else
          sOut = "Can't send cycle-timing values. Are we connected to WiFi? Is there anyone to send to?";
      }
      else
        sOut = "First type \"c sync on\" or \"c sync cycle on\"";
    }
    else
      sOut = "Did you mean \"c synchronize\"?";
  }
  // c timedate [2022-12-31T23:59:59] sets time and date, returns current time and date with Tsetok, Tnoset if error
  else if (cmd == COMMAND_TIMEDATE)
    sOut = "TimeDate: \"" + SetTimeDate(remCommand, true) + "\"";
  else if (cmd == COMMAND_SNTP){
    String subCommand = GetSubCommand(remCommand);
    if (subCommand == SC_SNTP_TIMEZONE){
      remCommand.toUpperCase();
      TSK.QueueTask(TASK_SET_SNTP_TIMEZONE, remCommand);
      sOut = "SNTP Timezone: \"" + remCommand + "\"";
    }
    else if (subCommand == SC_SNTP_INTERVAL){
      if (alldigits(remCommand)){
        TSK.QueueTask(TASK_SET_SNTP_INTERVAL, remCommand.toInt());
        sOut = "SNTP Interval: \"" + remCommand + "\"";
      }
      else
        sOut = "Invalid SNTP interval <0=off, 1-65535 hours>: " + remCommand;
    }
    else
      sOut = "Usage: \"c sntp interval <0=off, 1-65535 hours>\"";
  }
  else if (cmd == COMMAND_MAC){
    sOut = SetMacFromCommandString(remCommand);
  }
  else if (cmd == COMMAND_RELAY){
    sOut = SetRelayStateFromCommandString(remCommand);
  }
  else if (cmd == COMMAND_TEXT){
    sOut = SendTextFromCommandString(sIp, remCommand, iMdnsIdx, iMdnsCount);
  }
  else if (cmd == COMMAND_INFO){
    sOut = GetInfoFromCommandString(sIp, remCommand, iMdnsIdx, iMdnsCount);
  }
  else if (cmd == COMMAND_READ){
    sOut = ReadEEFromCommandString(sIp, remCommand, iMdnsIdx, iMdnsCount);
  }
  else if (cmd == COMMAND_WRITE){
    sOut = WriteEEFromCommandString(sIp, remCommand, iMdnsIdx, iMdnsCount);
  }
  else if (cmd == COMMAND_RESTART || cmd == COMMAND_REBOOT){
    sOut = RestartFromCommandString(sIp, remCommand, iMdnsIdx, iMdnsCount);
  }
  /* c key [up to 32 ANSI characters] AES cipher key. don't forget to turn encryption on: c sync encrypt on!
     hex escape allowed: \x9f, for \ use \\ */
  else if (cmd == COMMAND_CIPKEY){
    int len = remCommand.length();
    if (len > 0){
      remCommand = Unescape(remCommand);
      len = remCommand.length();
      if (len <= 0)
        sOut = "You can't use \\x00 in cipher key!";
      else if (len <= CIPKEY_MAX){
        TSK.QueueTask(TASK_SETCIPKEY, remCommand);
        sOut = "Setting new cipher key!";
      }
      else
        sOut = "Cipher Key is too long, max unescaped chars is: " + String(CIPKEY_MAX);
    }
    else
      // print the current cipher-key
      sOut = "Cipher Key <1-" + String(CIPKEY_MAX) +
          "> (hex escape allowed: \\x9f, for \\ use \\\\): \"" +
              g_sKey + "\" " + PrintCharsWithEscapes(g_sKey);
  }
  else if (cmd == COMMAND_TEST){
    String subCommand = GetSubCommand(remCommand);
    subCommand.toLowerCase();
    if (subCommand == SC_TEST_ON)
      g_bTest = true;
    else if (subCommand == SC_TEST_OFF)
      g_bTest = false;
    else
      sOut = "Usage: test on|off, ";
    sOut += "Test status: ";
    if (g_bTest)
      sOut += "on";
    else
      sOut += "off";
  }
  else if (cmd == COMMAND_LABEL){
    String subCommand = GetSubCommand(remCommand);
    subCommand.toLowerCase();
    if (subCommand == SC_LABEL_1){
      if (!remCommand.isEmpty())
        TSK.QueueTask(TASK_LABEL_A, remCommand);
      else
        // print the current label
        sOut = "Label 1: \"" + g_sLabelA + "\"";
    }
    else if (subCommand == SC_LABEL_2){
      if (!remCommand.isEmpty())
        TSK.QueueTask(TASK_LABEL_B, remCommand);
      else
        // print the current label
        sOut = "Label 2: \"" + g_sLabelB + "\"";
    }
  }
  else if (cmd == COMMAND_PULSE){
    sOut = SetPulseStateFromCommandString(remCommand);
  }
  else if (cmd == COMMAND_SYNC){
    sOut = SetSyncStateFromCommandString(remCommand);
  }
  else if (cmd == COMMAND_TIMESLOT){
    sOut = SetTimeSlotFromCommandString(remCommand);
  }
  else if (cmd == COMMAND_WIFI){
    sOut = SetWiFiStateFromCommandString(remCommand);
  }
  else
    sOut = "Invalid command!";

  return sOut;
}

// c mac (returns the current MAC)
// c mac 86:0D:8E:1A:11:A0 (sets new MAC address; bit 0 of 86 is multicast, do not set, bit 1 = locally administered, 1A, 11, A0 are vendor assigned)
// c mac reset (resets MAC to hardware default)
// c mac rand (generate random MACs each time - make sure your router is not filtering out unknown new MAC addresses!)
String SetMacFromCommandString(String& remCommand){
  String sOut;
  remCommand.toLowerCase();

  // if user just types "c mac" return the current MAC
  if (remCommand == ""){
    if (g_sMac == SC_MAC_RANDOM)
      sOut = "MAC mode is random, current MAC: " + WiFi.macAddress();
    else
      sOut = "MAC: " + WiFi.macAddress();
  }
  else if (remCommand == SC_MAC_RANDOM){
    g_sMac = remCommand;
    TSK.QueueTask(TASK_MAC);
    sOut = "Setting random MAC address mode...";
  }
  else if (remCommand == SC_MAC_RESET){
    g_sMac = "";
    TSK.QueueTask(TASK_MAC);
    sOut = "Restoring original hardware MAC address...";
  }
  else{
    uint8_t buf[6];
    if (MacStringToByteArray(remCommand.c_str(), buf) != NULL){
      g_sMac = remCommand;
      TSK.QueueTask(TASK_MAC);
      sOut = "Setting MAC address!";
// to preserve security, we don't want to send mac back! it's encoded coming this way but not on return.
//        sOut = "Setting MAC address: " + g_sMac;
    }
    else
      sOut = "Invalid MAC address (ex 84:0D:8E:1A:11:A0): " + remCommand;
  }
  return sOut;
}

// c 1-N|ip|all text
String SendTextFromCommandString(String& sIp, String& remCommand, int& iMdnsIdx, int& iMdnsCount){
  String sOut;
  if (iMdnsIdx == 0)
    // iMdnsIdx => 0=us, 1-iMdnsCount=remote unit, iMdnsCount+1=all remote units
    sOut = "Can't send text to ourself!";
  else if (g_bWiFiConnected){
    if (!iMdnsCount)
      sOut = "No remote units to send text!";
    else if (iMdnsIdx == iMdnsCount+1){
      int retCode = SendText(remCommand); // send to all IPs
      if (retCode > 0)
        sOut = "Text is being sent to " + String(retCode) + " remote unit(s)!";
      else
        sOut = "Failed to send text! code=" + String(retCode);
    }
    else{
      int retCode = SendText(iMdnsIdx-1, remCommand);
      if (retCode == 0)
        sOut = "Text being sent to: " + sIp;
      else
        sOut = "Failed to send text to: " + sIp + ", code=" + String(retCode);
    }
  }
  else
    sOut = "Must be connected to WiFi to send text!";
  return sOut;
}

// c info
String GetInfoFromCommandString(String& sIp, String& remCommand, int& iMdnsIdx, int& iMdnsCount){
  String sOut;
  String sCmd = String(CM_CMD_INFO); // cmd
  if (iMdnsIdx == 0){
    // iMdnsIdx => 0=us, 1-iMdnsCount=remote unit, iMdnsCount+1=all remote units
    sOut = GetStringInfo();
  }
  else if (g_bWiFiConnected){
    if (!iMdnsCount)
      sOut = "No remote units!";
    else if (iMdnsIdx == iMdnsCount+1){
      if (HMC.AddCommandAll(CMcmdReq, sCmd) <= 0)
        sOut = "Can't request";
      else
        sOut = "Requesting";
      sOut +=  " info from all remote units...";
    }
    else{
      if (!HMC.AddCommand(CMcmdReq, sCmd, iMdnsIdx-1, MDNS_STRING_SEND_SPECIFIC))
        sOut = "Can't request";
      else
        sOut = "Requesting";
      sOut +=  " info from: " + sIp;
    }
  }
  else
    sOut = "Must be connected to WiFi to get info from remote!";
  return sOut;
}

// c [optional 1-based mdns index or IP address, 0=this unit, all] restart [optional key] (restarts the processor - reboot)
// First do "c ip restart". The remote responds by printing a key... Now use "c ip restart key" to restart remote unit.
// c <optional idx|ip (0 is this unit, "all" is all units)> restart <optional key>
String RestartFromCommandString(String& sIp, String& remCommand, int& iMdnsIdx, int& iMdnsCount){
  String sOut;
  if (iMdnsIdx == 0){
    // iMdnsIdx => 0=us, 1-iMdnsCount=remote unit, iMdnsCount+1=all remote units
    if (!remCommand.isEmpty()){
      // here, the user has already armed local restart with "c restart". a restart verification key has been printed
      // on the console and the user has responded with "c restart <key>" - now, we validate the key and restart!
      if (remCommand == g_sRestartKey && g_sRestartIp == GetLocalIp().toString()){
        TSK.QueueTask(TASK_RESTART);
        sOut = "This unit is retarting...";
      }
      else
        sOut = "Bad restart key!";
    }
    else{
      // here, the user has requested local restart using "c restart". we print a key for them to enter
      // and are now "armed" for restart.
      g_sRestartKey = genRandMessage(RESTART_KEY_MIN_LENGTH, RESTART_KEY_MAX_LENGTH);
      g16_restartExpirationTimer = RESTART_EXPIRATION_TIME; 
      g_sRestartIp = GetLocalIp().toString();
      sOut = "Local restart armed. Type \"c restart " +
                                 g_sRestartKey + "\" for immediate local restart...";
    }
  }
  else if (g_bWiFiConnected){
    String sCmd = String(CM_CMD_RESTART)+CM_SEP+CM_CMD_RESTART_MAGIC;
    if (!iMdnsCount)
      sOut = "No remote units to restart!";
    else if (iMdnsIdx == iMdnsCount+1){
      if (remCommand.isEmpty()){
        if (HMC.AddCommandAll(CMcmdReq, sCmd) <= 0)
          sOut = "Failed to send";
        else
          sOut = "Sending";
        sOut += " CMreqCmd CM_CMD_RESTART to all remote units...";
      }
      else
        sOut = "\"c all restart <key>\" not allowed. Use \"c 1 restart key\", \"c 2 restart key\", Etc.";
    }
    else{
      if (!remCommand.isEmpty()){
        if (!HMC.AddCommand(CMrestart, remCommand, iMdnsIdx-1, MDNS_STRING_SEND_SPECIFIC))
          sOut = "Failed to send";
        else
          sOut = "Sending";
        sOut += " CMrestart to " + sIp + ". RestartKey = \"" + remCommand + "\"";
      }
      else{
        if (!HMC.AddCommand(CMcmdReq, sCmd, iMdnsIdx-1, MDNS_STRING_SEND_SPECIFIC))
          sOut = "Failed to send";
        else
          sOut = "Sending";
        sOut += " CMreqCmd CM_CMD_RESTART to " + sIp;
      }
    }
  }
  else
    sOut = "Must be connected to WiFi to restart remote!";
  return sOut;
}

// c [optional 1-based mdns index or IP address, 0=this unit, all] read [required key:optional spiffs namespace] (read value from preferences)
String ReadEEFromCommandString(String& sIp, String& remCommand, int& iMdnsIdx, int& iMdnsCount){
  String sOut;
  String sCmd = String(CM_CMD_READ)+CM_SEP+remCommand; // cmd:key:optional namespace
  if (iMdnsIdx == 0){
    // iMdnsIdx => 0=us, 1-iMdnsCount=remote unit, iMdnsCount+1=all remote units
    String sReadData = PC.GetPrefAsString(remCommand);
    if (!sReadData.isEmpty())
      sOut = "Read results for EE key:namespace \"" + remCommand + "\" = \"" + sReadData + "\"";
    else
      sOut = "No Read results for EE key:namespace \"" + remCommand + "\"!";
  }
  else if (g_bWiFiConnected){
    if (!iMdnsCount)
      sOut = "No remote units to read!";
    else if (iMdnsIdx == iMdnsCount+1){
      if (HMC.AddCommandAll(CMcmdReq, sCmd) <= 0)
        sOut = "Can't request";
      else
        sOut = "Requesting";
      sOut +=  " parameter: \"" + remCommand + "\" from all remote units...";
    }
    else{
      if (!HMC.AddCommand(CMcmdReq, sCmd, iMdnsIdx-1, MDNS_STRING_SEND_SPECIFIC))
        sOut = "Can't request";
      else
        sOut = "Requesting";
      sOut +=  " parameter: \"" + remCommand + "\" from: " + sIp;
    }
  }
  else
    sOut = "Must be connected to WiFi to read from remote!";
  return sOut;
}

// c [optional 1-based mdns index or IP address, 0=this unit, all] write [required key:optional spiffs namespace] [data] (write value to preferences)
String WriteEEFromCommandString(String& sIp, String& remCommand, int& iMdnsIdx, int& iMdnsCount){
  String sOut;
  String sKeyNs = GetSubCommand(remCommand); // key:optional namespace for SPIFFS (data to write remains in remCommand)
  String sDataAndKey = B64C.hnEncodeStr(remCommand)+CM_SEP+sKeyNs; // base 64 encode data to write so that it can ba "anything"
  String sCmd = String(CM_CMD_WRITE)+CM_SEP+sDataAndKey; // cmd:(data):key:optional namespace
  if (iMdnsIdx == 0){
    // iMdnsIdx => 0=us, 1-iMdnsCount=remote unit, iMdnsCount+1=all remote units
    // sKeyNs has eeKey or eeKey:eeNamespace (optional namespace, default is EE_PREFS_NAMESPACE)
    // data:key:optional namespace
    if (PC.SetPrefFromString(sDataAndKey)) // set flash-memory SPIFFS
      sOut = "Wrote to EE key:namespace ";
    else
      sOut = "Failed write to EE key:namespace ";
    sOut += "\"" + sKeyNs + "\" = \"" + remCommand + "\"";
  }
  else if (g_bWiFiConnected){
    if (!iMdnsCount)
      sOut = "No remote units to write!";
    else if (iMdnsIdx == iMdnsCount+1){
      // data:key:optional namespace
      if (HMC.AddCommandAll(CMcmdReq, sCmd) <= 0)
        sOut = "Can't write";
      else
        sOut = "Writing";
      sOut += " parameter: \"" + sKeyNs + "\" to all remote units...";
    }
    else{
      if (!HMC.AddCommand(CMcmdReq, sCmd, iMdnsIdx-1, MDNS_STRING_SEND_SPECIFIC))
        sOut = "Can't write";
      else
        sOut = "Writing";
      sOut +=  " parameter: \"" + sKeyNs + "\" = \"" + remCommand + "\" to: " + sIp;
    }
  }
  else
    sOut = "Must be connected to WiFi to write to remote!";
  return sOut;
}

// c lock (lock the interface when no passphrase is set)
// c lock "passphrase" (lock the interface when passphrase is set)
// c lock "passphrase" "passphrase" (set a passphrase CAUTION!!! WRITE IT DOWN AND SAVE!!!)
// c lock "old passphrase" "new passphrase" (change passphrase CAUTION!!! WRITE IT DOWN AND SAVE!!!)
// c lock "passphrase" "" (remove the passphrase and lock)
// c unlock (unlock the interface when no passphrase is set)
// c unlock "passphrase" (unlock the interface when passphrase is set)
String SetLockUnlockFromCommandString(String& sMainCmd, String& sCmd){
  String sOut;
  String subCommand = "";

  // parse off a possible sub-command
  bool bResetPwd = false;

  //prtln("sCmd: \"" + sCmd + "\"");

  // we require sub-command to be: word word word   or... "word word word" "word word word"
  // a trailing "" means clear-password
  int iQ1 = sCmd.indexOf('\"');
  int iQ2 = -1;
  int iQ3 = -1;
  int iQ4 = -1;
  if (iQ1 >= 0)
    iQ2 = sCmd.indexOf('\"', iQ1+1);
  if (iQ2 > 0)
    iQ3 = sCmd.indexOf('\"', iQ2+1);
  if (iQ3 > 0)
    iQ4 = sCmd.indexOf('\"', iQ3+1);

  // we allow: iQ1 = 0, iQ2 > 0, iQ3 > 0, iQ4 > 0
  // or iQ1 < 0. Anything else is invalid
  if (iQ1 >= 0 && iQ2 >= 0){
    if (iQ3 > 0 && iQ4 > 0){
      if (iQ1 == 0){ // "a b c" "d e f"
        if (iQ4 == iQ3+1)
          bResetPwd = true;
        else
          subCommand = sCmd.substring(iQ3+1, iQ4); // new pw
        sCmd = sCmd.substring(iQ1+1, iQ2); // old pw
      }
      else
        sCmd = ""; // reject command...
    }
    else if (iQ3 < 0 && iQ4 < 0){
      if (iQ1 == 0){ // "a b c" d e f
        if (iQ2 != iQ1+1){
          subCommand = sCmd.substring(iQ2+1); // new pw
          subCommand.trim(); // need this to avoid failing the check below!!!
          sCmd = sCmd.substring(iQ1+1, iQ2); // old pw
        }
        else
          sCmd = ""; // reject command...
      }
      else{ // a b c "d e f"
        if (iQ2 == iQ1+1)
          bResetPwd = true;
        else{
          subCommand = sCmd.substring(iQ1+1, iQ2); // new pw
          sCmd = sCmd.substring(0, iQ1); // old pw
          sCmd.trim(); // need this to avoid failing the check below!!!
        }
      }
    }
  }

  int len1 = sCmd.length();
  sCmd.trim();

  int len2 = subCommand.length();
  subCommand.trim();
  
  if (sCmd.length() != len1 || subCommand.length() != len2){
    sCmd = "";
    subCommand = "";
    bResetPwd = false;
    sOut = "Leading or trailing spaces in your password are not allowed!";
  }
  else{
    // convert \\ to \
    // convert \x05 Etc. to their character (\x01-\xff)
    sCmd = Unescape(sCmd);
    subCommand = Unescape(subCommand);
  }
  
  //if (remCommand.length())
  //  prtln("sCmd: \"" + sCmd + "\"");

  //if (subCommand.length())
  //  prtln("subCommand: \"" + subCommand + "\"");

  //if (bResetPwd)
  //  prtln("reset password");

  String currentPass = PC.GetWiFiString(EE_LOCKPASS, LOCKPASS_INIT);
  //prtln("current pass: \"" + currentPass + "\"");
  bool bPrintUsage = false;
  if (sMainCmd == COMMAND_LOCK){
    if (!IsLocked()){
      if (sCmd.isEmpty()){
        if (currentPass == LOCKPASS_INIT){
          g8_lockCount = 0;
          PC.PutWiFiU8(EE_LOCKCOUNT, g8_lockCount);
          sOut = "Interface locked!";
        }
        else
          bPrintUsage = true;
      }
      else{ // have a password typed-in!
        if (sCmd.length() > MAX_LOCKPASS_LENGTH || subCommand.length() > MAX_LOCKPASS_LENGTH)
          sOut = "Lock password max length is: " + String(MAX_LOCKPASS_LENGTH);
        else if (sCmd == subCommand && currentPass == LOCKPASS_INIT){
          // here we set password for system that's had password removed or that was never set
          PC.PutWiFiString(EE_LOCKPASS, sCmd);
          g8_lockCount = 0;
          PC.PutWiFiU8(EE_LOCKCOUNT, g8_lockCount);
          sOut = "Password set and interface locked!";
        }
        else if (sCmd == currentPass){
          // lock the system
          g8_lockCount = 0;
          PC.PutWiFiU8(EE_LOCKCOUNT, g8_lockCount);

          if (bResetPwd || subCommand == LOCKPASS_INIT){
            // here we remove the password
            PC.PutWiFiString(EE_LOCKPASS, LOCKPASS_INIT);
            sOut = "Password removed and interface locked!";
          }
          else if (subCommand.length() > 0){
            // here we set password for system that's had password removed or that was never set
            PC.PutWiFiString(EE_LOCKPASS, subCommand);
            sOut = "Password changed and interface locked!";
          }
          else // here we simply lock an unlocked system
            sOut = "Interface locked!";
        }
        else
          bPrintUsage = true;
      }
    }
    else
      sOut = "Interface already locked!";
  }
  else{ // c unlock
    if (IsLocked()){
      if (!bResetPwd && subCommand.isEmpty() && (currentPass == LOCKPASS_INIT || currentPass == sCmd)){
        g8_lockCount = 0xff;
        g16_unlockCounter++;
        PC.PutWiFiU8(EE_LOCKCOUNT, g8_lockCount);
        sOut = "Interface unlocked!";
      }
      else
        bPrintUsage = true;
    }
    else
      sOut = "Interface already unlocked!";
  }

  if (bPrintUsage)
    sOut = "c lock, c lock \"pass\" (lock), c lock \"pass\" \"pass\" (set pw), c lock \"pass\" \"\" (remove pw), c lock \"pass\" \"newpass\" (change pw)";

  return sOut;
}

// c pulse [a|b] [mode] [0=off,1=pulse-off,2=pulse-on,3=both]
// c pulse [a|b] [per|pw] [time in .25sec units]
String SetPulseStateFromCommandString(String& sCmd){
  String sOut;
  // c pulse a per min 10
  const char* sUsage = "Usage: c pulse a|b per|pw <time in .25sec units> OR c pulse a|b mode <0=off,1=pulse-off,2=pulse-on,3=both>";
  String sTemp = "Success! \"c " + String(COMMAND_PULSE) + ' ' + sCmd + '\"';
  String sChan = GetSubCommand(sCmd); // a/b
  sChan.toLowerCase();
  String sPerPwMode = GetSubCommand(sCmd); // per/pw/mode
  sPerPwMode.toLowerCase();
  String sMinMax = GetSubCommand(sCmd); // min/max (remCommand holds data after call)
  sMinMax.toLowerCase();
  int iVal;
  if (alldigits(sCmd))
    iVal = sCmd.toInt();
  else
    iVal = -1;
  if (sChan == SC_PULSE_A){
    if (sPerPwMode == SSC_PULSE_PER){
      if (sMinMax == SSSC_PULSE_MIN)
        g16_pulseMinPeriodA = iVal;
      else if (sMinMax == SSSC_PULSE_MAX)
        g16_pulseMaxPeriodA = iVal;
      else
        sOut = String(sUsage);
    }
    else if (sPerPwMode == SSC_PULSE_PW){
      if (sMinMax == SSSC_PULSE_MIN)
        g8_pulseMinWidthA = iVal;
      else if (sMinMax == SSSC_PULSE_MAX)
        g8_pulseMaxWidthA = iVal;
      else
        sOut = String(sUsage);
    }
    else if (sPerPwMode == SSC_PULSE_MODE){
      if (alldigits(sMinMax)) // our data will be in sMinMax, not in remCommand!
        iVal = sMinMax.toInt();
      else
        iVal = -1;
      if (iVal >= 0)
        g8_pulseModeA = iVal;
      else
        sOut = String(sUsage);
    }
    else
      sOut = String(sUsage);
  }
  else if (sChan == SC_PULSE_B){
    if (sPerPwMode == SSC_PULSE_PER){
      if (sMinMax == SSSC_PULSE_MIN)
        g16_pulseMinPeriodB = iVal;
      else if (sMinMax == SSSC_PULSE_MAX)
        g16_pulseMaxPeriodB = iVal;
      else
        sOut = String(sUsage);
    }
    else if (sPerPwMode == SSC_PULSE_PW){
      if (sMinMax == SSSC_PULSE_MIN)
        g8_pulseMinWidthB = iVal;
      else if (sMinMax == SSSC_PULSE_MAX)
        g8_pulseMaxWidthB = iVal;
      else
        sOut = String(sUsage);
    }
    else if (sPerPwMode == SSC_PULSE_MODE){
      if (alldigits(sMinMax)) // our data will be in sMinMax, not in remCommand!
        iVal = sMinMax.toInt();
      else
        iVal = -1;
      if (iVal >= 0)
        g8_pulseModeB = iVal;
      else
        sOut = String(sUsage);
    }
    else
      sOut = String(sUsage);
  }
  else
    sOut = String(sUsage);

  // sOut will be empty if no errors...
  if (sOut.isEmpty()){
    TSK.QueueTask(TASK_WRITE_PULSE_EE_VALUES);
    sOut = sTemp; // send back success message 
  }
  return sOut;
}

// c sync [on|off|rx|tx|time|cycle|encrypt] turn on|off sync between units. Examples: c sync encrypt on, c sync off
String SetSyncStateFromCommandString(String& sCmd){
  String sOut;
  const char MSG_HELP[] = "Usage: sync on|off or sync rx|tx|time|cycle|token|encrypt on|off";
  sCmd.toLowerCase(); // ok to set entire string lower-case...
  String subCommand = GetSubCommand(sCmd);
  if (sCmd.isEmpty()){
    if (subCommand == SSC_SYNC_ON){
      g_bSyncRx = true;
      g_bSyncTx = true;
      g_bSyncCycle = true;
      g_bSyncTime = true;
      g_bSyncEncrypt = true;
      TSK.QueueTask(TASK_SYNC);
      sOut = "Local sync, all functions on!";
    }
    else if (subCommand == SSC_SYNC_OFF){
      g_bSyncRx = false;
      g_bSyncTx = false;
      g_bSyncCycle = false;
      g_bSyncTime = false;
      g_bSyncEncrypt = false;
      TSK.QueueTask(TASK_SYNC);
      sOut = "Local sync, all functions off!";
    }
    else
      sOut = SyncFlagStatus() + MSG_HELP;
    return sOut;
  }
      
  int iFlag;
  if (sCmd == SSC_SYNC_OFF)
    iFlag = 0;
  else if (sCmd == SSC_SYNC_ON)
    iFlag = 1;
  else
    iFlag = 2;

  if (iFlag == 2)
    sOut = MSG_HELP;
  else{
    String sOnOff = (iFlag == 1) ? "on" : "off";
    if (subCommand == SC_SYNC_RX){
      g_bSyncRx = iFlag;
      TSK.QueueTask(TASK_SYNC);
      sOut = "Local rx sync: " + sOnOff;
    }
    else if (subCommand == SC_SYNC_TX){
      g_bSyncTx = iFlag;
      TSK.QueueTask(TASK_SYNC);
      sOut = "Local tx sync: " + sOnOff;
    }
    else if (subCommand == SC_SYNC_CYCLE){
      g_bSyncCycle = iFlag;
      TSK.QueueTask(TASK_SYNC);
      sOut = "Local cycle sync: " + sOnOff;
    }
    else if (subCommand == SC_SYNC_TIME){
      g_bSyncTime = iFlag;
      TSK.QueueTask(TASK_SYNC);
      sOut = "Local time sync: " + sOnOff;
    }
    else if (subCommand == SC_SYNC_ENCRYPT){
      g_bSyncEncrypt = iFlag;
      TSK.QueueTask(TASK_SYNC);
      sOut = "HTTP encrypt: " + sOnOff;
    }
    else
      sOut = MSG_HELP;
  }
  return sOut;
}

// c timeslot list (prints the slot's index (0-99) and data)
// c timeslot [slotindex] [format string - see below] replaces an existing timeslot
// c timeslot add [format string - see below]
//   A maximum of 100 time-events are allowed! Parameters may be separated by either a space or a comma.
//   Format example: (append a leading * if event is disabled (expired))
//   12/31/2020,12:59:59pm,AB:auto,r:inf,e:65535,t:min, (r is repeat count [1-65535|inf], e is repeat every [1-65535|off], t is repeat units (off|sec|min|hrs|day|wek|mon|yrs))
//   Optional cycle-timing: a:0-100,b:0-100,p:0-100,v:0-100,m:1-65535,u:0-3 (a and b are duty cycle, p is phase, v is period in percent)
//   (m is max-period 1-65535 of units, u is units [0=half-seconds, 1=seconds, 2=minutes, 3=hours])
//   Options: am|pm, A|B|AB, off|on|auto
//   i:y include cycle-timing, c:y ...in repeat events)
//   Setting 0 for a,b,v is random percentage
//   Setting 100 for p is random percentage
//   The actual cycle period is computed as (v*m)/100
// c timeslot del [slotindex]
// c timeslot delall
String SetTimeSlotFromCommandString(String& sCmd){
  String sOut;
  String subCommand = GetSubCommand(sCmd);
  subCommand.toLowerCase();
  if (subCommand == SC_TIMESLOT_ADD){
    t_event slotData = {0};
    if (TSC.StringToTimeSlot(sCmd, slotData)){
      if (TSC.AddTimeSlot(slotData))
        sOut = "Timeslot added!";
      else
        sOut = "Timeslot not added!";
    }
    else
      sOut = "Timeslot not added, to view proper format type: c help";
  }
  else if (subCommand == SC_TIMESLOT_DEL){
    int iVal = sCmd.toInt(); // 0-99 (get the index with SC_TIMESLOT_LIST
    if (iVal >= 0 && iVal < MAX_TIME_SLOTS){
      if (TSC.DeleteTimeSlot(iVal))
        sOut = "Timeslot deleted!";
      else
        sOut = "Timeslot not found!";
    }
  }
  else if (subCommand == SC_TIMESLOT_DELALL){
    TSK.QueueTask(TASK_RESET_SLOTS);
    sOut = "Erasing all timeslots!";
  }
  else if (subCommand == SC_TIMESLOT_LIST)
    sOut = TSC.PrintTimeSlots();
  else if (alldigits(subCommand)){ // c timeslot <index> <format string>... replace slot with new data
    int slotIndex = subCommand.toInt(); // 0-99 (get the index with SC_TIMESLOT_LIST
    if (slotIndex >= 0 && slotIndex < MAX_TIME_SLOTS){
      if (!TSC.IsTimeSlotEmpty(slotIndex)){
        t_event slotData = {0};
        if (TSC.StringToTimeSlot(sCmd, slotData)){
          if (TSC.PutTimeSlot(slotIndex, slotData))
            sOut = "Timeslot replaced!";
          else
            sOut = "Timeslot not replaced!";
        }
        else
          sOut = "Timeslot not replaced, to view proper format type: c help";
      }
      else
        sOut = "That timeslot is empty, use add instead!";
    }
  }
  else
    sOut = "Use: type c help for usage";
  return sOut;  
}

// c wifi toggle
// c wifi restore (restore wifi ssid/password defaults)
// c wifi aprestore (restore AP wifi ssid/password defaults)
// c wifi on (AP mode only, enable WiFi when AP/STA switch in STA position)
// c wifi off (AP mode only, disable WiFi when AP/STA switch in STA position)
// c wifi ssid [ssid 32 chars max]
// c wifi pass [password 64 chars max]
// c wifi host [new name 32 chars max[ sets the host name, do not add .local to end  
String SetWiFiStateFromCommandString(String& sCmd){
  String sOut;
  String subCommand = GetSubCommand(sCmd);
  subCommand.toLowerCase();
  if (subCommand == SC_WIFI_TOGGLE){
    TSK.QueueTask(TASK_WIFI_TOGGLE);
    sOut = "Restoring previous WiFi SSID and password!";
  }
  else if (subCommand == SC_WIFI_RESTORE){
    TSK.QueueTask(TASK_WIFI_RESTORE);
    sOut = "Setting factory WiFi SSID and password!";
  }
  else if (subCommand == SC_WIFI_AP_RESTORE){
    TSK.QueueTask(TASK_WIFI_AP_RESTORE);
    sOut = "Setting factory WiFi AP SSID and password!";
  }
  else if (subCommand == SC_WIFI_ON){
    if (g_bSoftAP){ // only permit this if logged in as access-point!
      TSK.QueueTask(TASK_WIFI_ON);
      g_bWiFiDisabled = false;
      sOut = "WiFi enabled for station mode!";
    }
    else
      sOut = "To enable WiFi in station mode, flip AP switch, log in as " + String(DEF_AP_SSID) + ", " + String(DEF_AP_PWD) + ".";
  }
  else if (subCommand == SC_WIFI_OFF){
    if (g_bSoftAP){ // only permit this if logged in as access-point!
      TSK.QueueTask(TASK_WIFI_OFF);
      g_bWiFiDisabled = true;
      sOut = "WiFi disabled for station mode!";
    }
    else
      sOut = "To disable WiFi in station mode, flip AP switch, log in as " + String(DEF_AP_SSID) + ", " + String(DEF_AP_PWD) + ".";
  }
  else{ // put all commands with data in remCommand here (c wifi ssid MySSID)
    if (subCommand == SC_WIFI_HOST)
      sOut = ProcessWiFiHostName(sCmd);
    else if (subCommand == SC_WIFI_SSID){
      int retCode = ProcessWifiSsid(sCmd);
      if (retCode == -2)
        sOut = "WiFI SSID must be 1-" + String(MAXSSID) + " chars!";
      else if (retCode == -1)
        sOut = "WiFi SSID not set! Present WiFi SSID is: \"" + sCmd + "\"!";
      else
        sOut = "WiFi SSID set!";
    }
    else if (subCommand == SC_WIFI_APSSID){
      int retCode = ProcessApWifiSsid(sCmd);
      if (retCode == -2)
        sOut = "WiFI AP SSID must be 1-" + String(MAXAPSSID) + " chars!";
      else if (retCode == -1)
        sOut = "WiFi AP SSID not set! Present WiFi AP SSID is: \"" + sCmd + "\"!";
      else
        sOut = "WiFi AP SSID set!";
    }
    else if (subCommand == SC_WIFI_PWD){
      int retCode = ProcessWifiPass(sCmd);
      if (retCode == -2)
        sOut = "WiFi password is max " + String(MAXPASS) + " chars!";
      else if (retCode == -1)
        sOut = "WiFi password not set! Present WiFi password is: \"" + sCmd + "\" " + PrintCharsWithEscapes(sCmd);
      else if (retCode == 2)
        sOut = "WiFI password removed!";
      else
        sOut = "WiFi password set!";
    }
    else if (subCommand == SC_WIFI_APPASS){
      int retCode = ProcessApWifiPass(sCmd);
      if (retCode == -2)
        sOut = "WiFi AP password is max " + String(MAXAPPASS) + " chars!";
      else if (retCode == -1)
        sOut = "WiFi AP password not set! Present WiFi AP password is: \"" + sCmd + "\" " + PrintCharsWithEscapes(sCmd);
      else if (retCode == 2)
        sOut = "WiFI AP password removed!";
      else
        sOut = "WiFi AP password set!";
    }
    else
      sOut = "Usage: c wifi ssid|pass|apssid|appass|host|on|off|toggle|restore!";
  }
  return sOut;
}

// Returns -2 if string too long, 0 if ok
// Returns -1 if valN is empty and returns present value by-reference
int ProcessWifiSsid(String &valN){
  int lenN = valN.length();
  if (lenN > MAXSSID)
    return -2;
  if (lenN == 0){
    valN = PC.GetWiFiString(EE_SSID, DEF_SSID);
    return -1;
  }
  if (valN != g_sSSID){
    PC.PutWiFiString(EE_SSID, valN); // save new ssid
    PC.PutWiFiString(EE_OLDSSID, g_sSSID); // backup current ssid
    g_sSSID = valN;
    //prtln("New SSID Stored: \"" + g_sSSID + "\"");
  }

  // obfuscate memory for security
  valN = OBFUSCATE_STR;
  lenN = 0;
  return 0;
}

// Returns -2 if string too long, 0 if ok
// Returns -1 if valN is empty and returns present value by-reference
int ProcessApWifiSsid(String &valN){
  int lenN = valN.length();
  if (lenN > MAXAPSSID)
    return -2;
  if (lenN == 0){
    valN = PC.GetWiFiString(EE_APSSID, DEF_AP_SSID);
    return -1;
  }
  if (valN != g_sApSSID){
    PC.PutWiFiString(EE_APSSID, valN); // save new ssid
    PC.PutWiFiString(EE_OLDAPSSID, g_sApSSID); // backup current ssid
    g_sApSSID = valN;
    //prtln("New AP SSID Stored: \"" + g_sApSSID + "\"");
  }

  // obfuscate memory for security
  valN = OBFUSCATE_STR;
  lenN = 0;
  return 0;
}

// Returns -2 if string too long, 0 if ok, 2 if password removed
// Returns -1 if valN is empty and returns present value by-reference
int ProcessWifiPass(String &valP){
  int lenP = valP.length();
  valP = Unescape(valP);
  lenP = valP.length();
  if (lenP > MAXPASS)
    return -2;
  if (lenP == 0){
    valP = PC.GetWiFiString(EE_PWD, DEF_PWD);
    return -1;
  }
  
  int retVal;
  if (valP == "\"\""){ // c wifi pass "" (removes the password)
    valP = "";
    retVal = 2;
  }
  else
    retVal = 0;

  String oldPwd = PC.GetWiFiString(EE_PWD, DEF_PWD);
  
  if (valP != oldPwd){
    PC.PutWiFiString(EE_OLDPWD, oldPwd); // save current pwd
    PC.PutWiFiString(EE_PWD, valP); // save new pwd
    //prtln("New WiFi pass Stored: \"" + valP + "\"");
  }

  // obfuscate memory for security
  oldPwd = OBFUSCATE_STR;
  valP = OBFUSCATE_STR;
  lenP = 0;
  return retVal;
}

// Returns -2 if string too long, 0 if ok, 2 if password removed
// Returns -1 if valN is empty and returns present value by-reference
int ProcessApWifiPass(String &valP){
  int lenP = valP.length();
  valP = Unescape(valP);
  lenP = valP.length();
  if (lenP > MAXAPPASS)
    return -2;
  if (lenP == 0){
    valP = PC.GetWiFiString(EE_APPWD, DEF_AP_PWD);
    return -1;
  }
  
  int retVal;
  if (valP == "\"\""){ // c wifi appass "" (removes the password)
    valP = "";
    retVal = 2;
  }
  else
    retVal = 0;

  String oldPwd = PC.GetWiFiString(EE_APPWD, DEF_AP_PWD);
  
  if (valP != oldPwd){
    PC.PutWiFiString(EE_OLDAPPWD, oldPwd); // save current pwd
    PC.PutWiFiString(EE_APPWD, valP); // save new pwd
    //prtln("New WiFi AP pass Stored: \"" + valP + "\"");
  }

  // obfuscate memory for security
  oldPwd = OBFUSCATE_STR;
  valP = OBFUSCATE_STR;
  lenP = 0;
  return retVal;
}

// c relay [1|2] print the relay's mode and state
// c relay [1|2] [on|off|auto]
String SetRelayStateFromCommandString(String& sCmd){
  String sOut;
  String subCommand = GetSubCommand(sCmd);
  sCmd.toLowerCase();
  if (subCommand == SC_RELAY_1){
    if (sCmd == SSC_RELAY_ON){
      if (g8_ssr1ModeFromWeb != SSR_MODE_ON){
        g8_ssr1ModeFromWeb = SSR_MODE_ON;
        SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
        TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
        sOut = "Relay 1: on!";
      }
      else
        sOut = "Relay 1 already on!";
    }
    else if (sCmd == SSC_RELAY_OFF){
      if (g8_ssr1ModeFromWeb != SSR_MODE_OFF){
        g8_ssr1ModeFromWeb = SSR_MODE_OFF;
        SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
        TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
        sOut = "Relay 1: off!";
      }
      else
        sOut = "Relay 1 already off!";
    }
    else if (sCmd == SSC_RELAY_AUTO){
      if (g8_ssr1ModeFromWeb != SSR_MODE_AUTO){
        g8_ssr1ModeFromWeb = SSR_MODE_AUTO;
        SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
        TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
        sOut = "Relay 1: auto!";
      }
      else
        sOut = "Relay 1 already auto!";
    }
    else{
      // print the relay's mode
      String sSsr1On = (g_devStatus & DEV_STATUS_1) ? "ON" : "OFF";
      sOut = "Relay 1 mode: " + SsrModeToString(g8_ssr1ModeFromWeb) + ", state: " + sSsr1On;
    }
  }
  else if (subCommand == SC_RELAY_2){
    if (sCmd == SSC_RELAY_ON){
      if (g8_ssr2ModeFromWeb != SSR_MODE_ON){
        g8_ssr2ModeFromWeb = SSR_MODE_ON;
        SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
        TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
        sOut = "Relay 2: on!";
      }
      else
        sOut = "Relay 2 already on!";
    }
    else if (sCmd == SSC_RELAY_OFF){
      if (g8_ssr2ModeFromWeb != SSR_MODE_OFF){
        g8_ssr2ModeFromWeb = SSR_MODE_OFF;
        SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
        TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
        sOut = "Relay 2: off!";
      }
      else
        sOut = "Relay 2 already off!";
    }
    else if (sCmd == SSC_RELAY_AUTO){
      if (g8_ssr2ModeFromWeb != SSR_MODE_AUTO){
        g8_ssr2ModeFromWeb = SSR_MODE_AUTO;
        SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
        TSK.QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
        sOut = "Relay 2: auto!";
      }
      else
        sOut = "Relay 2 already auto!";
    }
    else{
      // print the relay's mode
      String sSsr2On = (g_devStatus & DEV_STATUS_2) ? "ON" : "OFF";
      sOut = "Relay 2 mode: " + SsrModeToString(g8_ssr2ModeFromWeb) + ", state: " + sSsr2On;
    }
  }
  return sOut;
}

// returns a JavaScript message
// cmd has Name.local or Name
// sets the global g_sHostName var
String ProcessWiFiHostName(String sName){
  String sRet;
  
  // screen off ".local"
  int len = sName.length();
  String hn;

  // put new name in hn string, excluding the .local part
  for (int ii = 0; ii < len && ii <= MAXHOSTNAME; ii++){
    char c = sName[ii];
    if (c == '.')
      break;
    hn += c;
  }

  // set new name and queue it for writing to flash-memory
  if (hn != g_sHostName){
    len = hn.length();
    if (len >= 1 && len <= MAXHOSTNAME){
      g_sHostName = hn;
      sRet = "Changed hostname to: ";
      TSK.QueueTask(TASK_HOSTNAME);
    }
    else // send "not modified"
      sRet = "Error in host-name: ";
  }
  else
    sRet = "Name already set: ";
  sRet += g_sHostName;
  return sRet;
}

String ProcessWifiSsidPass(String &valN, String &valP){
  String sRet;
  int retCode = ProcessWifiSsid(valN);
  if (retCode == -1)
    sRet = "<script>alert('WiFi SSID not set! Present WiFi SSID is: \"" + valN + "\"!');";
  else if (retCode == -2)
    sRet = "<script>alert('SSID must be 1-" + String(MAXSSID) + " chars!');";
  else{
    retCode = ProcessWifiPass(valP);
    if (retCode == -1)
      sRet = "<script>alert('WiFi password not set! Present WiFi password is: + " + PrintCharsWithEscapes(valP) + "');";
    else if (retCode == -2)
      sRet = "<script>alert('password is max " + String(MAXPASS) + " chars!');";
    else
      sRet = "<script>alert('WiFi SSID and password set!');";
  }
  return sRet;
}

// https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/wifi.html
// uint8_t* softAPmacAddress(uint8_t* mac); set
// String softAPmacAddress(); read
// macMode is WIFI_IF_AP, WIFI_IF_STA
void ProcessMAC(wifi_interface_t macMode){
  randomSeed(esp_random());

  esp_wifi_set_vendor_ie(false, WIFI_VND_IE_TYPE_BEACON, WIFI_VND_IE_ID_0, NULL); // disabled

  // JP, US
  //default: {.cc=”CN”, .schan=1, .nchan=13, policy=WIFI_COUNTRY_POLICY_AUTO};
  //const wifi_country_t* country = {”USA”, 1, 11, WIFI_COUNTRY_POLICY_AUTO}; // WIFI_COUNTRY_POLICY_MANUAL
  wifi_country_t myCountry;
  if(esp_wifi_get_country(&myCountry) == ESP_OK){
    strcpy(myCountry.cc, WIFI_COUNTRY);
    myCountry.schan = WIFI_MIN_CHANNEL;
    myCountry.nchan = WIFI_MAX_CHANNEL;
    //myCountry.max_tx_power = g8_maxPower; // this is read-only. call esp_wifi_set_max_tx_power to set the maximum transmitting power
    //myCountry.policy = WIFI_COUNTRY_POLICY_AUTO;
    esp_err_t err = esp_wifi_set_country(&myCountry);
    if (err == ESP_OK)
      prtln("Country Code: " + String(myCountry.cc));
    //esp_err_t esp_wifi_set_max_tx_power(int8_t power) Set maximum transmitting power after WiFi start. 
   }

  //https://en.wikipedia.org/wiki/MAC_address#Universal_vs._local_(U/L_bit)
  //set MAC address
  g_sMac.toLowerCase();
  if (macMode == WIFI_IF_AP || g_sMac == SC_MAC_RANDOM)
    ProcessRandMAC(macMode);
  else if (g_sMac != ""){
    uint8_t buf[6];
    if (MacStringToByteArray(g_sMac.c_str(), buf) != NULL){
      esp_wifi_set_mac(macMode, buf);
      prtln("Custom MAC address: " + WiFi.macAddress());
    }
    else{ // invalid mac string, go to hardware MAC
      g_sMac = "";
      TSK.QueueTask(TASK_MAC);
      prtln("Invalid MAC address - reverting to original hardware MAC...");
    }
  }
  else
    prtln("Hardware MAC address: " + WiFi.macAddress());
}

// ESP_IF_WIFI_AP, ESP_IF_WIFI_STA - 1.0.4
// WIFI_IF_AP, WIFI_IF_STA - 2.0.2
void ProcessRandMAC(wifi_interface_t macMode){
  uint8_t buf[6];
  buf[0] = random(256) & 0xfe | 0x02; // clear multicast bit and set locally administered bit
  buf[1] = random(256);
  buf[2] = random(256);
  buf[3] = random(256);
  buf[4] = random(256);
  buf[5] = random(256);

// WiFI.
// String softAPmacAddress()
// String macAddress()
// isConnected()
// wifi_set_macaddr(SOFTAP_IF, buf);
// wifi_set_macaddr(STATION_IF, buf);
// esp_task_wdt_reset();

  esp_wifi_set_mac(macMode, buf);
  prtln("Rand MAC address: " + WiFi.macAddress());
}

void ProcessSerialCommand(String cmd){
  String s;
  if (cmd.length() > 2 && (cmd[0] == 'c' || cmd[0] == 'C') && cmd[1] == ' ')
    s = ProcessCommand(NULL, cmd);
  else if (IsLocked())
    s = "Interface is locked!";
  else
    s = "Bad Command!";

  prtln(s);
}
