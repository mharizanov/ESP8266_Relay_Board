PORT=/dev/cu.usbserial-FTVB6JWH1
ESPTOOL="$HOME/.platformio/packages/tool-esptoolpy/esptool.py"
echo "Compiling webpages into espfs file"
rm webpages.espfs
make webpages.espfs
echo "Burning webpages to ESP8266 relay board"
python3 $ESPTOOL --port $PORT  --baud 115200 write_flash 0x50000 webpages.espfs 
