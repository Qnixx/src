#include <firmware/acpi/acpi.h>
#include <firmware/acpi/def.h>
#include <lib/module.h>
#include <lib/limine.h>
#include <lib/string.h>
#include <lib/assert.h>
#include <tty/console.h>


MODULE_NAME("acpi");
MODULE_DESCRIPTION("ACPI helper module");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");


#define APIC_TYPE_IO_APIC               1
#define APIC_TYPE_INTERRUPT_OVERRIDE    2


static volatile struct limine_rsdp_request rsdp_req = {
  .id = LIMINE_RSDP_REQUEST,
  .revision = 0
};

static size_t rsdt_entry_count = 0;
static acpi_rsdt_t* rsdt = NULL;
static acpi_madt_t* madt = NULL;
void* ioapic_base = NULL;
void* lapic_base = NULL;


static void find_madt(void) {
  for (size_t i = 0; i < rsdt_entry_count; ++i) {
    acpi_header_t* cur = (acpi_header_t*)(uint64_t)rsdt->tables[i];
    if (kstrncmp(cur->signature, "APIC", 4) == 0) {
      madt = (acpi_madt_t*)cur;
      return;
    }
  }
}


static void parse_madt(void) {
  lapic_base = (void*)(uint64_t)(madt->lapic_addr);
  uint8_t* cur = (uint8_t*)(madt + 1);
  uint8_t* end = (uint8_t*)madt + madt->header.length;

  while (cur < end) {
    apic_header_t* apic_header = (apic_header_t*)cur;

    if (apic_header->type == APIC_TYPE_IO_APIC) {
      io_apic_t* ioapic = (io_apic_t*)cur;
      ioapic_base = (void*)(uint64_t)ioapic->io_apic_addr;
    }

    cur += apic_header->length;
  }
}


static uint8_t do_checksum(const acpi_header_t* hdr) {
  uint8_t sum = 0;

  for (size_t i = 0; i < hdr->length; ++i) {
    sum += ((char*)hdr)[i];
  }

  return sum % 0x100 == 0;
}


uint16_t acpi_remap_irq(uint8_t irq) {
  uint8_t* cur = (uint8_t*)(madt + 1);
  uint8_t* end = (uint8_t*)madt + madt->header.length;

  while (cur < end) {
    apic_header_t* apic_header = (apic_header_t*)cur;

    if (apic_header->type == APIC_TYPE_INTERRUPT_OVERRIDE) {
      apic_interrupt_override_t* intr_override = (apic_interrupt_override_t*)cur;

      if (intr_override->source == irq) {
        return intr_override->interrupt;
      }
    }

    cur += apic_header->length;
  }

  return irq;
}

void acpi_init(void) {
  acpi_rsdp_t* rsdp = rsdp_req.response->address;
  vnprintk("RSDP located @%x\n", rsdp);

  rsdt = (acpi_rsdt_t*)(uint64_t)rsdp->rsdtaddr;
  vnprintk("Verifying RSDT integrity..\n", 0);

  ASSERT(do_checksum(&rsdt->header), "ACPI RSDT integrity check failed! (checksum invalid)\n");
  vnprintk("RSDT integrity => OK\n", 0);

  rsdt_entry_count = (rsdt->header.length - sizeof(rsdt->header)) / 4;

  find_madt();
  ASSERT(madt != NULL, "Could not locate ACPI MADT!\n");

  vnprintk("Verifying MADT integrity..\n", 0);

  ASSERT(do_checksum(&madt->header), "ACPI MADT integrity check failed! (checksum invalid)\n");
  vnprintk("MADT integrity => OK\n", 0);

  parse_madt();
}
