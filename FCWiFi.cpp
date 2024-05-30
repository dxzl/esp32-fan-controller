// this file FCWiFi.cpp
#include "FanController.h"

void PollWiFiSwitch(){
  ReadWiFiSwitch();

  //prtln("g_bSoftAP:" + String(g_bSoftAP) + ", bApSwitchOn:" + String(bApSwitchOn) + ", g_bWiFiDisabled:" + String(g_bWiFiDisabled) + ", g_bWiFiConnected:" + String(g_bWiFiConnected) );

  if (g_bSoftAP){
    // if in AP mode and switch changed to STA mode, disconnect AP
    if (g8_wifiSwState != WIFI_SW_MODE_AP)
      WiFiStartAP(true); // disconnect AP WiFi Mode

    CheckWiFiConnected();
  }
  else{
    // see if we need to disconnect from router first, then check for entering AP mode
    CheckWiFiConnected();

    if (g8_wifiSwState == WIFI_SW_MODE_AP)
      WiFiStartAP(false); // connect in AP mode
  }
}

void CheckWiFiConnected(){
  if (g_bWiFiConnected){
    if (g_bWiFiDisabled || (g8_wifiSwState != WIFI_SW_MODE_STA))
      WiFiMonitorConnection(true); // disconnect from router
  }
  else{ // not connected to router
    if (!g_bWiFiDisabled && (g8_wifiSwState == WIFI_SW_MODE_STA)){
      WiFiMonitorConnection(false); // connect to router in STA mode
    }
    else if (g_bWiFiConnecting && (g8_wifiSwState == WIFI_SW_MODE_OFF)){
      WiFiMonitorConnection(true); // disconnect if in the process of connecting
    }
  }
}

void WiFiMonitorConnection(bool bDisconnect){
  if (g8_wifiSwState == WIFI_SW_MODE_OFF && (g_bSoftAP || g_bWiFiConnected || g_bWiFiConnecting || WiFi.getMode() != WIFI_OFF)){
    WiFiStop(true);
    return;
  }

  if (g_bSoftAP){
    if (bDisconnect || g8_wifiSwState != WIFI_SW_MODE_AP)
      WiFiStartAP(true); // disconnect AP mode (it will reconnect)
    else
      return;
  }

  //WiFi.status() codes
  //255 WL_NO_SHIELD: assigned when no WiFi shield is present;
  //0   WL_IDLE_STATUS: it is a temporary status assigned when WiFi.begin() is called and
  //      remains active until the number of attempts expires (resulting in WL_CONNECT_FAILED)
  //      or a connection is established (resulting in WL_CONNECTED);
  //1 WL_NO_SSID_AVAIL: assigned when no SSID are available;
  //2 WL_SCAN_COMPLETED: assigned when the scan networks is completed;
  //3 WL_CONNECTED: assigned when connected to a WiFi network;
  //4 WL_CONNECT_FAILED: assigned when the connection fails for all the attempts;
  //5 WL_CONNECTION_LOST: assigned when the connection is lost;
  //6 WL_DISCONNECTED: assigned when disconnected from a network;   // Connect to Wi-Fi
  int WiFiStatus = WiFi.status();

  if (g_bWiFiConnected){
    if (bDisconnect || WiFiStatus != WL_CONNECTED){
      if (bDisconnect)
        WiFiStop(false);
      else
        prtln("Connection failed, lost or disconnected...");
    }
//    else if (!(g_bManualTimeWasSet || g_bWiFiTimeWasSet) && g_bResetOrPowerLoss){
//    }
  }
  else if (bDisconnect)
    // we set bDisconnect when calling WiFiMonitorConnection(true) after changing the SSID/PW
    WiFiStop(false);
  else{
    if (WiFiStatus == WL_CONNECTED){
      g_bWiFiConnected = true;
      g_bWiFiConnecting = false;
      prtln(GetStringIP()); // set g_bWiFiConnected before calling GetStringIP()!
      dnsAndServerStart();
      FlashSequencerInit(g8_ledMode_SLOWFLASH); // start the sequence of flashing out the last octet of IP address...
    }
    else if (g_bWiFiDisabled){
      if (WiFi.getMode() != WIFI_OFF)
        WiFiStop(true);
    }
    else if (g_bWiFiConnecting){
      //prt(".");
    }
    else if (g_sSSID.length() != 0 && g8_wifiSwState == WIFI_SW_MODE_STA){ // connect to router
      QueueTask(TASK_WIFI_STA_CONNECT); // fetch pw from flash and connect
      g8_ledMode = g8_ledMode_FASTFLASH;
      g_bWiFiConnecting = true;
      prtln("queueing connection task: " + g_sSSID);
    }
  }
}

// bool softAPdisconnect(bool wifioff = false);
// bool disconnect(bool wifioff = false, bool eraseap = false);
// (Also called from WiFiStationDisconnected() with bWiFiOff = false)
void WiFiStop(bool bWiFiOff){
  if (g_bSoftAP){
    WiFi.softAPdisconnect(bWiFiOff);
    prtln("AP Mode disconnect...");
  }
  
  if (g_bWiFiConnected){
    WiFi.disconnect(bWiFiOff); // triggers WiFiStationDisconnected() which calls us reentrantly!
    prtln("STA Mode disconnect...");
  }
  else{
    prtln("Stopping applemidi, web-server and mDNS...");
    FlashSequencerStop();
  
    dnsAndServerStop(); // stop applemidi, dns and webserver (disconnect must be before this...(? check))
  
    // do this to avoid pot thinking knob was turned as we start
    // reading it again when g_bSoftAP or bWifIConnected goes false!
    g16_oldpot1Value = g16_pot1Value;
  
    g_bSoftAP = false;
    g_bWiFiConnected = false;
    g_bWiFiConnecting = false;
    WiFi.mode(WIFI_OFF);
  
    g8_fiveSecondTimer = FIVE_SECOND_TIME-2; // try restart in 2-3 seconds
  }
}

void WiFiStartAP(bool bDisconnect){
  if (bDisconnect)
    WiFiStop(false);
  else if (!g_bSoftAP && g_sApSSID.length() != 0){
    QueueTask(TASK_WIFI_AP_CONNECT); // fetch pw from flash and connect
  }
}

// https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiGeneric.h
// https://github.com/espressif/esp-idf/blob/master/components/esp_wifi/include/esp_wifi_types.h
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  if (g_bWiFiConnected){ // | g_bSoftAP???
//  if (info.disconnected.reason > WIFI_REASON_BEACON_TIMEOUT && g_bWiFiConnected){
    g_bWiFiConnected = false; // clear this BEFORE calling WiFiStop()!
    WiFiStop(false);

    //uint8_t ssid[32];         /**< SSID of disconnected AP */
    //uint8_t ssid_len;         /**< SSID length of disconnected AP */
    //uint8_t bssid[6];         /**< BSSID of disconnected AP */
    //uint8_t reason;
    Serial.print("WiFi lost connection. Reason: ");
    Serial.println(info.wifi_sta_disconnected.reason); // 201 = router unplugged, 200 = idle 1 sec. heartbeat
    Serial.println(MacArrayToString(info.wifi_sta_disconnected.bssid));
//    Serial.println("Trying to Reconnect");
//    WiFi.begin(ssid, password);}
  }
}

//void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
//    Serial.println("WiFi connected");
//    Serial.println("IP address: ");
//    Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
//}

//void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
//    Serial.println("Connected to router!");
//
//    Serial.print("SSID Length: ");
//    Serial.println(info.connected.ssid_len);
//
//    Serial.print("SSID: ");
//    for(int i=0; i<info.connected.ssid_len; i++){
//      Serial.print((char) info.connected.ssid[i]);
//    }
//
//    Serial.print("\nBSSID: ");
//    for(int i=0; i<6; i++){
//      Serial.printf("%02X", info.connected.bssid[i]);
//
//      if(i<5){
//        Serial.print(":");
//      }
//    }
//
//    Serial.print("\nChannel: ");
//    Serial.println(info.connected.channel);
//
//    Serial.print("Auth mode: ");
//    Serial.println(info.connected.authmode);
//}

//void WiFiEvent(WiFiEvent_t event){
//  printWiFiEventDetails(event);
//  if (event == ARDUINO_EVENT_STA_DISCONNECTED) {
//    // handle your specific event here
//  }
//}

//void printWiFiEventDetails(WiFiEvent_t event){
//    Serial.printf("[WiFi-event] event: %d\n", event);
//
//    switch (event){
//        case ARDUINO_EVENT_WIFI_READY:
//            Serial.println("WiFi interface ready");
//            break;
//        case ARDUINO_EVENT_WIFI_SCAN_DONE:
//            Serial.println("Completed scan for access points");
//            break;
//        case ARDUINO_EVENT_WIFI_STA_START:
//            Serial.println("WiFi client started");
//            break;
//        case ARDUINO_EVENT_WIFI_STA_STOP:
//            Serial.println("WiFi clients stopped");
//            break;
//        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
//            Serial.println("Connected to access point");
//            break;
//        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
//            Serial.println("Disconnected from WiFi access point");
//            break;
//        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
//            Serial.println("Authentication mode of access point has changed");
//            break;
//        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
//            Serial.print("Obtained IP address: ");
//            Serial.println(WiFi.localIP());
//            break;
//        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
//            Serial.println("Lost IP address and IP address is reset to 0");
//            break;
//        case ARDUINO_EVENT_WPS_ER_SUCCESS:
//            Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
//            break;
//        case ARDUINO_EVENT_WPS_ER_FAILED:
//            Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
//            break;
//        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
//            Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
//            break;
//        case ARDUINO_EVENT_WPS_ER_PIN:
//            Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
//            break;
//        case ARDUINO_EVENT_WIFI_AP_START:
//            Serial.println("WiFi access point started");
//            break;
//        case ARDUINO_EVENT_WIFI_AP_STOP:
//            Serial.println("WiFi access point  stopped");
//            break;
//        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
//            Serial.println("Client connected");
//            break;
//        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
//            Serial.println("Client disconnected");
//            break;
//        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
//            Serial.println("Assigned IP address to client");
//            break;
//        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
//            Serial.println("Received probe request");
//            break;
//        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
//            Serial.println("AP IPv6 is preferred");
//            break;
//        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
//            Serial.println("STA IPv6 is preferred");
//            break;
//        case ARDUINO_EVENT_ETH_GOT_IP6:
//            Serial.println("Ethernet IPv6 is preferred");
//            break;
//        case ARDUINO_EVENT_ETH_START:
//            Serial.println("Ethernet started");
//            break;
//        case ARDUINO_EVENT_ETH_STOP:
//            Serial.println("Ethernet stopped");
//            break;
//        case ARDUINO_EVENT_ETH_CONNECTED:
//            Serial.println("Ethernet connected");
//            break;
//        case ARDUINO_EVENT_ETH_DISCONNECTED:
//            Serial.println("Ethernet disconnected");
//            break;
//        case ARDUINO_EVENT_ETH_GOT_IP:
//            Serial.println("Obtained IP address");
//            break;
//        default: break;
//    }
//}
