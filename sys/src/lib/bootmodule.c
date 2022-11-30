#include <lib/string.h>
#include <lib/limine.h>

static volatile struct limine_module_request mod_req = {
  .id = LIMINE_MODULE_REQUEST,
  .revision = 0
};


struct limine_file* get_module(const char* path) {
  for (uint64_t i = 0; i < mod_req.response->module_count; ++i) {
    if (kstrcmp(mod_req.response->modules[i]->path, path) == 0) {
      return mod_req.response->modules[i];
    }
  }

  return NULL;
}
