#ifndef __DB_H__
#define __DB_H__

struct record_data {
  int addr;
  int year;
  int month;
  int day;
  int hour;
  int min;
  double cost;
  double amps;
  double watts;
  double ah; // watt hour and ampere hour are the units used inside the db
  double wh;
  bool isLiveData; // Flag used to know is this record is the live conumption 
                   // or the mean consumption (for the DB)
};

int db_open(void);
int db_begin_transaction(void);
int db_end_transaction(void);
int db_insert_hist(struct record_data *rec);
int db_update_status(void);

int update_stat_db(int y, int m, int d, int h, double kwh);

#endif // __DB_H__
