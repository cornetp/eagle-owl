#ifndef __DB_H__
#define __DB_H__

int db_open(void);
int db_insert_hist(int year, int month, int day, int hour, int min, double watts, double amps);

#endif // __DB_H__
