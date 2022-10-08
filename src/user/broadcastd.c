
#include "broadcastd.h"
#include "c_types.h"
#include "config.h"
#include "dht22.h"
#include "ds18b20.h"
//#include "espconn.h"
#include "espmissingincludes.h"
#include "httpclient.h"
#include "io.h"
#include "lwip/ip_addr.h"
#include "mem.h"
#include "mqtt.h"
#include "osapi.h"
#include "stdout.h"
#include "thermostat.h"
#include "user_interface.h"
#include "utils.h"

MQTT_Client mqttClient;

/*
 * ----------------------------------------------------------------------------
 * "THE MODIFIED BEER-WARE LICENSE" (Revision 42):
 * Mathew Hall wrote this file. As long as you
 * retain
 * this notice you can do whatever you want with this stuff. If we meet some
 * day,
 * and you think this stuff is worth it, you can buy sprite_tm a beer in return.
 * ----------------------------------------------------------------------------
 */

static ETSTimer MQTTbroadcastTimer;
static ETSTimer broadcastTimer;

static void ICACHE_FLASH_ATTR broadcastReading(void *arg) {

  char buf[384];
  char buf2[255];
  char t1[32];
  char t2[32];
  char t3[32];

  // double expand as sysCfg.broadcastd_url cntains placeholders as well
  os_sprintf(buf2, "http://%s:%d/%s", sysCfg.broadcastd_host, (int)sysCfg.broadcastd_port, sysCfg.broadcastd_url);

  if (sysCfg.sensor_dht22_enable) {
    dht_temp_str(t2);
    dht_humi_str(t3);
    os_sprintf(buf, buf2, currGPIO12State, currGPIO13State, currGPIO15State, "N/A", t2, t3);
  }

  if (sysCfg.sensor_ds18b20_enable) { // If DS18b20 daemon is enabled, then send up to 3 sensor's data instead
    ds_str(t1, 0);
    if (numds > 1)
      ds_str(t2, 1); // reuse to save space
    if (numds > 2)
      ds_str(t3, 2); // reuse to save space
    os_sprintf(buf, buf2, currGPIO12State, currGPIO13State, currGPIO15State, t1, t2, t3);
  }

  http_get(buf, http_callback_example);
  os_printf("Sent HTTP GET: %s\n\r", buf);
}

static ICACHE_FLASH_ATTR void MQTTbroadcastReading(void *arg) {

  char dht_temp[8];
  char dht_humi[8];
  char ds_temp[8];
  char therm_room_temp[8];
  char topic[64];
  char currentThermSetPoint[8] = "-9999";
  char payload[240];

  if (sysCfg.mqtt_enable == 1) {
    // os_printf("Sending MQTT\n");

    if (sysCfg.sensor_ds18b20_enable) {
      struct sensor_reading *result = read_ds18b20();
      if (result->success) {
        char temp[32];
        ds_str(temp, 0);
      }
    }

    // broadcast current state

    if (sysCfg.sensor_dht22_enable) {
      struct sensor_reading *result = readDHT();
      if (result->success) {
        dht_temp_str(dht_temp);
        dht_humi_str(dht_humi);
      } else {
        os_strcpy(dht_temp, "N/A");
        os_strcpy(dht_humi, "N/A");
      }
    } else {
      os_strcpy(dht_temp, "N/A");
      os_strcpy(dht_humi, "N/A");
    }

    if (sysCfg.sensor_ds18b20_enable) {
      ds_str(ds_temp, 0);
    } else {
      os_strcpy(ds_temp, "N/A");
    }

    if (sysCfg.thermostat1_input == 0) {
      os_strcpy(therm_room_temp, ds_temp);
    } else if (sysCfg.thermostat1_input == 1 || sysCfg.thermostat1_input == 2) {
      os_strcpy(therm_room_temp, dht_temp);
    } else if (sysCfg.thermostat1_input == 3) { // Mqtt reading should be degC *10
      if ((int)mqttTreading == -9999) {
        os_strcpy(therm_room_temp, "-9999");
      } else {
        os_sprintf(therm_room_temp, "%d.%d", (int)mqttTreading / 10, mqttTreading - ((int)mqttTreading / 10) * 10);
      }
    } else if (sysCfg.thermostat1_input == 4) { // Serial reading should be degC *10
      os_sprintf(therm_room_temp, "%d.%d", (int)serialTreading / 10, serialTreading - ((int)serialTreading / 10) * 10);
    } else if (sysCfg.thermostat1_input == 5) { // Fixed value
      os_strcpy(therm_room_temp, "10");
    } else {
      os_strcpy(therm_room_temp, "N/A");
    }

    if (sysCfg.thermostat1mode == 1) {
      // thermostat in Schedule mode
      os_sprintf(currentThermSetPoint, "%d.%d", (int)scheduleThermSetPoint / 10,
                 scheduleThermSetPoint - ((int)scheduleThermSetPoint / 10) * 10);

    } else {
      // thermostat in Manual mode
      os_sprintf(currentThermSetPoint, "%d.%d", (int)sysCfg.thermostat1manualsetpoint / 10,
                 sysCfg.thermostat1manualsetpoint - ((int)sysCfg.thermostat1manualsetpoint / 10) * 10);
    }

    os_sprintf(payload,
               "{\n\"ds18b20Temp\": \"%s\",\n\"dht22Temp\":\"%s\",\n\"dht22Humidity\":\"%s\",\n"
               "\"humidiStat\":%d,\n"
               "\"relay1\":%d,\n"
               "\"relay2\":%d,\n"
               "\"relay3\":%d,\n"
               "\"opMode\":%d,\n\"state\":%d,\n\"thermostatSetPoint\":\"%s\",\n"
               "\"roomTemp\":\"%s\",\n\"autoMode\": %d\n"
               "}\n",
               ds_temp, dht_temp, dht_humi, (int)sysCfg.thermostat1_input == 2 ? 1 : 0, currGPIO12State,
               currGPIO13State, currGPIO15State, (int)sysCfg.thermostat1opmode, (int)sysCfg.thermostat1state,
               (int)currentThermSetPoint, (char *)therm_room_temp, (int)sysCfg.thermostat1mode);

    os_sprintf(topic, "%s", sysCfg.mqtt_state_pub_topic);
    os_printf("Broadcastd: Publishing state via MQTT to \"%s\", length %d\n", topic, os_strlen(payload));
    MQTT_Publish(&mqttClient, topic, payload, os_strlen(payload), 0, 0);
  }
}

void ICACHE_FLASH_ATTR broadcastd_init(void) {

  if (sysCfg.mqtt_enable == 1) {
    os_printf("Arming MQTT broadcast timer\n");
    os_timer_setfn(&MQTTbroadcastTimer, MQTTbroadcastReading, NULL);
    os_timer_arm(&MQTTbroadcastTimer, 60000, 1);
  }

  if (sysCfg.broadcastd_enable == 1) {
    os_printf("Arming HTTP broadcast timer\n");
    os_timer_setfn(&broadcastTimer, broadcastReading, NULL);
    os_timer_arm(&broadcastTimer, 60000, 1);
  }
}
