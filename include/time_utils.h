#ifndef __TIME_UTILS_H
#define __TIME_UTILS_H

char *epoch_to_str(unsigned long epoch);
char *epoch_to_str_hhmm(unsigned long epoch);

int get_month(unsigned long *t, int year);
int get_year(unsigned long *t);

bool IsDST_EU(int day, int month, int dow);
bool IsDST_NA(int day, int month, int dow);

#endif
