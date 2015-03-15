#!/bin/sh
sudo ./esptool.py --port /dev/ttyUSB0 write_flash 0x12000 webpages.espfs

