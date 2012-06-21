#ifndef __EAGLEOWL_SQL_H__
#define __EAGLEOWL_SQL_H__


#define LIST_TABLES "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name"

#define CREATE_HISTORY_TBL  "CREATE TABLE energy_history(" \
                            "addr INT, year INT, month INT, day INT," \
                            " hour INT, min INT," \
                            " ch1_amps_avg INT, ch1_kw_avg INT, ghg INT, cost INT," \
                            " ch1_amps_min INT, ch1_amps_max INT, ch1_kw_min INT," \
                            " ch1_kw_max INT," \
                            " PRIMARY KEY(addr, year, month, day, hour, min));"

#define CREATE_PARAM_TBL    "CREATE TABLE energy_param(" \
                            "key VARCHAR, value VARCHAR," \
                            " PRIMARY KEY(key));"

#define CREATE_SENSOR_TBL   "CREATE TABLE energy_sensor(" \
                            "addr INT, name VARCHAR, model INT, tariff_uid VARCHAR," \
                            " PRIMARY KEY(addr));"

#define CREATE_TARIFFV2_TBL "CREATE TABLE energy_tariffv2("           \
                            "uid VARCHAR, name VARCHAR,"              \
                            " b1_time INT, b1_cost INT, b1_type INT," \
                            " b2_time INT, b2_cost INT, b2_type INT," \
                            " b3_time INT, b3_cost INT, b3_type INT," \
                            " b4_time INT, b4_cost INT, b4_type INT," \
                            " b5_time INT, b5_cost INT, b5_type INT," \
                            " b6_time INT, b6_cost INT, b6_type INT," \
                            " PRIMARY KEY(name));"

#define CREATE_YEAR_STAT    "CREATE TABLE energy_year_stat("              \
                            "addr INT, year INT, kwh_total INT,"          \
                            " kwh_week_total INT, kwh_weekend_total INT," \
                            " status INT,"                                \
                            " PRIMARY KEY(addr, year));"

#define CREATE_MONTH_STAT   "CREATE TABLE energy_month_stat("              \
                            "addr INT, year INT, month INT, kwh_total INT,"\
                            " kwh_week_total INT, kwh_weekend_total INT,"  \
                            " status INT,"                                 \
                            " PRIMARY KEY(addr, year, month));"

#define CREATE_DAY_STAT     "CREATE TABLE energy_day_stat("                \
                            "addr INT, year INT, month INT, day INT, "     \
                            "kwh_total INT, kwh_week_total INT, "          \
                            "kwh_weekend_total INT, status INT, "          \
                            " PRIMARY KEY(addr, year, month, day));"

#define CREATE_HOUR_STAT    "CREATE TABLE energy_hour_stat("                     \
                            "addr INT, year INT, month INT, day INT, hour INT, " \
                            "kwh_total INT, kwh_week_total INT, "                \
                            "kwh_weekend_total INT, status INT, "                \
                            " PRIMARY KEY(addr, year, month, day, hour));"

#define ATTACH_IMPORT_DB    "ATTACH DATABASE '%s' AS import_db"
#define DETACH_IMPORT_DB    "DETACH import_db"

#define MERGE_HISTORY_TBL "INSERT INTO energy_history " \
                          "SELECT * from import_db.energy_history " \
                          "WHERE NOT EXISTS ( " \
                          " SELECT * from energy_history WHERE" \
                          " import_db.energy_history.addr = energy_history.addr "\
                          " AND import_db.energy_history.year = energy_history.year "\
                          " AND import_db.energy_history.month = energy_history.month"\
                          " AND import_db.energy_history.day = energy_history.day" \
                          " AND import_db.energy_history.hour = energy_history.hour" \
                          " AND import_db.energy_history.min = energy_history.min)"

#define MERGE_PARAM_TBL     "INSERT INTO energy_param "             \
                            "SELECT * FROM import_db.energy_param " \
                            "WHERE import_db.energy_param.key "     \
                            "NOT IN (SELECT key FROM energy_param)"

#define MERGE_SENSOR_TBL    "INSERT INTO energy_sensor "             \
                            "SELECT * FROM import_db.energy_sensor " \
                            "WHERE import_db.energy_sensor.addr "    \
                            " NOT IN (SELECT addr FROM energy_sensor)"

#define MERGE_TARIFFV2_TBL  "INSERT INTO energy_tariffv2 "             \
                            "SELECT * FROM import_db.energy_tariffv2 " \
                            "WHERE import_db.energy_tariffv2.name "    \
                            " NOT IN (SELECT name FROM energy_tariffv2)"

#define COUNT_HISTORY_ELEM    "SELECT COUNT(*) FROM energy_history"

#define SELECT_DISTINCT_YEAR  "SELECT DISTINCT year FROM energy_history" \
                              " ORDER BY year"

#define SELECT_DISTINCT_MONTH "SELECT DISTINCT month FROM energy_history" \
                              " WHERE year = %d ORDER BY month"

#define SELECT_DISTINCT_DAY   "SELECT DISTINCT day FROM energy_history" \
                              " WHERE year = %d AND month = %d" \
                              " ORDER BY day"

#define SELECT_DISTINCT_HOUR  "SELECT DISTINCT hour FROM energy_history" \
                              " WHERE year = %d AND month = %d AND day = %d" \
                              " ORDER BY hour"

#define SELECT_MIN_CONSO   "SELECT (ch1_kw_avg/1000) FROM energy_history" \
                           " WHERE year = %d AND month = %d AND day = %d" \
                           " AND hour = %d"

#define INSERT_STAT_YEAR   "INSERT INTO energy_year_stat "   \
                           " VALUES (%d, %d, %f, %f, %f, %d)"

#define INSERT_STAT_MONTH  "INSERT INTO energy_month_stat "   \
                           " VALUES (%d, %d, %d, %f, %f, %f, %d)"

#define INSERT_STAT_DAY    "INSERT INTO energy_day_stat "   \
                           " VALUES (%d, %d, %d, %d, %f, %f, %f, %d)"

#define INSERT_STAT_HOUR   "INSERT INTO energy_hour_stat "   \
                           " VALUES (%d, %d, %d, %d, %d, %f, %f, %f, %d)"

#define INSERT_HISTORY_TBL "INSERT OR IGNORE INTO energy_history " \
                           " VALUES (%d, %d, %d, %d, %d, %d," \
                           " %f, %f, %d, %d, %f, %f, %f, %f);"

#endif//__EAGLEOWL_SQL_H__
