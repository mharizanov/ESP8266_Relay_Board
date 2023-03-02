PORT=/dev/cu.usbserial-FTVB6JWH1
#ESPTOOL="$HOME/.platformio/packages/tool-esptoolpy/esptool.py"
ESPTOOL="$HOME/esptool/esptool.py"
echo "Compiling webpages into espfs file"
rm webpages.espfs
make webpages.espfs
echo "Burning webpages to ESP8266 relay board"
echo "Move switch to PGM and hit the reset button"
read -p "Press enter to continue"
python3 $ESPTOOL --port $PORT  --baud 230400 write_flash 0x50000 ./firmware/webpages.espfs 
