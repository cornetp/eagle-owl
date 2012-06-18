#ifndef __CM160_H__
#define __CM160_H__

#define MAX_DEVICES   1 // Only one device supported now

struct cm160_device {
  struct usb_device *usb_dev;
  usb_dev_handle *hdev;
  int epin;  // IN end point address
  int epout; // OUT end point address
};

#endif // __CM160_H__

