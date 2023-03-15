#define DEFAULT_LAN_PORT 12476
#include "lwip/ip_addr.h"

#include "config.h"
#include "espconn.h"
#include "mem.h"
#include "stdarg.h"

#include <stdlib.h>

struct espconn pssdpudpconn;
esp_udp ssdp_udp;

void parseBytes(const char *str, char sep, uint8_t *bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base); // Convert byte
    str = strchr(str, sep);              // Find next separator
    if (str == NULL || *str == '\0') {
      break; // No more separators, exit
    }
    str++; // Point to next character after separator
  }
}

void syslog(const char *format, ...) {

  char buff[256] = "";
  char sysid[12] = "";
  ssdp_udp.remote_port = 514;

  os_sprintf(sysid, "%02X: ", system_get_chip_id());
  strcat(buff, sysid);

  va_list args;
  va_start(args, format);
  ets_vsnprintf(&buff[7], 244, format, args);
  va_end(args);

  if (sysCfg.syslog_enable) {

    parseBytes((const char *)sysCfg.syslog_host, '.', ssdp_udp.remote_ip, 4, 10);

    /*
      ssdp_udp.remote_ip[0] = 192;
      ssdp_udp.remote_ip[1] = 168;
      ssdp_udp.remote_ip[2] = 10;
      ssdp_udp.remote_ip[3] = 7;
    */
    ssdp_udp.local_port = DEFAULT_LAN_PORT;
    pssdpudpconn.type = ESPCONN_UDP;
    pssdpudpconn.proto.udp = &(ssdp_udp);

    espconn_create(&pssdpudpconn);

    int len = strlen(buff);
    int ret = espconn_sendto(&pssdpudpconn, (uint8_t *)buff, len);
    if (ret != 0) {
      os_printf("syslog: UDP send error!");
    }
  }
}
