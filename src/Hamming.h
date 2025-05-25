#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#endif

// =============== UTILS ===============

#define HM_BEGIN_MIX                    \
    uint8_t* buf = new uint8_t[size](); \
    if (!buf) return false;

#define HM_END_MIX           \
    memcpy(pack, buf, size); \
    delete[] buf;            \
    return true;

namespace hamm {

static void mix8(uint8_t* buf, uint8_t* pack, size_t size) {
    // for (size_t i = 0; i < size; i += 8) {
    //     for (uint8_t bit = 0; bit < 8; bit++) {
    //         for (uint8_t byte = 0; byte < 8; byte++) {
    //             // if ((pack[byte + i] >> bit) & 1) buf[bit + i] |= (1 << byte);
    //             buf[bit + i] >>= 1;
    //             if (pack[byte + i] & 1) buf[bit + i] |= (1 << 7);
    //             pack[byte + i] >>= 1;
    //         }
    //     }
    // }

    uint8_t n = 8;
    for (size_t i = 0; i < size; i += 8) {
        uint8_t top = i + 8 > size ? size - i : 8;
        for (uint8_t bit = 0; bit < 8; bit++) {
            for (uint8_t byte = 0; byte < top; byte++) {
                *buf >>= 1;
                if ((pack[byte + i] >> bit) & 1) *buf |= (1 << 7);
                if (!--n) n = 8, ++buf;
            }
        }
    }
}
static void unmix8(uint8_t* buf, uint8_t* pack, size_t size) {
    // for (size_t i = 0; i < size; i += 8) {
    //     for (uint8_t bit = 0; bit < 8; bit++) {
    //         for (uint8_t byte = 0; byte < 8; byte++) {
    //             // if ((pack[bit + i] >> byte) & 1) buf[byte + i] |= (1 << bit);
    //             buf[byte + i] >>= 1;
    //             if (pack[bit + i] & 1) buf[byte + i] |= (1 << 7);
    //             pack[bit + i] >>= 1;
    //         }
    //     }
    // }

    uint8_t n = 8;
    for (size_t i = 0; i < size; i += 8) {
        uint8_t top = i + 8 > size ? size - i : 8;
        for (uint8_t bit = 0; bit < 8; bit++) {
            for (uint8_t byte = 0; byte < top; byte++) {
                if (*pack & 1) buf[byte + i] |= (1 << bit);
                *pack >>= 1;
                if (!--n) n = 8, ++pack;
            }
        }
    }
}

#ifdef __AVR__
static const uint8_t table84[] PROGMEM = {0x0, 0xF, 0x33, 0x3C, 0x55, 0x5A, 0x66, 0x69, 0x96, 0x99, 0xA5, 0xAA, 0xC3, 0xCC, 0xF0, 0xFF};

#else
static const uint8_t table84[] = {0x0, 0xF, 0x33, 0x3C, 0x55, 0x5A, 0x66, 0x69, 0x96, 0x99, 0xA5, 0xAA, 0xC3, 0xCC, 0xF0, 0xFF};
#endif

}  // namespace hamm

// =============== UTILS ===============

template <uint8_t HAM_SIZE>  // порядок алгоритма (4-7)
class HammingT {
   public:
    static constexpr size_t BLOCK_SIZE = ((1 << HAM_SIZE) >> 3);           // вес блока в байтах (2-16)
    static constexpr size_t BLOCK_SIZE_B = (1 << HAM_SIZE);                // вес блока в битах (16-128)
    static constexpr size_t BLOCK_DATA_B = BLOCK_SIZE_B - (HAM_SIZE + 1);  // битов данных на блок (11-120)

    // размер запакованных данных по размеру исходных
    static constexpr size_t encodedSize(size_t size) {
        return ((size * 8 + BLOCK_DATA_B - 1) / BLOCK_DATA_B) * BLOCK_SIZE;
    }

    // размер распакованных данных по размеру запакованных. 0 - некорректный размер пакета
    static constexpr size_t decodedSize(size_t size) {
        return (size & (BLOCK_SIZE - 1)) ? 0 : ((size / BLOCK_SIZE) * BLOCK_DATA_B) >> 3;
    }

    // Запаковать данные во внешний буфер dest размера encodedSize() [должен быть инициализирован 0]
    static void encode(void* dest, const void* src, size_t size) {
        size_t maxB = ((size * 8 + BLOCK_DATA_B - 1) / BLOCK_DATA_B) * BLOCK_SIZE_B;
        size_t bit = 0, maxBit = size * 8;

        for (size_t b = 0; b < maxB; b += BLOCK_SIZE_B) {
            uint8_t count = 0, parity = 0;

            for (uint8_t i = 0; i < BLOCK_SIZE_B; i++) {
                // 1. Заполняем дату, минуя ячейки Хэмминга (0,1,2,4,8...)
                if ((i & (i - 1)) && (bit < maxBit) && _readB(src, bit++)) {
                    ++count;
                    _setB(dest, b + i);

                    // 2. Считаем и пишем parity для зон Хэмминга. Если это ячейка хэмминга и бит стоит, инвертируем parity
                    for (uint8_t j = 0; j < HAM_SIZE; j++) {
                        uint8_t v = 1 << j;
                        if (i & v) parity ^= v;
                    }
                }
            }

            for (uint8_t i = 0; i < HAM_SIZE; i++) {
                if ((parity >> i) & 1) {
                    ++count;                    // Записываем биты Хемминга
                    _setB(dest, b + (1 << i));  // записываем parity ячеек хэмминга
                }
            }

            // 3. Пишем общий parity для блока
            if (count & 1) _setB(dest, b);
        }
    }

    // Распаковать данные в себя. Вернёт true, если распакованы без ошибок или ошибки исправлены
    static bool decode(void* src, size_t size) {
        if (size & (BLOCK_SIZE - 1)) return false;  // не кратно размеру блока

        size_t bit = 0;
        size_t maxB = (size / BLOCK_SIZE) * BLOCK_SIZE_B;

        for (size_t b = 0; b < maxB; b += BLOCK_SIZE_B) {
            // 2. Получаем хэш ошибки и общий parity
            uint8_t sum = 0, count = 0;
            for (uint8_t i = 0; i < BLOCK_SIZE_B; i++) {
                if (_readB(src, b + i)) {
                    sum ^= i;
                    if (i) ++count;
                }
            }

            // 3. Анализируем результат
            if (sum) {                                            // есть ошибки
                if (_readB(src, b) == (count & 1)) return false;  // 2 и больше ошибок
                else _toggleB(src, b + sum);                      // данные восстановлены
            }

            // 4. Собираем дату из ячеек Хэмминга
            for (uint8_t i = 0; i < BLOCK_SIZE_B; i++) {
                if (i & (i - 1)) {  // не степень двойки
                    _readB(src, b + i) ? _setB(src, bit) : _clrB(src, bit);
                    ++bit;
                }
            }
        }
        return true;
    }

    // замешать запакованные данные. false если ошибка аллокации
    static bool mix(void* pack, size_t size) {
        HM_BEGIN_MIX;
        _mix(buf, (uint8_t*)pack, size);
        HM_END_MIX;
    }

    // размешать запакованные данные. false если ошибка аллокации
    static bool unmix(void* pack, size_t size) {
        HM_BEGIN_MIX;
        _unmix(buf, (uint8_t*)pack, size);
        HM_END_MIX;
    }

    // замешать запакованные данные. false если ошибка аллокации
    static bool mix8(void* pack, size_t size) {
        HM_BEGIN_MIX;
        hamm::mix8(buf, (uint8_t*)pack, size);
        HM_END_MIX;
    }

    // размешать запакованные данные. false если ошибка аллокации
    static bool unmix8(void* pack, size_t size) {
        HM_BEGIN_MIX;
        hamm::unmix8(buf, (uint8_t*)pack, size);
        HM_END_MIX;
    }

   private:
    static void _mix(void* buf, void* pack, size_t size) {
        size_t k = 0;
        size_t top = (size / BLOCK_SIZE) * BLOCK_SIZE_B;
        for (uint8_t i = 0; i < BLOCK_SIZE_B; i++) {
            for (size_t jb = 0; jb < top; jb += BLOCK_SIZE_B) {
                if (_readB(pack, jb + i)) _setB(buf, k);
                ++k;
            }
        }
    }
    static void _unmix(void* buf, void* pack, size_t size) {
        size_t k = 0;
        size_t top = (size / BLOCK_SIZE) * BLOCK_SIZE_B;
        for (uint8_t i = 0; i < BLOCK_SIZE_B; i++) {
            for (size_t jb = 0; jb < top; jb += BLOCK_SIZE_B) {
                if (_readB(pack, k)) _setB(buf, jb + i);
                ++k;
            }
        }
    }

    static inline void _setB(void* buf, size_t b) __attribute__((always_inline)) {
        ((uint8_t*)buf)[b >> 3] |= 1 << (b & 0b111);
    }
    static inline void _clrB(void* buf, size_t b) __attribute__((always_inline)) {
        ((uint8_t*)buf)[b >> 3] &= ~(1 << (b & 0b111));
    }
    static inline bool _readB(const void* buf, size_t b) __attribute__((always_inline)) {
        return (((const uint8_t*)buf)[b >> 3] >> (b & 0b111)) & 1;
    }
    static inline void _toggleB(void* buf, size_t b) __attribute__((always_inline)) {
        _readB(buf, b) ? _clrB(buf, b) : _setB(buf, b);
    }
};

class Hamming4 : public HammingT<4> {};
class Hamming5 : public HammingT<5> {};
class Hamming6 : public HammingT<6> {};
class Hamming7 : public HammingT<7> {};

/////////////

class Hamming3 {
   public:
    static constexpr size_t BLOCK_SIZE = 1;    // вес блока в байтах
    static constexpr size_t BLOCK_SIZE_B = 8;  // вес блока в битах
    static constexpr size_t BLOCK_DATA_B = 4;  // битов данных на блок

    // размер запакованных данных по размеру исходных
    static constexpr size_t encodedSize(size_t size) {
        return size * 2;
    }

    // размер распакованных данных по размеру запакованных. 0 - некорректный размер пакета
    static constexpr size_t decodedSize(size_t size) {
        return (size & 1) ? 0 : (size >> 1);
    }

    // Запаковать данные во внешний буфер dest размера encodedSize() [должен быть инициализирован 0]
    static void encode(void* dest, const void* src, size_t size) {
        encode((uint8_t*)dest, (const uint8_t*)src, size);
    }
    static void encode(uint8_t* dest, const uint8_t* src, size_t size) {
        while (size--) {
            *dest++ = encode(*src & 0xf);
            *dest++ = encode(*src >> 4);
            ++src;
        }
    }

    // Распаковать данные в себя. Вернёт true, если распакованы без ошибок или ошибки исправлены
    static bool decode(void* src, size_t size) {
        return decode((uint8_t*)src, size);
    }
    static bool decode(uint8_t* src, size_t size) {
        if (size & 1) return false;

        uint8_t* dest = src;
        while (size) {
            int16_t bl = decode(*src++);
            int16_t bh = decode(*src++);
            if (bl < 0 || bh < 0) return false;
            *dest++ = (bh << 4) | bl;
            size -= 2;
        }
        return true;
    }

    // замешать запакованные данные. false если ошибка аллокации
    static bool mix(void* pack, size_t size) {
        if (size & 1) return false;

        HM_BEGIN_MIX;
        _mix(buf, (uint8_t*)pack, size);
        HM_END_MIX;
    }

    // размешать запакованные данные. false если ошибка аллокации
    static bool unmix(void* pack, size_t size) {
        if (size & 1) return false;

        HM_BEGIN_MIX;
        _unmix(buf, (uint8_t*)pack, size);
        HM_END_MIX;
    }

    // замешать запакованные данные. false если ошибка аллокации
    static bool mix8(void* pack, size_t size) {
        if (size & 1) return false;

        HM_BEGIN_MIX;
        hamm::mix8(buf, (uint8_t*)pack, size);
        HM_END_MIX;
    }

    // размешать запакованные данные. false если ошибка аллокации
    static bool unmix8(void* pack, size_t size) {
        if (size & 1) return false;

        HM_BEGIN_MIX;
        hamm::unmix8(buf, (uint8_t*)pack, size);
        HM_END_MIX;
    }

    // запаковать 4 бита
    static uint8_t encode(uint8_t nibble) {
#ifdef __AVR__
        return pgm_read_byte(&hamm::table84[nibble]);
#else
        return hamm::table84[nibble];
#endif
    }

    // распаковать 8 бит в 4 бита
    static int16_t decode(uint8_t data) {
        switch (data) {
            case 0x00:
            case 0x01:
            case 0x0E:
            case 0x0F:
            case 0x32:
            case 0x33:
            case 0x3C:
            case 0x3D:
            case 0x54:
            case 0x55:
            case 0x5A:
            case 0x5B:
            case 0x66:
            case 0x67:
            case 0x68:
            case 0x69:
            case 0x96:
            case 0x97:
            case 0x98:
            case 0x99:
            case 0xA4:
            case 0xA5:
            case 0xAA:
            case 0xAB:
            case 0xC2:
            case 0xC3:
            case 0xCC:
            case 0xCD:
            case 0xF0:
            case 0xF1:
            case 0xFE:
            case 0xFF:
                break;

            default:
                uint8_t sum = 0, count = 0;
                for (uint8_t i = 0; i < 8; i++) {
                    if ((data >> i) & 1) {
                        sum ^= i;
                        if (i) ++count;
                    }
                }
                if (sum) {
                    if ((data & 1) == (count & 1)) return -1;
                    else data ^= (1 << sum);
                }
        }
        return (data >> 5) | ((data >> 3) & 1);
    }

   private:
    static void _mix(void* buf, void* pack, size_t size) {
        size_t k = 0, top = size * 8;
        for (uint8_t i = 0; i < 8; i++) {
            for (size_t jb = 0; jb < top; jb += 8) {
                if (_readB(pack, jb + i)) _setB(buf, k);
                ++k;
            }
        }
    }
    static void _unmix(void* buf, void* pack, size_t size) {
        size_t k = 0, top = size * 8;
        for (uint8_t i = 0; i < 8; i++) {
            for (size_t jb = 0; jb < top; jb += 8) {
                if (_readB(pack, k)) _setB(buf, jb + i);
                ++k;
            }
        }
    }

    static inline void _setB(void* buf, size_t b) __attribute__((always_inline)) {
        ((uint8_t*)buf)[b >> 3] |= 1 << (b & 0b111);
    }

    static inline bool _readB(const void* buf, size_t b) __attribute__((always_inline)) {
        return (((const uint8_t*)buf)[b >> 3] >> (b & 0b111)) & 1;
    }
};