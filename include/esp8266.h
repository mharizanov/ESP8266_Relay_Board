// Combined include file for esp8266
#ifndef _ESP8266_H_
#define _ESP8266_H_

#undef MEMLEAK_DEBUG
#define USE_OPTIMIZE_PRINTF

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <user_config.h>

#include <user_interface.h>

#include <c_types.h>
#include <espconn.h>
#include <ets_sys.h>
#include <gpio.h>
#include <lwip/ip_addr.h>
#include <mem.h>

#include <osapi.h>
#include <upgrade.h>

#include "espmissingincludes.h"
#include "uart_hw.h"

#ifdef __WIN32__
#include <_mingw.h>
#endif

#endif // _ESP8266_H_
