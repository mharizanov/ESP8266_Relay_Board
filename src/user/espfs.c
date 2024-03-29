/*
This is a simple read-only implementation of a file system. It uses a block of data coming from the
mkespfsimg tool, and can use that block to do abstracted operations on the files that are in there.
It's written for use with httpd, but doesn't need to be used as such.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

// These routines can also be tested by comping them in with the espfstest tool. This
// simplifies debugging, but needs some slightly different headers. The #ifdef takes
// care of that.

#ifdef __ets__
// esp build
#include "c_types.h"
//#include "espconn.h"
#include "espmissingincludes.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#else
// Test build
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define os_malloc malloc
#define os_free free
#define os_memcpy memcpy
#define os_strncmp strncmp
#define os_strcmp strcmp
#define os_strcpy strcpy
#define os_printf printf
#define ICACHE_FLASH_ATTR
extern char *espFsData;
#endif

#include "../mkespfsimage/espfsformat.h"
#include "espfs.h"
#include "httpdconfig.h"
#ifdef EFS_HEATSHRINK
#include "heatshrink_decoder.h"
#endif

struct EspFsFile {
  EspFsHeader *header;
  char decompressor;
  int32_t posDecomp;
  char *posStart;
  char *posComp;
  void *decompData;
};

/*
Available locations, at least in my flash, with boundaries partially guessed. This
is using 0.9.1/0.9.2 SDK on a not-too-new module.
0x00000 (0x10000): Code/data (RAM data?)
0x10000 (0x02000): Gets erased by something?
0x12000 (0x2E000): Free (filled with zeroes) (parts used by ESPCloud and maybe SSL)
0x40000 (0x20000): Code/data (ROM data?)
0x60000 (0x1C000): Free
0x7c000 (0x04000): Param store
0x80000 - end of flash

Accessing the flash through the mem emulation at 0x40200000 is a bit hairy: All accesses
*must* be aligned 32-bit accesses. Reading a short, byte or unaligned word will result in
a memory exception, crashing the program.
*/

// Copies len bytes over from dst to src, but does it using *only*
// aligned 32-bit reads. Yes, it's no too optimized but it's short and sweet and it works.

// ToDo: perhaps os_memcpy also does unaligned accesses?
void ICACHE_FLASH_ATTR memcpyAligned(char *dst, char *src, int len) {
  int x;
  int w, b;
  for (x = 0; x < len; x++) {
    b = ((int)src & 3);
    w = *((int *)(src - b));
    if (b == 0)
      *dst = (w >> 0);
    if (b == 1)
      *dst = (w >> 8);
    if (b == 2)
      *dst = (w >> 16);
    if (b == 3)
      *dst = (w >> 24);
    dst++;
    src++;
  }
}

// Open a file and return a pointer to the file desc struct.
EspFsFile ICACHE_FLASH_ATTR *espFsOpen(char *fileName) {
#ifdef __ets__
  char *p = (char *)(ESPFS_POS + 0x40200000);
#else
  char *p = espFsData;
#endif
  char *hpos;
  char namebuf[256];
  EspFsHeader h;
  EspFsFile *r;
  // Strip initial slashes
  while (fileName[0] == '/')
    fileName++;
  // Go find that file!
  while (1) {
    hpos = p;
    // Grab the next file header.
    os_memcpy(&h, p, sizeof(EspFsHeader));
    if (h.magic != 0x73665345) {
      os_printf("Magic mismatch. EspFS image broken.\n");
      return NULL;
    }
    if (h.flags & FLAG_LASTFILE) {
      os_printf("End of image.\n");
      return NULL;
    }
    // Grab the name of the file.
    p += sizeof(EspFsHeader);
    os_memcpy(namebuf, p, sizeof(namebuf));
    //		os_printf("Found file '%s'. Namelen=%x fileLenComp=%x, compr=%d flags=%d\n",
    //				namebuf, (unsigned int)h.nameLen, (unsigned int)h.fileLenComp, h.compression, h.flags);
    if (os_strcmp(namebuf, fileName) == 0) {
      // Yay, this is the file we need!
      p += h.nameLen;                                // Skip to content.
      r = (EspFsFile *)os_malloc(sizeof(EspFsFile)); // Alloc file desc mem
      //			os_printf("Alloc %p\n", r);
      if (r == NULL)
        return NULL;
      r->header = (EspFsHeader *)hpos;
      r->decompressor = h.compression;
      r->posComp = p;
      r->posStart = p;
      r->posDecomp = 0;
      if (h.compression == COMPRESS_NONE) {
        r->decompData = NULL;
#ifdef EFS_HEATSHRINK
      } else if (h.compression == COMPRESS_HEATSHRINK) {
        // File is compressed with Heatshrink.
        char parm;
        heatshrink_decoder *dec;
        // Decoder params are stored in 1st byte.
        memcpyAligned(&parm, r->posComp, 1);
        r->posComp++;
        os_printf("Heatshrink compressed file; decode parms = %x\n", parm);
        dec = heatshrink_decoder_alloc(16, (parm >> 4) & 0xf, parm & 0xf);
        r->decompData = dec;
#endif
      } else {
        os_printf("Invalid compression: %d\n", h.compression);
        return NULL;
      }
      return r;
    }
    // We don't need this file. Skip name and file
    p += h.nameLen + h.fileLenComp;
    if ((int)p & 3)
      p += 4 - ((int)p & 3); // align to next 32bit val
  }
}

// Read len bytes from the given file into buff. Returns the actual amount of bytes read.
int ICACHE_FLASH_ATTR espFsRead(EspFsFile *fh, char *buff, int len) {
  int flen;
  if (fh == NULL)
    return 0;
  // Cache file length.
  memcpyAligned((char *)&flen, (char *)&fh->header->fileLenComp, 4);
  // Do stuff depending on the way the file is compressed.
  if (fh->decompressor == COMPRESS_NONE) {
    int toRead;
    toRead = flen - (fh->posComp - fh->posStart);
    if (len > toRead)
      len = toRead;
    //		os_printf("Reading %d bytes from %x\n", len, (unsigned int)fh->posComp);
    memcpyAligned(buff, fh->posComp, len);
    fh->posDecomp += len;
    fh->posComp += len;
    //		os_printf("Done reading %d bytes, pos=%x\n", len, fh->posComp);
    return len;
#ifdef EFS_HEATSHRINK
  } else if (fh->decompressor == COMPRESS_HEATSHRINK) {
    int decoded = 0;
    unsigned int elen, rlen;
    char ebuff[16];
    heatshrink_decoder *dec = (heatshrink_decoder *)fh->decompData;
    //		os_printf("Alloc %p\n", dec);
    while (decoded < len) {
      // Feed data into the decompressor
      // ToDo: Check ret val of heatshrink fns for errors
      elen = flen - (fh->posComp - fh->posStart);
      if (elen == 0)
        return decoded; // file is read
      if (elen > 0) {
        memcpyAligned(ebuff, fh->posComp, 16);
        heatshrink_decoder_sink(dec, (uint8_t *)ebuff, (elen > 16) ? 16 : elen, &rlen);
        fh->posComp += rlen;
        if (rlen == elen) {
          heatshrink_decoder_finish(dec);
        }
      }
      // Grab decompressed data and put into buff
      heatshrink_decoder_poll(dec, (uint8_t *)buff, len - decoded, &rlen);
      fh->posDecomp += rlen;
      buff += rlen;
      decoded += rlen;
    }
    return len;
#endif
  }
  return 0;
}

// Close the file.
void ICACHE_FLASH_ATTR espFsClose(EspFsFile *fh) {
  if (fh == NULL)
    return;
#ifdef EFS_HEATSHRINK
  if (fh->decompressor == COMPRESS_HEATSHRINK) {
    heatshrink_decoder *dec = (heatshrink_decoder *)fh->decompData;
    heatshrink_decoder_free(dec);
    //		os_printf("Freed %p\n", dec);
  }
#endif
  //	os_printf("Freed %p\n", fh);
  os_free(fh);
}
