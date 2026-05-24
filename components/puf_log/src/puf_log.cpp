/// @file puf_log.cpp
/// @brief Кольцевой буфер логов с перехватом через esp_log_set_vprintf.

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <puf_log.hpp>
#include <vector>

namespace puf
{

namespace
{

char gBuf[kLogBufferSize];
size_t gHead = 0;
size_t gUsed = 0;
SemaphoreHandle_t gMutex = nullptr;
size_t gTruncated = 0;
bool gInitialized = false;

// Инициализирован безопасным fallback'ом (vprintf), чтобы любой вызов pufVprintf
// между esp_log_set_vprintf() и записью реального gOrigVprintf не упал в nullptr.
vprintf_like_t gOrigVprintf = &vprintf;

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
        } else
        {
            gHead = (gHead + 1U) % kLogBufferSize;
        }
    }

    (void)xSemaphoreGive(gMutex);
}

int pufVprintf(const char* fmt, va_list args)
{
    // Из ISR брать FreeRTOS mutex нельзя, а printf через USB Serial/JTAG
    // тоже небезопасен. Тихо игнорируем — лучше потерять одну строку лога
    // чем уронить устройство.
    if (xPortInIsrContext() != 0)
    {
        return 0;
    }

    va_list copy;
    va_copy(copy, args);

    char tmp[256];
    const int n = vsnprintf(tmp, sizeof(tmp), fmt, args);
    if (n > 0)
    {
        const size_t actual = std::min(static_cast<size_t>(n), sizeof(tmp) - 1U);
        ringAppend(tmp, actual);
        if (static_cast<size_t>(n) >= sizeof(tmp))
        {
            static const char kTrunc[] = "...[truncated]";
            ringAppend(kTrunc, sizeof(kTrunc) - 1U);
            ++gTruncated;
        }
        memset(tmp, 0, sizeof(tmp));
    }

    int origReturn = n;
    if (gOrigVprintf != nullptr)
    {
        origReturn = gOrigVprintf(fmt, copy);
    }

    va_end(copy);
    return origReturn;
}

}  // namespace

void LogInit()
{
    if (gInitialized)
    {
        return;
    }

    gMutex = xSemaphoreCreateMutex();
    if (gMutex == nullptr)
    {
        // Без мьютекса безопасно использовать буфер нельзя — оставляем
        // оригинальный esp_log путь без перехвата.
        return;
    }

    memset(gBuf, 0, sizeof(gBuf));
    gHead = 0;
    gUsed = 0;

    // gOrigVprintf уже инициализирован безопасным &vprintf — set активирует
    // pufVprintf немедленно, после чего перезаписываем реальный prev.
    vprintf_like_t prev = esp_log_set_vprintf(pufVprintf);
    gOrigVprintf = prev;
    gInitialized = true;
}

void LogDump()
{
    if (gMutex == nullptr)
    {
        return;
    }
    (void)xSemaphoreTake(gMutex, portMAX_DELAY);

    const size_t used = gUsed;
    std::vector<char> snapshot(used);
    for (size_t i = 0; i < used; ++i)
    {
        snapshot[i] = gBuf[(gHead + i) % kLogBufferSize];
    }

    memset(gBuf, 0, sizeof(gBuf));
    gHead = 0;
    gUsed = 0;

    (void)xSemaphoreGive(gMutex);

    printf("--- LOG DUMP BEGIN (%zu bytes) ---\n", used);
    if (!snapshot.empty())
    {
        (void)fwrite(snapshot.data(), 1U, snapshot.size(), stdout);
    }
    printf("\n--- LOG DUMP END ---\n");
    (void)fflush(stdout);
}

}  // namespace puf
