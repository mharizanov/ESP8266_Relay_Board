#define DEFAULT_LAN_PORT 12476
#include "lwip/ip_addr.h"

#include "espconn.h"
#include "mem.h"
#include "stdarg.h"
#include <stdlib.h>

#ifdef SYSLOGDBG

struct espconn pssdpudpconn;
esp_udp ssdp_udp;

void syslog(const char *format, ...) {

  char buff[256] = "";
  char sysid[12] = "";
  ssdp_udp.remote_port = 514;
  ssdp_udp.remote_ip[0] = 192;
  ssdp_udp.remote_ip[1] = 168;
  ssdp_udp.remote_ip[2] = 10;
  ssdp_udp.remote_ip[3] = 7;
  ssdp_udp.local_port = DEFAULT_LAN_PORT;
  pssdpudpconn.type = ESPCONN_UDP;
  pssdpudpconn.proto.udp = &(ssdp_udp);

  espconn_create(&pssdpudpconn);

  os_sprintf(sysid, "%02X: ", system_get_chip_id());
  strcat(buff, sysid);

  va_list args;
  va_start(args, format);
  ets_vsnprintf(&buff[7], 244, format, args);
  va_end(args);

  int len = strlen(buff);

  int ret = espconn_sendto(&pssdpudpconn, (uint8_t *)buff, len);
  if (ret != 0) {
    os_printf("syslog: UDP send error!");
  }
}
#endif