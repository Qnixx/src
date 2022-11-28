#include <arch/x86/tss.h>
#include <lib/string.h>
#include <mm/vmm.h>
#include <mm/heap.h>

static tss_entry_t* tss;

extern void load_tss(void);

void tss_init(void) {
  extern tss_desc_t* gdt_tss;

  tss = kmalloc(sizeof(tss_entry_t));
  kmemzero(tss, sizeof(tss_entry_t));

  uint64_t stack = (uint64_t)kmalloc(PAGE_SIZE) + (PAGE_SIZE - 1);
  uint64_t tss_base = (uint64_t)tss;

  tss->rsp0Low = (stack & 0xFFFFFF);
  tss->rsp0High = (stack >> 32);

  gdt_tss->seglimit = sizeof(tss_entry_t);
  gdt_tss->g = 0;

  gdt_tss->baseAddrLow = tss_base & 0xFFFF;
  gdt_tss->baseAddrMiddle = (tss_base >> 16) & 0xFF;
  gdt_tss->baseAddrOtherMiddle = (tss_base >> 24) & 0xFF;
  gdt_tss->baseAddrUpper = tss_base >> 32;
  gdt_tss->avl = 0;                                           // 0 for TSS.
  gdt_tss->dpl = 0;
  gdt_tss->p = 1;
  gdt_tss->reserved = 0;
  gdt_tss->type = 0x9;
  gdt_tss->zero = 0;

  load_tss();
}
