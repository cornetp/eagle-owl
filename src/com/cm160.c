#include <stdio.h>
#include <string.h>
#include <usb.h>

#include "cm160.h"
#include "usb_utils.h"

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

static char ID_MSG[11]   = { 0xA9, 0x49, 0x44, 0x54, 0x43, 0x4D, 0x56, 0x30, 0x30, 0x31, 0x01 };
static char WAIT_MSG[11] = { 0xA9, 0x49, 0x44, 0x54, 0x57, 0x41, 0x49, 0x54, 0x50, 0x43, 0x52 };

#define FRAME_ID_LIVE 0x51 
#define FRAME_ID_DB   0x59 // value used to store in the DB (ch1_kw_avg)

//struct usb_device *g_devices[MAX_DEVICES];
struct cm160_device g_devices[MAX_DEVICES];

static int process_frame(int dev_id, unsigned char *frame)
{
  int i;
  unsigned char data[1];
  unsigned int checksum = 0;
  usb_dev_handle *hdev = g_devices[dev_id].hdev;
  int epout = g_devices[dev_id].epout;

  if(strncmp((char *)frame, ID_MSG, 11) == 0)
  {
    printf("received ID MSG\n");
    data[0]=0x5A;
    usb_bulk_write(hdev, epout, (const char *)&data, sizeof(data), 1000);
  }
  else if(strncmp((char *)frame, WAIT_MSG, 11) == 0)
  {
    printf("received WAIT MSG\n");
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
      printf("data error: invalid checksum: expected 0x%x, got 0x%x\n", frame[10], checksum);
      return -1;
    }

    int volts = 230;
    int year = frame[1]+2000;
    int month = frame[2];
    int day = frame[3];
    int hour = frame[4];
    int minutes = frame[5];
    float cost = (frame[6]+(frame[7]<<8))/100.0;
    float amps = (frame[8]+(frame[9]<<8))*0.07;
    float watts = amps * volts;

    printf("%02d/%02d/%04d %02d:%02d : %f kW %s\n", day, month, year, hour, minutes, watts,
           (frame[0]==FRAME_ID_LIVE)?" < LIVE":" < DB");
  }
  return 0;
}

static int io_thread(int dev_id)
{
  int ret;
  usb_dev_handle *hdev = g_devices[dev_id].hdev;
  int epin = g_devices[dev_id].epin;
  unsigned char buffer[11*1024];
  static unsigned char word[11];
  static int remaining = 11;

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
    printf("read %d bytes: \n", ret);
    unsigned char *bufptr = (unsigned char *)buffer;
    if(remaining<11)
    {
      int rest = min(remaining, ret);
      memcpy((word+(11-remaining)), bufptr, rest);
      remaining -= rest;
      if(remaining == 0)
      {
        process_frame(dev_id, word);
        remaining = 11;
      }
      ret -= rest;
    }

    {
      int nb_words = ret/11;
      int rest = ret%11;
      while(nb_words--)
      {
        memcpy(word, bufptr, 11);
        bufptr+=11;
        process_frame(dev_id, word);
      };
/*  // usefull? seem that full word is retransmitted if not fully transmitted the first time.
      if(rest)
      {
        printf("rest %d chars\n", rest);
        memcpy(word, bufptr, rest);
        remaining -= rest;
      }*/
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

//  print_dev_infos(dev, hdev);

  int nep = dev->config->interface->altsetting->bNumEndpoints;
  for(i=0; i<nep; i++)
  {
    int ep = dev->config->interface->altsetting->endpoint[i].bEndpointAddress;
    if(ep&(1<<7))
      g_devices[dev_id].epin = ep;
    else
      g_devices[dev_id].epout = ep;
  }

  io_thread(dev_id);
 
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
  
    case  0: // child: detach from tty
      if (setsid() == -1)
      {
        fprintf(stderr, "can't detach from tty\n");
        exit(EXIT_FAILURE);
      }
      break;
    default: // parent: exit
      exit(EXIT_SUCCESS);
  }
}


int main(int argc, char *argv[])
{
  int i, reset=0;
  if(argc>1 && (strcmp(argv[1], "-d")==0) )
    demonize();

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
