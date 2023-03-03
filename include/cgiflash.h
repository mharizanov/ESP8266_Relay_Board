#ifndef CGIFLASH_H
#define CGIFLASH_H

#define FW_MAX_SIZE 503808
#include "httpd.h"
#include "spi_flash.h"

int cgiReadFlash(HttpdConnData *connData);
int cgiGetFirmwareNext(HttpdConnData *connData);
int cgiGetFirmwareVersion(HttpdConnData *connData);
int cgiUploadFirmware(HttpdConnData *connData);
int cgiRebootFirmware(HttpdConnData *connData);
// int cgiReset(HttpdConnData *connData);

#endif
