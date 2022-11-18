#include <dev/dev.h>
#include <mm/heap.h>

static dev_driver_t* dev_list = NULL;
static dev_driver_t* head = NULL;


dev_driver_t* find_driver(dev_class_t dev_class, dev_type_t dev_type) {
  dev_driver_t* current = dev_list;

  while (current != NULL) {
    switch (current->connection) {
      case DEV_CONNECTION_PCI:
        {
          pci_dev_descriptor_t* dev = current->connection_data;
          if (dev_type > 0) {
            // Find with both class and type.
            if (dev->device_class == dev_class && dev->device_type == dev_type)
              return current;
          } else {
            // Find with only class.
            if (dev->device_class == dev_class)
              return current;
          }
        }
        break;
    }

    current = current->next;
  }

  return NULL;
}


void driver_init(void* driver_descriptor) {
  if (dev_list == NULL) {
    dev_list = driver_descriptor;
    head = dev_list;
    head->next = NULL;
    return;
  }

  head->next = driver_descriptor;
  head = head->next;
  head->next = NULL;
}
