/*
Some random cgi routines.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

#include "cgi.h"
#include "config.h"
#include "dht22.h"
#include "ds18b20.h"
#include "espmissingincludes.h"
#include "httpd.h"
#include "io.h"
#include "lwip/ip_addr.h"
#include "mem.h"
#include "mqtt.h"
#include "stdout.h"
#include "thermostat.h"
#include "time_utils.h"
#include "user_interface.h"
#include <osapi.h>
#include <stdlib.h>
#include <string.h>

#include "jsmn.h"

// Template code
void ICACHE_FLASH_ATTR tplThermostat(HttpdConnData *connData, char *token, void **arg) {
  char buff[128];
  if (token == NULL)
    return;
  os_strcpy(buff, "Unknown");
  httpdSend(connData, buff, -1);
}

static int ICACHE_FLASH_ATTR jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

int ICACHE_FLASH_ATTR cgiThermostat(HttpdConnData *connData) {
  char buff[2048];
  char temp[128];
  char humi[32];
  // char therm_room_temp[8];

  int len = 0;

  httpdStartResponse(connData, 200);
  httpdHeader(connData, "Content-Type", "application/json");
  httpdHeader(connData, "Access-Control-Allow-Origin", "*");
  httpdEndHeaders(connData);

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }
  int roomTemp = getRoomTemp();
  //  os_sprintf(therm_room_temp, "%d.%d", (int)roomTemp / 10, abs(roomTemp - ((int)roomTemp / 10) * 10));

  os_strcpy(buff, "Unknown");
  os_strcpy(temp, "N/A");
  os_strcpy(humi, "N/A");

  len = httpdFindArg(connData->getArgs, "param", buff, sizeof(buff));
  if (len > 0) {
    if (os_strcmp(buff, "state") == 0) {
      os_sprintf(buff,
                 "{ \"room_temp\": %d,\n\"humidity\":\"%s\",\n"
                 "\"humidistat\":%d,\n\"thermostat_relay_active\":%d,\n\"relay1name\":\"%s\",\n\"opmode\":%d,\n\""
                 "enable\":%d,\n\"manual_setpoint\":%d,\"current_setpoint\":%d,\n\"thermostat1_input\":%d,\n\"schedule_"
                 "mode\": %d,\n"
                 "\"mqtthost\": \"%s\",\n \"time\": \"%d\"}\n",
                 roomTemp, humi, (int)sysCfg.thermostat1_input == 2 ? 1 : 0, thermostatRelayActive,
                 (char *)sysCfg.relay1_name, (int)sysCfg.thermostat1_opmode, (int)sysCfg.thermostat1_enable,
                 (int)sysCfg.thermostat1_manual_setpoint, (int)thermostat1CurrentSetPoint,
                 (int)sysCfg.thermostat1_input, (int)sysCfg.thermostat1_schedule_mode, (char *)sysCfg.mqtt_host,
                 sntp_get_current_timestamp());
    }

    if (os_strcmp(buff, "temperature") == 0) {
      if (sysCfg.sensor_dht22_enable) {
        dht_temp_str(temp);
      } else {
        ds_str(temp, 0);
      }
      os_sprintf(buff, "%s", temp);
    }

    if (os_strcmp(buff, "thermostat_opmode") == 0) {
      if (connData->post->len > 0) {
        sysCfg.thermostat1_opmode = (int)atoi(connData->post->buff);
        CFG_Save();
        os_printf("Handle thermostat opmode (%d) saved\n", (int)sysCfg.thermostat1_opmode);
      } else {
        os_sprintf(buff, "%d", (int)sysCfg.thermostat1_opmode);
      }
    }

    if (os_strcmp(buff, "thermostat_zonename") == 0) {
      if (connData->post->len > 0) {
        os_printf("N/A\n");
      } else {
        os_sprintf(buff, "%s", (char *)sysCfg.relay1_name);
      }
    }

    if (os_strcmp(buff, "thermostat_relay1state") == 0) {
      if (connData->post->len > 0) {
        os_printf("N/A\n");
      } else {
        os_sprintf(buff, "%d", relay1State);
      }
    }

    if (os_strcmp(buff, "thermostat_enable") == 0) {
      if (connData->post->len > 0) {
        sysCfg.thermostat1_enable = (int)atoi(connData->post->buff);
        thermostatRelayOff();
        CFG_Save();
        os_printf("Handle thermostat state (%d) saved\n", (int)sysCfg.thermostat1_enable);
      } else {
        os_sprintf(buff, "%d", (int)sysCfg.thermostat1_enable);
      }
    }

    if (os_strcmp(buff, "thermostat_manual_setpoint") == 0) {
      if (connData->post->len > 0) {
        sysCfg.thermostat1_manual_setpoint = (int)atoi(connData->post->buff);
        CFG_Save();
        os_printf("Handle thermostat manual setpoint save (%d)\n", (int)sysCfg.thermostat1_manual_setpoint);
      } else {
        os_sprintf(buff, "%d", (int)sysCfg.thermostat1_manual_setpoint);
      }
    }

    if (os_strcmp(buff, "thermostat_current_setpoint") == 0) {
      if (connData->post->len > 0) {

      } else {
        os_sprintf(buff, "%d", (int)thermostat1CurrentSetPoint);
      }
    }

    if (os_strcmp(buff, "thermostat_schedule_mode") == 0) {
      if (connData->post->len > 0) {
        sysCfg.thermostat1_schedule_mode = (int)atoi(connData->post->buff);
        CFG_Save();
        os_printf("Handle thermostat mode save (%d)\n", (int)sysCfg.thermostat1_schedule_mode);
      } else {
        os_sprintf(buff, "%d", (int)sysCfg.thermostat1_schedule_mode);
      }
    }

    if (os_strcmp(buff, "thermostat_schedule") == 0) {
      char *days[7] = {"mon", "tue", "wed", "thu", "fri", "sat", "sun"};
      if (connData->post->len > 0) {
        os_printf("Handle thermostat schedule save\n");

        int r;
        jsmn_parser p;
        jsmntok_t t[64]; /* We expect no more than 64 tokens per day*/

        jsmn_init(&p);
        r = jsmn_parse(&p, connData->post->buff, strlen(connData->post->buff), t, sizeof(t) / sizeof(t[0]));

        if (r < 0) {
          os_printf("Failed to parse JSON: %d\n", r);
          return HTTPD_CGI_DONE;
        }

        /* Assume the top-level element is an object */
        if (r < 1 || t[0].type != JSMN_OBJECT) {
          os_printf("Object expected\n");
          return HTTPD_CGI_DONE;
        }

        buff[0] = 0x0;

        int found = -1;
        for (int i = 0; i < 7; i++) {
          if (os_memcmp(connData->post->buff + t[1].start, days[i], 3) == 0)
            found = i;
        }

        if (found < 0) {
          os_printf("Could not find day schedule in JSON\n");
          return HTTPD_CGI_DONE;
        }

        os_printf("Schedule for %s found\n", days[found]);

        int sched = 0;
        for (int i = 3; i < r && sched < 8; i += 7) { // skip the day and day array strings

          // Number of tokens will be 1 for the day+1 for the day data + (number of schedules * 7 tokens in a schedule
          // element (one for the schedule itself then 6 tokens for start:val,end:val,setpoint:val)

          os_memcpy(temp, connData->post->buff + t[i + 2].start, t[i + 2].end - t[i + 2].start);
          temp[t[i + 2].end - t[i + 2].start] = 0x0;
          os_sprintf(buff + strlen(buff), "Start = %s\n", temp);
          sysCfg.thermostat1_schedule.weekSched[found].daySched[sched].start = atoi(temp);

          os_memcpy(temp, connData->post->buff + t[i + 4].start, t[i + 4].end - t[i + 4].start);
          temp[t[i + 4].end - t[i + 4].start] = 0x0;
          os_sprintf(buff + strlen(buff), "End = %s\n", temp);
          sysCfg.thermostat1_schedule.weekSched[found].daySched[sched].end = atoi(temp);

          os_memcpy(temp, connData->post->buff + t[i + 6].start, t[i + 6].end - t[i + 6].start);
          temp[t[i + 6].end - t[i + 6].start] = 0x0;
          os_sprintf(buff + strlen(buff), "Setpoint = %s\n", temp);
          sysCfg.thermostat1_schedule.weekSched[found].daySched[sched].setpoint = atoi(temp);

          sysCfg.thermostat1_schedule.weekSched[found].daySched[sched].active = 1;

          sched++;
        }
        if (sched < 8)
          sysCfg.thermostat1_schedule.weekSched[found].daySched[sched].active = 0; // mark the next schedule as inactive

        os_printf(buff);
        CFG_Save();

      } else {
        // Build schedule JSON from the config structure. Keeping the JSON string in memory is quite heavy on the
        // RAM/heap, so we re-construct it
        os_strcpy(buff, "{");
        for (int dow = 0; dow < 7; dow++) {
          os_sprintf(buff + strlen(buff), "\"%s\":[", days[dow]);
          for (int sched = 0; sched < 8 && sysCfg.thermostat1_schedule.weekSched[dow].daySched[sched].active == 1;
               sched++) {
            os_sprintf(buff + strlen(buff), "{\"s\":%d,\"e\":%d,\"sp\":%d}",
                       sysCfg.thermostat1_schedule.weekSched[dow].daySched[sched].start,
                       sysCfg.thermostat1_schedule.weekSched[dow].daySched[sched].end,
                       sysCfg.thermostat1_schedule.weekSched[dow].daySched[sched].setpoint);
            if (sched < 7 && sysCfg.thermostat1_schedule.weekSched[dow].daySched[sched + 1].active == 1)
              os_sprintf(buff + strlen(buff), ",");
          }
          os_sprintf(buff + strlen(buff), "]%s\n", dow < 6 ? "," : "");
        }
        os_sprintf(buff + strlen(buff), "}");
      }
    }
  }

  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}
