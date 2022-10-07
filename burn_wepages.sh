rm webpages.espfs
make webpages.espfs
python3 /Users/apearson/.platformio/packages/tool-esptoolpy/esptool.py --port /dev/cu.usbserial-FTVB6JWH1  --baud 115200 write_flash 0x50000 webpages.espfs 
