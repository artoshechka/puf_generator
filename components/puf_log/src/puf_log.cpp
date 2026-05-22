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
size_t gHead = 0;
size_t gUsed = 0;
SemaphoreHandle_t gMutex = nullptr;

vprintf_like_t gOrigVprintf = nullptr;

void ringAppend(const char* s, size_t len)
{
    if (len == 0U || gMutex == nullptr)
    {
        return;
    }
    (void)xSemaphoreTake(gMutex, portMAX_DELAY);

    for (size_t i = 0; i < len; ++i)
    {
        gBuf[(gHead + gUsed) % kLogBufferSize] = s[i];
        if (gUsed < kLogBufferSize)
        {
            ++gUsed;
        }
        else
        {
            gHead = (gHead + 1U) % kLogBufferSize;
        }
    }

    (void)xSemaphoreGive(gMutex);
}

int pufVprintf(const char* fmt, va_list args)
{
    va_list copy;
    va_copy(copy, args);

    char tmp[256];
    const int n = vsnprintf(tmp, sizeof(tmp), fmt, args);
    if (n > 0)
    {
        ringAppend(tmp, static_cast<size_t>(n));
    }

    if (gOrigVprintf != nullptr)
    {
        (void)gOrigVprintf(fmt, copy);
    }

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
    if (gMutex == nullptr)
    {
        return;
    }
    (void)xSemaphoreTake(gMutex, portMAX_DELAY);

    printf("--- LOG DUMP BEGIN (%zu bytes) ---\n", gUsed);

    const size_t first = kLogBufferSize - gHead;
    if (gUsed <= first)
    {
        (void)fwrite(gBuf + gHead, 1U, gUsed, stdout);
    }
    else
    {
        (void)fwrite(gBuf + gHead, 1U, first, stdout);
        (void)fwrite(gBuf, 1U, gUsed - first, stdout);
    }

    printf("\n--- LOG DUMP END ---\n");
    (void)fflush(stdout);

    gHead = 0;
    gUsed = 0;

    (void)xSemaphoreGive(gMutex);
}

}  // namespace puf
