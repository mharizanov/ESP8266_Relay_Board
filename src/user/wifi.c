/*
 * wifi.c
 *
 *  Created on: Dec 30, 2014
 *      Author: Minh
 */

#include "user_interface.h"

#include "config.h"
#include "debug.h"
#include "espconn.h"
#include "mem.h"
#include "os_type.h"
#include "osapi.h"
#include "wifi.h"

static ETSTimer WiFiLinker;
WifiCallback wifiCb = NULL;
static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;

static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg) {
  struct ip_info ipConfig;

  os_timer_disarm(&WiFiLinker);
  wifi_get_ip_info(STATION_IF, &ipConfig);
  wifiStatus = wifi_station_get_connect_status();
  if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0) {
    os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
    os_timer_arm(&WiFiLinker, 2000, 0);
  } else {
    if (wifi_station_get_connect_status() == STATION_WRONG_PASSWORD) {
      INFO("STATION_WRONG_PASSWORD\r\n");
      wifi_station_connect();
    } else if (wifi_station_get_connect_status() == STATION_NO_AP_FOUND) {

      INFO("STATION_NO_AP_FOUND\r\n");
      wifi_station_connect();
    } else if (wifi_station_get_connect_status() == STATION_CONNECT_FAIL) {

      INFO("STATION_CONNECT_FAIL\r\n");
      wifi_station_connect();
    } else if (wifi_station_get_connect_status() == STATION_IDLE) {
      INFO("STATION_IDLE\r\n");
    } else if (wifi_station_get_connect_status() == STATION_CONNECTING) {
      INFO("STATION_CONNECTING\r\n");
    } else if (wifi_station_get_connect_status() == STATION_GOT_IP) {
      INFO("STATION_STATION_GOT_IP\r\n");
    } else {
      os_printf("%s: status = %d\n\r", __func__, wifi_station_get_connect_status());
    }

    os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
    os_timer_arm(&WiFiLinker, 500, 0);
  }
  if (wifiStatus != lastWifiStatus) {
    lastWifiStatus = wifiStatus;
    if (wifiCb)
      wifiCb(wifiStatus);
  }
}

void ICACHE_FLASH_ATTR WIFI_Connect(WifiCallback cb) {
  struct station_config stationConf;
  struct ip_info info;

  INFO("WIFI_INIT\r\n");
  os_timer_disarm(&WiFiLinker);

  // wifi_set_opmode(STATION_MODE);
  wifi_station_set_auto_connect(FALSE);

  wifiCb = cb;

  os_memset(&stationConf, 0, sizeof(struct station_config));

  os_sprintf((char *)stationConf.ssid, "%s", sysCfg.sta_ssid);
  os_sprintf((char *)stationConf.password, "%s", sysCfg.sta_pass);

  wifi_get_ip_info(STATION_IF, &info);
  char *dhcp = (char *)sysCfg.sta_mode;
  char *ip, *mask, *gw;
  if (!dhcp || strcmp(dhcp, "dhcp") != 0) {
    ip = (char *)sysCfg.sta_ip;
    mask = (char *)sysCfg.sta_mask;
    gw = (char *)sysCfg.sta_gw;
    if (ip)
      info.ip.addr = ipaddr_addr(ip);
    if (mask)
      info.netmask.addr = ipaddr_addr(mask);
    if (gw)
      info.gw.addr = ipaddr_addr(gw);
    wifi_set_ip_info(STATION_IF, &info);
  }

  wifi_get_ip_info(SOFTAP_IF, &info);
  ip = (char *)sysCfg.ap_ip;
  mask = (char *)sysCfg.ap_mask;
  gw = (char *)sysCfg.ap_gw;
  if (ip)
    info.ip.addr = ipaddr_addr(ip);
  if (mask)
    info.netmask.addr = ipaddr_addr(mask);
  if (gw)
    info.gw.addr = ipaddr_addr(gw);

  if (wifi_get_opmode() != STATION_MODE)
    wifi_set_ip_info(SOFTAP_IF, &info);

  wifi_station_set_config(&stationConf);

  os_timer_disarm(&WiFiLinker);
  os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
  os_timer_arm(&WiFiLinker, 1000, 0);

  wifi_station_set_auto_connect(TRUE);
  wifi_station_connect();
}
