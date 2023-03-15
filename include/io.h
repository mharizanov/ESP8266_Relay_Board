void ICACHE_FLASH_ATTR ioGPIO(int ena, int gpio);
void ICACHE_FLASH_ATTR relayOnOff(int onOff, int relayNumber);

void ioInit(void);
extern char relay1State;
extern char relay2State;
extern char relay3State;