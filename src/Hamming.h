/*
    Библиотека для упаковки и распаковки данных по алгоритму Хэмминга (избыточные данные для восстановления)
    Документация: 
    GitHub: https://github.com/GyverLibs/Hamming
    Возможности:
    - Порядок алгоритма 4-8 (чем выше, тем надёжнее)
    - Восстановление данных, повреждённых при пересылке
    - Принимает любой тип данных
    
    AlexGyver, alex@alexgyver.ru
    https://alexgyver.ru/
    MIT License

    Версии:
    v1.0 - релиз
    v1.1 - исправлена критическая ошибка
    v1.2 - добавлена bool pack(uint8_t *ptr, uint32_t size)
    v1.3 - исправлена критическая ошибка
*/

#ifndef _Hamming_h
#define _Hamming_h
template <uint8_t HAM_SIZE = 5>        // порядок алгоритма (4-8)
class Hamming {
public:
    // запаковать данные в буфер
    template <typename T>
    bool pack(T &data) {
        return pack((uint8_t*)&data, (uint32_t)sizeof(T));
    }
    
    bool pack(uint8_t *ptr, uint32_t size) {
        // 0. Считаем и создаём буфер
        uint8_t signif = chunkSizeB - (HAM_SIZE + 1);       // битов даты на чанк
        chunkAmount = (size * 8ul + signif - 1) / signif;   // колво чанков (целоч. деление)
        bytes = chunkAmount * chunkSize;                    // размер буфера, байт
        if (buffer) free(buffer);                           // освобождаем старый
        buffer = (uint8_t*)malloc(bytes);                   // выделяем
        if (!buffer) return 0;                              // не удалось создать
        uint8_t buf[bytes];                                 // ещё буфер
        memset(buf, 0, bytes);                              // чисти чисти
        memset(buffer, 0, bytes);                           // чисти чисти
        int ptrCount = 0;
        
        for (int chunk = 0; chunk < chunkAmount; chunk++) { // каждый чанк
            // 1. Заполняем дату, минуя ячейки Хэмминга (0,1,2,4,8...)
            for (uint8_t i = 0; i < chunkSizeB; i++) {
                if ((i & (i - 1)) != 0) {                   // проверка на степень двойки
                    write(buf, chunk * chunkSizeB + i, read(ptr, ptrCount++));  // переписываем побитно
                }
            }

            // 2. Считаем и пишем parity для зон Хэмминга
            uint8_t parityH = 0;
            for (uint8_t i = 0; i < chunkSizeB; i++) {
                for (uint8_t j = 0; j < HAM_SIZE; j++) {
                    // если это ячейка хэмминга и бит стоит, инвертируем текущий parity
                    if ((i & (1 << j)) && read(buf, chunk * chunkSizeB + i)) parityH ^= (1 << j);
                }
            }
            for (uint8_t i = 0; i < HAM_SIZE; i++) {
                write(buf, chunk * chunkSizeB + (1 << i), (parityH >> i) & 1); // переписываем parity ячеек хэмминга
            }

            // 3. Считаем и пишем общий parity
            uint8_t count = 0;
            for (uint8_t i = 1; i < chunkSizeB; i++) {
                if (read(buf, chunk * chunkSizeB + i)) count++; // считаем
            }
            write(buf, chunk * chunkSizeB, count & 1);          // пишем
        }

        // 4. Перемешиваем
        uint32_t k = 0;
        for (uint8_t i = 0; i < chunkSizeB; i++) {
            for (uint8_t j = 0; j < chunkAmount; j++) {
                write(buffer, k++, read(buf, i + j * chunkSizeB));
            }
        }
        return 1;
    }
    // распаковать данные
    // возврат: 0 ОК, 1 исправлены ошибки, 2 и 3 - есть неисправленные ошибки, 4 - битый пакет, 5 - не удалось аллоцировать буфер
    uint32_t unpack(uint8_t* data, uint32_t size) {
        // 0. Считаем и создаём буфер
        if ((size & (chunkSize - 1)) != 0) return stat = 4;    // не кратно размеру чанка
        uint8_t signif = chunkSizeB - (HAM_SIZE + 1);   // битов даты на чанк
        chunkAmount = (uint32_t)size / chunkSize;       // колво чанков
        bytes = chunkAmount * signif / 8;               // размер буфера, байт (округл. вниз)
        if (buffer) free(buffer);                       // чисти старый
        buffer = (uint8_t*)malloc(bytes);               // выделяем
        if (!buffer) return stat = 5;                   // не удалось создать
        memset(buffer, 0, bytes);                       // чисти чисти
        uint8_t buf[size];
        int ptrCount = 0;
        stat = 0;

        // 1. Разбираем мешанину обратно
        uint32_t k = 0;
        for (uint8_t i = 0; i < chunkSizeB; i++) {
            for (uint8_t j = 0; j < chunkAmount; j++) {
                write(buf, i + j * chunkSizeB, read(data, k++));
            }
        }

        for (int chunk = 0; chunk < chunkAmount; chunk++) {   // каждый чанк
            // 2. Получаем хэш ошибки и общий parity
            uint8_t sum = 0, count = 0;
            for (uint8_t i = 0; i < chunkSizeB; i++) {
                if (read(buf, chunk * chunkSizeB + i)) {
                    sum ^= i;
                    if (i > 0) count++;
                }
            }

            // 3. Анализируем результат
            if (sum != 0) {                           // 1+ err
                if (read(buf, chunk * chunkSizeB) == (count & 1)) stat = max(stat, 2);  // 2 err
                else toggle(buf, chunk * chunkSizeB + sum);         // fix err
                stat = max(stat, 1);
            } else {
                if (read(buf, chunk * chunkSizeB) != (count & 1)) stat = max(stat, 3);  // parity err
            }

            // 4. Собираем дату из ячеек Хэмминга
            for (uint8_t i = 0; i < chunkSizeB; i++) {
                if ((i & (i - 1)) != 0) {   // проверка на степень двойки
                    write(buffer, ptrCount++, read(buf, chunk * chunkSizeB + i)); // переписываем побитно
                }
            }
        }
        return stat;
    }

    // возврат: возврат: 0 ОК, 1 исправлены ошибки, 2 и 3 - есть неисправленные ошибки, 4 - битый пакет, 5 - не удалось аллоцировать буфер
    uint8_t status() {
        return stat;
    }
    
    // размер буфера (больше чем размер входных данных)
    uint32_t length() {
        return bytes;
    }
    
    // деструктор
    ~Hamming() {
        stop();
    }
    
    // освободить буфер
    void stop() {
        if (buffer) free(buffer);
    }
    
    // внутренний буфер
    uint8_t *buffer = NULL;

private:
    void set(uint8_t* buf, uint32_t num) {
        bitSet(buf[num >> 3], num & 0b111);
    }
    void clear(uint8_t* buf, uint32_t num) {
        bitClear(buf[num >> 3], num & 0b111);
    }
    void write(uint8_t* buf, uint32_t num, bool state) {
        state ? set(buf, num) : clear(buf, num);
    }
    bool read(uint8_t* buf, uint32_t num) {
        return bitRead(buf[num >> 3], num & 0b111);
    }
    void toggle(uint8_t* buf, uint32_t num) {
        read(buf, num) ? clear(buf, num) : set(buf, num);
    }
    int stat;
    uint32_t bytes = 0;
    uint32_t chunkAmount = 0;
    const uint8_t chunkSizeB = (1 << HAM_SIZE);        // вес чанка в битах
    const uint8_t chunkSize = (1 << HAM_SIZE) >> 3;    // вес чанка в байтах
};
#endif