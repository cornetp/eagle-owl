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

#ifndef __CM160_H__
#define __CM160_H__

#define MAX_DEVICES   1 // Only one device supported now

struct cm160_device {
  struct usb_device *usb_dev;
  usb_dev_handle *hdev;
  int epin;  // IN end point address
  int epout; // OUT end point address
};

struct record_data {
  int addr;
  int year;
  int month;
  int day;
  int hour;
  int min;
  double cost;
  double amps;
  double watts;
  double ah; // watt hour and ampere hour are the units used inside the db
  double wh;
  bool isLiveData; // Flag used to know is this record is the live conumption
                   // or the mean consumption (for the DB)
};

struct settings {
    char *install_path;
    char *output_file_path;
    char *mqtt_host;
    int mqtt_port;
    char *mqtt_topic;
};

// CM160 protocol
#define FRAME_ID_LIVE 0x51
#define FRAME_ID_DB   0x59 // value used to store in the DB (ch1_kw_avg)

// min/max macros
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#endif // __CM160_H__

