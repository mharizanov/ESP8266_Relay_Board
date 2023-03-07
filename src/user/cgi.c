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
#include "lwip/sntp.h"
#include "mem.h"
#include "time_utils.h"
#include "user_interface.h"
#include <osapi.h>
#include <stdlib.h>
#include <string.h>

void ICACHE_FLASH_ATTR errorResponse(HttpdConnData *connData, int code, char *message) {
  noCacheHeaders(connData, code);
  httpdEndHeaders(connData);
  httpdSend(connData, message, -1);
  os_printf("HTTP %d error response: \"%s\"\n", code, message);
}

void ICACHE_FLASH_ATTR noCacheHeaders(HttpdConnData *connData, int code) {
  httpdStartResponse(connData, code);
  httpdHeader(connData, "Cache-Control", "no-cache, no-store, must-revalidate");
  httpdHeader(connData, "Pragma", "no-cache");
  httpdHeader(connData, "Expires", "0");
}

// Cgi that turns the Relays on or off according to the 'relayX' param in the GET data
int ICACHE_FLASH_ATTR cgiGPIO(HttpdConnData *connData) {
  int len;
  char buff[250];
  int gotcmd = 0;

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }

  len = httpdFindArg(connData->getArgs, "relay1", buff, sizeof(buff));
  if (len > 0) {
    relay1State = atoi(buff);
    relayOnOff(relay1State, 1);
    gotcmd = 1;
  }

  len = httpdFindArg(connData->getArgs, "relay2", buff, sizeof(buff));
  if (len > 0) {
    relay2State = atoi(buff);
    relayOnOff(relay2State, 2);
    gotcmd = 1;
  }

  len = httpdFindArg(connData->getArgs, "relay3", buff, sizeof(buff));
  if (len > 0) {
    relay3State = atoi(buff);
    relayOnOff(relay3State, 3);
    gotcmd = 1;
  }

  if (gotcmd == 1) {
    httpdRedirect(connData, "relay.tpl");
    return HTTPD_CGI_DONE;
  } else { // with no parameters returns JSON with relay state

    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "text/json");
    httpdHeader(connData, "Access-Control-Allow-Origin", "*");
    httpdEndHeaders(connData);
    len =
        os_sprintf(buff,
                   "{\"relay1\": %d,\n\"relay1name\":\"%s\",\n\"relay1_therm_controlled\": %d,\n\"relay2\": "
                   "%d,\n\"relay2name\":\"%s\",\n\"relay2_therm_controlled\": %d,\n "
                   "\"relay3\":%d,\n\"relay3name\":\"%s\",\n\"relay3_therm_controlled\": %d,\n \"relaytotal\": %d\n}\n",
                   relay1State, (char *)sysCfg.relay1_name, sysCfg.relay1_thermostat, relay2State,
                   (char *)sysCfg.relay2_name, sysCfg.relay2_thermostat, relay3State, (char *)sysCfg.relay3_name,
                   sysCfg.relay3_thermostat, sysCfg.relay_total);

    /*
        len = os_sprintf(buff, "{\"relay1\": %d\n,\"relay1name\":\"%s\", \n,\"relay1_therm_controlled\": %d\n, \
                          \"relay2\": %d\n,\"relay2name\":\"%s\", \n\"relay2_therm_controlled\": %d\n, \
                          \"relay3\": %d\n,\"relay3name\":\"%s\", \n\"relay3_therm_controlled\": %d\n, \
                          \"relaytotal\": %d\n}\n",
                         relay1State, (char *)sysCfg.relay1_name, sysCfg.relay1_thermostat, relay2State,
                         (char *)sysCfg.relay2_name, sysCfg.relay2_thermostat, relay3State, (char *)sysCfg.relay3_name,
                         sysCfg.relay3_thermostat, sysCfg.relay_total);

      */
    httpdSend(connData, buff, -1);
    return HTTPD_CGI_DONE;
  }
}

// Template code for the led page.
void ICACHE_FLASH_ATTR tplGPIO(HttpdConnData *connData, char *token, void **arg) {
  char buff[128];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");

  if (os_strcmp(token, "relay1name") == 0) {
    os_strcpy(buff, (char *)sysCfg.relay1_name);
  }

  if (os_strcmp(token, "relay2name") == 0) {
    os_strcpy(buff, (char *)sysCfg.relay2_name);
  }

  if (os_strcmp(token, "relay3name") == 0) {
    os_strcpy(buff, (char *)sysCfg.relay3_name);
  }

  if (os_strcmp(token, "relay1") == 0) {
    if (relay1State) {
      os_strcpy(buff, "on");
    } else {
      os_strcpy(buff, "off");
    }
    os_printf("Relay 1 is now ");
    os_printf(buff);
    os_printf("\n ");
  }
  if (os_strcmp(token, "relay2") == 0) {
    if (relay2State) {
      os_strcpy(buff, "on");
    } else {
      os_strcpy(buff, "off");
    }
    os_printf("Relay 2 is now ");
    os_printf(buff);
    os_printf("\n ");
  }
  if (os_strcmp(token, "relay3") == 0) {
    if (relay3State) {
      os_strcpy(buff, "on");
    } else {
      os_strcpy(buff, "off");
    }
    os_printf("Relay 3 is now ");
    os_printf(buff);
    os_printf("\n ");
  }
  if (os_strcmp(token, "relay-total") == 0) {
    os_sprintf(buff, "%d", sysCfg.relay_total);
  }
  httpdSend(connData, buff, -1);
}

static long hitCounter = 0;

// Template code for the counter on the index page.
void ICACHE_FLASH_ATTR tplCounter(HttpdConnData *connData, char *token, void **arg) {

  char buff[128];
  if (token == NULL)
    return;

  if (os_strcmp(token, "counter") == 0) {
    hitCounter++;
    os_sprintf(buff, "%ld", hitCounter);
  }

  if (os_strcmp(token, "freeheap") == 0) {
    os_sprintf(buff, "Free heap size:%d\n", system_get_free_heap_size());
  }

  if (os_strcmp(token, "fwver") == 0) {
    os_sprintf(buff, "%s", FWVER);
  }

  httpdSend(connData, buff, -1);
}

/*
// Cgi that reads the SPI flash. Assumes 512KByte flash.
int ICACHE_FLASH_ATTR cgiReadFlash(HttpdConnData *connData) {
  int *pos = (int *)&connData->cgiData;
  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }

  if (*pos == 0) {
    os_printf("Start flash download.\n");
    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "application/bin");
    httpdEndHeaders(connData);
    *pos = 0x40200000;
    return HTTPD_CGI_MORE;
  }
  // Send 1K of flash per call. We will get called again if we haven't sent 512K yet.
  espconn_sent(connData->conn, (uint8 *)(*pos), 1024);
  *pos += 1024;
  if (*pos >= 0x40200000 + (512 * 1024))
    return HTTPD_CGI_DONE;
  else
    return HTTPD_CGI_MORE;
}
*/
// Template code for the DHT 22 page.
void ICACHE_FLASH_ATTR tplDHT(HttpdConnData *connData, char *token, void **arg) {
  char buff[128];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");
  if (os_strcmp(token, "temperature") == 0) {
    dht_temp_str(buff);
  }
  if (os_strcmp(token, "humidity") == 0) {
    dht_humi_str(buff);
  }

  httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiDHT22(HttpdConnData *connData) {
  char buff[256];
  char temp[32];
  char humi[32];

  httpdStartResponse(connData, 200);
  httpdHeader(connData, "Content-Type", "text/json");
  httpdHeader(connData, "Access-Control-Allow-Origin", "*");
  httpdEndHeaders(connData);

  dht_temp_str(temp);
  dht_humi_str(humi);

  os_sprintf(buff, "{ \n\"temperature\": \"%s\"\n , \n\"humidity\": \"%s\"\n}\n", temp, humi);

  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}

// Template code for the DS18b20 page.
void ICACHE_FLASH_ATTR tplDS18b20(HttpdConnData *connData, char *token, void **arg) {
  char buff[384];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");

  if (os_strcmp(token, "numds") == 0) {
    os_sprintf(buff, "%d", numds);
  }

  if (os_strcmp(token, "temperatures") == 0) {

    int Treading, SignBit, Whole, Fract;

    os_strcpy(buff, "");

    for (int i = 0; i < numds; i++) {

      if (dsreading[i].success) {
        Treading = dsreading[i].temperature;

        SignBit = Treading & 0x8000;          // test most sig bit
        if (SignBit)                          // negative
          Treading = (Treading ^ 0xffff) + 1; // 2's comp

        Whole = Treading >> 4; // separate off the whole and fractional portions
        Fract = (Treading & 0xf) * 100 / 16;

        if (SignBit) // negative
          Whole *= -1;

        os_sprintf(buff + strlen(buff),
                   "Sensor %d (%02x %02x %02x %02x %02x %02x %02x %02x) reading is %d.%d&#8451;<br />", i + 1,
                   addr[i][0], addr[i][1], addr[i][2], addr[i][3], addr[i][4], addr[i][5], addr[i][6], addr[i][7],
                   Whole, Fract < 10 ? 0 : Fract);
      } else {
        os_sprintf(buff + strlen(buff), "Sensor %d (%02x %02x %02x %02x %02x %02x %02x %02x) reading is invalid<br />",
                   i + 1, addr[i][0], addr[i][1], addr[i][2], addr[i][3], addr[i][4], addr[i][5], addr[i][6],
                   addr[i][7]);
      }
    }
  }

  httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiDS18b20(HttpdConnData *connData) {
  char buff[256];
  char tmp[32];
  httpdStartResponse(connData, 200);
  httpdHeader(connData, "Content-Type", "text/json");
  httpdHeader(connData, "Access-Control-Allow-Origin", "*");
  httpdEndHeaders(connData);

  ds_str(tmp, 0);
  os_sprintf(buff, "{ \n\"temperature\": \"%s\"\n}\n", tmp);

  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR cgiState(HttpdConnData *connData) {
  char buff[512];
  char tmp[32];

  char temp[32];
  char humi[32];

  httpdStartResponse(connData, 200);
  httpdHeader(connData, "Content-Type", "text/json");
  httpdHeader(connData, "Access-Control-Allow-Origin", "*");
  httpdEndHeaders(connData);

  ds_str(tmp, 0);

  dht_temp_str(temp);
  dht_humi_str(humi);

  os_sprintf(buff,
             "{ \n\"relay1\": \"%d\"\n,\n\"relay2\": \"%d\"\n,\n\"relay3\": \"%d\",\n  \n\"DHT22temperature\": "
             "\"%s\"\n , \n\"DHT22humidity\": \"%s\"\n,\"DS18B20temperature\": \"%s\"\n}\n",
             relay1State, relay2State, relay3State, temp, humi, tmp);

  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR cgiSensor(HttpdConnData *connData) {

  char buff[512];
  char sensor[60];
  char ds_temp[8];
  char dht22JSON[128];
  char dht22temp[8];
  char dht22humi[8];

  httpdStartResponse(connData, 200);
  httpdHeader(connData, "Content-Type", "text/json");
  httpdHeader(connData, "Access-Control-Allow-Origin", "*");
  httpdEndHeaders(connData);

  dht_temp_str(dht22temp);
  dht_humi_str(dht22humi);

  if (sysCfg.sensor_ds18b20_enable && numds > 0) {
    os_sprintf(buff, "{\n\n\"ds18b20\":\n[\n");
    // publish all sensor ids and readings to ds18b20 MQTT topic
    for (int i = 0; i < numds; i++) {
      if (dsreading[i].success) {
        ds_str(ds_temp, i);
        os_sprintf(sensor, "\n{\n \"id\":\"%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\",\n\"temperature\":\"%s\"\n}\n",
                   addr[i][0], addr[i][1], addr[i][2], addr[i][3], addr[i][4], addr[i][5], addr[i][6], addr[i][7],
                   ds_temp);
        strcat(buff, sensor);
        if (i != numds - 1) {
          strcat(buff, ",");
        }
      }
      strcat(buff, "\n]\n");
    }
  } else {
    os_sprintf(buff, "{\n\"ds18b20\":[]\n");
  }

  if (sysCfg.sensor_dht22_enable) {
    os_sprintf(dht22JSON, ",\n\"dht22\":[{\"temperature\":\"%s\",\n \"humidity\":\"%s\"\n}\n]\n", dht22temp, dht22humi);
  } else {
    os_sprintf(dht22JSON, ",\n\"dht22\":[]\n");
  }

  strcat(buff, dht22JSON);
  strcat(buff, "\n}");

  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR cgiUI(HttpdConnData *connData) {

  char buff[128];

  httpdStartResponse(connData, 200);
  httpdHeader(connData, "Content-Type", "text/json");
  httpdEndHeaders(connData);

  os_strcpy(buff, "Unknown");
  httpdSend(connData, buff, -1);

  return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplUI(HttpdConnData *connData, char *token, void **arg) {

  char buff[128];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");

  if (os_strcmp(token, "relay1name") == 0) {
    os_strcpy(buff, (char *)sysCfg.relay1_name);
  }

  if (os_strcmp(token, "relay2name") == 0) {
    os_strcpy(buff, (char *)sysCfg.relay2_name);
  }

  if (os_strcmp(token, "relay3name") == 0) {
    os_strcpy(buff, (char *)sysCfg.relay3_name);
  }

  if (os_strcmp(token, "dht22-enable") == 0) {
    if (!sysCfg.sensor_dht22_enable) {
      os_strcpy(buff, " hidden");
    }
  }

  if (os_strcmp(token, "ds18b20-enable") == 0) {
    if (!sysCfg.sensor_ds18b20_enable) {
      os_strcpy(buff, " hidden");
    }
  }
  if (os_strcmp(token, "thermostat-enable") == 0) {
    if (!sysCfg.thermostat1_enable) {
      os_strcpy(buff, " hidden");
    }
  }

  httpdSend(connData, buff, -1);
}

void ICACHE_FLASH_ATTR tplMQTT(HttpdConnData *connData, char *token, void **arg) {

  char buff[128];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");

  if (os_strcmp(token, "mqtt-enable") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.mqtt_enable);
  }

  if (os_strcmp(token, "mqtt-use-ssl") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.mqtt_use_ssl);
  }

  if (os_strcmp(token, "mqtt-host") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_host);
  }

  if (os_strcmp(token, "mqtt-port") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.mqtt_port);
  }

  if (os_strcmp(token, "mqtt-keepalive") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.mqtt_keepalive);
  }

  if (os_strcmp(token, "mqtt-devid") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_devid);
  }

  if (os_strcmp(token, "mqtt-user") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_user);
  }

  if (os_strcmp(token, "mqtt-pass") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_pass);
  }

  if (os_strcmp(token, "mqtt-relay-subs-topic") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_relay_subs_topic);
  }

  if (os_strcmp(token, "mqtt-temp-subs-topic") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_temp_subs_topic);
  }

  if (os_strcmp(token, "mqtt-dht22-temp-pub-topic") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_dht22_temp_pub_topic);
  }

  if (os_strcmp(token, "mqtt-dht22-humi-pub-topic") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_dht22_humi_pub_topic);
  }

  if (os_strcmp(token, "mqtt-ds18b20-temp-pub-topic") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_ds18b20_temp_pub_topic);
  }

  if (os_strcmp(token, "mqtt-state-pub-topic") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_state_pub_topic);
  }

  if (os_strcmp(token, "mqtt-userJSON-pub-topic") == 0) {
    os_strcpy(buff, (char *)sysCfg.mqtt_userJSON_pub_topic);
  }
  httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiMQTT(HttpdConnData *connData) {
  char buff[128];
  int len;

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }

  len = httpdFindArg(connData->post->buff, "mqtt-enable", buff, sizeof(buff));
  sysCfg.mqtt_enable = (len > 0) ? 1 : 0;

  len = httpdFindArg(connData->post->buff, "mqtt-use-ssl", buff, sizeof(buff));
  sysCfg.mqtt_use_ssl = (len > 0) ? 1 : 0;

  len = httpdFindArg(connData->post->buff, "mqtt-host", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_host, buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-port", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.mqtt_port = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-keepalive", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.mqtt_keepalive = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-devid", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_devid, buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-user", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_user, buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-pass", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_pass, buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-relay-subs-topic", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_relay_subs_topic, buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-temp-subs-topic", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_temp_subs_topic, buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-dht22-temp-pub-topic", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_dht22_temp_pub_topic, buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-dht22-humi-pub-topic", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_dht22_humi_pub_topic, buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-ds18b20-temp-pub-topic", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_ds18b20_temp_pub_topic, buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-state-pub-topic", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_state_pub_topic, buff);
  }

  len = httpdFindArg(connData->post->buff, "mqtt-userJSON-pub-topic", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.mqtt_userJSON_pub_topic, buff);
  }

  CFG_Save();

  httpdRedirect(connData, "/");
  return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplHTTPD(HttpdConnData *connData, char *token, void **arg) {

  char buff[128];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");

  if (os_strcmp(token, "httpd-auth") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.httpd_auth);
  }

  if (os_strcmp(token, "httpd-port") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.httpd_port);
  }

  if (os_strcmp(token, "httpd-user") == 0) {
    os_strcpy(buff, (char *)sysCfg.httpd_user);
  }

  if (os_strcmp(token, "httpd-pass") == 0) {
    os_strcpy(buff, (char *)sysCfg.httpd_pass);
  }

  httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiHTTPD(HttpdConnData *connData) {
  char buff[128];
  int len;

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }

  len = httpdFindArg(connData->post->buff, "httpd-auth", buff, sizeof(buff));
  sysCfg.httpd_auth = (len > 0) ? 1 : 0;

  len = httpdFindArg(connData->post->buff, "httpd-port", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.httpd_port = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "httpd-user", buff, sizeof(buff));
  if (len > 0) {
    os_strcpy((char *)sysCfg.httpd_user, buff);
  }

  len = httpdFindArg(connData->post->buff, "httpd-pass", buff, sizeof(buff));
  if (len > 0) {
    os_strcpy((char *)sysCfg.httpd_pass, buff);
  }

  CFG_Save();

  httpdRedirect(connData, "/");
  return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplNTP(HttpdConnData *connData, char *token, void **arg) {

  char buff[128];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");

  if (os_strcmp(token, "ntp-enable") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.ntp_enable);
  }

  if (os_strcmp(token, "ntp-tz") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.ntp_tz);
  }

  if (os_strcmp(token, "dst_disabled") == 0 && sysCfg.DST == 0) {
    os_sprintf(buff, " selected ");
  } else if (os_strcmp(token, "dst_EU") == 0 && sysCfg.DST == 1) {
    os_sprintf(buff, " selected ");
  } else if (os_strcmp(token, "dst_NA") == 0 && sysCfg.DST == 2) {
    os_sprintf(buff, " selected ");
  }

  if (os_strcmp(token, "NTP") == 0) {
    // os_sprintf(buff, "Time: %s GMT%s%02d\n", epoch_to_str(sntp_time), sntp_tz > 0 ? "+" : "", sntp_tz);
    os_sprintf(buff, "Time: %s \n", epoch_to_str(sntp_get_current_timestamp()));
  }

  httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiNTP(HttpdConnData *connData) {
  char buff[128];
  int len;

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }

  len = httpdFindArg(connData->post->buff, "ntp-enable", buff, sizeof(buff));
  sysCfg.ntp_enable = len > 0 ? 1 : 0;

  len = httpdFindArg(connData->post->buff, "ntp-tz", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.ntp_tz = atoi(buff);
    sntp_set_timezone(sysCfg.ntp_tz);
  }

  len = httpdFindArg(connData->post->buff, "DST", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.DST = atoi(buff);
  }

  CFG_Save();

  httpdRedirect(connData, "/");
  return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR restartTimerCb(void *arg) {
  os_printf("Restarting..\n");
  system_restart();
}

int ICACHE_FLASH_ATTR cgiReset(HttpdConnData *connData) {
  static ETSTimer restartTimer;
  // Schedule restart
  os_timer_disarm(&restartTimer);
  os_timer_setfn(&restartTimer, restartTimerCb, NULL);
  os_timer_arm(&restartTimer, 2000, 0);
  httpdRedirect(connData, "restarting.html");
  return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR cgiRLYSettings(HttpdConnData *connData) {

  int len;
  char buff[128];

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }
  len = httpdFindArg(connData->post->buff, "relay-total", buff, sizeof(buff));
  sysCfg.relay_total = (len > 0) ? atoi(buff) : sysCfg.relay_total;

  len = httpdFindArg(connData->post->buff, "relay-latching-enable", buff, sizeof(buff));
  sysCfg.relay_latching_enable = (len > 0) ? 1 : 0;

  len = httpdFindArg(connData->post->buff, "relay1-name", buff, sizeof(buff));
  if (len > 0) {
    os_strcpy((char *)sysCfg.relay1_name, buff);
  }

  len = httpdFindArg(connData->post->buff, "relay2-name", buff, sizeof(buff));
  if (len > 0) {
    os_strcpy((char *)sysCfg.relay2_name, buff);
  }

  len = httpdFindArg(connData->post->buff, "relay3-name", buff, sizeof(buff));
  if (len > 0) {
    os_strcpy((char *)sysCfg.relay3_name, buff);
  }

  len = httpdFindArg(connData->post->buff, "relay1-thermostat", buff, sizeof(buff));
  sysCfg.relay1_thermostat = (len > 0) ? 1 : 0;

  len = httpdFindArg(connData->post->buff, "relay2-thermostat", buff, sizeof(buff));
  sysCfg.relay2_thermostat = (len > 0) ? 1 : 0;

  len = httpdFindArg(connData->post->buff, "relay3-thermostat", buff, sizeof(buff));
  sysCfg.relay3_thermostat = (len > 0) ? 1 : 0;

  len = httpdFindArg(connData->post->buff, "relay1-gpio", buff, sizeof(buff));
  sysCfg.relay1_gpio = (len > 0) ? atoi(buff) : sysCfg.relay1_gpio;

  len = httpdFindArg(connData->post->buff, "relay2-gpio", buff, sizeof(buff));
  sysCfg.relay2_gpio = (len > 0) ? atoi(buff) : sysCfg.relay2_gpio;

  len = httpdFindArg(connData->post->buff, "relay3-gpio", buff, sizeof(buff));
  sysCfg.relay3_gpio = (len > 0) ? atoi(buff) : sysCfg.relay3_gpio;

  CFG_Save();
  httpdRedirect(connData, "/");
  return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplRLYSettings(HttpdConnData *connData, char *token, void **arg) {

  char buff[128];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");

  if (os_strcmp(token, "relay-total-1") == 0 && sysCfg.relay_total == 1) {
    os_strcpy(buff, "selected");
  }

  if (os_strcmp(token, "relay-total-2") == 0 && sysCfg.relay_total == 2) {
    os_strcpy(buff, "selected");
  }

  if (os_strcmp(token, "relay-total-3") == 0 && sysCfg.relay_total == 3) {
    os_strcpy(buff, "selected");
  }

  if (os_strcmp(token, "relay-latching-enable") == 0) {
    os_strcpy(buff, sysCfg.relay_latching_enable == 1 ? "checked" : "");
  }

  if (os_strcmp(token, "relay1-name") == 0) {
    os_strcpy(buff, (char *)sysCfg.relay1_name);
  }

  if (os_strcmp(token, "relay1-thermostat") == 0) {
    os_strcpy(buff, sysCfg.relay1_thermostat == 1 ? "checked" : "");
  }

  if (os_strcmp(token, "relay2-name") == 0) {
    os_strcpy(buff, (char *)sysCfg.relay2_name);
  }

  if (os_strcmp(token, "relay2-thermostat") == 0) {
    os_strcpy(buff, sysCfg.relay2_thermostat == 1 ? "checked" : "");
  }

  if (os_strcmp(token, "relay3-name") == 0) {
    os_strcpy(buff, (char *)sysCfg.relay3_name);
  }

  if (os_strcmp(token, "relay3-thermostat") == 0) {
    os_strcpy(buff, sysCfg.relay3_thermostat == 1 ? "checked" : "");
  }

  httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiSensorSettings(HttpdConnData *connData) {

  int len;
  char buff[128];

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }

  len = httpdFindArg(connData->post->buff, "sensor-ds18b20-enable", buff, sizeof(buff));
  sysCfg.sensor_ds18b20_enable = (len > 0) ? 1 : 0;

  len = httpdFindArg(connData->post->buff, "sensor-dht22-enable", buff, sizeof(buff));
  sysCfg.sensor_dht22_enable = (len > 0) ? 1 : 0;

  CFG_Save();
  httpdRedirect(connData, "/");
  return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplSensorSettings(HttpdConnData *connData, char *token, void **arg) {

  char buff[128];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");

  if (os_strcmp(token, "sensor-ds18b20-enable") == 0) {
    os_strcpy(buff, sysCfg.sensor_ds18b20_enable == 1 ? "checked" : "");
  }

  if (os_strcmp(token, "sensor-dht22-enable") == 0) {
    os_strcpy(buff, sysCfg.sensor_dht22_enable == 1 ? "checked" : "");
  }

  httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiThermostatSettings(HttpdConnData *connData) {

  int len;
  char buff[128];

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }

  len = httpdFindArg(connData->post->buff, "therm1-input", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.thermostat1_input = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "therm1-input", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.thermostat1_input = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "therm1-hysteresis-high", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.thermostat1_hysteresis_high = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "thermostat1-hysteresis-low", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.thermostat1_hysteresis_low = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "therm-low-temp-colour-deg", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.therm_low_temp_colour_deg = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "therm-high-temp-colour-deg", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.therm_high_temp_colour_deg = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "therm-room-temp-timeout-secs", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.therm_room_temp_timeout_secs = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "therm-relay-rest-min", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.therm_relay_rest_min = atoi(buff);
  }

  CFG_Save();
  httpdRedirect(connData, "/");
  return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplThermostatSettings(HttpdConnData *connData, char *token, void **arg) {

  char buff[128];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");

  if (os_strcmp(token, "selected-ds18b20") == 0) {
    os_strcpy(buff, sysCfg.thermostat1_input == 0 ? "selected" : "");
  }

  if (os_strcmp(token, "selected-dht22t") == 0) {
    os_strcpy(buff, sysCfg.thermostat1_input == 1 ? "selected" : "");
  }

  if (os_strcmp(token, "selected-dht22h") == 0) {
    os_strcpy(buff, sysCfg.thermostat1_input == 2 ? "selected" : "");
  }

  if (os_strcmp(token, "selected-mqtt") == 0) {
    os_strcpy(buff, sysCfg.thermostat1_input == 3 ? "selected" : "");
  }

  if (os_strcmp(token, "selected-serial") == 0) {
    os_strcpy(buff, sysCfg.thermostat1_input == 4 ? "selected" : "");
  }

  if (os_strcmp(token, "selected-fixed") == 0) {
    os_strcpy(buff, sysCfg.thermostat1_input == 5 ? "selected" : "");
  }

  if (os_strcmp(token, "therm1-hysteresis-high") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.thermostat1_hysteresis_high);
  }

  if (os_strcmp(token, "therm1-hysteresis-low") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.thermostat1_hysteresis_low);
  }

  if (os_strcmp(token, "therm-low-temp-colour-deg") == 0) {
    os_sprintf(buff, "%d", sysCfg.therm_low_temp_colour_deg);
  }

  if (os_strcmp(token, "therm-high-temp-colour-deg") == 0) {
    os_sprintf(buff, "%d", sysCfg.therm_high_temp_colour_deg);
  }

  if (os_strcmp(token, "therm-room-temp-timeout-secs") == 0) {
    os_sprintf(buff, "%d", sysCfg.therm_room_temp_timeout_secs);
  }

  if (os_strcmp(token, "therm-relay-rest-min") == 0) {
    os_sprintf(buff, "%d", sysCfg.therm_relay_rest_min);
  }

  httpdSend(connData, buff, -1);
}

void ICACHE_FLASH_ATTR tplBroadcastD(HttpdConnData *connData, char *token, void **arg) {

  char buff[255];
  if (token == NULL)
    return;

  os_strcpy(buff, "Unknown");

  if (os_strcmp(token, "broadcastd-enable") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.broadcastd_enable);
  }

  if (os_strcmp(token, "broadcastd-port") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.broadcastd_port);
  }

  if (os_strcmp(token, "broadcastd-host") == 0) {
    os_strcpy(buff, (char *)sysCfg.broadcastd_host);
  }

  if (os_strcmp(token, "broadcastd-URL") == 0) {
    os_strcpy(buff, (char *)sysCfg.broadcastd_url);
  }

  if (os_strcmp(token, "broadcastd-thingspeak-channel") == 0) {
    os_sprintf(buff, "%d", (int)sysCfg.broadcastd_thingspeak_channel);
  }

  if (os_strcmp(token, "broadcastd-ro-apikey") == 0) {
    os_strcpy(buff, (char *)sysCfg.broadcastd_ro_apikey);
  }

  httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiBroadcastD(HttpdConnData *connData) {
  char buff[255];
  int len;

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }

  len = httpdFindArg(connData->getArgs, "js", buff, sizeof(buff));
  if (len > 0) {

    httpdStartResponse(connData, 200);
    httpdHeader(connData, "Content-Type", "text/javascript");
    httpdEndHeaders(connData);

    len = os_sprintf(buff, "var channel=%d;\nvar ro_apikey=\"%s\";\n", (int)sysCfg.broadcastd_thingspeak_channel,
                     (char *)sysCfg.broadcastd_ro_apikey);
    httpdSend(connData, buff, -1);
    return HTTPD_CGI_DONE;
  }

  len = httpdFindArg(connData->post->buff, "broadcastd-enable", buff, sizeof(buff));
  sysCfg.broadcastd_enable = (len > 0) ? 1 : 0;

  len = httpdFindArg(connData->post->buff, "broadcastd-port", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.broadcastd_port = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "broadcastd-host", buff, sizeof(buff));
  if (len > 0) {
    os_strcpy((char *)sysCfg.broadcastd_host, buff);
  }

  len = httpdFindArg(connData->post->buff, "broadcastd-URL", buff, sizeof(buff));
  if (len > 0) {
    os_strcpy((char *)sysCfg.broadcastd_url, buff);
  }

  len = httpdFindArg(connData->post->buff, "broadcastd-thingspeak-channel", buff, sizeof(buff));
  if (len > 0) {
    sysCfg.broadcastd_thingspeak_channel = atoi(buff);
  }

  len = httpdFindArg(connData->post->buff, "broadcastd-ro-apikey", buff, sizeof(buff));
  if (len > 0) {
    os_strcpy((char *)sysCfg.broadcastd_ro_apikey, buff);
  }

  CFG_Save();

  httpdRedirect(connData, "/");
  return HTTPD_CGI_DONE;
}
