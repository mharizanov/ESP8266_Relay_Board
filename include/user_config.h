#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4), packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR __attribute__((section(".iram0.text")))
#define ICACHE_RODATA_ATTR __attribute__((section(".irom.text")))

#define CFG_HOLDER 0x00FF0006
//#define CFG_LOCATION 0x3C     //Working value on old SDK
#define CFG_LOCATION 0x70 // NONOS-SDK - circa around position 458K in flash
// 0x7C on 512K flash is where the internal RF data lives - stay away from there(last 16k)
#define FWVER "1.4f/May 12th 2024"

/*DEFAULT CONFIGURATIONS*/

#define STA_MODE "dhcp"
#define STA_IP "192.168.1.17"
#define STA_MASK "255.255.255.0"
#define STA_GW "192.168.1.1"
//#define STA_SSID "dd-wrt"
//#define STA_PASS "a1b2c3d4e5"
#define STA_SSID "pearson.uk"
#define STA_PASS "spectrum"

#define STA_TYPE AUTH_WPA2_PSK

#define AP_IP "192.168.4.1"
#define AP_MASK "255.255.255.0"
#define AP_GW "192.168.4.1"

#define HTTPD_PORT 80
#define HTTPD_AUTH 0
#define HTTPD_USER "admin"
#define HTTPD_PASS "pass"

#define BROADCASTD_ENABLE 0
#define BROADCASTD_PORT 80
#define BROADCASTD_HOST "api.thingspeak.com"
#define BROADCASTD_URL "/update?key=**RWAPI**&field1=%d&field2=%d&field3=%d&field4=%s&field5=%s&field6=%s"
#define BROADCASTD_THINGSPEAK_CHANNEL 0
#define BROADCASTD_RO_APIKEY "**ROAPI**"

#define SYSLOG_ENABLE 0
#define SYSLOG_HOST "192.168.10.7"
#define NTP_ENABLE 1
#define NTP_TZ 1

#define MQTT_ENABLE 1
#define MQTT_HOST "192.168.10.3" // host name or IP "192.168.11.1"
#define MQTT_PORT 1883
#define MQTT_KEEPALIVE 120 /*seconds*/
#define MQTT_DEVID "ESP_%08X"
#define MQTT_USER "tasmota"
#define MQTT_PASS "tasmota"
#define MQTT_USE_SSL 0
#define MQTT_RELAY_SUBS_TOPIC "esp_%08X/in/relay/#"
#define MQTT_TEMP_SUBS_TOPIC "esp_%08X/out/temperature"
#define MQTT_STATE_PUB_TOPIC "esp_%08X/out/state"
#define MQTT_USERJSON_PUB_TOPIC "esp_%08X/out/userJSON"

#define MQTT_DHT22_TEMP_PUB_TOPIC "esp_%08X/out/dht22/temperature"
#define MQTT_DHT22_HUMI_PUB_TOPIC "esp_%08X/out/dht22/humidity"
#define MQTT_DS18B20_TEMP_PUB_TOPIC "esp_%08X/out/ds18b20/temperature"

#define SENSOR_DS18B20_ENABLE 0
#define SENSOR_DHT22_ENABLE 0

#define RELAY_LATCHING_ENABLE 0
#define RELAY_TOTAL 3 // number of relays
#define RELAY1NAME "Relay 1"
#define RELAY2NAME "Relay 2"
#define RELAY3NAME "Relay 3"

#define RELAY1GPIO 12
#define RELAY2GPIO 13
#define RELAY3GPIO 15

#define MQTT_BUF_SIZE 300
#define MQTT_RECONNECT_TIMEOUT 5 /*second*/
#define MQTT_CONNTECT_TIMER 5    /**/

#define THERM1_ZONE_NAME "Zone 1"

#endif
