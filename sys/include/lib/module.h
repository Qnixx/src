#ifndef MODULE_H_
#define MODULE_H_


#define MODULE_NAME(name) __attribute__((unused)) static const char* const __MODULE_NAME = name;
#define MODULE_DESCRIPTION(desc) __attribute__((unused)) static const char* const __MODULE_DESCRIPTION = desc;
#define MODULE_AUTHOR(author) __attribute__((unused)) static const char* const __MODULE_AUTHOR = author;
#define MODULE_LICENSE(license) __attribute__((unused)) static const char* const __MODULE_LICENSE = license;


#endif
