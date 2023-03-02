
#include "c_types.h"
#include "espmissingincludes.h"
#include "osapi.h"

#define IS_LEAP(year) (year % 4 == 0)
#define SEC_IN_NON_LEAP (86400 * 365)
#define SEC_IN_LEAP (86400 * 366)
#define SEC_IN_YEAR(year) (IS_LEAP(year) ? SEC_IN_LEAP : SEC_IN_NON_LEAP)

char buf[30];

unsigned char calendar[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
unsigned char calendar_leap[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

unsigned char *get_calendar(int year) { return IS_LEAP(year) ? calendar_leap : calendar; }

int ICACHE_FLASH_ATTR get_year(unsigned long *t) {
  int year = 1970;
  while (*t > SEC_IN_YEAR(year)) {
    *t -= SEC_IN_YEAR(year);
    year++;
  }
  return year;
}

int ICACHE_FLASH_ATTR get_month(unsigned long *t, int year) {
  unsigned char *cal = get_calendar(year);
  int i = 0;
  while (*t > cal[i] * 86400) {
    *t -= cal[i] * 86400;
    i++;
  }
  return i + 1;
}

char *ICACHE_FLASH_ATTR epoch_to_str(unsigned long epoch) {
  int year = get_year(&epoch);
  unsigned char month = get_month(&epoch, year);
  unsigned char day = 1 + (epoch / 86400);
  epoch = epoch % 86400;
  unsigned char hour = epoch / 3600;
  epoch %= 3600;
  unsigned char min = epoch / 60;
  unsigned char sec = epoch % 60;

  os_sprintf(buf, "%02d:%02d:%02d %02d/%02d/%02d", hour, min, sec, day, month, year);
  return buf;
}

char *ICACHE_FLASH_ATTR epoch_to_str_hhmm(unsigned long epoch) {
  epoch = epoch % 86400;
  unsigned char hour = epoch / 3600;
  epoch %= 3600;
  unsigned char min = epoch / 60;

  os_sprintf(buf, "%02d:%02d", hour, min);
  return buf;
}

bool IsDST_EU(int day, int month, int dow) {
  if (month < 3 || month > 10)
    return false;
  if (month > 3 && month < 10)
    return true;

  int previousSunday = day - dow;

  if (month == 3)
    return previousSunday >= 25;
  if (month == 10)
    return previousSunday < 25;

  return false; // this line never gonna happend
}

bool IsDST_NA(int day, int month, int dow) {
  // January, February, and December are out.
  if (month < 3 || month > 11) {
    return false;
  }
  // April to October are in
  if (month > 3 && month < 11) {
    return true;
  }
  int previousSunday = day - dow;
  // In march, we are DST if our previous Sunday was on or after the 8th.
  if (month == 3) {
    return previousSunday >= 8;
  }
  // In November we must be before the first Sunday to be DST.
  // That means the previous Sunday must be before the 1st.
  return previousSunday <= 1;
}
