This is an automatic translation, may be incorrect in some places. See sources and examples!

# Hamming
Library for packaging and unpacking data on the Hamming algorithm (excess data for recovery)
- the order of the algorithm 4-7 (the higher, the more reliable, but longer processing)
- restoration of data damaged during shipping
- accepts any type of data

## compatibility
Compatible with all arduino platforms (used arduino functions)

## Content
- [installation] (# Install)
- [initialization] (#init)
- [use] (#usage)
- [Example] (# Example)
- [versions] (#varsions)
- [bugs and feedback] (#fedback)

<a id="install"> </a>
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
- Read more detailed instructions for installing libraries [here] (https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%BD%D0%BE%BE%BE%BED0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: errors and bugs are corrected in the new versions, as well as optimization and new features are added
- through the IDE library manager: find the library how to install and click "update"
- Manually: ** remove the folder with the old version **, and then put a new one in its place.“Replacement” cannot be done: sometimes in new versions, files that remain when replacing are deleted and can lead to errors!


<a id="init"> </a>
## initialization
`` `CPP
Hamming <5> buf;// <> - code order (4-7)
`` `

<a id="usage"> </a>
## Usage
`` `CPP
// Pull data of any type to the buffer, returns the status of the operation
Uint8_t Pack (T & Data);
uint8_t pack (uint8_t *ptr, uint32_t size);

// unpack data from the buffer, returns the status of the operation
uint8_t unpack (uint8_t* data, uint32_t size);

uint8_t status ();// returns the status of the last operation
uint32_t Length ();// buffer size (more than the size of the input data)
VOID Stop ();// Free the buffer
uint8_t *buffer;// Inner buffer

// Operation Statuses (Pack/Unpack):
// 0 - ok
// 1 - Fixed errors (unpack)
// 2 - there are incorrect errors (unpack)
// 3 - Parity Error (Unpack)
// 4 - broken package (unpack)
// 5 - Boofer allocation error
`` `

<a id="EXAMPLE"> </a>
## Example
The rest of the examples look at ** Examples **!
`` `CPP
#include <Hamming.h>
VOID setup () {
  Serial.Begin (9600);

  // Created a date (any type)
  Char Data0 [] = "Hello, World! Lorem IPSUM";

  // packed
  Hamming <5> buf;// <> - code order (4-7)
  // Pack in the internal buffer buf.buffer
  buf.pack (DATA0);// 12MS
  
  // packed data are stored in BUF.Buffer with the size of buf.length ()
  // you can "send them"
  
  // ======== "Transfer" ============

  // about@Bali part (twoCranberries byt!)
  buf.buffer [5] = 0;
  buf.buffer [6] = 0;
  
  // ======== "Reception" ===========

  // unpacking.The order in <> is the same!
  Hamming <5> buf2;
  
  // We transmit "accepted" packed data and their length (it also needs to be transferred or know)
  buf2.unpack (buf.buffer, buf.length ());// 6MS
  
  // Display as a line
  Serial.println ((char*) buf2.buffer);
  
  // Display the Status of unpacking
  Serial.println (buf2.status ());
}

VOID loop () {
}
`` `

<a id="versions"> </a>
## versions
- V1.0
- V1.1 - Critical error is fixed
- V1.2 - Added Bool Pack (Uint8_t *PTR, Uint32_T SIZE)
- V1.3 - Critical error is fixed
- v1.3.1 - minor improvements

<a id="feedback"> </a>
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