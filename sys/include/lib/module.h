#ifndef MODULE_H_
#define MODULE_H_


#define MODULE(name) __attribute__((unused)) static const char* __MODULE_NAME = name
#define MODULE_NAME __MODULE_NAME

#endif
