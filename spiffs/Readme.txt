// Online Obfuscator tool: https://obfuscator.io/
// Just run fcspiffs.bat, you can just double-click it.

// My arduino ESP32 sketch checks for spiffs in file-name
// so just navigate to .bin file from fan-controller web-page
// and upload. It builds an image from data directory one level up.
// The data directory should contain all web-server html and css files.

// https://www.instructables.com/id/Set-Up-an-ESP8266-Automatic-Update-Server/
// updater software arduino uses for data upload:
// C:\Users\Scott\Documents\Arduino\hardware\espressif\esp32\libraries\Update\src
// Once you have it, building the SPIFFS binary is simple. I have a one-line batch file
// for the 1M version which takes the version number as a parameter (%1)

//  THIS IS WORKING!!!! (S.S. 5/24/2020)
//  This is for the Arduino ESP32 tools->partition scheme->Minimal SPIFFS(1.9MB APP with OTA 190KB SPIFFS)
//  min_spiffs.csv in documents\arduino\hardware\espressif\esp32\tools\partitions
//  offset: 0x3D0000, image size (-s): 0x30000,
"C:\Users\Scott\Documents\Arduino\hardware\espressif\esp32\tools\mkspiffs\mkspiffs.exe" -p 256 -b 4096 -s 0x30000 -c data/ spiffs_%1.bin

//  default_8MB.csv in documents\arduino\hardware\espressif\esp32\tools\partitions
//  offset: 0x670000, image size (-s): 0x190000,
// mkspiffs -p 256 -b 4096 -s 0x190000 -c data/ x.bin

// Make any changes to javascript files from \Documents\Arduino\projects\ESP32\FanController\origdata, then upload the .js file to
// https://obfuscator.io/ and put the obfuscated file into \Documents\Arduino\projects\ESP32\FanController\data, then rebuild
// fc.spiffs.bin by double-clicking fcspiffs.bat. Then upload the bin file using
// Arduino IDE 1.x Tools->ESP32 Sketch Data Upload. NOTE: the menu item won't appear until you
// unzip ESP32FS.zip into \Documents\Arduino\tools\. It has the python data-file uploader tool.
