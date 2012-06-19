#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>

#include "eagleowl_sql.h"

#ifdef DEBUG
  #define dbg_print printf
#else
  #define dbg_print(...)
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
{
  return (hour > 7 && hour < 23)?true:false;
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

static bool validate_imported_db(sqlite3* db)
{
  // list tables
//  char *errmsg;
  char sql[] = LIST_TABLES;
/*
  int ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
  if(ret != SQLITE_OK)
  {
    printf("validate_imported_db error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }*/

  sqlite3_stmt *stmt;
  int ret = sqlite3_prepare(db, sql, sizeof(sql), &stmt, NULL);
  if(ret != SQLITE_OK)
  {
    printf("sqlite3_prepare error: %s\n", sqlite3_errmsg(db));
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

static int merge_main_db(sqlite3 **db, char *import_db_name)
{
  int ret = SQLITE_OK;
  char *errmsg;

  char sql[512];
  sprintf(sql, ATTACH_IMPORT_DB, import_db_name);
  if((ret = sqlite3_exec(*db, sql, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("attach database error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, MERGE_HISTORY_TBL, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("merge energy_history error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, MERGE_PARAM_TBL, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("merge energy_param error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, MERGE_SENSOR_TBL, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("merge energy_sensor error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, MERGE_TARIFFV2_TBL, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("merge energy_tariffv2 error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }
  if((ret = sqlite3_exec(*db, DETACH_IMPORT_DB, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("detach error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }

  return ret;
}
/*
#define TOTO   "INSERT OR IGNORE INTO energy_year_stat " \
               " VALUES (%s, %s, %s, 0, 0, 0); "\
               " UPDATE energy_year_stat  "\
               " SET kwh_total = kwh_total + %s " \
               " WHERE addr = %s and year = %s"
int update_stat_cb(void *p_data, int num_fields, char **p_fields, char **p_col_names)
{
  sqlite3 **stat_db = (sqlite3 **)p_data;
  if(num_fields != 6) // addr, year, month, day, hour, conso
  {
    printf("update_stat_cb error: bad number of fields %d\n", num_fields);
    return -1;
  }

  int ret;
  char *errmsg;
  char sql[512];

 // too slow!
  sprintf(sql, TOTO, p_fields[0], p_fields[1], p_fields[5], 
                     p_fields[5], p_fields[0], p_fields[1]);

  if((ret = sqlite3_exec(*stat_db, sql, NULL, NULL, &errmsg)) != SQLITE_OK)
  {
    printf("update_stat_cb insert error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return ret;
  }

  return 0;
}
*/
static int update_stat_db(sqlite3 **db, sqlite3 **stat_db)
{
  int ret = SQLITE_OK;

  printf("Update stat db (can take a lot of time...)\n");

  sqlite3_stmt *stmt;
  sqlite3_prepare(*db, COUNT_HISTORY_ELEM, sizeof(COUNT_HISTORY_ELEM), &stmt, NULL);
  sqlite3_step(stmt);
  int num_elems = atoi((char *)sqlite3_column_text(stmt, 0));
  int processed = 0;
 
  char sql[512];
  sqlite3_stmt *distinct_year;
  sqlite3_stmt *distinct_month;
  sqlite3_stmt *distinct_day;
  sqlite3_stmt *distinct_hour;
  sqlite3_stmt *min_conso;
  ret = sqlite3_prepare(*db, SELECT_DISTINCT_YEAR, sizeof(SELECT_DISTINCT_YEAR), 
                        &distinct_year, NULL);
  if(ret != SQLITE_OK)
  {
    printf("update_stat_db sqlite3_prepare error: %s\n", sqlite3_errmsg(*db));
    return ret;
  }


// TODO: take into account add field
  while(sqlite3_step(distinct_year)==SQLITE_ROW)
  {
    int year = atoi((char *)sqlite3_column_text(distinct_year, 0));
    double total_year = 0.0, total_year_w = 0.0, total_year_we = 0.0;
    sprintf(sql, SELECT_DISTINCT_MONTH, year);
    sqlite3_prepare(*db, sql, sizeof(sql), &distinct_month, NULL);

    while(sqlite3_step(distinct_month)==SQLITE_ROW)
    {
      int month = atoi((char *)sqlite3_column_text(distinct_month, 0));
      double total_month = 0.0, total_month_w = 0.0, total_month_we = 0.0;
      sprintf(sql, SELECT_DISTINCT_DAY, year, month);
      sqlite3_prepare(*db, sql, sizeof(sql), &distinct_day, NULL);
      while(sqlite3_step(distinct_day)==SQLITE_ROW)
      {
        int day = atoi((char *)sqlite3_column_text(distinct_day, 0));
        double total_day = 0.0, total_day_w = 0.0, total_day_we = 0.0;
        sprintf(sql, SELECT_DISTINCT_HOUR, year, month, day);
        sqlite3_prepare(*db, sql, sizeof(sql), &distinct_hour, NULL);
        while(sqlite3_step(distinct_hour)==SQLITE_ROW)
        {
          int hour = atoi((char *)sqlite3_column_text(distinct_hour, 0));
          double total_hour = 0.0, total_hour_w = 0.0, total_hour_we = 0.0;;
          sprintf(sql, SELECT_MIN_CONSO, year, month, day, hour);
          ret = sqlite3_prepare(*db, sql, sizeof(sql), &min_conso, NULL);
          if(ret != SQLITE_OK){
            printf("prepare error: %s\n", sqlite3_errmsg(*db));
            continue;
          }
          while(sqlite3_step(min_conso)==SQLITE_ROW)
          {
            double conso = atof((char *)sqlite3_column_text(min_conso, 0));
            total_hour += conso;
            if(get_day_of_week(year, month, day) < 5 && is_full_tariff(hour))
              total_hour_w += conso;
            else
              total_hour_we += conso;
            processed++;
          }
          sprintf(sql, INSERT_STAT_HOUR, 0, year, month, day, hour, 
                  total_hour, total_hour_w, total_hour_we, 0);
          sqlite3_exec(*stat_db, sql, NULL, NULL, NULL);
          total_day += total_hour;
          total_day_w += total_hour_w;
          total_day_we += total_hour_we;
          printf("\r%.2f%%", 100.0*((float)processed/num_elems));
          fflush(stdout);
        }
        // write total_day in stat
//        printf("total day %d/%d/%d : %f\n", day, month, year, total_day);
        sprintf(sql, INSERT_STAT_DAY, 0, year, month, day, total_day, 
                total_day_w, total_day_we,0);
        sqlite3_exec(*stat_db, sql, NULL, NULL, NULL);
        total_month += total_day;
        total_month_w += total_day_w;
        total_month_we += total_day_we;
      }
      // write total_month
      sprintf(sql, INSERT_STAT_MONTH, 0, year, month, total_month, 
              total_month_w, total_month_we,0);
      sqlite3_exec(*stat_db, sql, NULL, NULL, NULL);
      total_year += total_month;
      total_year_w += total_month_w;
      total_year_we += total_month_we;
    }
    // write total_year
    sprintf(sql, INSERT_STAT_YEAR, 0, year, total_year, total_year_w, total_year_we,0);
    sqlite3_exec(*stat_db, sql, NULL, NULL, NULL);
  }
  
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
  sqlite3 *import_db = NULL, *db = NULL, *stat_db = NULL;
  if(argc < 2)
  {
    print_help(argv[0]);
    return -1;
  }

  int ret = sqlite3_open_v2(argv[1], &import_db, SQLITE_OPEN_READONLY, NULL);
  if(ret != SQLITE_OK || import_db == NULL) {
    printf("Could not open database %s: open returned %d\n", argv[1], ret);
    return -1;
  }
  if(!validate_imported_db(import_db))
  {
    printf("error: %s is not a valid OWL database\n", argv[1]);
    goto end;
  }
  dbg_print("%s db is valid\n", argv[1]);

  sqlite3_close(import_db);
  import_db = NULL;

  // open db and stat_db
  if(sqlite3_open_v2(EAGLE_OWL_DB, &db, SQLITE_OPEN_READWRITE, NULL)!=SQLITE_OK)
    create_main_db(&db);

  if(sqlite3_open_v2(EAGLE_OWL_STAT_DB, &stat_db, SQLITE_OPEN_READWRITE, NULL) 
     !=SQLITE_OK)
    create_stat_db(&stat_db);

  merge_main_db(&db, argv[1]);
  update_stat_db(&db, &stat_db);
 
end:
  if(import_db)
    sqlite3_close(import_db);
  if(db)
    sqlite3_close(db);
  if(stat_db)
    sqlite3_close(stat_db);
  return 0;
}
