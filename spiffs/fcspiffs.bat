@echo off
rem https://www.instructables.com/id/Set-Up-an-ESP8266-Automatic-Update-Server/
rem updater software arduino uses for data upload:
rem C:\Users\Scott\Documents\Arduino\hardware\espressif\esp32\libraries\Update\src
rem Once you have it, building the SPIFFS binary is simple. I have a one-line batch file
rem for the 1M version which takes the version number as a parameter (%1)
rem  THIS IS WORKING!!!! (S.S. 5/24/2020)
rem  This is for the Arduino ESP32 tools->partition scheme->Minimal SPIFFS(1.9MB APP with OTA 190KB SPIFFS)
rem  min_spiffs.csv in documents\arduino\hardware\espressif\esp32\tools\partitions
rem  offset: 0x3D0000, image size (-s): 0x30000,
rem  My fan-controller .ino arduino ESP32 sketch checks for "spiffs" in file-name
rem  then uses: Update.begin(SPIFFS.totalBytes(), U_SPIFFS)rem 
rem mkspiffs.exe -p 256 -b 4096 -s 0x30000 -c ..\data fc.spiffs.bin
mkspiffs.exe -p 256 -b 4096 -s 0x170000 -c ..\data fc.spiffs.bin
pause