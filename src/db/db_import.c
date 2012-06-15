#include <stdio.h>
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
static int create_eagleowl_db(sqlite3 **db)
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

static int create_eagleowl_stat_db(sqlite3 **db)
{
  int ret = SQLITE_OK;
  printf("%s doesn't exist -> create it.\n", EAGLE_OWL_STAT_DB);
  sqlite3_open_v2(EAGLE_OWL_STAT_DB, db, 
                  SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
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

  // open db and stat_db
  if(sqlite3_open_v2(EAGLE_OWL_DB, &db, SQLITE_OPEN_READWRITE, NULL)!=SQLITE_OK)
    create_eagleowl_db(&db);

  if(sqlite3_open_v2(EAGLE_OWL_STAT_DB, &stat_db, SQLITE_OPEN_READWRITE, NULL) 
     !=SQLITE_OK)
    create_eagleowl_stat_db(&stat_db);
 
end:
  if(import_db)
    sqlite3_close(import_db);
  if(db)
    sqlite3_close(db);
  if(stat_db)
    sqlite3_close(stat_db);
  return 0;
}
