/*
 * eagle-owl application.
 *
 * Copyright (C) 2012 Philippe Cornet <phil.cornet@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __DB_H__
#define __DB_H__
#include <sqlite3.h> 

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
void db_close(void);
int db_begin_transaction(void);
int db_end_transaction(void);
int db_insert_hist(struct record_data *rec);
int db_update_status(void);

void update_stat(sqlite3_context *context, int argc, sqlite3_value **argv);
int update_stat_db(int y, int m, int d, int h, double kwh);

#endif // __DB_H__
