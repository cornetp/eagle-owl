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

#endif//__EAGLEOWL_SQL_H__
