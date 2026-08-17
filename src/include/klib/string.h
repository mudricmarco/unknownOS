#pragma once

#include <stdint.h>

int strcmp(const char *s1, const char *s2);

char *strcpy(char *restrict dest, const char *restrict src);

void int_to_string(int64_t n, char* str);