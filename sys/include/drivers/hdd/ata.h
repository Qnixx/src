#ifndef HDD_ATA_H_
#define HDD_ATA_H_

#include <lib/types.h>


void ata_init(void);
void ata_read(uint16_t* out_buf, uint32_t lba, uint16_t sector_count);
void ata_write(uint16_t* in_buf, uint32_t lba, uint16_t sector_count);


#endif
