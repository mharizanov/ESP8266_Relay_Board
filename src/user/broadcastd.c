
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
    os_sprintf(buf, buf2, relay1State, relay2State, relay3State, "N/A", t2, t3);
  }

  if (sysCfg.sensor_ds18b20_enable) { // If DS18b20 daemon is enabled, then send up to 3 sensor's data instead
    ds_str(t1, 0);
    if (numds > 1)
      ds_str(t2, 1); // reuse to save space
    if (numds > 2)
      ds_str(t3, 2); // reuse to save space
    os_sprintf(buf, buf2, relay1State, relay2State, relay3State, t1, t2, t3);
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
      os_sprintf(topic, "%s", sysCfg.mqtt_dht22_temp_pub_topic);
      MQTT_Publish(&mqttClient, topic, dht_temp, os_strlen(dht_temp), 0, 0);
      os_printf("Published \"%s\" to topic \"%s\"\n", dht_temp, topic);

      os_sprintf(topic, "%s", sysCfg.mqtt_dht22_humi_pub_topic);
      MQTT_Publish(&mqttClient, topic, dht_humi, os_strlen(dht_humi), 0, 0);
      os_printf("Published \"%s\" to topic \"%s\"\n", dht_humi, topic);

    } else {
      os_strcpy(dht_temp, "N/A");
      os_strcpy(dht_humi, "N/A");
    }

    if (sysCfg.sensor_ds18b20_enable) {
      // publish all sensor ids and readings to ds18b20 MQTT topic
      for (int i = 0; i < numds; i++) {
        if (dsreading[i].success) {
          ds_str(ds_temp, i);
          os_sprintf(topic, "%s/%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", sysCfg.mqtt_ds18b20_temp_pub_topic,
                     addr[i][0], addr[i][1], addr[i][2], addr[i][3], addr[i][4], addr[i][5], addr[i][6], addr[i][7]);

          MQTT_Publish(&mqttClient, topic, ds_temp, os_strlen(ds_temp), 0, 0);
          os_printf("Published \"%s\" to topic \"%s\"\n", ds_temp, topic);
        }
      }
    } else {
      // State publish MQTT (below)publishes temp of 1st ds18b20 sensor
      // in its message, if ds18b20 not enabled set 1st sensor temp to N/A
      os_strcpy(ds_temp, "N/A");
    }

    /*
        if (sysCfg.thermostat1_input == 0) {
          os_strcpy(therm_room_temp, ds_temp);
        } else if (sysCfg.thermostat1_input == 1 || sysCfg.thermostat1_input == 2) {
          os_strcpy(therm_room_temp, dht_temp);
        } else if (sysCfg.thermostat1_input == 3) { // Mqtt reading should be degC *10
          if ((int)mqttTreading == -9999) {
            // MQTT reading invalid or stale, report -9999 as therm_room_temp
            os_strcpy(therm_room_temp, "-9999");
          } else {
            os_sprintf(therm_room_temp, "%d.%d", (int)mqttTreading / 10, mqttTreading - ((int)mqttTreading / 10) * 10);
          }
        } else if (sysCfg.thermostat1_input == 4) { // Serial reading should be degC *10
          os_sprintf(therm_room_temp, "%d.%d", (int)serialTreading / 10, serialTreading - ((int)serialTreading / 10) *
       10); } else if (sysCfg.thermostat1_input == 5) { // Fixed value os_strcpy(therm_room_temp, "10"); } else {
          os_strcpy(therm_room_temp, "N/A");
        }
    */
    int roomTemp = getRoomTemp();
    os_sprintf(therm_room_temp, "%d.%d", (int)roomTemp / 10, abs(roomTemp - ((int)roomTemp / 10) * 10));

    if (sysCfg.thermostat1_schedule_mode == 1) {
      // thermostat in Schedule mode
      os_sprintf(currentThermSetPoint, "%d.%d", (int)thermostat1ScheduleSetPoint / 10,
                 abs(thermostat1ScheduleSetPoint - ((int)thermostat1ScheduleSetPoint / 10) * 10));
    } else {
      // thermostat in Manual mode
      os_sprintf(currentThermSetPoint, "%d.%d", (int)sysCfg.thermostat1_manual_setpoint / 10,
                 abs(sysCfg.thermostat1_manual_setpoint - ((int)sysCfg.thermostat1_manual_setpoint / 10) * 10));
    }

    os_sprintf(payload,
               "{\n\"ds18b20Temp\": \"%s\",\n\"dht22Temp\":\"%s\",\n\"dht22Humidity\":\"%s\",\n"
               "\"humidiStat\":%d,\n"
               "\"relay1\":%d,\n"
               "\"relay2\":%d,\n"
               "\"relay3\":%d,\n"
               "\"thermostatRelayActive\":%d,\n"
               "\"opMode\":%d,\n\"thermostatEnabled\":%d,\n\"thermostatSetPoint\":\"%d\",\n"
               "\"roomTemp\":\"%s\",\n\"autoMode\": %d\n"
               "}\n",
               ds_temp, dht_temp, dht_humi, (int)sysCfg.thermostat1_input == 2 ? 1 : 0, relay1State, relay2State,
               relay3State, thermostatRelayActive, (int)sysCfg.thermostat1_opmode, (int)sysCfg.thermostat1_enable,
               (int)currentThermSetPoint, (char *)therm_room_temp, (int)sysCfg.thermostat1_schedule_mode);

    os_sprintf(topic, "%s", sysCfg.mqtt_state_pub_topic);
    os_printf("Broadcastd: Publishing state via MQTT to \"%s\", length %d\n", topic, os_strlen(payload));
    MQTT_Publish(&mqttClient, topic, payload, os_strlen(payload), 0, 0);
  }

  // publish userJSON received on serial interface
  if (strlen((const char *)userJSON) > 3 &&
      (sntp_get_current_timestamp() - userJSONreadingTS < sysCfg.mqtt_keepalive)) {
    os_sprintf(topic, "%s", sysCfg.mqtt_userJSON_pub_topic);
    os_sprintf(payload, "%s", (const char *)userJSON);

    MQTT_Publish(&mqttClient, topic, payload, os_strlen(payload), 0, 0);
    os_printf("Published \"%s\" to topic \"%s\"\n", payload, topic);
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
