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

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sqlite3.h> 
#include "db.h"
#include "sql_cmd.h"

#define EAGLE_OWL_DB 		"eagleowl.db"
#define EAGLE_OWL_STAT_DB 	"eagleowl_stat.db"

static sqlite3 *db = NULL;
static sqlite3 *stat_db = NULL;

// Returns the day of the week: 0 = monday ... 6 = sunday
static inline int get_day_of_week(int y, int m, int d)
{
  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  y -= m < 3;
  return (y + y/4 - y/100 + y/400 + t[m-1] + d - 1) % 7;
}

static inline bool is_full_tariff(int hour)
{ // TODO: get info from energy_tariffv2 table!
  return (hour >= 8 && hour < 23)?true:false;
}

// Create eagleowl_db (same format as the one from OWL)
static int create_main_db()
{
  int ret = SQLITE_OK;
  printf("%s doesn't exist -> create it.\n", EAGLE_OWL_DB);
  sqlite3_open_v2(EAGLE_OWL_DB, &db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);

  SQL_EXEC(db, CREATE_HISTORY_TBL, "Create energy_history table");
  SQL_EXEC(db, CREATE_PARAM_TBL, "Create energy_param table");
  SQL_EXEC(db, CREATE_SENSOR_TBL, "Create energy_sensor table");
  SQL_EXEC(db, CREATE_TARIFFV2_TBL, "Create energy_tariffv2 table");

  return ret;
}

static int create_stat_db()
{
  int ret = SQLITE_OK;
  printf("%s doesn't exist -> create it.\n", EAGLE_OWL_STAT_DB);
  sqlite3_open_v2(EAGLE_OWL_STAT_DB, &stat_db, 
                  SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
  SQL_EXEC(stat_db, CREATE_YEAR_STAT, "Create energy_year_stat table");
  SQL_EXEC(stat_db, CREATE_MONTH_STAT, "Create energy_month_stat table");
  SQL_EXEC(stat_db, CREATE_DAY_STAT, "Create energy_year_day table");
  SQL_EXEC(stat_db, CREATE_HOUR_STAT, "Create energy_year_hour table");

  return ret;
}

int db_open(void)
{
  int ret = SQLITE_OK;
  if(!db)
  {
    ret = sqlite3_open_v2(EAGLE_OWL_DB, &db, SQLITE_OPEN_READWRITE, NULL);
    if(ret != SQLITE_OK)
      ret = create_main_db();

    if(ret == SQLITE_OK)
    {
      sqlite3_create_function(db, "update_stat_db", 5, SQLITE_UTF8, NULL, 
                              &update_stat, NULL, NULL);

      SQL_EXEC(db, CREATE_UPDATE_STAT_TRIGGER, "Create update_stat trigger");
    }
  }
  
  if(!stat_db && ret == SQLITE_OK)
  {
    ret = sqlite3_open_v2(EAGLE_OWL_STAT_DB, &stat_db, SQLITE_OPEN_READWRITE, NULL);
    if(ret != SQLITE_OK)
      ret = create_stat_db();
  }
  return ret;
}

void db_close(void)
{
  if(db)
  {
    sqlite3_close(db);
    db = NULL;
  }
  if(stat_db)
  {
    sqlite3_close(stat_db);
    stat_db = NULL;
  }
}

int db_begin_transaction()
{
  SQL_EXEC(db, "BEGIN TRANSACTION", "begin transaction");
  SQL_EXEC(stat_db, "BEGIN TRANSACTION", "begin transaction");
  return 0;
}

int db_end_transaction()
{
  SQL_EXEC(db, "END TRANSACTION", "begin transaction");
  SQL_EXEC(stat_db, "END TRANSACTION", "begin transaction");
  return 0;
}

int db_insert_hist(struct record_data *rec)
{
  int ret = SQLITE_OK;
  char *errmsg;
  bool retry = false;

  db_open();

  if(!db || !stat_db)
  {
    fprintf(stderr, "Error: db_insert_hist dbs not opened!\n");
    return -1;
  }

  char sql[512];
  int addr = rec->addr;
  int ghg = 43; // TODO: get it from energy_param table
  int cost = rec->cost;
  int y = rec->year;
  int m = rec->month;
  int d = rec->day;
  int h = rec->hour;
  int min = rec->min;
  double ah = rec->ah;
  double wh = rec->wh;
  sprintf(sql, INSERT_HISTORY_TBL, addr, y, m, d, h, min, ah, wh, ghg, cost,
                                   ah, ah, wh, wh);
  do
  {
    retry = false;
    ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if(ret == SQLITE_BUSY)
    {
      sqlite3_free(errmsg);
      retry = true;
      usleep(20);
    }
    else if(ret != SQLITE_OK)
    {
      printf("db_insert_hist error: %s\n", errmsg);
      sqlite3_free(errmsg);
    }
  }
  while(retry);

  return ret;
}

void update_stat(sqlite3_context *context, int argc, sqlite3_value **argv)
{
  if(argc != 5)
    return;

  int y = sqlite3_value_int(argv[0]);
  int m = sqlite3_value_int(argv[1]);
  int d = sqlite3_value_int(argv[2]);
  int h = sqlite3_value_int(argv[3]);
  double kwh = sqlite3_value_double(argv[4]);
//  printf("update_stat callback called %d/%d/%d @%dh: %f kwh\n", y, m, d, h, kwh);
  update_stat_db(y, m, d, h, kwh);
}

int update_stat_db(int y, int m, int d, int h, double kwh)
{
  int addr = 0; // TODO
  char sql[512];
  double day_conso = 0;
  double night_conso = 0;
  
  if(!db || !stat_db)
  {
    fprintf(stderr, "Error: db_insert_hist dbs not opened!\n");
    return -1;
  }

  if(get_day_of_week(y, m, d) < 5 && is_full_tariff(h)) // TODO: use tariffs from db
    day_conso += kwh;
  else
    night_conso += kwh;

  // update energy_hour_stat 
  sprintf(sql, UPDATE_STAT_HOUR, kwh, day_conso, night_conso, y, m, d, h, 
                                 addr, y, m, d, h, kwh, day_conso, night_conso);
  SQL_EXEC(stat_db, sql, "Update stat_hour DB");

  // update energy_day_stat 
  sprintf(sql, UPDATE_STAT_DAY, kwh, day_conso, night_conso, y, m, d,
                                addr, y, m, d, kwh, day_conso, night_conso);
  SQL_EXEC(stat_db, sql, "Update stat_day DB");
  
  // update energy_month_stat 
  sprintf(sql, UPDATE_STAT_MONTH, kwh, day_conso, night_conso, y, m,
                                  addr, y, m, kwh, day_conso, night_conso);
  SQL_EXEC(stat_db, sql, "Update stat_month DB");

  // update energy_year_stat
  sprintf(sql, UPDATE_STAT_YEAR, kwh, day_conso, night_conso, y,
                                 addr, y, kwh, day_conso, night_conso);
  SQL_EXEC(stat_db, sql, "Update stat_year DB");

  return SQLITE_OK;
}

// status is use to visualize in the calendar if the data for a day is complete
// or partial with the following color code:
// grey   : no data for that day
// orange : data, but incomplete
// blue   : data is complete (sum of 60x24 minutes kwh values)
int db_update_status(void)
{
  char sql[512];
  if(!stat_db)
  {
    fprintf(stderr, "Error: db_update_status stat_db not opened!\n");
    return -1;
  }

  // update status
  sprintf(sql, "UPDATE energy_hour_stat SET status = 1 WHERE record_count = %d", 60);
  SQL_EXEC(stat_db, sql, "Update energy_hour_stat status");

  sprintf(sql, "UPDATE energy_day_stat SET status = 1 WHERE record_count = %d", 60*24);
  SQL_EXEC(stat_db, sql, "Update energy_day_stat status");

  return SQLITE_OK;
}

