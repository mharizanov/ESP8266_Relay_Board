#!/bin/sh
sudo ./esptool.py --port /dev/ttyUSB0 write_flash 0x00000 firmware/0x00000.bin
sudo ./esptool.py --port /dev/ttyUSB0 write_flash 0x3C000 blank.bin

