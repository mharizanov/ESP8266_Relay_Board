/* config.h
 *
 * Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of Redis nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_
#include "os_type.h"
#include "user_config.h"

#define THERMOSTAT_MANUAL 0
#define THERMOSTAT_AUTO 1

#define THERMOSTAT_HEATING 0
#define THERMOSTAT_COOLING 1

#define USERJSONMAX 250 // max size of JSON string submitted via serial

typedef struct {
  uint16_t start;
  uint16_t end;
  uint16_t setpoint; // Degrees C in * 100 i.e. 2350=23.5*C
  uint16_t active;   // pad to 4 byte boundary
} dayScheduleElement;

typedef struct {
  dayScheduleElement daySched[8]; // Max 8 schedules per day
} daySchedule;

typedef struct {
  daySchedule weekSched[7]; // 7 days per week
} weekSchedule;

typedef struct {

  // 4 byte alignment, hence uint32_t
  uint32_t cfg_holder;

  uint8_t sta_mode[8];
  uint8_t sta_ip[16];
  uint8_t sta_mask[16];
  uint8_t sta_gw[16];

  uint8_t sta_ssid[32];
  uint8_t sta_pass[32];
  uint8_t ap_ip[32];
  uint8_t ap_mask[32];

  uint32_t sta_type;

  uint8_t ap_gw[32];
  uint8_t broadcastd_ro_apikey[32];
  uint8_t httpd_user[16];
  uint8_t httpd_pass[16];

  uint32_t httpd_port;
  uint32_t httpd_auth;
  uint32_t broadcastd_enable;
  uint32_t broadcastd_port;

  uint8_t broadcastd_host[32];
  uint8_t broadcastd_url[256];
  uint8_t DST;
  uint8_t ntp_enable;

  uint32_t broadcastd_thingspeak_channel;
  int32_t ntp_tz;

  uint32_t mqtt_enable;
  uint8_t mqtt_host[64];
  uint32_t mqtt_port;
  uint32_t mqtt_keepalive;

  uint8_t mqtt_devid[32];
  uint8_t mqtt_user[32];
  uint8_t mqtt_pass[64];
  uint8_t mqtt_relay_subs_topic[64];

  uint8_t mqtt_temp_subs_topic[64];
  uint8_t mqtt_dht22_temp_pub_topic[64];
  uint8_t mqtt_dht22_humi_pub_topic[64];
  uint8_t mqtt_ds18b20_temp_pub_topic[64];

  uint8_t mqtt_state_pub_topic[64];
  uint8_t mqtt_userJSON_pub_topic[64];
  uint8_t sensor_ds18b20_enable;
  uint8_t sensor_dht22_enable;
  uint16_t mqtt_use_ssl;

  uint8_t relay_total;
  uint8_t relay_latching_enable;
  uint8_t relay1_state;
  uint8_t relay2_state;

  uint8_t relay3_state;
  uint8_t relay1_thermostat;
  uint8_t relay2_thermostat;
  uint8_t relay3_thermostat;

  uint8_t relay1_name[24];
  uint8_t relay2_name[24];
  uint8_t relay3_name[24];

  uint8_t relay1_gpio;
  uint8_t relay2_gpio;
  uint8_t relay3_gpio;
  uint8_t therm_relay_rest_min;

  uint16_t therm_low_temp_colour_deg;
  uint16_t therm_high_temp_colour_deg;

  uint32_t therm_room_temp_timeout_secs;
  uint32_t thermostat1_input;
  uint32_t thermostat1_schedule_mode;
  uint32_t thermostat1_manual_setpoint;

  uint32_t thermostat1_enable;
  uint32_t thermostat1_opmode;
  uint32_t thermostat1_hysteresis_high;
  uint32_t thermostat1_hysteresis_low;

  weekSchedule thermostat1_schedule;
  /*
      uint32_t thermostat2enable;
      uint32_t thermostat2manualsetpoint;
      uint32_t thermostat2mode;
      uint32_t thermostat2opmode;
      uint32_t thermostat2_hysteresis_high;
      uint32_t thermostat2_hysteresis_low;
      weekSchedule thermostat2schedule;

      uint32_t thermostat3enable;
      uint32_t thermostat3manualsetpoint;
      uint32_t thermostat3mode;
      uint32_t thermostat3opmode;
      uint32_t thermostat3_hysteresis_high;
      uint32_t thermostat3_hysteresis_low;
      weekSchedule thermostat3schedule;
  */

} SYSCFG;

typedef struct {
  uint8 flag;
  uint8 pad[3];
} SAVE_FLAG;

void CFG_Save();
void CFG_Load();

extern SYSCFG sysCfg;

#endif /* USER_CONFIG_H_ */
