#include "os_type.h"
#include <time.h>

void thermostat_init(uint32_t polltime);
void thermostatRelayOn(void);
void thermostatRelayOff(void);
extern int getRoomTemp();
extern time_t thermostatRelayOffTime;
extern int thermostat1ScheduleSetPoint;
extern int thermostat1CurrentSetPoint;
extern int thermostatRelayActive; // set to 1 if any thermostat controlled relay are on
