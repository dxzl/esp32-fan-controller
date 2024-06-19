// this file Cmd.cpp
#include "FanController.h"

// ------------------ USB command-line processing -------------------

const char VERSION_STR[] = DTS_VERSION;

// "c mac random, c mac reset, c mac 0xe6, cd:43:a3:6e:b5
const char COMMAND_MAC[]     = "mac";
const char SC_MAC_RANDOM[]   = "rand";
const char SC_MAC_RESET[]    = "reset";

// type just "c relay 1" to print the relay mode and state
const char COMMAND_RELAY[]   = "relay";
const char SC_RELAY_1[]      = "1";
const char SC_RELAY_2[]      = "2";
const char SSC_RELAY_ON[]    = "on";
const char SSC_RELAY_OFF[]   = "off";
const char SSC_RELAY_AUTO[]  = "auto";

// set "on" this unit will broadcast messages of random length and content to all other of our devices
const char COMMAND_TEST[]    = "test";
const char SC_TEST_ON[]      = "on";
const char SC_TEST_OFF[]     = "off";

// type just "c label 1" to print the label
const char COMMAND_LABEL[]   = "label";
const char SC_LABEL_1[]      = "1";
const char SC_LABEL_2[]      = "2";

// you can type these commands into the HOSTNAME web-edit field and submit the command!

// erase a flash-memory segment...
const char COMMAND_RESET[]   = "reset";
const char SC_RESET_PREFS[]  = "prefs";
const char SC_RESET_SLOTS[]  = "slots";
const char SC_RESET_WIFI[]   = "wifi";

// these represent feature on/off bits related to the custom, secured communication between ESP32
// units via sending commands from the AsyncHTTPRequest_Generic.h library to the remote unit's ESPAsyncWebServer.h
// library
const char COMMAND_SYNC[]    = "sync";
const char SC_SYNC_RX[]      = "rx";
const char SC_SYNC_TX[]      = "tx";
const char SC_SYNC_CYCLE[]   = "cycle";
const char SC_SYNC_TOKEN[]   = "token";
const char SC_SYNC_TIME[]    = "time";
const char SC_SYNC_ENCRYPT[] = "encrypt";
const char SSC_SYNC_ON[]     = "on";
const char SSC_SYNC_OFF[]    = "off";

// cycle pulse-off/on
// independant timing cycle with period and pulse-width that can vary between min and max values.
// mode for each of channel A and B of OFF, On-to-Off, Off-to-On and Both. Timing units are .25Sec/tick
// Example commands: 
// c pulse a per 100 (sets pulse-period for channel A to 100/4 = 25sec)
// c pulse b pw 20 (sets pulse-width for channel B to 12/4 = 3sec)
// c pulse a mode 0 (turn off pulse-feature for channel A)
const char COMMAND_PULSE[]   = "pulse";
const char SC_PULSE_A[]      = "a";
const char SC_PULSE_B[]      = "b";
const char SSC_PULSE_PER[]   = "per";
const char SSC_PULSE_PW[]    = "pw";
const char SSC_PULSE_MODE[]  = "mode";
const char SSSC_PULSE_MIN[]  = "min";
const char SSSC_PULSE_MAX[]  = "max";

// NOTE: timeslot # <new slot> replaces a slot!
const char COMMAND_TIMESLOT[]   = "timeslot";
const char SC_TIMESLOT_ADD[]    = "add";
const char SC_TIMESLOT_DEL[]    = "del";
const char SC_TIMESLOT_DELALL[] = "delall";
const char SC_TIMESLOT_LIST[]   = "list";

const char COMMAND_WIFI[]    = "wifi";
const char SC_WIFI_TOGGLE[]  = "toggle";
const char SC_WIFI_RESTORE[] = "restore";
const char SC_WIFI_AP_RESTORE[] = "aprestore";
const char SC_WIFI_ON[]      = "on";
const char SC_WIFI_OFF[]     = "off";
const char SC_WIFI_SSID[]    = "ssid";
const char SC_WIFI_PWD[]    = "pass";
const char SC_WIFI_APSSID[]  = "apssid";
const char SC_WIFI_APPASS[]  = "appass";
const char SC_WIFI_HOST[]    = "host";

const char COMMAND_DCA[]      = "dca";
const char COMMAND_DCB[]      = "dcb";
const char COMMAND_HELP[]     = "help";
const char COMMAND_INFO[]     = "info";
const char COMMAND_LOCK[]     = "lock";
const char COMMAND_PERIOD[]   = "period";
const char COMMAND_PERMAX[]   = "permax";
const char COMMAND_PERUNITS[] = "perunits";
const char COMMAND_PHASE[]    = "phase";
const char COMMAND_RESTART[]  = "restart";
const char COMMAND_TEXT[]     = "text";
const char COMMAND_TIMEDATE[] = "timedate";
const char COMMAND_TOKEN[]    = "token";
const char COMMAND_CIPKEY[]   = "key";
const char COMMAND_UPDATE[]   = "update";
const char COMMAND_UPLOAD[]   = "upload";
const char COMMAND_UNLOCK[]   = "unlock";
const char COMMAND_VERSION[]  = "version";
const char COMMAND_PREFS[]    = "prefs";
const char COMMAND_MAX_POWER[] = "maxpower";

// returns subSubCommand and returns by-reference the input string with subSubCommand trimmed off
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
  ii++; // skip space
  for (; ii<len; ii++)
    sRem += remCommand[ii];
  remCommand = sRem;
  return subCommand;
}

String ProcessCommand(AsyncWebServerRequest *request, String &cmd){
  String sOut;

  cmd = cmd.substring(2); // parse off the "C "
  cmd.trim();

  String remCommand = "";
  int idx = cmd.indexOf(' '); // look for space after command...

  if (idx >= 0){
    remCommand = cmd.substring(idx+1);
    remCommand.trim();
    cmd = cmd.substring(0, idx);
  }

  cmd.toLowerCase();
  //prtln("cmd: \"" + cmd + "\"");

  if (cmd == COMMAND_UNLOCK || cmd == COMMAND_LOCK){
    String subCommand = "";

    // parse off a possible sub-command
    bool bResetPwd = false;

    //prtln("remCommand: \"" + remCommand + "\"");

    // we require sub-command to be: word word word   or... "word word word" "word word word"
    // a trailing "" means clear-password
    int iQ1 = remCommand.indexOf('\"');
    int iQ2 = -1;
    int iQ3 = -1;
    int iQ4 = -1;
    if (iQ1 >= 0)
      iQ2 = remCommand.indexOf('\"', iQ1+1);
    if (iQ2 > 0)
      iQ3 = remCommand.indexOf('\"', iQ2+1);
    if (iQ3 > 0)
      iQ4 = remCommand.indexOf('\"', iQ3+1);

    // we allow: iQ1 = 0, iQ2 > 0, iQ3 > 0, iQ4 > 0
    // or iQ1 < 0. Anything else is invalid
    if (iQ1 >= 0 && iQ2 >= 0){
      if (iQ3 > 0 && iQ4 > 0){
        if (iQ1 == 0){ // "a b c" "d e f"
          if (iQ4 == iQ3+1)
            bResetPwd = true;
          else
            subCommand = remCommand.substring(iQ3+1, iQ4); // new pw

          remCommand = remCommand.substring(iQ1+1, iQ2); // old pw
        }
        else
          remCommand = ""; // reject command...
      }
      else if (iQ3 < 0 && iQ4 < 0){
        if (iQ1 == 0){ // "a b c" d e f
          if (iQ2 != iQ1+1){
            subCommand = remCommand.substring(iQ2+1); // new pw
            subCommand.trim(); // need this to avoid failing the check below!!!
            remCommand = remCommand.substring(iQ1+1, iQ2); // old pw
          }
          else
            remCommand = ""; // reject command...
        }
        else{ // a b c "d e f"
          if (iQ2 == iQ1+1)
            bResetPwd = true;
          else{
            subCommand = remCommand.substring(iQ1+1, iQ2); // new pw
            remCommand = remCommand.substring(0, iQ1); // old pw
            remCommand.trim(); // need this to avoid failing the check below!!!
          }
        }
      }
    }

    int len1 = remCommand.length();
    int len2 = subCommand.length();
    remCommand.trim();
    subCommand.trim();
    if (remCommand.length() != len1 || subCommand.length() != len2){
      remCommand = "";
      subCommand = "";
      bResetPwd = false;
      sOut = "Leading or trailing spaces in your password are not allowed!";
    }

    //if (remCommand.length())
    //  prtln("remCommand: \"" + remCommand + "\"");

    //if (subCommand.length())
    //  prtln("subCommand: \"" + subCommand + "\"");

    //if (bResetPwd)
    //  prtln("reset password");

    String currentPass = PC.GetWiFiPrefString(EE_LOCKPASS, LOCKPASS_INIT);
    //prtln("current pass: \"" + currentPass + "\"");
    bool bPrintUsage = false;

    if (cmd == COMMAND_LOCK){
      if (!IsLocked()){
        if (remCommand.isEmpty()){
          if (currentPass == LOCKPASS_INIT){
            g8_lockCount = 0;
            PC.PutWiFiPrefByte(EE_LOCKCOUNT, g8_lockCount);
            sOut = "Interface locked!";
          }
          else
            bPrintUsage = true;
        }
        else{ // have a password typed-in!
          if (remCommand.length() > MAX_LOCKPASS_LENGTH || subCommand.length() > MAX_LOCKPASS_LENGTH)
            sOut = "Lock password max length is: " + String(MAX_LOCKPASS_LENGTH);
          else if (remCommand == subCommand && currentPass == LOCKPASS_INIT){
            // here we set password for system that's had password removed or that was never set
            PC.PutWiFiPrefString(EE_LOCKPASS, remCommand);
            g8_lockCount = 0;
            PC.PutWiFiPrefByte(EE_LOCKCOUNT, g8_lockCount);
            sOut = "Password set and interface locked!";
          }
          else if (remCommand == currentPass){
            // lock the system
            g8_lockCount = 0;
            PC.PutWiFiPrefByte(EE_LOCKCOUNT, g8_lockCount);

            if (bResetPwd || subCommand == LOCKPASS_INIT){
              // here we remove the password
              PC.PutWiFiPrefString(EE_LOCKPASS, LOCKPASS_INIT);
              sOut = "Password removed and interface locked!";
            }
            else if (subCommand.length() > 0){
              // here we set password for system that's had password removed or that was never set
              PC.PutWiFiPrefString(EE_LOCKPASS, subCommand);
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
        if (!bResetPwd && subCommand.isEmpty() && (currentPass == LOCKPASS_INIT || currentPass == remCommand)){
          g8_lockCount = 0xff;
          g16_unlockCounter++;
          PC.PutWiFiPrefByte(EE_LOCKCOUNT, g8_lockCount);
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
  }
  else if (cmd == COMMAND_HELP){
    // here we implement the c help command for the serial port only...
    if (!request){
      PrintSpiffs(HELP2_FILENAME);
      PrintSpiffs(HELP1_FILENAME);
    }
  }
  else if (cmd == COMMAND_INFO){
    sOut = "Unlocked " + String(g16_unlockCounter) + " times since power applied!";
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
    QueueTask(TASK_PRINT_PREFERENCES);
  }
  else if (cmd == COMMAND_VERSION){
    sOut = String(VERSION_STR) + ", " + GetStringIP();
  }
  else if (cmd == COMMAND_RESTART){
    QueueTask(TASK_RESTART);
    sOut = "Retarting...";
  }
  else if (cmd == COMMAND_RESET){
    if (remCommand == SC_RESET_SLOTS){
      QueueTask(TASK_RESET_SLOTS);
      sOut = "Resetting slots to defaults!";
    }
    else if (remCommand == SC_RESET_PREFS){
      QueueTask(TASK_RESET_PREFS);
      sOut = "Resetting main preferences to defaults!";
    }
    else if (remCommand == SC_RESET_WIFI){
      QueueTask(TASK_RESET_WIFI);
      sOut = "Resetting all wifi hostname, SSID and passwords to defaults!";
    }
    else
      sOut = "Invalid command!";
  }
  else if (cmd == COMMAND_TOKEN){
    if (alldigits(remCommand)){
      int iVal = remCommand.toInt();
      if (iVal >= MIN_TOKEN && iVal <= MAX_TOKEN){
        if (iVal != g_defToken){
          if (g_bWiFiConnected && HMC.AddTableCommandAll(CMtoken, iVal) > 0){
            IML.ClearAllTokOkFlags(); // cancel any preceding token-change operation...
            g_pendingDefToken = iVal;
            sOut = "Starting system-wide default token change to " + remCommand;
          }
          else{
            g_defToken = iVal;
            g_oldDefToken = g_defToken; // don't re-transmit it!
            g_pendingDefToken = NO_TOKEN;
            QueueTask(TASK_SETTOKEN);
            sOut = "Setting new token to " + remCommand + " for local unit only!";
          }
        }
        else
          sOut = remCommand + " was already set!";
      }
      else
        sOut = "Token range 1-63. Current token: " + String(g_defToken);
    }
    else
      sOut = "Use c token <1-63>, Current token: " + String(g_defToken);
  }
  else if (cmd == COMMAND_MAX_POWER){
    int iVal = remCommand.toInt();
    if (remCommand.length() > 0 && iVal >= MAX_POWER_MIN && iVal <= MAX_POWER_MAX){
      if (iVal != g8_maxPower){
        QueueTask(TASK_MAX_POWER, iVal);
        sOut = "Setting max power to " + String(iVal);
      }
      else
        sOut = String(iVal) + " was already set!";
    }
    else
      sOut = "maxPower range " + String(MAX_POWER_MIN) + "-" + String(MAX_POWER_MAX) + ". Current maxPower: " + String(g8_maxPower);
  }
  else if (cmd == COMMAND_PERMAX){
    int iVal = remCommand.toInt();
    if (remCommand.length() > 0 && iVal >= PERMAX_MIN && iVal <= PERMAX_MAX){
      if (iVal != g_perVals.perMax){
        QueueTask(TASK_PARMS, SUBTASK_PERMAX, iVal);
        sOut = "Setting permax to " + String(iVal);
      }
      else
        sOut = String(iVal) + " was already set!";
    }
    else
      sOut = "perMax range 1-65535. Current perMax: " + String(g_perVals.perMax);
  }
  else if (cmd == COMMAND_PERUNITS){
    int iVal = remCommand.toInt();
    if (remCommand.length() > 0 && iVal >= PERUNITS_MIN && iVal <= PERUNITS_MAX){
      if (iVal != g_perVals.perUnits){
        QueueTask(TASK_PARMS, SUBTASK_PERUNITS, iVal);
        sOut = "Setting perunits to " + GetPerUnitsString(iVal);
      }
      else
        sOut = String(iVal) + " was already set!";
    }
    else
      sOut = "perUnits range 0-3. Current perUnits: " + String(g_perVals.perUnits);
  }
  else if (cmd == COMMAND_PERIOD){
    int iVal = remCommand.toInt();
    if (remCommand.length() > 0 && iVal >= PERIOD_MIN && iVal <= PERIOD_MAX){
      if (iVal != g_perVals.perVal){
        QueueTask(TASK_PARMS, SUBTASK_PERVAL, iVal);
        sOut = "Setting period to " + GetPerDCString(iVal);
      }
      else
        sOut = String(iVal) + " was already set!";
    }
    else
      sOut = "perVal range 0(random)-100. Current perVal: " + String(g_perVals.perVal);
  }
  else if (cmd == COMMAND_DCA){
    int iVal = remCommand.toInt();
    if (remCommand.length() > 0 && iVal >= DUTY_CYCLE_MIN && iVal <= DUTY_CYCLE_MAX){
      if (iVal != g_perVals.dutyCycleA){
        QueueTask(TASK_PARMS, SUBTASK_DCA, iVal);
        sOut = "Setting duty-cycle A to " + GetPerDCString(iVal);
      }
      else
        sOut = String(iVal) + " was already set!";
    }
    else
      sOut = "dutyCycleA range 0(random)-100. Current dutyCycleA: " + String(g_perVals.dutyCycleA);
  }
  else if (cmd == COMMAND_DCB){
    int iVal = remCommand.toInt();
    if (remCommand.length() > 0 && iVal >= DUTY_CYCLE_MIN && iVal <= DUTY_CYCLE_MAX){
      if (iVal != g_perVals.dutyCycleB){
        QueueTask(TASK_PARMS, SUBTASK_DCB, iVal);
        sOut = "Setting duty-cycle B to " + GetPerDCString(iVal);
      }
      else
        sOut = String(iVal) + " was already set!";
    }
    else
      sOut = "dutyCycleB range 0(random)-100. Current dutyCycleB: " + String(g_perVals.dutyCycleB);
  }
  else if (cmd == COMMAND_PHASE){
    int iVal = remCommand.toInt();
    if (remCommand.length() > 0 && iVal >= PHASE_MIN && iVal <= PHASE_MAX){
      if (iVal != g_perVals.phase){
        QueueTask(TASK_PARMS, SUBTASK_PHASE, iVal);
        sOut = "Setting phase " + GetPhaseString(iVal);
      }
      else
        sOut = String(iVal) + " was already set!";
    }
    else
      sOut = "phase range 0-100(random). Current phase: " + String(g_perVals.phase);
  }
  else if (cmd == COMMAND_TIMEDATE)
    sOut = "TimeDate: \"" + SetTimeDate(remCommand) + "\"";
  else if (cmd == COMMAND_MAC){
    remCommand.toLowerCase();

    // if user just types "c mac" return the current MAC
    if (remCommand == ""){
      if (g_sMac == SC_MAC_RANDOM){
        sOut = "MAC mode is random, current MAC: " + WiFi.macAddress();
      }
      else{
        sOut = "MAC: " + WiFi.macAddress();
      }
    }
    else if (remCommand == SC_MAC_RANDOM){
      g_sMac = remCommand;
      QueueTask(TASK_MAC);
      sOut = "Setting random MAC address mode...";
    }
    else if (remCommand == SC_MAC_RESET){
      g_sMac = "";
      QueueTask(TASK_MAC);
      sOut = "Restoring original hardware MAC address...";
    }
    else{
      uint8_t buf[6];
      if (MacStringToByteArray(remCommand.c_str(), buf) != NULL){
        g_sMac = remCommand;
        QueueTask(TASK_MAC);
        sOut = "Setting MAC address!";
// to preserve security, we don't want to send mac back! it's encoded coming this way but not on return.
//        sOut = "Setting MAC address: " + g_sMac;
      }
      else
        sOut = "Invalid MAC address (ex 84:0D:8E:1A:11:A0): " + remCommand;
    }
  }
  else if (cmd == COMMAND_RELAY){
    String subCommand = GetSubCommand(remCommand);
    remCommand.toLowerCase();
    if (subCommand == SC_RELAY_1){
      if (remCommand == SSC_RELAY_ON){
        if (g8_ssr1ModeFromWeb != SSR_MODE_ON){
          g8_ssr1ModeFromWeb = SSR_MODE_ON;
          SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
          sOut = "Relay 1: on!";
        }
        else
          sOut = "Relay 1 already on!";
      }
      else if (remCommand == SSC_RELAY_OFF){
        if (g8_ssr1ModeFromWeb != SSR_MODE_OFF){
          g8_ssr1ModeFromWeb = SSR_MODE_OFF;
          SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
          sOut = "Relay 1: off!";
        }
        else
          sOut = "Relay 1 already off!";
      }
      else if (remCommand == SSC_RELAY_AUTO){
        if (g8_ssr1ModeFromWeb != SSR_MODE_AUTO){
          g8_ssr1ModeFromWeb = SSR_MODE_AUTO;
          SetSSRMode(GPOUT_SSR1, g8_ssr1ModeFromWeb);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_A);
          sOut = "Relay 1: auto!";
        }
        else
          sOut = "Relay 1 already auto!";
      }
      else{
        // print the relay's mode
        String sSsr1On = g_bSsr1On ? "ON" : "OFF";
        sOut = "Relay 1 mode: " + SsrModeToString(g8_ssr1ModeFromWeb) + ", state: " + sSsr1On;
      }
    }
    else if (subCommand == SC_RELAY_2){
      if (remCommand == SSC_RELAY_ON){
        if (g8_ssr2ModeFromWeb != SSR_MODE_ON){
          g8_ssr2ModeFromWeb = SSR_MODE_ON;
          SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
          sOut = "Relay 2: on!";
        }
        else
          sOut = "Relay 2 already on!";
      }
      else if (remCommand == SSC_RELAY_OFF){
        if (g8_ssr2ModeFromWeb != SSR_MODE_OFF){
          g8_ssr2ModeFromWeb = SSR_MODE_OFF;
          SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
          sOut = "Relay 2: off!";
        }
        else
          sOut = "Relay 2 already off!";
      }
      else if (remCommand == SSC_RELAY_AUTO){
        if (g8_ssr2ModeFromWeb != SSR_MODE_AUTO){
          g8_ssr2ModeFromWeb = SSR_MODE_AUTO;
          SetSSRMode(GPOUT_SSR2, g8_ssr2ModeFromWeb);
          QueueTask(TASK_PARMS, SUBTASK_RELAY_B);
          sOut = "Relay 2: auto!";
        }
        else
          sOut = "Relay 2 already auto!";
      }
      else{
        // print the relay's mode
        String sSsr2On = g_bSsr2On ? "ON" : "OFF";
        sOut = "Relay 2 mode: " + SsrModeToString(g8_ssr2ModeFromWeb) + ", state: " + sSsr2On;
      }
    }
  }
  else if (cmd == COMMAND_CIPKEY){
    int len = remCommand.length();
    if (len > 0 && len <= CIPKEY_MAX)
      QueueTask(TASK_SETCIPKEY, remCommand);
    else
      // print the current cipher-key
      sOut = "Cipher Key <1-" + String(CIPKEY_MAX) + ">: \"" + g_sKey + "\"";
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
        QueueTask(TASK_LABEL_A, remCommand);
      else
        // print the current label
        sOut = "Label 1: \"" + g_sLabelA + "\"";
    }
    else if (subCommand == SC_LABEL_2){
      if (!remCommand.isEmpty())
        QueueTask(TASK_LABEL_B, remCommand);
      else
        // print the current label
        sOut = "Label 2: \"" + g_sLabelB + "\"";
    }
  }
  else if (cmd == COMMAND_TEXT){
    String subCommand = GetSubCommand(remCommand);
    int retCode = -1;
    int count = IML.GetCount();
    if (g_bWiFiConnected && count > 0 && !subCommand.isEmpty() && !remCommand.isEmpty()){
      if (alldigits(subCommand)){
        int idx = subCommand.toInt();
        if (idx >= 0 && idx <= count){
          if (idx == 0){
            retCode = SendText(remCommand); // send to all IPs
            if (retCode > 0)
              sOut = "Text is being sent to " + String(retCode) + " remote unit(s)!";
            else
              sOut = "Failed to send text! code=" + String(retCode);
          }else{
            retCode = SendText(idx-1, remCommand);
            if (retCode == 0)
              sOut = "Text being sent to: " + IML.GetIP(idx-1).toString();
            else
              sOut = "Failed to send text to: " + subCommand + ", code=" + String(retCode);
          }
        }
      }else{
        if (subCommand == "all"){
          retCode = SendText(remCommand); // send to all IPs
          if (retCode > 0)
            sOut = "Text is being sent to " + String(retCode) + " remote unit(s)!";
          else
            sOut = "Failed to send text! code=" + String(retCode);
        }else{
          retCode = SendText(subCommand, remCommand);
          if (retCode == 0)
            sOut = "Text being sent to: " + IML.GetIP(idx-1).toString();
          else
            sOut = "Failed to send text to: " + subCommand + ", code=" + String(retCode);
        }
      }
    }
    else
      sOut = "Failed to send text! MAXTXTLEN=" + String(MAXTXTLEN) + ", code=" + String(retCode);
  }
  else if (cmd == COMMAND_PULSE){
    // c pulse a per min 10
    const char* sUsage = "Usage: c pulse a|b per|pw <time in .25sec units> OR c pulse a|b mode <0=off,1=pulse-off,2=pulse-on,3=both>";
    String sTemp = "Success! \"c " + cmd + ' ' + remCommand + '\"';
//prtln("remCommand: \"" + remCommand + "\"");
    String sChan = GetSubCommand(remCommand); // a/b
    sChan.toLowerCase();
//prtln(" sChan: \"" + sChan + "\"");
//prtln(" remCommand: \"" + remCommand + "\"");
    String sPerPwMode = GetSubCommand(remCommand); // per/pw/mode
    sPerPwMode.toLowerCase();
//prtln("  sPerPwMode: \"" + sPerPwMode + "\"");
//prtln("  remCommand: \"" + remCommand + "\"");
    String sMinMax = GetSubCommand(remCommand); // min/max (remCommand holds data after call)
    sMinMax.toLowerCase();
//prtln("   sMinMax: \"" + sMinMax + "\"");
//prtln("   remCommand: \"" + remCommand + "\"");
    int iVal;
    if (alldigits(remCommand))
      iVal = remCommand.toInt();
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
      QueueTask(TASK_WRITE_PULSE_EE_VALUES);
      sOut = sTemp; // send back success message 
    }
  }
  else if (cmd == COMMAND_SYNC){
    const char MSG_HELP[] = "Usage: sync on|off or sync rx|tx|time|cycle|token|encrypt on|off";
    remCommand.toLowerCase(); // ok to set entire string lower-case...
    String subCommand = GetSubCommand(remCommand);
    if (remCommand.isEmpty()){
      if (subCommand == SSC_SYNC_ON){
        g_bSyncRx = true;
        g_bSyncTx = true;
        g_bSyncCycle = true;
        g_bSyncToken = true;
        g_bSyncTime = true;
        g_bSyncEncrypt = true;
        QueueTask(TASK_SYNC);
        sOut = "Local sync, all functions on!";
      }else if (subCommand == SSC_SYNC_OFF){
        g_bSyncRx = false;
        g_bSyncTx = false;
        g_bSyncCycle = false;
        g_bSyncToken = false;
        g_bSyncTime = false;
        g_bSyncEncrypt = false;
        QueueTask(TASK_SYNC);
        sOut = "Local sync, all functions off!";
      }
      else
        sOut = SyncFlagStatus() + MSG_HELP;
      return sOut;
    }
        
    int iFlag;
    if (remCommand == SSC_SYNC_OFF)
      iFlag = 0;
    else if (remCommand == SSC_SYNC_ON)
      iFlag = 1;
    else
      iFlag = 2;

    if (iFlag == 2)
      sOut = MSG_HELP;
    else{
      String sOnOff = (iFlag == 1) ? "on" : "off";
      if (subCommand == SC_SYNC_RX){
        g_bSyncRx = iFlag;
        QueueTask(TASK_SYNC);
        sOut = "Local rx sync: " + sOnOff;
      }
      else if (subCommand == SC_SYNC_TX){
        g_bSyncTx = iFlag;
        QueueTask(TASK_SYNC);
        sOut = "Local tx sync: " + sOnOff;
      }
      else if (subCommand == SC_SYNC_CYCLE){
        g_bSyncCycle = iFlag;
        QueueTask(TASK_SYNC);
        sOut = "Local cycle sync: " + sOnOff;
      }
      else if (subCommand == SC_SYNC_TOKEN){
        g_bSyncToken = iFlag;
        QueueTask(TASK_SYNC);
        sOut = "Local token sync: " + sOnOff;
      }
      else if (subCommand == SC_SYNC_TIME){
        g_bSyncTime = iFlag;
        QueueTask(TASK_SYNC);
        sOut = "Local time sync: " + sOnOff;
      }
      else if (subCommand == SC_SYNC_ENCRYPT){
        g_bSyncEncrypt = iFlag;
        QueueTask(TASK_SYNC);
        sOut = "HTTP encrypt: " + sOnOff;
      }
      else
        sOut = MSG_HELP;
    }
  }
  else if (cmd == COMMAND_TIMESLOT){
    String subCommand = GetSubCommand(remCommand);
    subCommand.toLowerCase();
    if (subCommand == SC_TIMESLOT_ADD){
      t_event slotData = {0};
      if (TSC.StringToTimeSlot(remCommand, slotData)){
        if (TSC.AddTimeSlot(slotData))
          sOut = "Timeslot added!";
        else
          sOut = "Timeslot not added!";
      }
      else
        sOut = "Timeslot not added, to view proper format type: c help";
    }
    else if (subCommand == SC_TIMESLOT_DEL){
      int iVal = remCommand.toInt(); // 0-99 (get the index with SC_TIMESLOT_LIST
      if (iVal >= 0 && iVal < MAX_TIME_SLOTS){
        if (TSC.DeleteTimeSlot(iVal))
          sOut = "Timeslot deleted!";
        else
          sOut = "Timeslot not found!";
      }
    }
    else if (subCommand == SC_TIMESLOT_DELALL){
      QueueTask(TASK_RESET_SLOTS);
      sOut = "Erasing all timeslots!";
    }
    else if (subCommand == SC_TIMESLOT_LIST){
      if (g_slotCount > 0){
        sOut = "Total timeslots: " + String(g_slotCount) + "\n";
        for (int ii = 0; ii < MAX_TIME_SLOTS; ii++){
          t_event slotData = {0};
          if (TSC.GetTimeSlot(ii, slotData)) // Get the time-slot into slotData by-reference
            sOut += "TimeSlot " + String(ii) + "=\"" + TSC.TimeSlotToSpaceSepString(slotData) + "\"\n";
        }
      }
    }
    else if (alldigits(subCommand)){ // c timeslot <index> <format string>... replace slot with new data
      int slotIndex = subCommand.toInt(); // 0-99 (get the index with SC_TIMESLOT_LIST
      if (slotIndex >= 0 && slotIndex < MAX_TIME_SLOTS){
        if (!TSC.IsTimeSlotEmpty(slotIndex)){
          t_event slotData = {0};
          if (TSC.StringToTimeSlot(remCommand, slotData)){
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
  }
  else if (cmd == COMMAND_WIFI){
    String subCommand = GetSubCommand(remCommand);
    subCommand.toLowerCase();
    if (subCommand == SC_WIFI_TOGGLE){
      QueueTask(TASK_WIFI_TOGGLE);
      sOut = "Restoring previous WiFi SSID and password!";
    }
    else if (subCommand == SC_WIFI_RESTORE){
      QueueTask(TASK_WIFI_RESTORE);
      sOut = "Setting factory WiFi SSID and password!";
    }
    else if (subCommand == SC_WIFI_AP_RESTORE){
      QueueTask(TASK_WIFI_AP_RESTORE);
      sOut = "Setting factory WiFi AP SSID and password!";
    }
    else if (subCommand == SC_WIFI_ON){
      if (g_bSoftAP){ // only permit this if logged in as access-point!
        QueueTask(TASK_WIFI_ON);
        g_bWiFiDisabled = false;
        sOut = "WiFi enabled for station mode!";
      }
      else
        sOut = "To enable WiFi in station mode, flip AP switch, log in as " + String(DEF_AP_SSID) + ", " + String(DEF_AP_PWD) + ".";
    }
    else if (subCommand == SC_WIFI_OFF){
      if (g_bSoftAP){ // only permit this if logged in as access-point!
        QueueTask(TASK_WIFI_OFF);
        g_bWiFiDisabled = true;
        sOut = "WiFi disabled for station mode!";
      }
      else
        sOut = "To disable WiFi in station mode, flip AP switch, log in as " + String(DEF_AP_SSID) + ", " + String(DEF_AP_PWD) + ".";
    }
    else{ // put all commands with data in remCommand here (c wifi ssid MySSID)
      if (subCommand == SC_WIFI_HOST)
        sOut = ProcessWiFiHostName(remCommand);
      else if (subCommand == SC_WIFI_SSID){
        if (remCommand.length() > 0){
          int retCode = ProcessWifiSsid(remCommand);
          if (retCode == -2)
            sOut = "WiFI SSID must be 1-" + String(MAXSSID) + " chars!";
          else if (retCode == 1)
            sOut = "WiFI SSID set to default: \"" + String(DEF_SSID) + "\"!";
          else
            sOut = "WiFi SSID set!";
        }
        else
          sOut = "WiFi SSID: \"" + g_sSSID + "\", Pass: \"" + PC.GetWiFiPrefString(EE_PWD, DEF_PWD) +"\"";
      }
      else if (subCommand == SC_WIFI_APSSID){
        if (remCommand.length() > 0){
          int retCode = ProcessApWifiSsid(remCommand);
          if (retCode == -2)
            sOut = "WiFI AP SSID must be 1-" + String(MAXAPSSID) + " chars!";
          else if (retCode == 1)
            sOut = "WiFI AP SSID set to default: \"" + String(DEF_AP_SSID) + "\"!";
          else
            sOut = "WiFi AP SSID set!";
        }
        else
          sOut = "WiFI AP SSID: \"" + g_sApSSID + "\", Pass: \"" + PC.GetWiFiPrefString(EE_APPWD, DEF_AP_PWD) +"\"";
      }
      else if (subCommand == SC_WIFI_PWD){
        int retCode = ProcessWifiPass(remCommand);
        if (retCode == -2)
          sOut = "WiFi password is max " + String(MAXPASS) + " chars!";
        else if (retCode == 1)
          sOut = "WiFI password set to default: \"" + String(DEF_PWD) + "\"!";
        else if (retCode == 2)
          sOut = "WiFI password removed!";
        else
          sOut = "WiFi password set!";
      }
      else if (subCommand == SC_WIFI_APPASS){
        int retCode = ProcessApWifiPass(remCommand);
        if (retCode == -2)
          sOut = "WiFi AP password is max " + String(MAXAPPASS) + " chars!";
        else if (retCode == 1)
          sOut = "WiFi AP password set to default: \"" + String(DEF_AP_SSID) + "\"!";
        else if (retCode == 2)
          sOut = "WiFI AP password removed!";
        else
          sOut = "WiFi AP password set!";
      }
      else
        sOut = "Usage: c wifi ssid|pass|apssid|appass|host|on|off|toggle|restore!";
    }
  }
  else
    sOut = "Invalid command!";

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
      QueueTask(TASK_HOSTNAME);
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
  if (retCode == -2)
    sRet = "<script>alert('SSID must be 1-" + String(MAXSSID) + " chars!');";
  else{
    retCode = ProcessWifiPass(valP);
    if (retCode == -2)
      sRet = "<script>alert('password is max " + String(MAXPASS) + " chars!');";
    else if (retCode == 1)
      sRet = "<script>alert('WiFi SSID set and password set to default!');";
    else
      sRet = "<script>alert('WiFi SSID and password set!');";
  }
  return sRet;
}

//String ProcessApWifiSsidPass(String &valN, String &valP){
//  String sRet;
//  int retCode = ProcessApWifiSsid(valN);
//  if (retCode == -2)
//    sRet = "<script>alert('WiFI AP SSID must be 1-" + String(MAXAPSSID) + " chars!');";
//  else if (retCode == 1)
//    sRet = "<script>alert('WiFi AP SSID set and password set to default!');";
//  else{
//    retCode = ProcessApWifiPass(valP);
//    if (retCode == -2)
//      sRet = "<script>alert('AP password is max " + String(MAXAPPASS) + " chars!');";
//    else if (valP.isEmpty())
//      sRet = "<script>alert('WiFi AP SSID set and password set to default!');";
//    else
//      sRet = "<script>alert('WiFi AP SSID and password set!');";
//  }
//  return sRet;
//}

// Returns -2 if string too long, 0 if ok, 1 if default set
int ProcessWifiSsid(String &valN){
  int lenN = valN.length();
  if (lenN > MAXSSID)
    return -2;
//  if (valN[0] == ' ' || valN[lenN-1] == ' ')
//    return -3;
  int retVal;
  if (lenN == 0){
    valN = String(DEF_SSID);
    retVal = 1;
  }
  else{
    retVal = 0;
  }
  if (valN != g_sSSID){
    PC.PutWiFiPrefString(EE_SSID, valN); // save new ssid
    PC.PutWiFiPrefString(EE_OLDSSID, g_sSSID); // backup current ssid
    g_sSSID = valN;
    //prtln("New SSID Stored: \"" + g_sSSID + "\"");
  }

  // obfuscate memory for security
  valN = OBFUSCATE_STR;
  lenN = 0;
  return retVal;
}

// Returns -2 if string too long, 0 if ok, 1 if default set
int ProcessApWifiSsid(String &valN){
  int lenN = valN.length();
  if (lenN > MAXAPSSID)
    return -2;
//  if (valN[0] == ' ' || valN[lenN-1] == ' ')
//    return -3;
  int retVal;
  if (lenN == 0){
    valN = String(DEF_AP_SSID);
    retVal = 1;
  }
  else{
    retVal = 0;
  }
  if (valN != g_sApSSID){
    PC.PutWiFiPrefString(EE_APSSID, valN); // save new ssid
    PC.PutWiFiPrefString(EE_OLDAPSSID, g_sApSSID); // backup current ssid
    g_sApSSID = valN;
    //prtln("New AP SSID Stored: \"" + g_sApSSID + "\"");
  }

  // obfuscate memory for security
  valN = OBFUSCATE_STR;
  lenN = 0;
  return retVal;
}

// Returns -2 if string too long, 0 if ok, 1 if default set, 2 if password removed
int ProcessWifiPass(String &valP){
  int lenP = valP.length();
  if (lenP > MAXPASS)
    return -2;
//  if (valP[0] == ' ' || valP[lenP-1] == ' ')
//    return -3;

  int retVal;
  if (valP == "\"\""){ // setting c wifi appass "" removes the password
    valP = "";
    retVal = 2;
  }
  else if (lenP == 0){
    valP = String(DEF_PWD);
    retVal = 1;
  }
  else{
    retVal = 0;
  }

  String oldPwd = PC.GetWiFiPrefString(EE_PWD, DEF_PWD);
  
  if (valP != oldPwd){
    PC.PutWiFiPrefString(EE_OLDPWD, oldPwd); // save current pwd
    PC.PutWiFiPrefString(EE_PWD, valP); // save new pwd
    //prtln("New WiFi pass Stored: \"" + valP + "\"");
  }

  // obfuscate memory for security
  oldPwd = OBFUSCATE_STR;
  valP = OBFUSCATE_STR;
  lenP = 0;
  return retVal;
}

// Returns -2 if string too long, 0 if ok, 1 if default set, 2 if password removed
int ProcessApWifiPass(String &valP){
  int lenP = valP.length();
  if (lenP > MAXAPPASS)
    return -2;
//  if (valP[0] == ' ' || valP[lenP-1] == ' ')
//    return -3;

  int retVal;
  if (valP == "\"\""){ // setting c wifi pass "" removes the password
    valP = "";
    retVal = 2;
  }
  else if (lenP == 0){
    valP = String(DEF_AP_PWD);
    retVal = 1;
  }
  else{
    retVal = 0;
  }

  String oldPwd = PC.GetWiFiPrefString(EE_APPWD, DEF_AP_PWD);

  if (valP != oldPwd){
    PC.PutWiFiPrefString(EE_OLDAPPWD, oldPwd); // save current pwd
    PC.PutWiFiPrefString(EE_APPWD, valP); // save new pwd
    //prtln("New AP WiFi pass Stored: \"" + valP + "\"");
  }

  // obfuscate memory for security
  oldPwd = OBFUSCATE_STR;
  valP = OBFUSCATE_STR;
  lenP = 0;
  return retVal;
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
      QueueTask(TASK_MAC);
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
