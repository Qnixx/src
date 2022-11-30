#include <fs/initrd.h>
#include <lib/assert.h>
#include <lib/bootmodule.h>
#include <lib/string.h>
#include <lib/limine.h>

struct TarHeader {
  char filename[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag[1];
};

static const char* initrd = NULL;

static size_t _getsize(const char* in) {
  size_t size = 0;
  size_t j;
  size_t count = 1;

  for (j = 11; j > 0; --j, count *= 8) {
    size += ((in[j-1]-'0')*count);
  }

  return size;
}

static void _init(void) {
  initrd = get_module("/Ignis/initrd.sys")->address;
  ASSERT(initrd != NULL, "Could not find initrd.sys!\n");
}

const char* initrd_open(const char* path) {
  if (initrd == NULL) {
    _init();
  }

  if (kstrlen(path) > 99) {
    return NULL;
  }

  uint64_t addr = (uint64_t)initrd + 0x200;
  struct TarHeader* header = (struct TarHeader*)addr;

  while (header->filename[0] != '\0') {
    header = (struct TarHeader*)addr;
    size_t sz = _getsize(header->size);

    if (kstrcmp(header->filename, path) == 0) {
      return ((char*)addr) + 0x200;
    }

    addr += (((sz + 511) / 512) + 1) * 512;
  }

  return NULL;
}
