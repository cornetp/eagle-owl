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

#ifndef __EAGLEOWL_SQL_H__
#define __EAGLEOWL_SQL_H__

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

#define CREATE_UPDATE_STAT_TRIGGER "CREATE TRIGGER IF NOT EXISTS updatestat_cb "\
                                   "AFTER INSERT ON energy_history"             \
                                   " BEGIN SELECT update_stat_db(NEW.year,"     \
                                   "  NEW.month, NEW.day, NEW.hour,"            \
                                   "  NEW.ch1_kw_avg/1000);"                    \
                                   " END;"

#define DELETE_UPDATE_STAT_TRIGGER "DROP TRIGGER IF EXISTS updatestat_cb;"

// record_count: number of minutes records that have been used to compute the stat
// e.g: for a full day_stat: record_count should be 24*60 (24 hours of 60 minutes)
// status: 0: the stat is complete (record_count is the max value)
//         1: the stat is incomplete (record_count is lower than the max value)

#define CREATE_YEAR_STAT    "CREATE TABLE energy_year_stat("              \
                            "addr INT, year INT, kwh_total INT,"          \
                            " kwh_week_total INT, kwh_weekend_total INT," \
                            " record_count INT, status INT,"              \
                            " PRIMARY KEY(addr, year));"

#define CREATE_MONTH_STAT   "CREATE TABLE energy_month_stat("              \
                            "addr INT, year INT, month INT, kwh_total INT,"\
                            " kwh_week_total INT, kwh_weekend_total INT,"  \
                            " record_count INT, status INT,"               \
                            " PRIMARY KEY(addr, year, month));"

#define CREATE_DAY_STAT     "CREATE TABLE energy_day_stat("                         \
                            "addr INT, year INT, month INT, day INT, "              \
                            "kwh_total INT, kwh_week_total INT, "                   \
                            "kwh_weekend_total INT, record_count INT, status INT, " \
                            " PRIMARY KEY(addr, year, month, day));"

#define CREATE_HOUR_STAT    "CREATE TABLE energy_hour_stat("                        \
                            "addr INT, year INT, month INT, day INT, hour INT, "    \
                            "kwh_total INT, kwh_week_total INT, "                   \
                            "kwh_weekend_total INT, record_count INT, status INT, " \
                            " PRIMARY KEY(addr, year, month, day, hour));"

#define ATTACH_IMPORT_DB    "ATTACH DATABASE '%s' AS import_db"
#define DETACH_IMPORT_DB    "DETACH import_db"

#define MERGE_HISTORY_TBL "INSERT INTO energy_history "                               \
                          "SELECT * from import_db.energy_history "                   \
                          "WHERE NOT EXISTS ( "                                       \
                          " SELECT * from energy_history WHERE"                       \
                          " import_db.energy_history.addr = energy_history.addr "     \
                          " AND import_db.energy_history.year = energy_history.year " \
                          " AND import_db.energy_history.month = energy_history.month"\
                          " AND import_db.energy_history.day = energy_history.day"    \
                          " AND import_db.energy_history.hour = energy_history.hour"  \
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

#define INSERT_HISTORY_TBL "INSERT OR IGNORE INTO energy_history " \
                           " VALUES (%d, %d, %d, %d, %d, %d," \
                           " %f, %f, %d, %d, %f, %f, %f, %f);"

#define UPDATE_STAT_HOUR   "INSERT OR REPLACE INTO energy_hour_stat"               \
                           " SELECT addr, year, month, day, hour, kwh_total + %f," \
                           "  kwh_week_total + %f, kwh_weekend_total + %f,"        \
                           "  record_count + 1 AS c, status FROM energy_hour_stat" \
                           "  WHERE year=%d AND month=%d AND day=%d AND hour=%d"   \
                           " UNION "                                               \
                           " SELECT %d, %d, %d, %d, %d, %f, %f, %f, 1 as r, 0"     \
                           " ORDER BY r DESC"                                      \
                           " LIMIT 1;"

#define UPDATE_STAT_DAY    "INSERT OR REPLACE INTO energy_day_stat"                \
                           " SELECT addr, year, month, day, kwh_total + %f,"       \
                           "  kwh_week_total + %f, kwh_weekend_total + %f,"        \
                           "  record_count + 1 AS c, status FROM energy_day_stat"  \
                           "  WHERE year = %d AND month = %d AND day = %d"         \
                           " UNION "                                               \
                           " SELECT %d, %d, %d, %d, %f, %f, %f, 1 as r, 0"         \
                           " ORDER BY r DESC"                                      \
                           " LIMIT 1;"

#define UPDATE_STAT_MONTH  "INSERT OR REPLACE INTO energy_month_stat"              \
                           " SELECT addr, year, month, kwh_total + %f,"            \
                           "  kwh_week_total + %f, kwh_weekend_total + %f,"        \
                           "  record_count + 1 AS c, status FROM energy_month_stat"\
                           "  WHERE year = %d AND month = %d"                      \
                           " UNION "                                               \
                           " SELECT %d, %d, %d, %f, %f, %f, 1 as r, 0"             \
                           " ORDER BY r DESC"                                      \
                           " LIMIT 1;"

#define UPDATE_STAT_YEAR   "INSERT OR REPLACE INTO energy_year_stat"               \
                           " SELECT addr, year, kwh_total + %f,"                   \
                           "  kwh_week_total + %f, kwh_weekend_total + %f,"        \
                           "  record_count + 1 AS c, status FROM energy_year_stat" \
                           "  WHERE year = %d"                                     \
                           " UNION "                                               \
                           " SELECT %d, %d, %f, %f, %f, 1 as r, 0"                 \
                           " ORDER BY r DESC"                                      \
                           " LIMIT 1;"

#endif//__EAGLEOWL_SQL_H__
