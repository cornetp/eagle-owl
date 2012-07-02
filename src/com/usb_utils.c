#include <stdio.h>
#include <usb.h>
#include "cm160.h"

#define OWL_VENDOR_ID 0x0fde
#define CM160_DEV_ID  0xca05

extern struct cm160_device g_devices[MAX_DEVICES];

static int scan_device(struct usb_device *dev, int *dev_cnt)
{
  if(dev->descriptor.idVendor == OWL_VENDOR_ID && 
     dev->descriptor.idProduct == CM160_DEV_ID)
  {
    printf("Found compatible device #%d: %04x:%04x (%s)\n",
           *dev_cnt, dev->descriptor.idVendor, 
           dev->descriptor.idProduct, dev->filename);
    g_devices[*dev_cnt].usb_dev = dev;
    (*dev_cnt)++;
  }

  return 0;
}

int scan_usb()
{
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

