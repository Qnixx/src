#include <efi.h>
#include <efilib.h>


EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {
  InitializeLib(imageHandle, systemTable);
  Print(L"YOOO!\n");
  __asm__ __volatile__("cli; hlt");
}
