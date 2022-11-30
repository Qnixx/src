#ifndef BOOTMODULE_H_
#define BOOTMODULE_H_

#include <lib/limine.h>

struct limine_file* get_module(const char* path);

#endif // BOOTMODULE_H_
