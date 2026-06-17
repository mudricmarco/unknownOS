#include <klib/string.h>
#include <stdint.h>
#include <stddef.h>

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void int_to_string(int64_t n, char* str) {
    int64_t i = 0;
    int64_t is_negative = 0;

    if (n == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (n < 0) {
        is_negative = 1;
    }

    do {
        int64_t digit = n % 10;
        if (digit < 0) {
            digit = -digit;
        }
        str[i++] = digit + '0';
        n /= 10;
    } while (n != 0);

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    uint64_t start = 0;
    uint64_t end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}