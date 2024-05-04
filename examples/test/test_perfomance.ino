#include "Hamming.h"
#include "printf.h"

#define LEVEL 5

void setup() {
  Serial.begin(9600);
  printf_begin();

  Hamming<LEVEL> packer;   //упаковщик, <> - порядок кода (4-7)
  Hamming<LEVEL> unpacker; //распаковщик

  printf("\n ============================ \n");
  printf(" ======== START TEST ======== \n");
  printf(" ============================ \n");

  // создали дату (любой тип)
  char data0[] = "Hello, world! Lorem Ipsum";
  printf("Test data is string \"%s\"\n", data0);

  // пакуем во внутренний буфер buf.buffer
  long start = micros();
  packer.pack(data0);  // 12мс
  long elapsed =  micros() - start;
  
  printf("Packed data elapsed %d us\n", elapsed);
  //Копируем перед изменением, чтобы вывести в конце
  char packed[packer.length()];
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
  printf("unpacked data status %d elapsed %d us, ",  unpacker.status(), elapsed);
  printf("string \"%s\" with size %d bytes\n", (char*)unpacker.buffer, unpacker.length());  

  //Выводим данные в HEX
  printf("packed    [%d]: ",  sizeof(packed));
  printData(packed, sizeof(packed));
  printf("corrupted [%d]: ", packer.length());
  printData(packer.buffer, packer.length());
  printf("unpacked  [%d]: ", unpacker.length());
  printData(unpacker.buffer, unpacker.length());  
  printf("test data [%d]: ", sizeof(data0));
  printData(data0, sizeof(data0));  

  if (unpacker.length() != sizeof(data0)) {    
    printf(" ERROR - ORIGINAL SIZE %d, UNPACKED SIZE %d \n", sizeof(data0), unpacker.length());  
  }
  String actual = (char*)unpacker.buffer;
  if (String(data0).compareTo(actual) != 0) {
    printf(" ERROR - UNPACKED DATA IS NOT EQUAL ORIGINAL");  
  }
}

void loop() {
}


void printData(uint8_t *buffer, uint8_t len) {  
  for (int i = 0; i < len; ) {    
    uint32_t tmp = buffer[i++];
    if (i < len) tmp |= ((uint32_t)buffer[i++]) << 8;
    if (i < len) tmp |=  ((uint32_t)buffer[i++]) << 16;
    if (i < len) tmp |=  ((uint32_t)buffer[i++]) << 24;
    printf("0x%.8lX", tmp);
    if (i < len) printf(", ");
  }
  printf("\n");
}
