#!/bin/sh
sudo ./esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash 0x00000 firmware/0x00000.bin
sleep 5
sudo ./esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash 0x40000 firmware/0x40000.bin
sleep 5
sudo ./esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash 0x12000 webpages.espfs
sleep 5
sudo ./esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash 0x3C000 blank.bin

