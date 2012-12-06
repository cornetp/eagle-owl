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
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

#include "db.h"
#include "sql_cmd.h"

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

static char *owl_db_tables[OWL_DB_TABLES_CNT] = {
  "energy_history",
  "energy_param",
  "energy_sensor",
  "energy_tariffv2"
};


#define EAGLE_OWL_DB 		"eagleowl.db"
#define EAGLE_OWL_STAT_DB 	"eagleowl_stat.db"

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

static int import_db_cb(void *context, int argc, char **argv, char **azColName)
{
  struct record_data rec;

  rec.year  = atoi(argv[0]);
  rec.month = atoi(argv[1]);
  rec.day   = atoi(argv[2]);
  rec.hour  = atoi(argv[3]);
  rec.min   = atoi(argv[4]);
  rec.wh    = atof(argv[5]);
  rec.ah    = atof(argv[6]);

  rec.addr  = 0;    // TODO
  rec.cost  = 2466; // TODO

  static int counter = 0;
  int num_elems = (int) context;

  db_insert_hist(&rec);

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

  db_update_status();
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
