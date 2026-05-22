/// @file main.cpp
/// @author Artemenko Anton
/// @brief Entry point — enrollment and authentication via PUF fingerprint

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
        // enrollment: first run
        const puf::Fingerprint fp = generator->Generate();
        storage.Store(fp);
        ESP_LOGI(kTag, "Enrollment: fingerprint stored (%zu bits)", generator->FingerprintBits());
        for (const uint8_t byte : fp) printf("%02x", byte);
        printf("\n");
    }
    else
    {
        // authentication
        const puf::Fingerprint reference = storage.Load();
        const puf::Fingerprint candidate = generator->Generate();
        puf::HammingAuthenticator auth(reference);
        const bool ok = auth.Authenticate(candidate);
        ESP_LOGI(kTag, "Authentication: %s", ok ? "success" : "denied");
        for (const uint8_t byte : candidate) printf("%02x", byte);
        printf("\n");
    }
}
