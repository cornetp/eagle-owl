#ifndef __CM160_H__
#define __CM160_H__

#define MAX_DEVICES   1 // Only one device supported now

struct cm160_device {
  struct usb_device *usb_dev;
  usb_dev_handle *hdev;
  int epin;  // IN end point address
  int epout; // OUT end point address
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

