#include <stdint.h>
#include <stddef.h>
#include <klib/math.h>

//? I should probably refactor this to use a more efficient algorithm, but for now, this is a simple and straightforward implementation.
char* int_to_hex_string(uint64_t value, char* buffer) {
    const char hex_chars[] = "0123456789ABCDEF";
    size_t i = 0;

    buffer[i++] = '0';
    buffer[i++] = 'x';

    if (value == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return buffer;
    }

    size_t start_digits = i;

    while (value > 0) {
        buffer[i++] = hex_chars[value % 16];
        value /= 16;
    }

    buffer[i] = '\0';

    size_t start = start_digits;
    size_t end = i - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }

    return buffer;
}