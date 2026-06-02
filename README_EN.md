This is an automatic translation and may be incorrect in some places. See the source README and examples for authoritative information.

[![latest](https://img.shields.io/github/v/release/GyverLibs/Hamming.svg?color=brightgreen)](https://github.com/GyverLibs/Hamming/releases/latest/download/Hamming.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/Hamming.svg)](https://registry.platformio.org/libraries/gyverlibs/Hamming)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/Hamming?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# Hamming
Library for packing and unpacking data using the Hamming algorithm (Extended Hamming)
- Algorithm order (Hamming bit count) 3-7
- Recovery of data damaged during transfer: recovery of 1 bit per block, or determination of impossibility of recovery
- Optionally mixing data for more reliable transmission - can recover several corrupted bits/bytes in a row

> Updated to v2, incompatible with the old version! The old version can be connected as`#include <HammingOld.h>`

### Compatibility
Compatible with all Arduino platforms (Arduino features are used)

## Contents
- [Documentation.](#doc)
- [Example](#example)
- [Versions](#versions)
- [Installation](#install)
- [Bugs and feedback](#feedback)

<a id="doc"></a>

## Documentation.
### Base
The new version of the library contains several classes according to the amount of redundant packet data:

| Class | Block size, byte | Block size, bit | Block data, bit | 100 byte packet size, byte |
| ------------------------------------------- | ------------- | ----------------------------------------- | |
|`Hamming3` | 1                  | 8                 | 4                   | 200                          |
| `Hamming4` | 2                  | 16                | 11                  | 146                          |
| `Hamming5` | 4                  | 32                | 26                  | 124                          |
| `Hamming6` | 8                  | 64                | 57                  | 120                          |
| `Hamming7` | 16                 | 128               | 120                 | 112                          |

The higher the class, the more:

- Larger block size (minimum package size)
- Less total packet size, because higher ratio of data bits to block size
- Longer processing and lower reliability

### benchmark
The test was conducted on the AVR ATmega328p 16MHz, time in μs.

#### Hamming4
| Package size |`encode` | `mix` | `mix8` | `decode` |
| ------------- | -------- | ----- | ------ | -------- |
| 25            | 1673     | 1557  | 486    | 2811     |
| 50            | 3315     | 3034  | 1078   | 5469     |
| 100           | 7240     | 5977  | 2185   | 10794    |

> Versions 4/5/6/7 are not much different in time

#### Hamming3
| Package size |`encode` | `mix` | `mix8` | `decode` |
| ------------- | -------- | ----- | ------ | -------- |
| 25            | 94       | 1933  | 689    | 230      |
| 50            | 185      | 3847  | 1373   | 472      |
| 100           | 366      | 7690  | 2741   | 944      |

> `Hamming3`Implemented tabularly and works many times faster than others

### Functions
```cpp
// size of packed data by size of initial
size_t encodedSize(size_t size);

// the size of the unpacked data by the size of the packaged. 0 - Incorrect package size
size_t decodedSize(size_t size);

// Mix the packed data. false if allocation error
bool mix(void* pack, size_t size);

// Stir the packed data. false if allocation error
bool unmix(void* pack, size_t size);

// Mix the packed data. false if allocation error
bool mix8(void* pack, size_t size);

// Stir the packed data. false if allocation error
bool unmix8(void* pack, size_t size);

// Pack the data into an external dest buffer of encodedSize() [must be initialized 0]
void encode(void* dest, const void* src, size_t size);

// Unpack the data into yourself. Return true if unpacked without errors or errors corrected
bool decode(void* src, size_t size);
```

### Use of use
- Take the data, get the size of the buffer (more than the data itself), encode the data into it
- Transmitted the buffer to another device
- We decoded the data. If the data is damaged during the transfer, it will be restored as far as possible.

> [!CAUTION]
> Since after encoding, the package has a size multiple of the block size in the selected order, the real size of the package will be lost - when unpacking it will be calculated mathematically. For example,`Hamming5`The data size is 10 bytes. The package size will be 16 bytes (rounded in a large direction), and when unpacking you will get 13 bytes. You need to know the actual size of the packet on the receiver side or add it to the packet itself, the library does not.

### Reliability
The algorithm can recover a single “spoiled” bit in a block. The smaller the block size, the more “noise” the packet can potentially survive transmission. The blocks follow each other, so if you damage two neighboring bits within the same block, you can not recover the data.

In mode.`Hamming3`A packet takes up the size of double data, which is like sending data twice. But if there is interference, the data will simply be spoiled, how many packets do not send. And with Hamming, you can lose a decent part of the package (12.5% in this case), and the data can be restored.

> When creating your communication protocols, it is recommended to add CRC and packet size to the package, such as in the library.[GyverWire](https://github.com/GyverLibs/GyverWire)

### Stirring
If you "mix" the packet (arrange the bits of blocks one after another), then after stirring you can recover several spoiled bits in a row, or even bytes - depending on the size of the packet, because the error will affect different blocks. The package is stirred after packaging (before shipping) and stirred back (after receiving) before unpacking. Mixing and stirring the packet is a long operation and allocates buffer-sized memory, added manually at the discretion of the user.

Two mixing algorithms are available - full`mix`/`unmix`8-byte`mix8`/`unmix8`. Complete is done for a long time and completely stretches the blocks in the package. 8-byte works in blocks of 8 bytes, the rest is kneaded by the remainder. Full mixing allows you to recover several spoiled bytes in a row (with a large packet size), and 8-bit only 1 byte for every 8 bytes. With single spoiled bits, they work the same way.

<a id="example"></a>

## Example
```cpp
#include <Hamming.h>

void setup() {
    Serial.begin(115200);
    Serial.println("=== START ===");

    // Create a date (any type)
    char str[] = "Hello, world! Hamming encoding";

    Serial.print("data len: "), Serial.println(sizeof(str));

    // size
    size_t elen = Hamming4::encodedSize(sizeof(str));

    Serial.print("pack len: "), Serial.println(elen);

    // bagboard
    uint8_t p[elen] = {};

    // packaging
    Hamming4::encode(p, str, sizeof(str));

    // mixing
    Hamming4::mix8(p, elen);
    // Hamming4::mix(p, elen);

    // ===============
    // imitation

    p[3] = 0;  // byte
    // bitWrite(p[9], 3, !bitRead(p[9], 3) //bit

    // ===============

    // stirring
    Hamming4::unmix8(p, elen);
    // Hamming4::unmix(p, elen);

    // volume
    size_t dlen = Hamming4::decodedSize(sizeof(p));

    Serial.print("decode len: "), Serial.println(dlen);

    // unpacking (in the same buffer)
    bool res = Hamming4::decode(p, sizeof(p));

    if (res) Serial.write((char*)p, dlen), Serial.println();
    else Serial.println("Data error!");

    Serial.println("==== END ====");
}
```

<a id="versions"></a>

## Versions
- v1.0
- v1.1 - a critical error has been corrected
- v1.2 - added bool pack(uint8 t *ptr, uint32 t size)
- v1.3 - critical error corrected
- v1.3.1 - minor improvements
- v2.0.0 - Strong speed and memory optimization, new tools

<a id="install"></a>

## Installation
- The library can be found by the name **Hamming** and installed through the library manager in:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Download the library](https://github.com/GyverLibs/Hamming/archive/refs/heads/main.zip).zip archive for manual installation:
    - Unpack and put in *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Unpack and put in *C:\Program Files\Arduino\libraries* (Windows x32)
    - Unpack and put in *Documents/Arduino/libraries/ *
    - (Arduino IDE) Automatic installation from .zip: *Sketch/Connect library/Add .ZIP library...* and specify downloaded archive
- Read more detailed instructions for installing libraries[here](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: new versions fix errors and bugs, as well as optimize and add new features.
- Through the library manager IDE: find the library as when installing and click "Update"
- Manually: **Delete the folder with the old version** and then put the new one in its place. “Replacement” can not be done: sometimes new versions delete files that will remain when replaced and can lead to errors!

<a id="feedback"></a>

## Bugs and feedback
If you find bugs, create **Issue**, or better write to the mail immediately.[alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
The library is open for revision and your **Pull Requests*!

When reporting bugs or incorrect work of the library, it is necessary to specify:
- Library version
- What is used by the IC
- SDK version (for ESP)
- Arduino IDE version
- Are embedded examples that use features and designs that cause bugs in your code working correctly?
- What code was downloaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed. Not a canvas of a thousand lines, but a minimum code.
