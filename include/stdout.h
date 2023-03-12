void stdoutInit();

// Temperature reading timestamp (used for thermostat if configured)
extern time_t serialTreadingTS;
extern int serialTreading;
extern char *userJSON[250];
// Temperature reading timestamp (used for MQTT publishing if configured)
extern time_t userJSONreadingTS;
