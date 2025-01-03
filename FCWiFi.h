#ifndef FCWiFiH
#define FCWiFiH

#include <Arduino.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <WiFi.h>

void CheckWiFiConnected();
void PollWiFiSwitch();
void WiFiMonitorConnection(bool bDisconnect=false);
void WiFiStop(bool bWiFiOff);
void WiFiStartAP(bool bDisconnect=false);
//void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
//void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
//void WiFiEvent(WiFiEvent_t event);
//void printWiFiEventDetails(WiFiEvent_t event);
IPAddress GetLocalIp();

#endif
