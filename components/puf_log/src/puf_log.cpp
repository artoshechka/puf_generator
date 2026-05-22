/// @file puf_log.cpp
/// @brief Кольцевой буфер логов с перехватом через esp_log_set_vprintf.

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <puf_log.hpp>

namespace puf
{

namespace
{

char gBuf[kLogBufferSize];
size_t gHead = 0;  ///< индекс следующей записи (кольцо)
size_t gUsed = 0;  ///< байт занято (≤ kLogBufferSize)
SemaphoreHandle_t gMutex = nullptr;

vprintf_like_t gOrigVprintf = nullptr;  ///< оригинальный обработчик ESP_LOG

/// Добавляет строку в кольцевой буфер, вытесняя старые записи при переполнении.
void ringAppend(const char* s, size_t len)
{
    if (len == 0 || gMutex == nullptr) return;
    xSemaphoreTake(gMutex, portMAX_DELAY);

    for (size_t i = 0; i < len; ++i)
    {
        gBuf[(gHead + gUsed) % kLogBufferSize] = s[i];
        if (gUsed < kLogBufferSize)
        {
            ++gUsed;
        } else
        {
            // Буфер полон — сдвигаем голову, затирая самую старую запись.
            gHead = (gHead + 1) % kLogBufferSize;
        }
    }

    xSemaphoreGive(gMutex);
}

/// Обработчик, который пишет в буфер И в оригинальный UART.
int pufVprintf(const char* fmt, va_list args)
{
    // va_copy должен быть до vsnprintf: после первого прохода args становится
    // неопределённым, передавать его повторно — UB.
    va_list copy;
    va_copy(copy, args);

    char tmp[256];
    int n = vsnprintf(tmp, sizeof(tmp), fmt, args);
    if (n > 0) ringAppend(tmp, static_cast<size_t>(n));

    // Продолжаем выводить в UART чтобы монитор работал как обычно.
    if (gOrigVprintf) gOrigVprintf(fmt, copy);

    va_end(copy);
    return n;
}

}  // namespace

void LogInit()
{
    gMutex = xSemaphoreCreateMutex();
    memset(gBuf, 0, sizeof(gBuf));
    gHead = 0;
    gUsed = 0;
    gOrigVprintf = esp_log_set_vprintf(pufVprintf);
}

void LogDump()
{
    if (gMutex == nullptr) return;
    xSemaphoreTake(gMutex, portMAX_DELAY);

    printf("--- LOG DUMP BEGIN (%zu bytes) ---\n", gUsed);

    // Линеаризуем кольцевой буфер: сначала от head до конца массива,
    // затем от начала массива до head (если буфер заполнен и обернулся).
    const size_t first = kLogBufferSize - gHead;
    if (gUsed <= first)
    {
        fwrite(gBuf + gHead, 1, gUsed, stdout);
    } else
    {
        fwrite(gBuf + gHead, 1, first, stdout);
        fwrite(gBuf, 1, gUsed - first, stdout);
    }

    printf("\n--- LOG DUMP END ---\n");
    fflush(stdout);

    // Очищаем буфер после дампа.
    gHead = 0;
    gUsed = 0;

    xSemaphoreGive(gMutex);
}

}  // namespace puf
