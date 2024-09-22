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
    v1.3.1 - мелкие улучшения
*/

/*
    статусы операции (pack/unpack):
    0 - ОК
    1 - исправлены ошибки (unpack)
    2 - есть неисправленные ошибки (unpack)
    3 - parity error (unpack)
    4 - битый пакет (unpack)
    5 - ошибка аллокации буфера
*/

#ifndef _Hamming_h
#define _Hamming_h

template <uint8_t HAM_SIZE = 5>        // порядок алгоритма (4-7)
class Hamming {
public:
    // запаковать данные в буфер, возвращает статус операции
    template <typename T>
    uint8_t pack(T &data) {
        return pack((uint8_t*)&data, (uint32_t)sizeof(T));
    }
    
    uint8_t pack(uint8_t *ptr, uint32_t size) {
        // 0. Считаем и создаём буфер
        stat = 0;
        uint8_t signif = chunkSizeB - (HAM_SIZE + 1);       // битов даты на чанк
        chunkAmount = (size * 8ul + signif - 1) / signif;   // колво чанков (целоч. деление)
        bytes = chunkAmount * chunkSize;                    // размер буфера, байт
        if (buffer) free(buffer);                           // освобождаем старый
        buffer = (uint8_t*)malloc(bytes);                   // выделяем
        if (!buffer) return stat = 5;                       // не удалось создать
        uint8_t buf[bytes];                                 // ещё буфер
        memset(buf, 0, bytes);                              // чисти чисти
        memset(buffer, 0, bytes);                           // чисти чисти
        int ptrCount = 0;
        
        for (int chunk = 0; chunk < chunkAmount; chunk++) { // каждый чанк
            uint8_t countSet = 0;
            uint8_t parityH = 0;
            int chunkBegin = chunk * chunkSizeB;
            for (uint8_t i = 0; i < chunkSizeB; i++) {
                // 1. Заполняем дату, минуя ячейки Хэмминга (0,1,2,4,8...)
                bool bit = false;
                if (i & (i - 1)) {                   // проверка на степень двойки
                    bit = read(ptr, ptrCount++);
                    if (bit) {  //буффер обнулен, так что можно записывать только единицы
                        countSet++;
                        write(buf, chunkBegin + i, bit);  // записываем побитно                        
                    }
                }

                // 2. Считаем и пишем parity для зон Хэмминга
                if (bit) {
                    for (uint8_t j = 0; j < HAM_SIZE; j++) {
                        // если это ячейка хэмминга и бит стоит, инвертируем текущий parity
                        uint8_t shifted = 1 << j;
                        if (i & shifted) parityH ^= shifted;
                    }
                }
            }

            for (uint8_t i = 0; i < HAM_SIZE; i++) {
                bool value = (parityH >> i) & 1;
                if (value) {
                    //Записываем биты Хемминга
                    countSet++;
                    write(buf, chunkBegin + (1 << i), value); // записываем parity ячеек хэмминга
                }
            }

            // 3. Пишем общий parity для чанка
            bool commonParity =  countSet & 1;
            if (commonParity) {
                write(buf, chunkBegin, commonParity); // пишем
            }
        }

        // 4. Перемешиваем
        uint32_t k = 0;
        bool value = false;
        for (uint8_t i = 0; i < chunkSizeB; i++) {
            for (uint8_t j = 0; j < chunkAmount; j++) {
                value = read(buf, i + j * chunkSizeB);
                if (value) {
                    write(buffer, k, value);
                }
                k++;
            }
        }
        return stat;
    }
    
     // распаковать данные, возвращает статус операции
    uint8_t unpack(uint8_t* data, uint32_t size) {
        // 0. Считаем и создаём буфер
        stat = 0;
        if ((size & (chunkSize - 1)) != 0) return stat = 4;    // не кратно размеру чанка
        uint8_t signif = chunkSizeB - (HAM_SIZE + 1);   // битов даты на чанк
        chunkAmount = (uint32_t)size / chunkSize;       // колво чанков
        bytes = (chunkAmount * signif) / 8;               // размер буфера, байт (округл. вниз)
        if (buffer) free(buffer);                       // чисти старый
        buffer = (uint8_t*)malloc(bytes);               // выделяем
        if (!buffer) return stat = 5;                   // не удалось создать
        memset(buffer, 0, bytes);                       // чисти чисти
        uint8_t buf[size];
        memset(buf, 0, size);                           // чисти чисти
        int ptrCount = 0;
        
        // 1. Разбираем мешанину обратно
        uint32_t k = 0;
        bool value;
        for (uint8_t i = 0; i < chunkSizeB; i++) {
            for (uint8_t j = 0; j < chunkAmount; j++) {
                value = read(data, k++);
                if (value) {
                    write(buf, i + j * chunkSizeB, value);
                }
            }
        }

        for (int chunk = 0; chunk < chunkAmount; chunk++) {   // каждый чанк
            int chunkBegin = chunk * chunkSizeB;
            // 2. Получаем хэш ошибки и общий parity
            uint8_t sum = 0, count = 0;
            for (uint8_t i = 0; i < chunkSizeB; i++) {
                if (read(buf, chunkBegin + i)) {
                    sum ^= i;
                    if (i > 0) count++;
                }
            }

            // 3. Анализируем результат
            if (sum != 0) {                                                             // есть ошибки
                if (read(buf, chunkBegin) == (count & 1)) stat = max(stat, 2);  // 2 и больше ошибок
                else toggle(buf, chunkBegin + sum);                             // данные восстановлены
                stat = max(stat, 1);
            } else {
                if (read(buf, chunkBegin) != (count & 1)) stat = max(stat, 3);  // parity error
            }

            // 4. Собираем дату из ячеек Хэмминга
            for (uint8_t i = 0; i < chunkSizeB; i++) {
                if (i & (i - 1)) {   // проверка на степень двойки
                    value  = read(buf, chunkBegin + i);
                    if (value) {
                        write(buffer, ptrCount, value); // записываем побитно
                    }
                    ptrCount++;
                }
            }
        }
        return stat;
    }

    // возвращает статус последней операции
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
    inline void set(uint8_t* buf, uint32_t num) {
        bitSet(buf[num >> 3], num & 0b111);
    }
    inline void clear(uint8_t* buf, uint32_t num) {
        bitClear(buf[num >> 3], num & 0b111);
    }
    inline void write(uint8_t* buf, uint32_t num, bool state) {
        state ? set(buf, num) : clear(buf, num);
    }
    inline bool read(uint8_t* buf, uint32_t num) {
        return bitRead(buf[num >> 3], num & 0b111);
    }
    inline void toggle(uint8_t* buf, uint32_t num) {
        read(buf, num) ? clear(buf, num) : set(buf, num);
    }
    int stat;
    uint32_t bytes = 0;
    uint32_t chunkAmount = 0;
    const uint8_t chunkSizeB = (1 << HAM_SIZE);        // вес чанка в битах
    const uint8_t chunkSize = (1 << HAM_SIZE) >> 3;    // вес чанка в байтах
};
#endif
