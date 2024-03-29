/*
Cgi/template routines for the /wifi url.
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
#include "espmissingincludes.h"
#include "httpd.h"
#include "io.h"
#include "mem.h"
#include "user_interface.h"
#include <osapi.h>
#include <stdlib.h>
#include <string.h>

// Enable this to disallow any changes in AP settings
//#define DEMO_MODE

// WiFi access point data
typedef struct {
  char ssid[32];
  char rssi;
  char enc;
} ApData;

// Scan result
typedef struct {
  char scanInProgress; // if 1, don't access the underlying stuff from the webpage.
  ApData **apData;
  int noAps;
} ScanResultData;

// Static scan status storage.
static ScanResultData cgiWifiAps;

#define CONNTRY_IDLE 0
#define CONNTRY_WORKING 1
#define CONNTRY_SUCCESS 2
#define CONNTRY_FAIL 3
// Connection result var
static int connTryStatus = CONNTRY_IDLE;
static ETSTimer resetTimer;

// Callback the code calls when a wlan ap scan is done. Basically stores the result in
// the cgiWifiAps struct.
void ICACHE_FLASH_ATTR wifiScanDoneCb(void *arg, STATUS status) {
  int n;
  struct bss_info *bss_link = (struct bss_info *)arg;
  os_printf("wifiScanDoneCb %d\n", status);
  if (status != OK) {
    cgiWifiAps.scanInProgress = 0;
    return;
  }

  // Clear prev ap data if needed.
  if (cgiWifiAps.apData != NULL) {
    for (n = 0; n < cgiWifiAps.noAps; n++)
      os_free(cgiWifiAps.apData[n]);
    os_free(cgiWifiAps.apData);
  }

  // Count amount of access points found.
  n = 0;
  while (bss_link != NULL) {
    bss_link = bss_link->next.stqe_next;
    n++;
  }
  // Allocate memory for access point data
  cgiWifiAps.apData = (ApData **)os_malloc(sizeof(ApData *) * n);
  cgiWifiAps.noAps = n;
  os_printf("Scan done: found %d APs\n", n);

  // Copy access point data to the static struct
  n = 0;
  bss_link = (struct bss_info *)arg;
  while (bss_link != NULL) {
    if (n >= cgiWifiAps.noAps) {
      // This means the bss_link changed under our nose. Shouldn't happen!
      // Break because otherwise we will write in unallocated memory.
      os_printf("Huh? I have more than the allocated %d aps!\n", cgiWifiAps.noAps);
      break;
    }
    // Save the ap data.
    cgiWifiAps.apData[n] = (ApData *)os_malloc(sizeof(ApData));
    cgiWifiAps.apData[n]->rssi = bss_link->rssi;
    cgiWifiAps.apData[n]->enc = bss_link->authmode;
    strncpy(cgiWifiAps.apData[n]->ssid, (char *)bss_link->ssid, 32);

    bss_link = bss_link->next.stqe_next;
    n++;
  }
  // We're done.
  cgiWifiAps.scanInProgress = 0;
}

// Routine to start a WiFi access point scan.
static void ICACHE_FLASH_ATTR wifiStartScan() {
  //	int x;
  if (cgiWifiAps.scanInProgress)
    return;
  cgiWifiAps.scanInProgress = 1;
  wifi_station_scan(NULL, wifiScanDoneCb);
}

// This CGI is called from the bit of AJAX-code in wifi.tpl. It will initiate a
// scan for access points and if available will return the result of an earlier scan.
// The result is embedded in a bit of JSON parsed by the javascript in wifi.tpl.
int ICACHE_FLASH_ATTR cgiWiFiScan(HttpdConnData *connData) {
  int len;
  int i;
  char buff[512];
  httpdStartResponse(connData, 200);
  httpdHeader(connData, "Content-Type", "text/json");
  httpdEndHeaders(connData);

  if (cgiWifiAps.scanInProgress == 1) {
    // We're still scanning. Tell Javascript code that.
    len = os_sprintf(buff, "{\n \"result\": { \n\"inProgress\": \"1\"\n }\n}\n");
    httpdSend(connData, buff, len);
  } else {
    // We have a scan result. Pass it on.
    len = os_sprintf(buff, "{\n \"result\": { \n\"inProgress\": \"0\", \n\"APs\": [\n");
    httpdSend(connData, buff, len);
    if (cgiWifiAps.apData == NULL)
      cgiWifiAps.noAps = 0;
    for (i = 0; i < cgiWifiAps.noAps; i++) {
      // Fill in json code for an access point
      len = os_sprintf(buff, "{\"essid\": \"%s\", \"rssi\": \"%d\", \"enc\": \"%d\"}%s\n", cgiWifiAps.apData[i]->ssid,
                       cgiWifiAps.apData[i]->rssi, cgiWifiAps.apData[i]->enc, (i == cgiWifiAps.noAps - 1) ? "" : ",");
      httpdSend(connData, buff, len);
    }
    len = os_sprintf(buff, "]\n}\n}\n");
    httpdSend(connData, buff, len);
    // Also start a new scan.
    wifiStartScan();
  }
  return HTTPD_CGI_DONE;
}

// Temp store for new ap info.
static struct station_config stconf;

// This routine is ran some time after a connection attempt to an access point. If
// the connect succeeds, this gets the module in STA-only mode.
static void ICACHE_FLASH_ATTR resetTimerCb(void *arg) {
  int x = wifi_station_get_connect_status();
  if (x == STATION_GOT_IP) {
    // Go to STA mode. This needs a reset, so do that.
    os_printf("Got IP. Going into STA mode..\n");
    wifi_set_opmode(1);
    system_restart();
  } else {
    connTryStatus = CONNTRY_FAIL;
    os_printf("Connect fail. Not going into STA-only mode.\n");
    // Maybe also pass this through on the webpage?
  }
}

// Actually connect to a station. This routine is timed because I had problems
// with immediate connections earlier. It probably was something else that caused it,
// but I can't be arsed to put the code back :P
static void ICACHE_FLASH_ATTR reassTimerCb(void *arg) {
  int x;
  os_printf("Try to connect to AP....\n");
  wifi_station_disconnect();
  wifi_station_set_config(&stconf);
  wifi_station_connect();
  x = wifi_get_opmode();
  if (x != 1) {
    // Schedule disconnect/connect
    os_timer_disarm(&resetTimer);
    os_timer_setfn(&resetTimer, resetTimerCb, NULL);
    os_timer_arm(&resetTimer, 15000, 0); // time out after 15 secs of trying to connect
  }
}

// This cgi uses the routines above to connect to a specific access point with the
// given ESSID using the given password.
int ICACHE_FLASH_ATTR cgiWiFiConnect(HttpdConnData *connData) {
  char essid[128];
  char passwd[128];
  char buff[128];
  static ETSTimer reassTimer;
  int len, gotcmd = 0;

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }

  httpdFindArg(connData->post->buff, "essid", essid, sizeof(essid));
  httpdFindArg(connData->post->buff, "passwd", passwd, sizeof(passwd));

  os_strncpy((char *)stconf.ssid, essid, 32);
  os_strncpy((char *)stconf.password, passwd, 64);
  os_printf("Try to connect to AP %s pw %s\n", essid, passwd);

  os_sprintf((char *)sysCfg.sta_ssid, essid);
  os_sprintf((char *)sysCfg.sta_pass, passwd);

  len = httpdFindArg(connData->post->buff, "sta-mode", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.sta_mode, buff);
    gotcmd = 1;
  }

  len = httpdFindArg(connData->post->buff, "sta-ip", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.sta_ip, buff);
    gotcmd = 1;
  }

  len = httpdFindArg(connData->post->buff, "sta-mask", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.sta_mask, buff);
    gotcmd = 1;
  }

  len = httpdFindArg(connData->post->buff, "sta-gw", buff, sizeof(buff));
  if (len > 0) {
    os_sprintf((char *)sysCfg.sta_gw, buff);
    gotcmd = 1;
  }

  if (gotcmd == 1) {
    CFG_Save();
  }
  // Schedule disconnect/connect
  os_timer_disarm(&reassTimer);
  os_timer_setfn(&reassTimer, reassTimerCb, NULL);
// Set to 0 if you want to disable the actual reconnecting bit
#ifdef DEMO_MODE
  httpdRedirect(connData, "/config/wifi");
#else
  os_timer_arm(&reassTimer, 500, 0);
  httpdRedirect(connData, "connecting.html");
#endif
  return HTTPD_CGI_DONE;
}

// This cgi uses the routines above to connect to a specific access point with the
// given ESSID using the given password.
int ICACHE_FLASH_ATTR cgiWiFiSetMode(HttpdConnData *connData) {
  int len;
  char buff[128];

  if (connData->conn == NULL) {
    // Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }

  len = httpdFindArg(connData->getArgs, "mode", buff, sizeof(buff));
  if (len != 0) {
    os_printf("cgiWifiSetMode: %s\n", buff);
#ifndef DEMO_MODE
    wifi_set_opmode(atoi(buff));
    system_restart();
#endif
  }
  httpdRedirect(connData, "/config/wifi");
  return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR cgiWiFiConnStatus(HttpdConnData *connData) {
  char buff[512];
  int len;
  struct ip_info info;
  int st = wifi_station_get_connect_status();
  httpdStartResponse(connData, 200);
  httpdHeader(connData, "Content-Type", "text/json");
  httpdEndHeaders(connData);
  if (connTryStatus == CONNTRY_IDLE) {
    len = os_sprintf(buff, "{\n \"status\": \"idle\"\n }\n");
  } else if (connTryStatus == CONNTRY_WORKING || connTryStatus == CONNTRY_SUCCESS) {
    if (st == STATION_GOT_IP) {
      wifi_get_ip_info(0, &info);
      len = os_sprintf(buff, "{\n \"status\": \"success\",\n \"ip\": \"%d.%d.%d.%d\" }\n",
                       (int)(info.ip.addr >> 0) & 0xff, (int)(info.ip.addr >> 8) & 0xff,
                       (int)(info.ip.addr >> 16) & 0xff, (int)(info.ip.addr >> 24) & 0xff);
      // Reset into AP-only mode sooner.
      os_timer_disarm(&resetTimer);
      os_timer_setfn(&resetTimer, resetTimerCb, NULL);
      os_timer_arm(&resetTimer, 1000, 0);
    } else {
      len = os_sprintf(buff, "{\n \"status\": \"working\"\n }\n");
    }
  } else {
    len = os_sprintf(buff, "{\n \"status\": \"fail\"\n }\n");
  }

  httpdSend(connData, buff, len);
  return HTTPD_CGI_DONE;
}

// Template code for the WLAN page.
int ICACHE_FLASH_ATTR tplWlan(HttpdConnData *connData, char *token, void **arg) {
  char buff[512];
  int x;
  static struct station_config stconf;
  if (token == NULL)
    return HTTPD_CGI_DONE;
  wifi_station_get_config(&stconf);

  os_strcpy(buff, "Unknown");
  if (os_strcmp(token, "WiFiMode") == 0) {
    x = wifi_get_opmode();
    if (x == 1)
      os_strcpy(buff, "Client");
    if (x == 2)
      os_strcpy(buff, "SoftAP");
    if (x == 3)
      os_strcpy(buff, "STA+AP");
  } else if (os_strcmp(token, "currSsid") == 0) {
    os_strcpy(buff, (char *)stconf.ssid);
  } else if (os_strcmp(token, "WiFiPasswd") == 0) {
    os_strcpy(buff, (char *)stconf.password);
  } else if (os_strcmp(token, "WiFiapwarn") == 0) {
    x = wifi_get_opmode();
    if (x == 2) {
      os_strcpy(buff,
                "<b>Can't scan in this mode.</b> Click <a href=\"setmode.cgi?mode=3\">here</a> to go to STA+AP mode.");
    } else {
      os_strcpy(buff, "Click <a href=\"setmode.cgi?mode=2\">here</a> to go to standalone AP mode.");
    }
  } else if (os_strcmp(token, "IPAddress") == 0) {
    struct ip_info pTempIp;
    wifi_get_ip_info(0x00, &pTempIp);
    os_sprintf(buff, "%d.%d.%d.%d\r\n", IP2STR(&pTempIp.ip));
  } else if (os_strcmp(token, "selecteddhcp") == 0) {
    if (os_strcmp((char *)sysCfg.sta_mode, "dhcp") == 0) {
      os_strcpy(buff, "selected = \"selected\"");
    } else
      os_strcpy(buff, " ");
  } else if (os_strcmp(token, "selectedstatic") == 0) {
    if (os_strcmp((char *)sysCfg.sta_mode, "static") == 0) {
      os_strcpy(buff, "selected = \"selected\"");
    } else
      os_strcpy(buff, " ");
  } else if (os_strcmp(token, "sta-ip") == 0) {
    os_strcpy(buff, (char *)sysCfg.sta_ip);
  } else if (os_strcmp(token, "sta-mask") == 0) {
    os_strcpy(buff, (char *)sysCfg.sta_mask);
  } else if (os_strcmp(token, "sta-gw") == 0) {
    os_strcpy(buff, (char *)sysCfg.sta_gw);
  }

  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}
