#ifndef MATH_H_
#define MATH_H_

#define ALIGN_DOWN(address, align)  ((address) & ~((align)-1))
#define ALIGN_UP(address, align)    (((address) + (align)-1) & ~((align)-1))
#define BIG_ENDIAN(v) ((((v) & 0xFF) << 8) | (((v) >> 8) & 0xFF))

#endif
