// ets_vprintf(write_mychar, s, args);

// extern void syslog(char *msgtosend);

extern void syslog(const char *format, ...);

#define DBG(format, ...)                                                                                               \
  do {                                                                                                                 \
    syslog(format, ##__VA_ARGS__);                                                                                     \
    os_printf(format, ##__VA_ARGS__);                                                                                  \
  } while (0)
