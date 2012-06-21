#include <stdio.h>
#include <sqlite3.h> 
#include "db.h"
#include "../db/eagleowl_sql.h"

#define EAGLE_OWL_DB 		"eagleowl.db"
#define EAGLE_OWL_STAT_DB 	"eagleowl_stat.db"

// Create eagleowl_db (same format as the one from OWL)
static int create_main_db(sqlite3 **db)
{
  int ret = SQLITE_OK;
  printf("%s doesn't exist -> create it.\n", EAGLE_OWL_DB);
  sqlite3_open_v2(EAGLE_OWL_DB, db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);

  char *errmsg;
  if((ret = sqlite3_exec(*db, CREATE_HISTORY_TBL, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("Create energy_history table error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, CREATE_PARAM_TBL, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("Create energy_param table error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, CREATE_SENSOR_TBL, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("Create energy_sensor table error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, CREATE_TARIFFV2_TBL, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("Create energy_tariffv2 table error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  return ret;
}

static int create_stat_db(sqlite3 **db)
{
  int ret = SQLITE_OK;
  printf("%s doesn't exist -> create it.\n", EAGLE_OWL_STAT_DB);
  sqlite3_open_v2(EAGLE_OWL_STAT_DB, db, 
                  SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
  char *errmsg;
  if((ret = sqlite3_exec(*db, CREATE_YEAR_STAT, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("Create energy_year_stat table error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, CREATE_MONTH_STAT, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("Create energy_month_stat table error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, CREATE_DAY_STAT, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("Create energy_day_stat table error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, CREATE_HOUR_STAT, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("Create energy_hour_stat table error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  return ret;
}

static int open_eagleowl_dbs(sqlite3 **db, sqlite3 **stat_db)
{
  int ret = SQLITE_OK;
  if(sqlite3_open_v2(EAGLE_OWL_DB, db, SQLITE_OPEN_READWRITE, NULL)!=SQLITE_OK)
    create_main_db(db);
  
  if(sqlite3_open_v2(EAGLE_OWL_STAT_DB, stat_db, SQLITE_OPEN_READWRITE, NULL) !=SQLITE_OK)
    create_stat_db(stat_db);
  return ret;
}

static void close_eagleowl_dbs(sqlite3 *db, sqlite3 *stat_db)
{
  if(db) sqlite3_close(db);
  if(stat_db) sqlite3_close(stat_db);
}

int db_insert_hist(int y, int m, int d, int h, int min, double watts, double amps)
{
  sqlite3 *db = NULL, *stat_db = NULL;
  int ret = open_eagleowl_dbs(&db, &stat_db);
  char *errmsg;

  char sql[512];
  int addr = 0;
  int ghg = 43; // TODO: get it from energy_param table
  int cost = 2466; // TODO: get it from energy_tariffv2
  sprintf(sql, INSERT_HISTORY_TBL, addr, y, m, d, h, min, amps/60, watts/60, ghg, cost,
                                   amps/60, amps/60, watts/60, watts/60); 
  if((ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("db_insert_hist error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }

  close_eagleowl_dbs(db, stat_db);
  return 0;
}
