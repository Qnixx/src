#ifndef MUTEX_H_
#define MUTEX_H_

#include <lib/types.h>


typedef uint8_t mutex_t;


void mutex_acquire(mutex_t* lock);
void mutex_release(mutex_t* lock);


#endif
