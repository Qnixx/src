#ifndef XHCI_H_
#define XHCI_H_

#include <lib/types.h>

typedef struct {
  uint8_t cap_length;
  uint8_t rsvd;
  uint16_t hci_version;
  uint32_t hcsparams1;
  uint32_t hcsparams2;
  uint32_t hcsparams3;
  uint32_t hccparams1;
  uint32_t dboff;
  uint32_t rtsoff;
  uint32_t hccparams2;
} xhci_caps_t;

typedef struct {
  uint32_t usbcmd;
  uint32_t usbsts;
  uint32_t pagesize;
  uint8_t rsv0[8];
  uint32_t dnctrl;
  uint32_t crcr;
  uint8_t rsv1[16];
  uint32_t config;
} xhci_ops_t;

typedef struct {
  uint32_t portsc;
  uint32_t portpmsc;
  uint32_t portli;
  uint32_t porthlpmc;
} xhci_port_t;

void xhci_init(void);


#endif
