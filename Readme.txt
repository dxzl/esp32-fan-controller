first generate main .bin file from Arduino IDE: "Sketch->Export Compiled Binary"
double-click fixname.bat to rename to fc.bin
then go into spiffs and double-click fcspiffs.bat to genetate fc.spiffs.bin

start the fan-controller and go to its web page "http://dts7.local or http://dts10.local", Etc.
Enter "c update" then Submit in the HostName field.
Login as "dts" pass "update"
Navigate to one or the other .bin files and Update.

In browser (me-no-dev):

https://github.com/espressif/arduino-esp32

Read: Instructions for board-manager.

https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md

v1.04 is current release:

In File->Preferences, add one of below the additional board web-page:

https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json

Stable:

https://dl.espressif.com/dl/package_esp32_dev_index.json

// for esp8266:
https://arduino.esp8266.com/stable/package_esp8266com_index.json
