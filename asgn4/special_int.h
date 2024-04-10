#ifndef GIVEN_H
#define GIVEN_H

#include <stdint.h>
#include <arpa/inet.h>

uint32_t extract_special_int(char *where, int len);

int insert_special_int(char *where, size_t size, int32_t val);

#endif
