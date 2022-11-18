#ifndef DEV_H_
#define DEV_H_

#include <lib/types.h>

/*
 *   Device class will be for example: DEV_BLOCK
 *   while a device type is more specfic and can
 *   be for example: DEV_BLOCK_SATA.
 *  
 *
 */

#define DEV_BLOCK 0x1
  #define DEV_BLOCK_SATA 0x1

#define DEV_CONNECTION_PCI 0x2

typedef uint16_t connection_t;
typedef uint16_t connection_ver_t;
typedef uint16_t dev_class_t;          // Example: DEV_BLOCK.
typedef uint16_t dev_type_t;           // Example: DEV_BLOCK_SATA.


typedef struct {
  /* PCI information */
  uint32_t vendor_id;
  uint32_t device_id;
  
  /* Information about device */
  dev_class_t device_class;
  dev_type_t device_type; 
} pci_dev_descriptor_t;


typedef struct DevDriver {
  /* Driver information */
  const char* name;
  
  /* Driver interaction */
  uint64_t(*ioctl)(uint64_t req, ...);
  void* ifaces;       // Usually allocated with kmalloc().
  size_t iface_count;

  /* Connection information */
  connection_t connection;
  void* connection_data;

  /* For internal usage by the driver manager */
  struct DevDriver* next;
} dev_driver_t;


void driver_init(void* driver_descriptor);
dev_driver_t* find_driver(dev_class_t dev_class, dev_type_t dev_type);

#endif
