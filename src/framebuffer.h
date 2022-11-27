#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include <efi.h>
#include <efilib.h>


struct GopInfo {
  VOID* Base;
  UINTN BufferSize;
  UINTN Width;
  UINTN Height;
  UINTN PixelsPerScanLine;
};


UINT8 FramebufferInit(struct GopInfo* GopInfo);


#endif
