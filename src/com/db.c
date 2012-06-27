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

int db_insert_hist(int y, int m, int d, int h, int min, double kwh, double kah)
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
  sprintf(sql, INSERT_HISTORY_TBL, addr, y, m, d, h, min, kah, kwh, ghg, cost,
                                   kah, kah, kwh, kwh);
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

  if((last_insert = sqlite3_last_insert_rowid(db)) != prev_insert)
  { // new statement inserted -> update the stat db
    update_stat_db(y, m, d, h, min, kwh, kah);
    ret = 1;
  }
  else
    ret = 0;
    //printf("insert ignored for %d/%d/%d %d:%d (%f)\n", d, m, y, h, min, last_insert);

  prev_insert = last_insert;

  //close_eagleowl_dbs(db, stat_db);
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

int update_stat_db(int y, int m, int d, int h, int min, double kwh, double kah)
{
  static sqlite3_int64 prev_insert = 0;
  sqlite3_int64 last_insert = 0;

  int ret = SQLITE_OK;
  char *errmsg;
  if(!db || !stat_db)
  {
    fprintf(stderr, "Error: db_insert_hist dbs not opened!\n");
    return -1;
  }

  int addr = 0;
  char sql[512];
  // update energy_hour_stat 
  sprintf(sql, INSERT_STAT_HOUR, addr, y, m, d, h, kwh, kwh, kwh, 0);
//  printf("%s\n", sql);
  sqlite3_exec(stat_db, sql, NULL, NULL, &errmsg);
  last_insert = sqlite3_last_insert_rowid(stat_db);
  if(last_insert == prev_insert)
  {// insert failed -> record already exists -> update it
    sprintf(sql, UPDATE_STAT_HOUR, kwh, addr, y, m, d, h);
//    printf(" - %s\n", sql);
    sqlite3_exec(stat_db, sql, NULL, NULL, &errmsg);
    if(ret != SQLITE_OK)
    {
      printf("update_stat_db error: %s\n", errmsg);
      sqlite3_free(errmsg);
    }
  }
  prev_insert = last_insert;

  // update energy_day_stat 
  sprintf(sql, INSERT_STAT_DAY, addr, y, m, d, kwh, kwh, kwh, 0);
  sqlite3_exec(stat_db, sql, NULL, NULL, &errmsg);
  last_insert = sqlite3_last_insert_rowid(stat_db);
  if(last_insert == prev_insert)
  {// insert failed -> record already exists -> update it
    sprintf(sql, UPDATE_STAT_DAY, kwh, addr, y, m, d);
    sqlite3_exec(stat_db, sql, NULL, NULL, &errmsg);
    if(ret != SQLITE_OK)
    {
      printf("update_stat_db error: %s\n", errmsg);
      sqlite3_free(errmsg);
    }
  }
  prev_insert = last_insert;
  
  // update energy_month_stat 
  sprintf(sql, INSERT_STAT_MONTH, addr, y, m, kwh, kwh, kwh, 0);
  sqlite3_exec(stat_db, sql, NULL, NULL, &errmsg);
  last_insert = sqlite3_last_insert_rowid(stat_db);
  if(last_insert == prev_insert)
  {// insert failed -> record already exists -> update it
    sprintf(sql, UPDATE_STAT_MONTH, kwh, addr, y, m);
    sqlite3_exec(stat_db, sql, NULL, NULL, &errmsg);
    if(ret != SQLITE_OK)
    {
      printf("update_stat_db error: %s\n", errmsg);
      sqlite3_free(errmsg);
    }
  }
  prev_insert = last_insert;

  // update energy_year_stat 
  sprintf(sql, INSERT_STAT_YEAR, addr, y, kwh, kwh, kwh, 0);
  sqlite3_exec(stat_db, sql, NULL, NULL, &errmsg);
  last_insert = sqlite3_last_insert_rowid(stat_db);
  if(last_insert == prev_insert)
  {// insert failed -> record already exists -> update it
    sprintf(sql, UPDATE_STAT_YEAR, kwh, addr, y);
    sqlite3_exec(stat_db, sql, NULL, NULL, &errmsg);
    if(ret != SQLITE_OK)
    {
      printf("update_stat_db error: %s\n", errmsg);
      sqlite3_free(errmsg);
    }
  }
  prev_insert = last_insert;

  // Normally the data arrive in chronological order, so we can expect that 
  // when we receive a time with minute 59, it is the last we will receive for the 
  // currebt hour, so we can compute the status field (complete / incomplete data) 
  if(min == 59)
  { // check if we have all data for that hour and set the status value
    if(h == 23)
    { // also check if the day is complete in this case
    }
  }
  return 0;
}
