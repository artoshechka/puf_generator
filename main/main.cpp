/// @file main.cpp
/// @author Artemenko Anton
/// @brief Точка входа — демонстрация генерации PUF-идентификатора

#include <esp_log.h>

#include <cstdio>
#include <esp32_puf_factory.hpp>

static const char* kTag = "puf";

extern "C" void app_main()
{
    puf::Esp32PufFactory factory;
    const auto generator = factory.CreateRoPuf(256);
    const auto fp = generator->Generate();

    ESP_LOGI(kTag, "Отпечаток устройства (%zu бит):", generator->FingerprintBits());
    for (const uint8_t byte : fp)
    {
        printf("%02x", byte);
    }
    printf("\n");
}
