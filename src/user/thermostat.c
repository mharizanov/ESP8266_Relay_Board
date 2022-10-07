
#include "config.h"
#include "dht22.h"
#include "ds18b20.h"
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "gpio.h"
#include "io.h"
#include "lwip/sntp.h"
#include "mqtt.h"
#include "os_type.h"
#include "osapi.h"
#include "stdout.h"
#include "time_utils.h"

#include "thermostat.h"

#include <stdlib.h>

time_t thermostatRelayOffTime = 0;

static int ICACHE_FLASH_ATTR wd(int year, int month, int day) {
  size_t JND = day + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5) +
               (365 * (year + 4800 - ((14 - month) / 12))) + ((year + 4800 - ((14 - month) / 12)) / 4) -
               ((year + 4800 - ((14 - month) / 12)) / 100) + ((year + 4800 - ((14 - month) / 12)) / 400) - 32045;
  return (int)JND % 7;
}

void ICACHE_FLASH_ATTR thermostat(int current_t, int setpoint) {
  if (current_t < setpoint - sysCfg.thermostat1hysteresislow) {
    os_printf("Thermostat: Current temperature (%d) is below setpoint.\n", current_t);
    if (sysCfg.thermostat1opmode == THERMOSTAT_HEATING)
      thermostatRelayOn();
    else
      thermostatRelayOff();
  } else if (current_t > setpoint + sysCfg.thermostat1hysteresishigh) {
    os_printf("Thermostaat: Current temperature (%d) is above setpoint.\n", current_t);
    if (sysCfg.thermostat1opmode == THERMOSTAT_HEATING)
      thermostatRelayOff();
    else
      thermostatRelayOn();
  }
}

void ICACHE_FLASH_ATTR thermostatRelayOn() {
  // to avoid relay cycling only turn the relay on
  // after it has been off for sysConfig.therm_relay_rest_min
  if (sntp_get_current_timestamp() > thermostatRelayOffTime + (sysCfg.therm_relay_rest_min * 60)) {
    currGPIO12State = 1;
    ioGPIO(currGPIO12State, 12);
  } else {
    os_printf("Thermostat: Attempt to turn thermostat relay on during rest period, ignored\n");
  }
}

void ICACHE_FLASH_ATTR thermostatRelayOff() {
  currGPIO12State = 0;
  ioGPIO(currGPIO12State, 12);
  thermostatRelayOffTime = sntp_get_current_timestamp();
}

static void ICACHE_FLASH_ATTR pollThermostatCb(void *arg) {
  unsigned long epoch = sntp_get_current_timestamp();
  int year = get_year(&epoch);
  int month = get_month(&epoch, year);
  int day = day = 1 + (epoch / 86400);
  int dow = wd(year, month, day);
  epoch = epoch % 86400;
  unsigned int hour = epoch / 3600;
  epoch %= 3600;
  unsigned int min = epoch / 60;
  int minadj = (min * 100 / 60);
  int currtime = hour * 100 + minadj;

  if (sysCfg.thermostat1state == 0) {
    os_printf("Thermostat: Not enabled.\n");
    return;
  }

  long Treading = -9999;

  if (sysCfg.sensor_dht22_enable) {
    struct sensor_reading *result = readDHT();
    if (result->success) {
      Treading = result->temperature * 100;
      if (sysCfg.thermostat1_input == 2) // Humidistat
        Treading = result->humidity * 100;
    }
  } else {
    if (sysCfg.sensor_ds18b20_enable && sysCfg.thermostat1_input == 0) {
      struct sensor_reading *result = read_ds18b20();
      if (result->success) {
        int SignBit, Whole, Fract;
        Treading = result->temperature;

        SignBit = Treading & 0x8000;          // test most sig bit
        if (SignBit)                          // negative
          Treading = (Treading ^ 0xffff) + 1; // 2's comp

        Whole = Treading >> 4; // separate off the whole and fractional portions
        Fract = (Treading & 0xf) * 100 / 16;

        if (SignBit) // negative
          Whole *= -1;
        Treading = Whole * 100 + Fract;
      }
    } // ds8b20 enabled
  }

  if (sysCfg.thermostat1_input == 3) { // MQTT input to thermostat

    if (sntp_get_current_timestamp() - mqttTreadingTS > sysCfg.mqtt_temp_timeout_secs) {
      // mqttTreading too old
      os_printf("Thermostat: MQTT temperature reading stale (older than %d minutes)\n",
                sysCfg.mqtt_temp_timeout_secs * 60);
      Treading = -9999;
    } else {
      Treading = mqttTreading * 10; // Treading is tenth of a degree, eg 24.5 = 2450
    }
  }

  if (sysCfg.thermostat1_input == 4) { // Serial input to thermostat
    Treading = serialTreading;         // Treading is tenth of a degree, eg 24.5 = 2450
  }

  if (sysCfg.thermostat1_input == 5) { // Fixed value to thermostat
    Treading = 1000;
  }

  if (Treading == -9999 || Treading > 4000 || Treading < -2000) { // Check for valid reading
    // if reading is > 40C, or < -3C or -9999 (invalid read) treat as invalid
    os_printf("Thermostat: Invalid temperature reading (%d is not in range -20C to +40C) turning off relay.\n",
              (int)Treading);
    // turn heating off - do not act on bad data !
    thermostatRelayOff();
    return;
  }

  if (sysCfg.thermostat1mode == THERMOSTAT_MANUAL) {
    thermostat(Treading, (int)sysCfg.thermostat1manualsetpoint);
    return;
  }

  if (year < 2022) { // Something is wrong with the NTP time, maybe not enabled?
    os_printf("Thermostat: NTP time seems incorrect - year is < 2022. \n");
    return;
  }

  for (int sched = 0; sched < 8 && sysCfg.thermostat1schedule.weekSched[dow].daySched[sched].active == 1; sched++) {
    if (currtime >= sysCfg.thermostat1schedule.weekSched[dow].daySched[sched].start &&
        currtime < sysCfg.thermostat1schedule.weekSched[dow].daySched[sched].end) {
      os_printf("Thermostat: Current schedule (%d) setpoint is: %d\n", sched,
                sysCfg.thermostat1schedule.weekSched[dow].daySched[sched].setpoint);
      thermostat(Treading, sysCfg.thermostat1schedule.weekSched[dow].daySched[sched].setpoint);
    }
  }
}

void ICACHE_FLASH_ATTR thermostat_init(uint32_t polltime) {

  os_printf("Thermostat: init; poll interval of %d sec\n", (int)polltime / 1000);

  static ETSTimer thermostatTimer;
  os_timer_setfn(&thermostatTimer, pollThermostatCb, NULL);
  os_timer_arm(&thermostatTimer, polltime, 1);
}
