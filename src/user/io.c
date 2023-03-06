
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

#include "c_types.h"
#include "ets_sys.h"
#include "user_interface.h"
//#include "espconn.h"
#include "config.h"
#include "gpio.h"
#include "mem.h"
#include "osapi.h"

#include "espmissingincludes.h"
#define BTNGPIO 0

static ETSTimer resetBtntimer;

char relay1State = 0;
char relay2State = 0;
char relay3State = 0;

void ICACHE_FLASH_ATTR ioGPIO(int ena, int gpio) {
  // gpio_output_set is overkill. ToDo: use better macros
  if (ena) {
    gpio_output_set((1 << gpio), 0, (1 << gpio), 0);
  } else {
    gpio_output_set(0, (1 << gpio), (1 << gpio), 0);
  }
}

void ICACHE_FLASH_ATTR relayOnOff(int onOff, int relayNumber) {

  if (relayNumber == 1 && !sysCfg.relay1_thermostat && sysCfg.relay_total > 0) {
    ioGPIO(onOff, RELAY1GPIO);
    relay1State = onOff;
    if (sysCfg.relay_latching_enable) {
      sysCfg.relay1_state = onOff;
      CFG_Save();
    }
  } else if (relayNumber == 2 && !sysCfg.relay2_thermostat && sysCfg.relay_total > 1) {
    ioGPIO(onOff, RELAY2GPIO);
    relay2State = onOff;
    if (sysCfg.relay_latching_enable) {
      sysCfg.relay2_state = onOff;
      CFG_Save();
    }
  } else if (relayNumber == 3 && !sysCfg.relay3_thermostat && sysCfg.relay_total > 2) {
    ioGPIO(onOff, RELAY3GPIO);
    relay3State = onOff;
    if (sysCfg.relay_latching_enable) {
      sysCfg.relay3_state = onOff;
      CFG_Save();
    }
  } else {
    os_printf(
        "relayOnOff:Invalid relay number %d or relay is controlled by thermostat only (configured for %d relays).\n",
        relayNumber, sysCfg.relay_total);
  }
}

static void ICACHE_FLASH_ATTR resetBtnTimerCb(void *arg) {
  static int resetCnt = 0;
  if (!GPIO_INPUT_GET(BTNGPIO)) {
    resetCnt++;
  } else {
    if (resetCnt >= 6) { // 3 sec pressed
      wifi_station_disconnect();
      wifi_set_opmode(0x3); // reset to AP+STA mode
      os_printf("Reset to AP mode. Restarting system...\n");
      system_restart();
    }
    resetCnt = 0;
  }
}

void ICACHE_FLASH_ATTR ioInit() {

  // Set GPIO0, GPIO12-14 to output mode
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);

  // set GPIO to init state
  GPIO_OUTPUT_SET(0, (1 << 0));
  // GPIO_OUTPUT_SET(2,0);

  if (sysCfg.relay_latching_enable) {

    os_printf("Relay latching is %d, Relay1=%d,Relay2=%d,Relay3=%d\n\r", (int)sysCfg.relay_latching_enable,
              (int)sysCfg.relay1_state, (int)sysCfg.relay2_state, (int)sysCfg.relay3_state);

    relay1State = (int)sysCfg.relay1_state;
    relay2State = (int)sysCfg.relay2_state;
    relay3State = (int)sysCfg.relay3_state;

    ioGPIO((int)sysCfg.relay1_state, RELAY1GPIO);
    ioGPIO((int)sysCfg.relay2_state, RELAY2GPIO);
    ioGPIO((int)sysCfg.relay3_state, RELAY3GPIO);
  }

  else {
    ioGPIO(0, RELAY1GPIO);
    ioGPIO(0, RELAY2GPIO);
    ioGPIO(0, RELAY3GPIO);
  }

  // gpio_output_set(0, 0, (1<<12), (1<<BTNGPIO));
  // gpio_output_set(0, 0, (1<<13), (1<<14));

  os_timer_disarm(&resetBtntimer);
  os_timer_setfn(&resetBtntimer, resetBtnTimerCb, NULL);
  os_timer_arm(&resetBtntimer, 500, 1);
}
