#ifndef CmdH
#define CmdH

#include <Arduino.h>

// command-line handlers
void ProcessSerialCommand(String cmd);
String ProcessCommand(AsyncWebServerRequest *request, String &cmd);
String GetSubCommand(String &remCommand);
String ProcessWifiSsidPass(String &valN, String &valP);
//String ProcessApWifiSsidPass(String &valN, String &valP);
int ProcessWifiSsid(String &valN);
int ProcessWifiPass(String &valP);
int ProcessApWifiSsid(String &valN);
int ProcessApWifiPass(String &valP);
String ProcessWiFiHostName(String sName);
void ProcessRandMAC(wifi_interface_t macMode);
void ProcessMAC(wifi_interface_t macMode);

#endif
