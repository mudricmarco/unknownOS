#pragma once

#include <stdint.h>
#include <stddef.h>

int strcmp(const char *s1, const char *s2);

char *strcpy(char *restrict dest, const char *restrict src);
char *strncpy(char *dest, const char *src, size_t n);

void int_to_string(int64_t n, char* str);