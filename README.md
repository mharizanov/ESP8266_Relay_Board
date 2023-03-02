#FOTA Update Version - Requires NONOS-SDK 3.0.6 AND 1MB flash minimum.
Use command line make to build, PlatformIO does not have this capability or up to date libraries at present.

1) Edit the Makefile and set the paths to the SDK and XTENSA compiler; set the FLASH_SIZE for your board
2) Edit burn_boot.sh and burn_webpages.sh and set the path to ESPTOOL for your environment 
3) make clean
4) make
5) ./burn_boot.sh

Note, as per standard approach, only boot.bin and user1.bin are burnt to flash. 
user2.bin is burnt as a OTA upgrade.


Uses the new SDK partition_table_t struct and should autoconfigure as per the Makefile.
Only tested on 1Mb flash; will not work on 512Kb for sure, however larger flash size should work (as supported by ESP8266 chip itself)


#Three Channel WiFi Relay/Thermostat Board

The Three Channel WiFi Relay/Thermostat Board is a WiFi connected multi-purpose relay board based on the [ESP8266 SoC], original designed by Martin Harizanov and sold by Open Energy Monitor Store in the UK

This fork of the code is actively maintained (2023), builds cleanly in Platfrom IO and has been rebased against NON-RTOS-SDK 2.2.1.
You will need the platformIO NON-RTOS-SDK 8266 build framework installed to build. https://docs.platformio.org/en/latest/frameworks/esp8266-nonos-sdk.html


Various enhancements have been made:

###Enhancements to Martin's code
- Full support for MQTT thermostat source
- Enhanced MQTT reporting (state, DS18B20 JSON)
- A void thermostat cycling 
- New Config items for MQTT thermostat timeout & thermostat cycling
- Display of a bad or stale temperature readings highlighted in thermostat UI
- Fix bug where thermostat off time is update is it is already off
- Change thermostat to work in tenths of a degree, not hundredths (direct compatibility with emonTh + emonGLCD)
- Publish all ds18b20 sensors to MQTT by device-id
- Choose which relays associate to each thermostat

.

Firmware update

To change firmware, set the programming switch to “PGM” position and attach *3.3V* FTDI cable with GND (black wire) towards the USB plug.
I find any of the cheap ESP programming USB plugs work well. JeeLabs USBBUB-II was not reliable. YMMV.
Also see the (https://github.com/esp8266/esp8266-wiki/wiki/Uploading) ESP8266 WiKi for more details on uploading code.

[![image](https://harizanov.com/wp-content/uploads/2015/02/esp8266-board-firmware-update-mode-300x225.jpg)]

Do not forget to flip back the PGM switch to the right side when done, or the code won’t start and the module will be stuck on bootloader mode.

You will need esptool.py installed.
Precompiled firmware images are in the firmware directory. Use the burn_all.sh and burn_webpages.sh scripts to write firmware to the device.
Modify the burn*.sh files appropriately to set paths for you USB port and path to esptool.py.

Note, the first time you do this all config will be LOST and you need to set the device up again from scratch (you need to join the devices WIFI AP and connect 192.168.4.1 in a browser).

If you are building and flashing using PIO, you still need to run the burn_webpages.sh script to deploy the webpages to the device.



--Original Readme.MD follows--

###Highlights

- Option for on-board power supply
- Up to three high quality [10A relays]
- Powered by the WiFi [ESP8266 SoC] module
- HTTP API to control the relays
- [MQTT] support
- [NTP] for network time
- HTTP daemon settings, including security/authentication setup
- HTTP UI for configuration and control
 - Thermostat function with weekly scheduling
 - Manual relay control
- Broadcast using HTTP GET to services like [ThingSpeak] and [emonCMS]
- Integration with [ThingSpeak] for charting/analytics visualization
- Temperature sensor support
 - DS18B20
 - DHT22

**NOTE**: The board connects to and controls high voltage, knowledge and care is required to handle it
See more at the [Three Channel WiFi Relay/Thermostat Board WiKi]

#Author
[Martin Harizanov]

#License
This project is licensed under [Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License]. 

[![License](http://i.creativecommons.org/l/by-nc-sa/3.0/88x31.png)](http://creativecommons.org/licenses/by-nc-sa/3.0/)

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

[Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License]: http://creativecommons.org/licenses/by-nc-sa/3.0/
[Three Channel WiFi Relay/Thermostat Board WiKi]: http://harizanov.com/wiki/wiki-home/three-channel-wifi-relaythermostat-board/
[emonCMS]: http://emoncms.org 
[ThingSpeak]: http://thingspeak.com
[10A relays]: http://www.te.com/catalog/pn/en/3-1393239-6
[ESP8266 SoC]: https://espressif.com/en/products/esp8266/
[MQTT]: http://en.wikipedia.org/wiki/MQTT
[NTP]: http://en.wikipedia.org/wiki/Network_Time_Protocol
[Martin Harizanov]:http://harizanov.com/
[@mharizanov]:http://twitter.com/mharizanov
