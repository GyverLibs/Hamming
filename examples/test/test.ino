#include <Arduino.h>
#include <Hamming.h>

void setup() {
    Serial.begin(115200);
    Serial.println("=== START ===");

    // создали дату (любой тип)
    char str[] = "Hello, world! Hamming encoding";

    Serial.print("data len: "), Serial.println(sizeof(str));

    // размер запакованных данных
    size_t elen = Hamming4::encodedSize(sizeof(str));

    Serial.print("pack len: "), Serial.println(elen);

    // буфер для пакета
    uint8_t p[elen] = {};

    // упаковка
    Hamming4::encode(p, str, sizeof(str));

    // замешивание
    Hamming4::mix8(p, elen);
    // Hamming4::mix(p, elen);

    // ======== ПЕРЕДАЧА ========
    // имитация порчи данных

    p[3] = 0;  // байт
    // bitWrite(p[9], 3, !bitRead(p[9], 3));  // бит

    // ======== ПЕРЕДАЧА ========

    // размешивание
    Hamming4::unmix8(p, elen);
    // Hamming4::unmix(p, elen);

    // размер распакованных данных
    size_t dlen = Hamming4::decodedSize(sizeof(p));

    Serial.print("unpack len: "), Serial.println(dlen);

    // распаковка (в этот же буфер)
    bool res = Hamming4::decode(p, sizeof(p));

    if (res) Serial.write((char*)p, dlen), Serial.println();
    else Serial.println("Data error!");

    Serial.println("==== END ====");
}

void loop() {
}