#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <usb.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "cm160.h"
#include "usb_utils.h"
#include "db.h"

static char ID_MSG[11] = { 
      0xA9, 0x49, 0x44, 0x54, 0x43, 0x4D, 0x56, 0x30, 0x30, 0x31, 0x01 };
static char WAIT_MSG[11] = { 
      0xA9, 0x49, 0x44, 0x54, 0x57, 0x41, 0x49, 0x54, 0x50, 0x43, 0x52 };


#define HISTORY_SIZE 65536 // 30 * 24 * 60 = 43200 theoric history size

struct cm160_device g_devices[MAX_DEVICES];
static unsigned char history[HISTORY_SIZE][11];

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

  FILE *fp =  fopen(".live", "w");
  if(fp)
  {
    fprintf(fp, "%02d/%02d/%04d %02d:%02d - %.02f kW\n", 
            rec->day, rec->month, rec->year, rec->hour, rec->min, w);
    fclose(fp);
  }
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

// Insert history into DB worker thread
void insert_db_history(void *data)
{
  int i;
  int num_elems = (int)data;
  // For an unknown reason, the cm160 sometimes sends a value > 12 for month
  // -> in that case we use the last valid month received.
  static int last_valid_month = 0; 
  printf("insert %d elems\n", num_elems);
  printf("insert into db...\n");
  clock_t cStartClock = clock();

  db_begin_transaction();
  for(i=0; i<num_elems; i++)
  {
    unsigned char *frame = history[i];
    struct record_data rec;
    decode_frame(frame, &rec);

    if(rec.month < 0 || rec.month > 12)
      rec.month = last_valid_month;
    else
      last_valid_month = rec.month;

    db_insert_hist(&rec);
    printf("\r %.1f%%", min(100, 100*((double)i/num_elems)));
    fflush(stdout);
  }
  db_update_status();
  db_end_transaction();

  printf("\rinsert into db... 100%%\n");
  fflush(stdout);
  printf("update db in %4.2f seconds\n", 
         (clock() - cStartClock) / (double)CLOCKS_PER_SEC);
}

static int process_frame(int dev_id, unsigned char *frame)
{
  static bool receive_history = true;
  static int id = 0;
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
      if(receive_history && id < HISTORY_SIZE)
      {
        if(id == 0)
          printf("downloading history...\n");
        else if(id%10 == 0)
        { // print progression status
          // rough estimation : we should received a month of history
          // -> 31x24x60 minute records
          printf("\r %.1f%%", min(100, 100*((double)id/(31*24*60))));
          fflush(stdout);
        }
        // cache the history in a buffer, we will insert it in the db later.
        memcpy(history[id++], frame, 11);
      }
      else
      {
        db_insert_hist(&rec);
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
        printf("\rdownloading history... 100%%\n");
        fflush(stdout);
        receive_history = false;
        // Now, insert the history into the db
        pthread_t thread;
        pthread_create(&thread, NULL, (void *)&insert_db_history, (void *)id);
      }
      
      process_live_data(&rec);
      printf("LIVE: %02d/%02d/%04d %02d:%02d : %f W\n",
             rec.day, rec.month, rec.year, rec.hour, rec.min, rec.watts);
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

  // read/write main loop
  io_loop(dev_id);
 
  usb_release_interface(hdev, 0);
  return 0;
}

static void demonize()
{
  switch (fork())
  {
    case -1:
      fprintf(stderr, "can't start daemon\n");
      exit(EXIT_FAILURE);
  
    case  0: // child
      // Change the file mode mask
      umask(0);
      // detach from tty
      if (setsid() == -1)
      {
        fprintf(stderr, "can't detach from tty\n");
        exit(EXIT_FAILURE);
      }
      // Close out the standard file descriptors
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
      break;
    default: // parent: exit
      exit(EXIT_SUCCESS);
  }
}

int main(int argc, char *argv[])
{
  int i;
  if(argc>1 && (strcmp(argv[1], "-d")==0) )
    demonize();

  db_open();

  int dev_cnt = 0;
  while((dev_cnt = scan_usb()) == 0)
    sleep(2);
  printf("Found %d compatible device%s\n", dev_cnt, dev_cnt>1?"s":"");

  for(i = 0; i < dev_cnt; i++)
  {
    // do stuff with the detected device...
    if(!(g_devices[i].hdev = usb_open(g_devices[i].usb_dev)))
    {
      fprintf(stderr, "failed to open device\n");
      break;
    }
    handle_device(i);

    usb_close(g_devices[i].hdev);
  } 
  return 0;
}

