#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_


#define PROT_READ   (1 << 0)
#define PROT_WRITE  (1 << 1)
#define PROT_EXEC   (1 << 2)

#ifdef _KERNEL
#define PROT_USER (1 << 3)      /* User access allowed */
#endif


#endif
