#!/bin/sh
sudo ./esptool.py --port /dev/ttyUSB0 write_flash 0x40000 firmware/0x40000.bin

