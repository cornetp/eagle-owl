#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sqlite3.h> 
#include "db.h"
#include "../db/eagleowl_sql.h"

#define EAGLE_OWL_DB 		"eagleowl.db"
#define EAGLE_OWL_STAT_DB 	"eagleowl_stat.db"

static sqlite3 *db = NULL;
static sqlite3 *stat_db = NULL;

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

void db_close()
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

int db_open(void)
{
  int ret = SQLITE_OK;
  if(!db)
  {
    ret = sqlite3_open_v2(EAGLE_OWL_DB, &db, SQLITE_OPEN_READWRITE, NULL);
    if(ret != SQLITE_OK)
      ret = create_main_db();
  }
  
  if(!stat_db && ret == SQLITE_OK)
  {
    ret = sqlite3_open_v2(EAGLE_OWL_STAT_DB, &stat_db, SQLITE_OPEN_READWRITE, NULL);
    if(ret != SQLITE_OK)
      ret = create_stat_db();
  }
  return ret;
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

int db_insert_hist(int y, int m, int d, int h, int min, double wh, double ah)
{
  int ret = SQLITE_OK;
  char *errmsg;
  bool retry = false;
  static sqlite3_int64 prev_insert = 0;
  sqlite3_int64 last_insert = 0;

  db_open();

  if(!db || !stat_db)
  {
    fprintf(stderr, "Error: db_insert_hist dbs not opened!\n");
    return -1;
  }

  char sql[512];
  int addr = 0;
  int ghg = 43; // TODO: get it from energy_param table
  int cost = 2466; // TODO: get it from energy_tariffv2
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

  last_insert = sqlite3_last_insert_rowid(db);
  if(last_insert != prev_insert)
  { // new statement inserted -> update the stat db
    update_stat_db(y, m, d, h, wh/1000);
  }

  prev_insert = last_insert;

  return ret;
}

int update_stat_db(int y, int m, int d, int h, double kwh)
{
  static sqlite3_int64 prev_insert = 0;
  sqlite3_int64 last_insert = 0;

  if(!db || !stat_db)
  {
    fprintf(stderr, "Error: db_insert_hist dbs not opened!\n");
    return -1;
  }

  int addr = 0;
  char sql[512];

  double day_conso = 0;
  double night_conso = 0;
  if(get_day_of_week(y, m, d) < 5 && is_full_tariff(h))
    day_conso += kwh;
  else
    night_conso += kwh;

  // update energy_hour_stat 
  sprintf(sql, INSERT_STAT_HOUR, addr, y, m, d, h, kwh, day_conso, night_conso, 1, 0);
  SQL_EXEC(stat_db, sql, "insert_stat_hour");
  last_insert = sqlite3_last_insert_rowid(stat_db);
  if(last_insert == prev_insert)
  {// insert failed -> record already exists -> update it
    sprintf(sql, UPDATE_STAT_HOUR, kwh, day_conso, night_conso, addr, y, m, d, h);
    SQL_EXEC(stat_db, sql, "update_stat_hour");
  }
  prev_insert = last_insert;

  // update energy_day_stat 
  sprintf(sql, INSERT_STAT_DAY, addr, y, m, d, kwh, day_conso, night_conso, 1, 0);
  SQL_EXEC(stat_db, sql, "insert_stat_day");
  last_insert = sqlite3_last_insert_rowid(stat_db);
  if(last_insert == prev_insert)
  {// insert failed -> record already exists -> update it
    sprintf(sql, UPDATE_STAT_DAY, kwh, day_conso, night_conso, addr, y, m, d);
    SQL_EXEC(stat_db, sql, "update_stat_day");
  }
  prev_insert = last_insert;
  
  // update energy_month_stat 
  sprintf(sql, INSERT_STAT_MONTH, addr, y, m, kwh, day_conso, night_conso, 0);
  SQL_EXEC(stat_db, sql, "insert_stat_month");
  last_insert = sqlite3_last_insert_rowid(stat_db);
  if(last_insert == prev_insert)
  {// insert failed -> record already exists -> update it
    sprintf(sql, UPDATE_STAT_MONTH, kwh, day_conso, night_conso, addr, y, m);
    SQL_EXEC(stat_db, sql, "update_stat_month");
  }
  prev_insert = last_insert;

  // update energy_year_stat 
  sprintf(sql, INSERT_STAT_YEAR, addr, y, kwh, day_conso, night_conso, 0);
  SQL_EXEC(stat_db, sql, "insert_stat_year");
  last_insert = sqlite3_last_insert_rowid(stat_db);
  if(last_insert == prev_insert)
  {// insert failed -> record already exists -> update it
    sprintf(sql, UPDATE_STAT_YEAR, kwh, day_conso, night_conso, addr, y);
    SQL_EXEC(stat_db, sql, "update_stat_year");
  }
  prev_insert = last_insert;

  return SQLITE_OK;
}

int db_update_status()
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

