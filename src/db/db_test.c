#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

sqlite3* db = 0;
static char *month_name[13] = {
  "nul",
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec",
};

static char *day_name[7] = {
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saterday",
  "Sunday",
};

int compute_total_month_cb(void *p_data, int num_fields, char **p_fields, char **p_col_names) 
{
  if(num_fields != 3){
    printf("compute_total_month_cb error: bad number of fields %d", num_fields);
    return -1;
  }

  printf("%s %s -> %s\n", month_name[atoi(p_fields[0])], p_fields[1], p_fields[2]);
  return 0;
}

static int compute_total_month()
{
  char *errmsg;
  int   ret;
  int   nrecs = 0;
  char stmt[] = "select month, year, sum(ch1_kw_avg / 1000) from energy_history group by year, month";

  ret = sqlite3_exec(db, stmt, compute_total_month_cb, &nrecs, &errmsg);
  if(ret!=SQLITE_OK) {
    printf("Error in select statement %s [%s].\n", stmt, errmsg);
  }
  return 0;
}

int compute_total_year_cb(void *p_data, int num_fields, char **p_fields, char **p_col_names) 
{
  if(num_fields != 2){
    printf("compute_total_year_cb error: bad number of fields %d", num_fields);
    return -1;
  }

  printf("%s -> %s\n", p_fields[0], p_fields[1]);
  return 0;
}
static int compute_total_year()
{
  char *errmsg;
  int   ret;
  int   nrecs = 0;
  char stmt[] = "select year, sum(ch1_kw_avg / 1000) from energy_history group by year";

  ret = sqlite3_exec(db, stmt, compute_total_year_cb, &nrecs, &errmsg);
  if(ret!=SQLITE_OK) {
    printf("Error in select statement %s [%s].\n", stmt, errmsg);
  }
  return 0;
}

static int get_day_of_week(int y, int m, int d)
{
  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  y -= m < 3;
  return (y + y/4 - y/100 + y/400 + t[m-1] + d - 1) % 7;
}

static float w_total = 0;
static float we_total = 0;
int get_we_conso_cb(void *p_data, int num_fields, char **p_fields, char **p_col_names) 
{
  if(num_fields != 5) // year, month, day, hour, conso
  {
    printf("get_we_conso_cb error: bad number of fields %d\n", num_fields);
    return -1;
  }
  if(( get_day_of_week(atoi(p_fields[0]), atoi(p_fields[1]), atoi(p_fields[2])) < 5) &&
     ( atoi(p_fields[3]) > 7 && atoi(p_fields[3]) < 23 ))
    w_total += atof(p_fields[4]);
  else
    we_total += atof(p_fields[4]);
  return 0;
};

static int get_we_conso()
{
  char *errmsg;
  int   ret;
  int   nrecs = 0;
  char stmt[] = "select year, month, day, hour, (ch1_kw_avg / 1000) from energy_history";
  ret = sqlite3_exec(db, stmt, get_we_conso_cb, &nrecs, &errmsg);
  if(ret!=SQLITE_OK) {
    printf("Error in select statement %s [%s].\n", stmt, errmsg);
  }
  return 0;
}

static void printhelp(char *prog_name)
{
  printf("usage:\n");
  printf("  %s <db_file>\n", prog_name);
}

int main(int argc, char *argv[])
{
  if(argc<2){
    printhelp(argv[0]);
    return -1;
  }
  int ret = sqlite3_open_v2(argv[1], &db, SQLITE_OPEN_READONLY, NULL);
  if(ret != SQLITE_OK || db == 0) {
    printf("Could not open database %s: open returned %d\n", argv[1], ret);
    return 1;
  }

  compute_total_year();
  //compute_total_month();
  get_we_conso();
  printf("total week     = %f\n", w_total);
  printf("total weekends = %f\n", we_total);

  sqlite3_close(db);

  return 0;
}

