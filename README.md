# esp32-fan-controller
Code for ESP32 fan-controller with custom printed circuit board using the Arduino IDE.

This code may be useful to paruse for ideas or or for adapting to your own ESP32 hardware project. Over-the-air WiFi firmware updates are supported.

It is for a custom PCB I ordered using EasyEDA upon which the ESP32 module is soldered and has switching transistors that drive solid-state relays. I should note that the PCB needs work. The ESP32 footprint is off by a bit so I had to bend the module's pins to fit the PCB holes for soldering.

The device is controlled via an ordinary web-browser over WiFi and can be remotely updated. There is a switch to switch between WiFi access-point mode and station mode (router connection). I use the AP mode to enter WiFi router credentials then flip the switch to connect to my router.

NOTE: This is built with the Arduino IDE and ESP32 for Arduino 2.0.2  https://github.com/espressif/arduino-esp32
Contact: dxzl@live.com

![Pic1](pics/Image00001.png)
![Pic2](pics/Image00002.png)
![Pic3](pics/Image00003.png)