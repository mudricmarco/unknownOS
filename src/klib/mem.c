// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// They must be implemented as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!

// I took this implementations from the internet, so I don't know if they are the best one, but it works and is fast enough for now.
// No need to reinvent the wheel :)

#include <stdint.h>
#include <stddef.h>
#include <klib/mem.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict d = dest;
    const uint8_t *restrict s = src;

    if (!n) return dest;

    d[0] = s[0];
    d[n-1] = s[n-1];
    if (n <= 2) return dest;
    
    d[1] = s[1];
    d[n-2] = s[n-2];
    d[2] = s[2];
    d[n-3] = s[n-3];
    if (n <= 6) return dest;
    
    d[3] = s[3];
    d[n-4] = s[n-4];
    if (n <= 8) return dest;
    
    d[4] = s[4];
    d[n-5] = s[n-5];
    d[5] = s[5];
    d[n-6] = s[n-6];
    d[6] = s[6];
    d[n-7] = s[n-7];
    d[7] = s[7];
    d[n-8] = s[n-8];
    if (n <= 16) return dest;

    size_t k = -(uintptr_t)d & 7;
    for (size_t i = 0; i < k; i++) {
        d[i] = s[i];
    }
    d += k;
    s += k;
    n -= k;

    n &= -8;
    size_t total_words = n / 8;

    uint64_t *restrict d64 = (uint64_t *restrict)d;
    const uint64_t *restrict s64 = (const uint64_t *restrict)s;

    size_t i = 0;
    while (total_words >= 4) {
        d64[i]     = s64[i];
        d64[i + 1] = s64[i + 1];
        d64[i + 2] = s64[i + 2];
        d64[i + 3] = s64[i + 3];
        i += 4;
        total_words -= 4;
    }

    while (total_words > 0) {
        d64[i] = s64[i];
        i++;
        total_words--;
    }

    return dest;
}

void *memset(void *dest, int c, size_t n) {
    unsigned char *s = dest;
    size_t k;

    if (!n) return dest;
    
    s[0] = s[n-1] = c;
    if (n <= 2) return dest;
    s[1] = s[n-2] = c;
    s[2] = s[n-3] = c;
    if (n <= 6) return dest;
    s[3] = s[n-4] = c;
    if (n <= 8) return dest;
    s[4] = s[n-5] = c;
    s[5] = s[n-6] = c;
    s[6] = s[n-7] = c;
    s[7] = s[n-8] = c;
    if (n <= 16) return dest;

    k = -(uintptr_t)s & 7;
    s += k;
    n -= k;
    
    n &= -8;
    n /= 8;

    uint64_t *ws = (uint64_t *)s;
    uint64_t wc = c & 0xFF;
    wc |= (wc << 8) | (wc << 16) | (wc << 24);
    wc |= (wc << 32);

    while (n >= 4) {
        ws[0] = wc;
        ws[1] = wc;
        ws[2] = wc;
        ws[3] = wc;
        ws += 4;
        n -= 4;
    }

    while (n > 0) {
        *ws++ = wc;
        n--;
    }

    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *d = dest;
    const uint8_t *s = src;

    if (!n || d == s) return dest;

    if (d < s || d >= s + n) {
        return memcpy(dest, src, n);
    }

    d[0] = s[0];
    d[n-1] = s[n-1];
    if (n <= 2) return dest;

    d[1] = s[1];
    d[n-2] = s[n-2];
    d[2] = s[2];
    d[n-3] = s[n-3];
    if (n <= 6) return dest;

    d[3] = s[3];
    d[n-4] = s[n-4];
    if (n <= 8) return dest;

    d[4] = s[4]; d[n-5] = s[n-5];
    d[5] = s[5]; d[n-6] = s[n-6];
    d[6] = s[6]; d[n-7] = s[n-7];
    d[7] = s[7]; d[n-8] = s[n-8];
    if (n <= 16) return dest;

    d += n;
    s += n;

    size_t k = (uintptr_t)d & 7;
    for (size_t i = 0; i < k; i++) {
        d--;
        s--;
        *d = *s;
    }
    n -= k;

    n &= -8;
    size_t total_words = n / 8;

    uint64_t *restrict d64 = (uint64_t *restrict)d;
    const uint64_t *restrict s64 = (const uint64_t *restrict)s;

    while (total_words >= 4) {
        d64 -= 4;
        s64 -= 4;
        d64[3] = s64[3];
        d64[2] = s64[2];
        d64[1] = s64[1];
        d64[0] = s64[0];
        total_words -= 4;
    }

    while (total_words > 0) {
        d64--;
        s64--;
        *d64 = *s64;
        total_words--;
    }

    return dest;
}

int memcmp(const void *p1, const void *p2, size_t n)
{
    size_t i;

    if (p1 == p2)
    {
        return 0;
    }

    for (i = 0; (i < n) && (*(uint8_t *)p1 == *(uint8_t *)p2);
        i++, p1 = 1 + (uint8_t *)p1, p2 = 1 + (uint8_t *)p2);
        
    return (i == n) ? 0 : (*(uint8_t *)p1 - *(uint8_t *)p2);
}
