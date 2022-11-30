
#include <lib/elf.h>
#include <mm/heap.h>
#include <mm/mmap.h>

void* elf_load(const char* file_data) {
  const char* const ORIG_FILE_PTR = file_data;
  const char* ptr = ORIG_FILE_PTR;

  uint8_t is_magic_correct =
         ptr[EI_MAG0] == ELFMAG0
      && ptr[EI_MAG1] == ELFMAG1
      && ptr[EI_MAG2] == ELFMAG2
      && ptr[EI_MAG3] == ELFMAG3;

  if (!(is_magic_correct)) {
    return NULL;
  }

  // Get the header.
  Elf64_Ehdr* eh = (Elf64_Ehdr*)ptr;

  // Allocate memory for the program headers.
  size_t phdrs_size = eh->e_phnum * eh->e_phentsize;
  Elf64_Phdr* phdrs = kmalloc(phdrs_size);
  ptr = ORIG_FILE_PTR + eh->e_phoff;

  // Copy program headers.
  for (uint64_t i = 0; i < phdrs_size; ++i) {
    ((char*)phdrs)[i] = ptr[i];
  }

  for (Elf64_Phdr* phdr = phdrs; (char*)phdr < (char*)phdrs + eh->e_phnum * eh->e_phentsize; phdr = (Elf64_Phdr*)((char*)phdr + eh->e_phentsize)) {
    if (phdr->p_type == PT_LOAD) {
      size_t n_pages = (phdr->p_memsz + 0x1000 - 1);
      Elf64_Addr segment = phdr->p_vaddr;
      ptr = ORIG_FILE_PTR + phdr->p_offset;

      // Map the segment.
      k_mmap((void*)segment, n_pages, PROT_READ | PROT_WRITE | PROT_USER);

      // Copy the data.
      for (uint64_t i = 0; i < phdr->p_filesz; ++i) {
        ((char*)segment)[i] = ptr[i];
      }
    }
  }

  return ((__attribute__((sysv_abi))void(*)(void))eh->e_entry);
}
