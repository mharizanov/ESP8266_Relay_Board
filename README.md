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
