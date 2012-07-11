#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

#include "eagleowl_sql.h"

#ifdef DEBUG
  #define dbg_print printf
#else
  #define dbg_print(...)
#endif

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define OWL_DB_TABLES_CNT 4

int update_stat_db(int y, int m, int d, int h, int min, double kwh, double kah);

static char *owl_db_tables[OWL_DB_TABLES_CNT] = {
  "energy_history",
  "energy_param",
  "energy_sensor",
  "energy_tariffv2"
};


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

// checks if the elem "name" is present in the table
static bool is_present(char *name, char table[][256], int size)
{
  int i;
  for(i = 0; i < size; i++)
  {
    if(strcmp(name, table[i]) == 0)
    {
      dbg_print(" -> %s found\n", name);
      return true;
    }
  }
  dbg_print(" -> %s NOT found\n", name);
  return false;
}

static bool validate_imported_db(sqlite3* imp_db)
{
  // list tables
//  char *errmsg;
  char sql[] = LIST_TABLES;

  sqlite3_stmt *stmt;
  int ret = sqlite3_prepare(imp_db, sql, sizeof(sql), &stmt, NULL);
  if(ret != SQLITE_OK)
  {
    printf("sqlite3_prepare error: %s\n", sqlite3_errmsg(imp_db));
    return false;
  }

  int table_cnt = 0;
  char tbl[OWL_DB_TABLES_CNT][256];
  while(sqlite3_step(stmt)==SQLITE_ROW && table_cnt<OWL_DB_TABLES_CNT)
    snprintf(tbl[table_cnt++], 256, "%s", sqlite3_column_text(stmt, 0));

  if(table_cnt != OWL_DB_TABLES_CNT)
  {
    printf("invalid number of tables (expected %d, found %d)", 
           OWL_DB_TABLES_CNT, table_cnt);
    ret = -1;
  }
  else
  {
    int i;
    int present_tables_cnt = 0;
    for(i=0; i<OWL_DB_TABLES_CNT; i++)
    {
      if(is_present(owl_db_tables[i], tbl, table_cnt))
        present_tables_cnt++;
    }
    if(present_tables_cnt != OWL_DB_TABLES_CNT)
      ret = -1;
  }
  
  sqlite3_reset(stmt);
  return (ret==0)?true:false;
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

int db_begin_transaction(void)
{
  SQL_EXEC(db, "BEGIN TRANSACTION", "begin transaction");
  SQL_EXEC(stat_db, "BEGIN TRANSACTION", "begin transaction");
  return SQLITE_OK;
}

int db_end_transaction(void)
{
  SQL_EXEC(db, "END TRANSACTION", "end transaction");
  SQL_EXEC(stat_db, "END TRANSACTION", "end transaction");
  return SQLITE_OK;
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

  if((last_insert = sqlite3_last_insert_rowid(db)) != prev_insert)
  { // new statement inserted -> update the stat db
    update_stat_db(y, m, d, h, min, wh/1000, ah/1000);
  }

  prev_insert = last_insert;

  return ret;
}

int update_stat_db(int y, int m, int d, int h, int min, double kwh, double kah)
{
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

static int import_db_cb(void *context, int argc, char **argv, char **azColName)
{
  int y     = atoi(argv[0]);
  int m     = atoi(argv[1]);
  int d     = atoi(argv[2]);
  int h     = atoi(argv[3]);
  int min   = atoi(argv[4]);
  float wh = atof(argv[5]);
  float ah = atof(argv[6]);
  static int counter = 0;
  int num_elems = (int) context;

  db_insert_hist(y, m, d, h, min, wh, ah);

  printf("\r %.1f%%", 100*((double)counter++/num_elems));
  fflush(stdout);

  return SQLITE_OK;
}

static int import_db(sqlite3 **imp_db)
{
  int ret = SQLITE_OK;
  char sql[512];

  db_begin_transaction();

  sqlite3_stmt *stmt;
  sqlite3_prepare(*imp_db, COUNT_HISTORY_ELEM, sizeof(COUNT_HISTORY_ELEM), &stmt, NULL);
  sqlite3_step(stmt);
  int num_elems = atoi((char *)sqlite3_column_text(stmt, 0));

  sprintf(sql, "select year, month, day, hour, min, ch1_kw_avg, ch1_amps_avg"
               " from energy_history");

  char *errmsg;
  if((ret = sqlite3_exec(*imp_db, sql, import_db_cb, (void*)num_elems, &errmsg)) 
     != SQLITE_OK)
  {
    printf("import_db select error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  printf("\n");

  // update status
  sprintf(sql, "UPDATE energy_hour_stat SET status = 1 WHERE record_count = %d", 60);
  SQL_EXEC(stat_db, sql, "Update energy_hour_stat status");

  sprintf(sql, "UPDATE energy_day_stat SET status = 1 WHERE record_count = %d", 60*24);
  SQL_EXEC(stat_db, sql, "Update energy_day_stat status");

  db_end_transaction();
  return ret;
}

static void print_help(char *prog_name)
{
  printf("Usage:\n");
  printf("  %s <dbfile-to-import>\n", prog_name);
  printf("\n");
}

int main(int argc, char *argv[])
{
  sqlite3 *imp_db = NULL;
  if(argc < 2)
  {
    print_help(argv[0]);
    return -1;
  }

  int ret = sqlite3_open_v2(argv[1], &imp_db, SQLITE_OPEN_READONLY, NULL);
  if(ret != SQLITE_OK || imp_db == NULL) {
    printf("Could not open database %s: open returned %d\n", argv[1], ret);
    return -1;
  }
  if(!validate_imported_db(imp_db))
  {
    printf("error: %s is not a valid OWL database\n", argv[1]);
    goto end;
  }
  dbg_print("%s db is valid\n", argv[1]);


  // open db and stat_db
  db_open();

  import_db(&imp_db);

  sqlite3_close(imp_db);
  imp_db = NULL;
 
end:
  if(imp_db)
    sqlite3_close(imp_db);
  db_close();
  return 0;
}
