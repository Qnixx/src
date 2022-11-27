#include <efi.h>
#include <efilib.h>
#include <framebuffer.h>
#include <elf.h>

#define KERNEL_PATH L"kernel.sys"


static struct GopInfo GopInfo;

struct QxProtocol {
};


static VOID DumpGOPInfo(VOID) {
  Print(L"GOP Base => %x\n", GopInfo.Base);
  Print(L"GOP Size => %x\n", GopInfo.BufferSize);
  Print(L"GOP Width => %x\n", GopInfo.BufferSize);
  Print(L"GOP Height => %x\n", GopInfo.BufferSize);
  Print(L"GOP PixelsPerScanLine => %x\n", GopInfo.PixelsPerScanLine);
}


static EFI_FILE_HANDLE GetVolume(EFI_HANDLE ImageHandle) {
  EFI_LOADED_IMAGE* LoadedImage = NULL;
  EFI_GUID LipGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_FILE_IO_INTERFACE* IOVolume;
  EFI_GUID FSGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_FILE_HANDLE Volume;

  // Fetch the loaded image protocol.
  uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &LipGUID, (VOID**)&LoadedImage);

  // Get the volume handle.
  uefi_call_wrapper(BS->HandleProtocol, 3, LoadedImage->DeviceHandle, &FSGuid, (VOID*)&IOVolume);
  uefi_call_wrapper(IOVolume->OpenVolume, 2, IOVolume, &Volume);
  return Volume;
}


static UINT64 GetFileSize(EFI_FILE_HANDLE FileHandle) {
  EFI_FILE_INFO* FileInfo = LibFileInfo(FileHandle);
  UINT64 Ret = FileInfo->FileSize;
  FreePool(FileInfo);
  return Ret;
}


static UINT8* LoadFile(CHAR16* Path, EFI_HANDLE ImageHandle) {
  EFI_FILE_HANDLE Volume = GetVolume(ImageHandle);

  EFI_FILE_HANDLE FileHandle;
  EFI_STATUS Status = uefi_call_wrapper(Volume->Open, 5, Volume, &FileHandle, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);

  if (Status != EFI_SUCCESS) {
    return NULL;
  }

  UINT64 ReadSize = GetFileSize(FileHandle);
  UINT8* Buffer = AllocatePool(ReadSize);

  uefi_call_wrapper(FileHandle->Read, 3, FileHandle, &ReadSize, Buffer);
  uefi_call_wrapper(FileHandle->Close, 1, FileHandle);
  return Buffer;
}

static VOID CheckELF(Elf64_Ehdr Eh) {
  uint8_t MagCheckPass =
       Eh.e_ident[EI_MAG0] == ELFMAG0
    && Eh.e_ident[EI_MAG1] == ELFMAG1
    && Eh.e_ident[EI_MAG2] == ELFMAG2
    && Eh.e_ident[EI_MAG3] == ELFMAG3;

  uint8_t HeaderCheckPass =
       Eh.e_ident[EI_CLASS] == ELFCLASS64
    && Eh.e_type == ET_EXEC
    && Eh.e_machine == EM_X86_64
    && Eh.e_version == EV_CURRENT;

  if (!(HeaderCheckPass) || !(MagCheckPass)) {
    Print(L"!!Kernel ELF Eh bad!!\n");
    asm volatile("cli; hlt");
  }

  Print(L"Integrity check passed, loading kernel..\n");
}


static UINTN EfiMapKey;
static VOID GetMemoryMap(VOID) {
  UINTN EfiMemoryMapSize = 0;
  UINTN EfiDescriptorSize;
  UINT32 EfiDescriptorVersion;
  EFI_MEMORY_DESCRIPTOR* EfiMemoryMap = NULL;

  EFI_STATUS Status = BS->GetMemoryMap(
                        &EfiMemoryMapSize,
                        EfiMemoryMap,
                        &EfiMapKey,
                        &EfiDescriptorSize,
                        &EfiDescriptorVersion
                      );

  if (Status != EFI_BUFFER_TOO_SMALL) {
    Print(L"!!Something went wrong getting the memory map!!\n");
    asm volatile("cli; hlt");
  }

  EfiMemoryMap = (EFI_MEMORY_DESCRIPTOR*)AllocatePool(EfiMemoryMapSize + 2 * EfiDescriptorSize);

  if (EfiMemoryMap == NULL) {
    Print(L"!!Something went wrong getting the memory map!!\n");
    asm volatile("cli; hlt");
  }

  Status = BS->GetMemoryMap(
                        &EfiMemoryMapSize,
                        EfiMemoryMap,
                        &EfiMapKey,
                        &EfiDescriptorSize,
                        &EfiDescriptorVersion
                      );

  if (EFI_ERROR(Status)) {
    Print(L"!!Something went wrong getting the memory map!!\n");
    asm volatile("cli; hlt");
  }
}


__attribute__((noreturn)) static VOID LoadKernel(EFI_HANDLE ImageHandle) {
  while (1);
}


EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
  InitializeLib(ImageHandle, SystemTable);
  uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

  if (FramebufferInit(&GopInfo) != 0) {
    Print(L"!!UNABLE TO INITIALIZE GRAPHICS OUTPUT PROTOCOL!!\n");
    asm volatile("cli; hlt");
  }

  Print(L"Booting..\n");
  DumpGOPInfo();
  LoadKernel(ImageHandle);

  return EFI_SUCCESS;
}
