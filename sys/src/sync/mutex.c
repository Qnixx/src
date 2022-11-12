#include <sync/mutex.h>


void mutex_acquire(mutex_t* lock) {
  while (__atomic_test_and_set(lock, __ATOMIC_ACQUIRE));
}


void mutex_release(mutex_t* lock) {
  __atomic_clear(lock, __ATOMIC_RELEASE);
}
