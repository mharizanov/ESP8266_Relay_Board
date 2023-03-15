echo "Buring all code to ESP8266 relay board." 
echo "Move switch to PGM and hit the reset button"
read -p "Press enter to continue"

PORT=/dev/cu.usbserial-FTVB6JWH1
ESPTOOL="$HOME/.platformio/packages/tool-esptoolpy/esptool.py"

"$HOME/.platformio/penv/bin/python" $ESPTOOL --before no_reset --after soft_reset --chip esp8266 --port "$PORT" --baud 115200 write_flash 0x10000 ./firmware/0x10000.bin 0x7c000 ./espSDKFirmware/esp_init_data_default.bin 0x7e000 ./espSDKFirmware/blank.bin  0x0 ./firmware/0x00000.bin

echo "Done. Check above for errors"
sleep 2
./burn_webpages.sh

