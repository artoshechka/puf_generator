/// @file nvs_fingerprint_storage.cpp
/// @author Artemenko Anton
/// @brief Реализация NVS-хранилища отпечатка для ESP32

#include <nvs.h>
#include <nvs_flash.h>

#include <nvs_fingerprint_storage.hpp>
#include <stdexcept>

namespace puf
{

void NvsFingerprintStorage::Store(const Fingerprint& fp)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(kNvsNamespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) throw std::runtime_error("nvs_open failed");

    err = nvs_set_blob(handle, kNvsKey, fp.data(), fp.size());
    if (err == ESP_OK) nvs_commit(handle);
    nvs_close(handle);

    if (err != ESP_OK) throw std::runtime_error("nvs_set_blob failed");
}

Fingerprint NvsFingerprintStorage::Load()
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(kNvsNamespace, NVS_READONLY, &handle);
    if (err != ESP_OK) throw std::runtime_error("nvs_open failed");

    size_t size = 0;
    err = nvs_get_blob(handle, kNvsKey, nullptr, &size);
    if (err != ESP_OK)
    {
        nvs_close(handle);
        throw std::runtime_error("nvs_get_blob size failed");
    }

    Fingerprint fp(size);
    err = nvs_get_blob(handle, kNvsKey, fp.data(), &size);
    nvs_close(handle);

    if (err != ESP_OK) throw std::runtime_error("nvs_get_blob failed");
    return fp;
}

bool NvsFingerprintStorage::HasFingerprint() const
{
    nvs_handle_t handle;
    if (nvs_open(kNvsNamespace, NVS_READONLY, &handle) != ESP_OK) return false;

    size_t size = 0;
    bool has = nvs_get_blob(handle, kNvsKey, nullptr, &size) == ESP_OK;
    nvs_close(handle);
    return has;
}

void NvsFingerprintStorage::Delete()
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(kNvsNamespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) throw std::runtime_error("nvs_open failed");

    err = nvs_erase_key(handle, kNvsKey);
    if (err == ESP_OK) nvs_commit(handle);
    nvs_close(handle);

    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        throw std::runtime_error("nvs_erase_key failed");
}

}  // namespace puf
