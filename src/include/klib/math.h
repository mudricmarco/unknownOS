#include <stdint.h>
#include <stddef.h>

// --- Conversion functions ---

static inline size_t bit_to_byte(size_t bits) {
    return bits / 8;
}

static inline size_t byte_to_bit(size_t bytes) {
    return bytes * 8;
}

static inline size_t byte_to_kibibyte(size_t bytes) {
    return bytes / 1024;
}

static inline size_t kibibyte_to_byte(size_t kibibytes) {
    return kibibytes * 1024;
}

static inline size_t byte_to_mebibyte(size_t bytes) {
    return bytes / 1024 / 1024;
}

static inline size_t mebibyte_to_byte(size_t mebibytes) {
    return mebibytes * 1024 * 1024;
}

static inline size_t div_round_up(size_t num, size_t den) {
    return (num + den - 1) / den;
}

static inline size_t bit_to_byte_ceil(size_t bits) {
    return div_round_up(bits, 8);
}

static inline size_t mebibyte_to_gibibyte(size_t mebibytes) {
    return mebibytes / 1024;
}

static inline size_t gibibyte_to_mebibyte(size_t gibibytes) {
    return gibibytes * 1024;
}

