#ifndef BLOCK_DISK_H_
#define BLOCK_DISK_H_

#include <lib/types.h>
#include <dev/dev.h>


void disk_read_lba(dev_driver_t* driver_desc, uint64_t lba, uint32_t sector_count, uint16_t* buf);
void disk_write_lba(dev_driver_t* driver_desc, uint64_t lba, uint32_t sector_count, uint16_t* buf);

#endif
