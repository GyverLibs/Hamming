This is an automatic translation, may be incorrect in some places. See sources and examples!

# Hamming
Library for packaging and unpacking data on the Hamming algorithm (Extnded Hamming)
-the order of the algorithm (number of bits of Hamming) 3-7
- restoration of data damaged during shipping: restoration 1 bit per block, or determining the impossibility of restoring
- optionally mixing data for a more reliable transmission - can restore several spoiled bit/byte in a row

> The library is updated to version V2, incompatible with the old one!The old version can be connected as `# Include <Hammingold.h>`

## compatibility
Compatible with all arduino platforms (used arduino functions)

## Content
- [documentation] (#doc)
- [Example] (# Example)
- [versions] (#varsions)
- [installation] (# Install)
- [bugs and feedback] (#fedback)

<a id = "doc"> </a>

## Documentation
### base
The new version of the library contains several classes in terms of the number of excess package data:

|Class |Block size, byte |Block size, bit |Data in the block, bit |Package size 100 bytes, byte |
|---------- |------------------ |----------------- |------------------- |--------------------------- |
|`Hamming3` |1 |8 |4 |200 |
|`Hamming4` |2 |16 |11 |146 |
|`Hamming5` |4 |32 |26 |124 |
|`Hamming6` |8 |64 |57 |120 |
|`Hamming7` |16 |128 |120 |112 |

The higher the class, the:

- larger block size (minimum package size)
- less the final size of the package, becauseabove the ratio of data bits to the size of the block
- longer processing and below reliability

### Benchmark
The test was held on AVR Atmega328p 16MHz, time in the ISS.

### Hamming4
|Package size |`encode` |`mix` |`mix8` |`decode` |
|------------- |-------- |----- |------ |-------- |
|25 |1673 |1557 |486 |2811 |
|50 |3315 |3034 |1078 |5469 |
|100 |7240 |5977 |2185 |10794 |

> Versions 4/5/6/7 are not very different in time

### Hamming3
|Package size |`encode` |`mix` |`mix8` |`decode` |
|------------- |-------- |----- |------ |-------- |
|25 |94 |1933 |689 |230 |
|50 |185 |3847 |1373 |472 |
|100 |366 |7690 |2741 |944 |

> `Hamming3` implemented tablely and works many times faster than the rest

### functions
`` `CPP
// size of packed data in terms of source
Size_T ENCODEDSIZE (Size_T SIZE);

// The size of the unpacked data by the size of packed.0 - incorrect package size
Size_t DecededSize (Size_t Size);

// Knock the packed data.FALSE if an allocation error
Bool Mix (VOID* Pack, Size_t Size);

// Stir the packed data.FALSE if an allocation error
Bool Unmix (VOID*Pack, Size_t Size);

// Knock the packed data.FALSE if an allocation error
Bool MIX8 (VOID* PACK, SIZE_T SIZE);

// Stir the packed data.FALSE if an allocation error
Bool Unmix8 (Void* Pack, Size_t Size);

// Pull data in the external buffer of the size of the EncodeedSize () [should be initialized 0]
VOID ENCODE (VOID* Dest, const VOID* SRC, SIZE_T SIZE);

// unpack the data into yourself.Will return True if it is unpacked without errors or errors corrected
Bool decode (VOID* SRC, SIZE_T SIZE);
`` `

### Usage
- took the data, received the size of the buffer (more than the data themselves), encoded the data in it
- "handed over" the buffer to another device
- We divorced the data obtained.If the data was damaged in the process of transmission, they themselves will be restored if possible

> [! Caution]
> Since after encoding the package has a size multiple of the size of the block in the selected manner, the real size of the package will be lost - when unpacking it is calculated mathematically.For example, `Hamming5`, data size 10 bytes.The size of the package will turn out 16 bytes (widened in a large way), and when unpacking will turn out 13 bytes.The real size of the package must be known on the receiver side or add to the package itself, the library does not.

## reliability
The algorithm can restore one "spoiled" bit in the block.The smaller the size of the block, the greater the "noise" potentially can survive the package when transmitting.Blocks go one after another, so if you damage two neighboring bats within the same block, the data cannot be restored.

In `Hamming3 'mode, the package has a double data size, i.e.This is how to send data twice.But if there are interference, the data will simply be spoiled, how many packages do not send.And with Hamming you can lose a decent part of the package (12.5% ​​in this case) and the data will be restored.

> When creating their communication protocols, it is recommended to add CRC and packet size, such as in the library [gyverwire] (https://github.com/gyverlibs/gyverwire)

### Mixing
If you “mix” the package (place the blocks of the blocks in turn one after another), then after stirring you can restore several spoiled bit in a row, or even bytes, depends on the size of the package, becauseThe error will affect different blocks.The package is mixed after packaging (before sending) and stirred back (after receiving) before unpacking.Mixing and stirring the package - a long operation and selects memory the size of a buffer, is added manually at the discretion of the user.

Two mixing algorithms are available - complete `mix`/` unmix` and 8 -back `mix8`/` unmix8`.The full is performed for a long time and completely stretches the blocks on the package.8-byte works 8 byte blocks, the remainder kneads by the balance.Full mixing allows you to restore several spoiled bytes in a row (with a large package size), and 8 -bit - only 1 bytes for every 8 bytes.With single spoiled bats, they work the same way.

<a id = "Example"> </a>

## Example
`` `CPP
#include <hamming.h>

VOID setup () {
Serial.Begin (115200);
Serial.println ("=== Start ===");

// Created a date (any type)
Chard [] = "Hello, World! Hamming Encoding";

Serial.print ("Data Len:"), serial.println (SIZEOF (str));

// Size of packed data
Size_t Elen = Hamming4 :: EncodeedSize (Sizeof (Str));

Serial.print ("Pack Len:"), serial.println (Elen);

// buffer for package
uint8_t p [elen] = {};

// package
Hamming4 :: Encode (p, str, sizeof (str));

// knitting
Hamming4 :: mix8 (p, elen);
// hamming4 :: mix (p, elen);

// ======== Transmission ===============
// imitation of data damage

P [3] = 0;// Bayt
// bitwrite (p [9], 3,! Bitread (p [9], 3));// bit

// ======== Transmission ===============

// Stiring
Hamming4 :: unmix8 (P, Elen);
// hamming4 :: unmix (p, elen);

// Size of unpacked data
Size_t Dlen = Hamming4 :: DecDedsize (Sizeof (P));

Serial.print ("Decode Len:"), serial.println (Dlen);

// unpacking (in the same buffer)
Bool rES = Hamming4 :: Decode (p, sizeof (p));

if (res) serial.write ((char*) p, dlen), serial.println ();
Else serial.println ("Data Error!");

Serial.println ("==== end ====");
}
`` `

<a ID = "Versions"> </a>

## versions
- V1.0
- V1.1 - Critical error is fixed
- V1.2 - Added Bool Pack (Uint8_t *PTR, Uint32_T SIZE)
- V1.3 - Critical error is fixed
- v1.3.1 - minor improvements
- v2.0.0 - strong optimization of speed and memory, new tools

<a id = "Install"> </a>

## Installation
- The library can be found by the name ** Hamming ** and installed through the library manager in:
- Arduino ide
- Arduino ide v2
- Platformio
- [download the library] (https://github.com/gyverlibs/hamming/archive/refs/heads/main.zip). Zip archive for manual installation:
- unpack and put in * C: \ Program Files (X86) \ Arduino \ Libraries * (Windows X64)
- unpack and put in * C: \ Program Files \ Arduino \ Libraries * (Windows X32)
- unpack and put in *documents/arduino/libraries/ *
- (Arduino id) Automatic installation from. Zip: * sketch/connect the library/add .Zip library ... * and specify downloaded archive
- Read more detailed instructions for installing libraries[here] (https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: errors and bugs are corrected in the new versions, as well as optimization and new features are added
- through the IDE library manager: find the library how to install and click "update"
- Manually: ** remove the folder with the old version **, and then put a new one in its place.“Replacement” cannot be done: sometimes in new versions, files that remain when replacing are deleted and can lead to errors!

<a id = "Feedback"> </a>

## bugs and feedback
Create ** Issue ** when you find the bugs, and better immediately write to the mail [alex@alexgyver.ru] (mailto: alex@alexgyver.ru)
The library is open for refinement and your ** pull Request ** 'ow!

When reporting about bugs or incorrect work of the library, it is necessary to indicate:
- The version of the library
- What is MK used
- SDK version (for ESP)
- version of Arduino ide
- whether the built -in examples work correctly, in which the functions and designs are used, leading to a bug in your code
- what code has been loaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed.Not a canvas of a thousand lines, but a minimum code