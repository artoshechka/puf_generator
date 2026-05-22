/// @file main.cpp
/// @author Artemenko Anton
/// @brief Точка входа — энролмент и аутентификация по PUF-отпечатку

#include <sdkconfig.h>

#include <esp_log.h>
#include <nvs_flash.h>

#include <cstdio>
#include <esp32_puf_factory.hpp>
#include <hamming_authenticator.hpp>
#include <majority_voter.hpp>
#include <nvs_fingerprint_storage.hpp>

static const char* kTag = "puf";

extern "C" void app_main()
{
    nvs_flash_init();

    puf::Esp32PufFactory factory;
    auto raw = factory.CreateRoPuf(CONFIG_PUF_FINGERPRINT_BITS);
    auto generator = std::make_unique<puf::MajorityVoter>(
        std::move(raw), CONFIG_PUF_MAJORITY_ROUNDS);

    puf::NvsFingerprintStorage storage;

    if (!storage.HasFingerprint())
    {
        // энролмент: первый запуск
        const puf::Fingerprint fp = generator->Generate();
        storage.Store(fp);
        ESP_LOGI(kTag, "Энролмент: отпечаток сохранён (%zu бит)", generator->FingerprintBits());
        for (const uint8_t byte : fp) printf("%02x", byte);
        printf("\n");
    }
    else
    {
        // аутентификация
        const puf::Fingerprint reference = storage.Load();
        const puf::Fingerprint candidate = generator->Generate();
        puf::HammingAuthenticator auth(reference);
        const bool ok = auth.Authenticate(candidate);
        ESP_LOGI(kTag, "Аутентификация: %s", ok ? "успех" : "отказ");
        for (const uint8_t byte : candidate) printf("%02x", byte);
        printf("\n");
    }
}
