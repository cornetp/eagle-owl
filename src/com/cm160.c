#include <stdio.h>
#include <string.h>
#include <usb.h>

#define MAX_DEVICES   1 // Only one device supported now

#define OWL_VENDOR_ID 0x0fde
#define CM160_DEV_ID  0xca05

static struct usb_device *g_devices[MAX_DEVICES];

static int scan_device(struct usb_device *dev, int *dev_cnt)
{
//  printf("found device %04x:%04x\n", dev->descriptor.idVendor, dev->descriptor.idProduct);
  if(dev->descriptor.idVendor == OWL_VENDOR_ID && dev->descriptor.idProduct == CM160_DEV_ID)
  {
    printf("Found compatible device #%d: %04x:%04x (%s)\n",
           *dev_cnt, dev->descriptor.idVendor, dev->descriptor.idProduct, dev->filename);
    g_devices[*dev_cnt] = dev;
    (*dev_cnt)++;
  }

  return 0;
}

static int scan_usb()
{
  printf("%s() called\n", __FUNCTION__);

  int dev_cnt = 0;

  usb_init();
  usb_find_busses();
  usb_find_devices();

  struct usb_bus *bus = NULL;
  struct usb_device *dev = NULL;
  for(bus = usb_get_busses(); bus; bus = bus->next)
  {
    for(dev = bus->devices; dev; dev=dev->next)
    {
      scan_device(dev, &dev_cnt);
      if(dev_cnt==MAX_DEVICES) return dev_cnt;
    }
  }

  return dev_cnt;
}

static void print_dev_infos(struct usb_device *dev, usb_dev_handle *hdev)
{
  char string[50];
  usb_get_string_simple(hdev, dev->descriptor.iManufacturer, string, sizeof(string));
  printf("Device Manfucaturer : %s\n", string);
  usb_get_string_simple(hdev, dev->descriptor.iProduct, string, sizeof(string));
  printf("Product Name : %s\n", string);
  usb_get_string_simple(hdev, dev->descriptor.iSerialNumber, string, sizeof(string));
  printf("Device Serial Number: %s\n", string);
}

static int get_word(usb_dev_handle *hdev, int epin, char *word)
{
  int ret=0, size=0;
  while(size<11)
  {
    ret = usb_bulk_read(hdev, epin, word, sizeof(word), 10000);
    if(ret<0) return -1;
    size += ret;
  }
  return size;
}

static int process_frame(unsigned char *frame)
{
  int i;
  unsigned int checksum = 0;

  if(frame[0] != 0x59 && frame[0] != 0x51)
  {
    printf("data error: invalid ID 0x%x\n", frame[0]);
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

  printf("%02d/%02d/%04d %02d:%02d : %f kW\n", day, month, year, hour, minutes, watts);
}

static int handle_device(struct usb_device *dev, usb_dev_handle *hdev)
{
  int r;

  if( 0 != (r = usb_set_configuration(hdev, dev->config[0].bConfigurationValue)) )
  {
    printf("usb_set_configuration returns %d (%s)\n", r, usb_strerror());
  }

  //reserving the device interface for our applicatoin , if another driver/software is using the device , it will return ‘interface busy’
  if((r=usb_claim_interface(hdev, dev->config[0].interface->altsetting->bInterfaceNumber)) < 0)
  {
    printf("Interface cannot be claimed: %d\n", r);
    return r;
  }

  printf("Interface Claim Status : %d\n",r);
//  print_dev_infos(dev, hdev);

  int epin=0, epout=0;
  int nep = dev->config->interface->altsetting->bNumEndpoints;
  int i;
  //unsigned char buffer[11];
  unsigned char buffer[88];
  unsigned char word[12];
  memset(buffer, 0, sizeof(buffer));
  memset(word, 0, sizeof(word));
  for(i=0; i<nep; i++)
  {
    int ep = dev->config->interface->altsetting->endpoint[i].bEndpointAddress;
    if(ep&(1<<7))
      epin = ep;
    else
      epout = ep;
  }
  printf("epin = 0x%x - epout = 0x%x\n", epin, epout);
  char id_msg[11] = { 0xA9, 0x49, 0x44, 0x54, 0x43, 0x4D, 0x56, 0x30, 0x30, 0x31, 0x01 };
  char wait_msg[11] = { 0xA9, 0x49, 0x44, 0x54, 0x57, 0x41, 0x49, 0x54, 0x50, 0x43, 0x52 };
  unsigned char data[1];
  data[0] = 0xA5;	
        r = usb_bulk_write(hdev, epout, &data, sizeof(data), 1000);
        printf("bulk_write 0x%x returned %d\n", data[0], r);

  while(1)
  {
    memset(buffer, 0, sizeof(buffer));
    r = usb_bulk_read(hdev, epin, buffer, sizeof(buffer), 10000);
    //r = get_word(hdev, epin, buffer);
    if(r<0)
    {
      printf("bulk_read returned %d (%s)\n", r, usb_strerror());
      return -1;
    }
//    printf("read %d bytes: \n", r);
    //if(r%11) continue;
    unsigned char *bufptr = (unsigned char *)buffer;
    if(r%11 == 0){
      int nb_words = r/11;
      //printf("received %d words\n", nb_words);
      while(nb_words--){
        memcpy(word, bufptr, 11);
        bufptr+=11;
      //  for(i=0; i<11; i++)
      //  {
      //    printf("0x%02x - ", word[i]);
      //  }
        if(strncmp((char *)word, id_msg, 11) == 0)
          data[0]=0x5A;
        else if(strncmp((char *)word, wait_msg, 11) == 0)
          data[0]=0xA5;
        else
        {
//          printf("received data ");
 //         for(i=0; i<11; i++)
 //           printf("0x%02x - ", word[i]);
//          printf("\n\n");
          process_frame((unsigned char *)word);
          continue;
        }
//        printf("> %s\n", word);
        r = usb_bulk_write(hdev, epout, &data, sizeof(data), 1000);
//        printf(" -> WRITE 0x%x \n\n", data[0]);
      };    
    }
  }
 
  usb_release_interface(hdev,0);
  return 0;
}

int main(int argc, char *argv[])
{
  int i, reset=0;
  int dev_cnt = scan_usb();
  printf("Found %d compatible device%s\n", dev_cnt, dev_cnt>1?"s":"");

  if(argc>1 && (strcmp(argv[1], "-r")==0) )
  {
    printf("reset devices!\n");
    reset = 1;
  }

  for(i = 0; i < dev_cnt; i++)
  {
    // do stuff with the detected device...
    usb_dev_handle *hdev;
    if(!(hdev = usb_open(g_devices[i])))
    {
      printf("failed to open device\n");
      break;
    }
    printf("device opened successfully\n");
    if(reset)
    {
      int ret = usb_reset(hdev);
      printf("usb_reset returned %d\n", ret);
    }
    else
      handle_device(g_devices[i], hdev);

    usb_close(hdev);
  }
  
  return 0;

}
