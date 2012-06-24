#ifndef __DB_H__
#define __DB_H__

#define SQL_EXEC(db,sql,desc)                                               \
        {                                                                   \
          int ret = SQLITE_OK;                                              \
          char *err;                                                        \
          if((ret = sqlite3_exec(db, sql, NULL, NULL, &err)) != SQLITE_OK)  \
          {                                                                 \
            printf("%s error: %s\n", desc, err);                            \
            sqlite3_free(err);                                              \
            return ret;                                                     \
          }                                                                 \
        }

int db_open(void);
int db_insert_hist(int year, int month, int day, int hour, int min, double watts, double amps);

#endif // __DB_H__
