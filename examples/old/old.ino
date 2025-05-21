#include <HammingOld.h>

void setup() {
    Serial.begin(9600);

    // создали дату (любой тип)
    char data0[] = "Hello, world! Lorem Ipsum";

    // запаковали
    Hamming<5> buf;  // <> - порядок кода (4-7)
    // пакуем во внутренний буфер buf.buffer
    buf.pack(data0);  // 12мс

    // запакованные данные хранятся в buf.buffer с размером buf.length()
    // можно их "отправить"

    // ======== "ПЕРЕДАЧА" ========

    // про@бали часть (два байта!)
    buf.buffer[5] = 0;
    buf.buffer[6] = 0;

    // ======== "ПРИЁМ" ========

    // распаковываем. Порядок в <> такой же!
    Hamming<5> buf2;

    // передаём "принятые" запакованные данные и их длину (её тоже надо передать или знать)
    buf2.unpack(buf.buffer, buf.length());  // 6мс

    // выводим как строку
    Serial.println((char*)buf2.buffer);

    // выводим статус распаковки
    Serial.println(buf2.status());
}

void loop() {
}
