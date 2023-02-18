#include "os_type.h"
#include <time.h>

void thermostat_init(uint32_t polltime);
void thermostatRelayOn(void);
void thermostatRelayOff(void);
extern int getRoomTemp();
extern time_t thermostatRelayOffTime;
extern int scheduleThermSetPoint;
