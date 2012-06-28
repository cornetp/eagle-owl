#ifndef __DB_H__
#define __DB_H__

int db_open(void);
int db_begin_transaction();
int db_end_transaction();
int db_insert_hist(int year, int month, int day, int hour, int min, double kwh, double kah);
int update_stat_db(int y, int m, int d, int h, double kwh);

#endif // __DB_H__
