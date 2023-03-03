echo "Buring all code to ESP8266 relay board." 
echo "Move switch to PGM and hit the reset button"
read -p "Press enter to continue"

PORT=/dev/cu.usbserial-FTVB6JWH1
#ESPTOOL="$HOME/.platformio/packages/tool-esptoolpy/esptool.py"
ESPTOOL="$HOME/esptool/esptool.py"

SDK_BASE=$HOME/esp/ESP8266_NONOS_SDK

#"$HOME/.platformio/penv/bin/python" $ESPTOOL --before no_reset --after soft_reset --chip esp8266 --port "$PORT" --baud 115200 write_flash -fs 1MB 0x1000 ./firmware/user1.bin 0xfc000 $SDK_BASE/bin/esp_init_data_default_v08.bin 0xfe000 $SDK_BASE/bin/blank.bin  0x0 $SDK_BASE/bin/boot_v1.7.bin
"$HOME/.platformio/penv/bin/python" $ESPTOOL --before no_reset --after soft_reset --chip esp8266 --port "$PORT" --baud 230400 write_flash -fs 1MB 0x1000 ./firmware/user1.bin 0xfc000 $SDK_BASE/bin/esp_init_data_default_v08.bin 0xfe000 $SDK_BASE/bin/blank.bin  0x0 $SDK_BASE/bin/boot_v1.7.bin

echo "Done. Check above for errors"
sleep 2
./burn_webpages.sh

