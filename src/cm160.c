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
#include <stdbool.h>
#include <string.h>
#include <usb.h>
#include <libconfig.h>
#include <pthread.h>
#include <fcntl.h>
#include <mosquitto.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "cm160.h"
#include "usb_utils.h"
//#include "db.h"
#include "demonize.h"

static char ID_MSG[11] = {
      0xA9, 0x49, 0x44, 0x54, 0x43, 0x4D, 0x56, 0x30, 0x30, 0x31, 0x01 };
static char WAIT_MSG[11] = {
      0xA9, 0x49, 0x44, 0x54, 0x57, 0x41, 0x49, 0x54, 0x50, 0x43, 0x52 };


#define HISTORY_SIZE 65536 // 30 * 24 * 60 = 43200 theoric history size

#define CP210X_IFC_ENABLE       0x00
#define CP210X_GET_LINE_CTL     0x04
#define CP210X_SET_MHS          0x07
#define CP210X_GET_MDMSTS       0x08
#define CP210X_GET_FLOW         0x14
#define CP210X_GET_BAUDRATE     0x1D
#define CP210X_SET_BAUDRATE     0x1E

/* CP210X_IFC_ENABLE */
#define UART_ENABLE             0x0001
#define UART_DISABLE            0x0000

struct cm160_device g_devices[MAX_DEVICES];
static unsigned char history[HISTORY_SIZE][11];

struct settings st;
FILE *fout=NULL;
struct mosquitto *mosq=NULL;
config_t cfg, *cf;

static void process_live_data(struct record_data *rec)
{
  static double _watts = -1;
  double w = rec->watts;

  if(!rec->isLiveData) // special case: update only the time
  {
    if(_watts == -1)
      return;
    w = _watts;
  }
  else
    _watts = w;

/*  FILE *fp =  fopen("bds/.live", "w");
  if(fp)
  {
    if(rec->hour!=255) // to avoid writing strange values (i.e. date 2255, hour 255:255) that sometimes I got
      fprintf(fp, "%02d/%02d/%04d %02d:%02d - %.02f kW\n",
              rec->day, rec->month, rec->year, rec->hour, rec->min, w);
    fclose(fp);
  }*/
}

static void decode_frame(unsigned char *frame, struct record_data *rec)
{
  int volt = 230; // TODO: use the value from energy_param table (supply_voltage)
  rec->addr = 0; // TODO: don't use an harcoded addr value for the device...
  rec->year = frame[1]+2000;
  rec->month = frame[2];
  rec->day = frame[3];
  rec->hour = frame[4];
  rec->min = frame[5];
  rec->cost = (frame[6]+(frame[7]<<8))/100.0;
  rec->amps = (frame[8]+(frame[9]<<8))*0.07; // mean intensity during one minute
  rec->watts = rec->amps * volt; // mean power during one minute
  rec->ah = rec->amps/60; // -> we must devide by 60 to convert into ah and wh
  rec->wh = rec->watts/60;
  rec->isLiveData = (frame[0] == FRAME_ID_LIVE)? true:false;
}


bool receive_history = true;
int frame_id = 0;
static int process_frame(int dev_id, unsigned char *frame)
{
  int i;
  unsigned char data[1];
  unsigned int checksum = 0;
  static int last_valid_month = 0;
  usb_dev_handle *hdev = g_devices[dev_id].hdev;
  int epout = g_devices[dev_id].epout;

  if(strncmp((char *)frame, ID_MSG, 11) == 0)
  {
//    printf("received ID MSG\n");
    data[0]=0x5A;
    usb_bulk_write(hdev, epout, (const char *)&data, sizeof(data), 1000);
  }
  else if(strncmp((char *)frame, WAIT_MSG, 11) == 0)
  {
//    printf("received WAIT MSG\n");
    data[0]=0xA5;
    usb_bulk_write(hdev, epout, (const char *)&data, sizeof(data), 1000);
  }
  else
  {
    if(frame[0] != FRAME_ID_LIVE && frame[0] != FRAME_ID_DB)
    {
      printf("data error: invalid ID 0x%x\n", frame[0]);
      for(i=0; i<11; i++)
        printf("0x%02x - ", frame[i]);
      printf("\n");
      return -1;
    }

    for(i=0; i<10; i++)
      checksum += frame[i];
    checksum &= 0xff;
    if(checksum != frame[10])
    {
      printf("data error: invalid checksum: expected 0x%x, got 0x%x\n",
             frame[10], checksum);
      return -1;
    }

    struct record_data rec;
    decode_frame(frame, &rec);

    if(rec.month < 0 || rec.month > 12)
      rec.month = last_valid_month;
    else
      last_valid_month = rec.month;

    if(frame[0]==FRAME_ID_DB)
    {
      if(receive_history && frame_id < HISTORY_SIZE)
      {
        if(frame_id == 0)
          printf("OWL-CM160 sends last month data the first time it's connected.\nSkipping history data before reading live values...\n");
        else if(frame_id%10 == 0)
        { // print progression status
          // rough estimation : we should received a month of history
          // -> 31x24x60 minute records
//	printf("fid: %d calc: %d\n",frame_id,100*frame_id/(31*24*60));
//	if (((100*frame_id/(31*24*60)) % 10)==0)
//		printf(".");
//          printf("\r %.1f%%", min(100, 100*((double)frame_id/(31*24*60))));
          fflush(stdout);
        }
        // cache the history in a buffer, we will insert it in the db later.
        memcpy(history[frame_id++], frame, 11);
      }
      else
      {
//        db_insert_hist(&rec);
//        db_update_status();
        process_live_data(&rec); // the record is not live data, but we do that to
                                 // update the time in the .live file
                                 // (the cm160 send a DB frame when a new minute starts)
      }
    }
    else
    {
      if(receive_history)
      { // When we receive the first live data,
        // we know that the history is totally downloaded
        printf("History download complete. Receiving live values from now on\n");
        fflush(stdout);
        receive_history = false;
      }

      process_live_data(&rec);
      printf("LIVE: %02d/%02d/%04d %02d:%02d : %f W\n",
             rec.day, rec.month, rec.year, rec.hour, rec.min, rec.watts);
	  // Write value to disk (if enabled)
      if (st.output_file_path!=NULL) {
          fprintf(fout,"%04d%02d%02d-%02d:%02d: %f W\n",rec.year,rec.month,rec.day,rec.hour,rec.min,rec.watts);
          fflush(fout);
      }
	  // Send value to MQTT broker (if enabled)
	  if (st.mqtt_host!=NULL) {
		  char aux[100];
		  sprintf(aux,"%f",rec.watts);
		  if (mosquitto_connect(mosq,st.mqtt_host,st.mqtt_port,5)==MOSQ_ERR_SUCCESS) {
			  mosquitto_publish(mosq,NULL,st.mqtt_topic,strlen(aux),aux,0,false);
			  mosquitto_disconnect(mosq);
		  }
		  else {
			  printf("Error connecting to MQTT broker\n");
		  }
	  }

    }
  }
  return 0;
}

static int io_loop(int dev_id)
{
  int ret;
  usb_dev_handle *hdev = g_devices[dev_id].hdev;
  int epin = g_devices[dev_id].epin;
  unsigned char buffer[512];
  unsigned char word[11];

  memset(buffer, 0, sizeof(buffer));
  memset(word, 0, sizeof(word));

  while(1)
  {
    memset(buffer, 0, sizeof(buffer));
    ret = usb_bulk_read(hdev, epin, (char*)buffer, sizeof(buffer), 10000);
    if(ret < 0)
    {
      printf("bulk_read returned %d (%s)\n", ret, usb_strerror());
      return -1;
    }
//    printf("read %d bytes: \n", ret);
    unsigned char *bufptr = (unsigned char *)buffer;
    int nb_words = ret/11; // incomplete words are resent
    while(nb_words--)
    {
      memcpy(word, bufptr, 11);
      bufptr+=11;
      process_frame(dev_id, word);
    }
  }
  return 0;
}

static int handle_device(int dev_id)
{
  int r, i;
  struct usb_device *dev = g_devices[dev_id].usb_dev;
  usb_dev_handle *hdev = g_devices[dev_id].hdev;

  usb_detach_kernel_driver_np(hdev, 0);

  if( 0 != (r = usb_set_configuration(hdev, dev->config[0].bConfigurationValue)) )
  {
    printf("usb_set_configuration returns %d (%s)\n", r, usb_strerror());
    return -1;
  }

  if((r = usb_claim_interface(hdev, 0)) < 0)
  {
    printf("Interface cannot be claimed: %d\n", r);
    return r;
  }

  int nep = dev->config->interface->altsetting->bNumEndpoints;
  for(i=0; i<nep; i++)
  {
    int ep = dev->config->interface->altsetting->endpoint[i].bEndpointAddress;
    if(ep&(1<<7))
      g_devices[dev_id].epin = ep;
    else
      g_devices[dev_id].epout = ep;
  }

  // Set baudrate
  int baudrate = 250000;
  r = usb_control_msg(hdev, USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_ENDPOINT_OUT,
                      CP210X_IFC_ENABLE, UART_ENABLE, 0, NULL, 0, 500);
  r = usb_control_msg(hdev, USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_ENDPOINT_OUT,
                      CP210X_SET_BAUDRATE, 0, 0, (char *)&baudrate, sizeof(baudrate), 500);
  r = usb_control_msg(hdev, USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_ENDPOINT_OUT,
                      CP210X_IFC_ENABLE, UART_DISABLE, 0, NULL, 0, 500);

// read/write main loop
  io_loop(dev_id);

  usb_release_interface(hdev, 0);
  return 0;
}


void read_configuration(void) {
	const char *buf=NULL;

	cf=&cfg;
	config_init(cf);

	// Set defaults for settings
	st.install_path=NULL;
	st.output_file_path=NULL;
	st.mqtt_host=NULL;
	st.mqtt_port=1883;
	st.mqtt_topic="eagleowl/w";

	// Look for a config file in /etc
	if (config_read_file(cf,"/etc/eagleowl.conf")) {
		// If it exists, use the values there
		if (config_lookup_string(cf,"install_path",&buf))
			st.install_path=(char *)buf;
		if (config_lookup_string(cf,"output_file_path",&buf)) {
			st.output_file_path=(char *)buf;
			// Open file for further writes
			if ((fout=fopen(st.output_file_path,"at"))==NULL) {
				fprintf(stderr,"Can't open output file %s\n",st.output_file_path);
				exit(EXIT_FAILURE);
			}
		}
		if (config_lookup_string(cf,"mqtt_host",&buf))
			st.mqtt_host=(char *)buf;
		if (config_lookup_int(cf,"mqtt_port",&st.mqtt_port)==CONFIG_FALSE)
			st.mqtt_port=1883;
		if (config_lookup_string(cf,"mqtt_topic",&buf))
			st.mqtt_topic=(char *)buf;
	}
	else {
		fprintf(stderr,"Warning, no configuration file defined or bad syntax: %s\n",config_error_text(cf));
	}

	// Print configuration
	printf("Settings:\n");
	printf("  Install path: %s\n",st.install_path);
	if (st.output_file_path!=NULL)
		printf("  File output enabled, filename: %s\n",st.output_file_path);
	if (st.mqtt_host!=NULL) 
		printf("  MQTT support enabled, will publish to %s:%d, topic %s\n",st.mqtt_host,st.mqtt_port,st.mqtt_topic);

}

int main(int argc, char **argv)
{
	read_configuration();

  // If MQTT support is enabled, initialize the library
  if (st.mqtt_host!=NULL) {
	  mosquitto_lib_init();
	  mosq=mosquitto_new(NULL,true,NULL);
  }

  int dev_cnt;
  if(argc>1 && (strcmp(argv[1], "-d")==0) )
    demonize(argv[0]);

  while(1)
  {
    //db_open();
    dev_cnt = 0;
    receive_history = true;
    frame_id = 0;
    printf("Waiting for cm160 device to be connected\n");
    while((dev_cnt = scan_usb()) == 0)
      sleep(2);
    printf("Found %d compatible device%s\n", dev_cnt, dev_cnt>1?"s":"");

    // Only 1 device supported
    if(!(g_devices[0].hdev = usb_open(g_devices[0].usb_dev)))
    {
      fprintf(stderr, "failed to open device\n");
      //db_close();
      break;
    }
    handle_device(0);
    usb_close(g_devices[0].hdev);
    //db_close();
  }

  if (fout!=NULL)
      fclose(fout);

  if (st.mqtt_host!=NULL) {
	  mosquitto_destroy(mosq);
	  mosquitto_lib_cleanup();
  }
  config_destroy(cf);

  return 0;
}

