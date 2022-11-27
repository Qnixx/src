#include <framebuffer.h>

UINT8 FramebufferInit(struct GopInfo* GopInfo) {
  EFI_GUID GopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop;
  EFI_STATUS Status = uefi_call_wrapper(BS->LocateProtocol, 3, &GopGuid, NULL, (VOID**)&Gop);

  if (EFI_ERROR(Status)) return 1;

  GopInfo->Base = (VOID*)Gop->Mode->FrameBufferBase;
  GopInfo->BufferSize = Gop->Mode->FrameBufferSize;
  GopInfo->Width = Gop->Mode->Info->HorizontalResolution;
  GopInfo->Height = Gop->Mode->Info->VerticalResolution;
  GopInfo->PixelsPerScanLine = Gop->Mode->Info->PixelsPerScanLine;
  return 0;
}
