#include <Hamming.h>
#include "CustomDebug.h"

#define LEVEL 5

void printData(uint8_t *buffer, uint8_t len) {  
  for (int i = 0; i < len; ) {    
    uint32_t tmp = buffer[i++];
    if (i < len) tmp |= ((uint32_t)buffer[i++]) << 8;
    if (i < len) tmp |=  ((uint32_t)buffer[i++]) << 16;
    if (i < len) tmp |=  ((uint32_t)buffer[i++]) << 24;
    debughex(tmp);
    if (i < len) debug1(", ");
  }
  debugln1(" ");
}

void lightOn() {
    digitalWrite(LED_BUILTIN, HIGH);
}

void lightOff() {
    digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(5000);
  lightOn();

  Hamming<LEVEL> packer;   //упаковщик, <> - порядок кода (4-7)
  Hamming<LEVEL> unpacker; //распаковщик

  debugln1(" ============================ ");
  debugln1(" ======== START TEST ======== ");
  debugln1(" ============================ ");

  // создали дату (любой тип)
  uint8_t data0[] = "Hello, world! Lorem Ipsum";
  debugln2("Test data is string", (char*)data0);

  // пакуем во внутренний буфер buf.buffer
  long start = micros();
  packer.pack(data0);  // 12мс
  long elapsed =  micros() - start;
  
  debugln3("Packed data elapsed ",  elapsed, " us");
  //Копируем перед изменением, чтобы вывести в конце
  uint8_t packed[packer.length()];
  memcpy(packed, packer.buffer,  packer.length());

  // запакованные данные хранятся в buf.buffer с размером buf.length()
  // можно их "отправить"
  
  // ======== "ПЕРЕДАЧА" ========

  // потеряли часть (два байта!)
  packer.buffer[5] = 0;
  packer.buffer[6] = 0;
    
  // ======== "ПРИЁМ" ========
  
  // передаём "принятые" запакованные данные и их длину (её тоже надо передать или знать)
  start = micros();
  unpacker.unpack(packer.buffer, packer.length());  // 6мс
  elapsed =  micros() - start;

  debugln5("unpacked data status  ",  unpacker.status(), " elapsed ",  elapsed, " us");
  debugln5("string  ", (char*)unpacker.buffer, "with size ", unpacker.length()," bytes");  

  //Выводим данные в HEX
  debug3("packed    ",  sizeof(packed), " :");
  printData(packed, sizeof(packed));
  debug3("corrupted ",  packer.length(), " :");
  printData(packer.buffer, packer.length());
  debug3("unpacked  ",  unpacker.length(), " :");
  printData(unpacker.buffer, unpacker.length());  
  debug3("test data ",  sizeof(data0), " :");
  printData(data0, sizeof(data0));  

  if (unpacker.length() != sizeof(data0)) {    
    debugln4(" ORIGINAL SIZE ",  sizeof(data0), " UNPACKED SIZE ", unpacker.length());
  }
  String actual = (char*)unpacker.buffer;
  if (String((char*)data0).compareTo(actual) != 0) {
    debugln1(" ERROR - UNPACKED DATA IS NOT EQUAL ORIGINAL"); 
  }
  lightOff();
}

void loop() {
  while(true)  {  
    lightOn();
    delay(500);
    lightOff();
    delay(1000);
  } 
}