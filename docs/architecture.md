# Архитектура

```mermaid
classDiagram
    namespace puf_core {
        class IPufGenerator {
            <<interface>>
            +Generate() Fingerprint
            +FingerprintBits() size_t
        }
    }

    namespace sram_puf {
        class SramPuf {
            -base_ uint8_t*
            -byteCount_ size_t
            -bits_ size_t
            +SramPuf(base, byteCount, bits)
            +Generate() Fingerprint
            +FingerprintBits() size_t
        }
    }

    namespace puf_factory {
        class IPufFactory {
            <<interface>>
            +CreateSramPuf(bits size_t) unique_ptr~IPufGenerator~
        }
        class Esp32PufFactory {
            +CreateSramPuf(bits size_t) unique_ptr~IPufGenerator~
        }
    }

    namespace nvs_storage {
        class IFingerprintStorage {
            <<interface>>
            +Store(fp Fingerprint) void
            +Load() Fingerprint
            +HasFingerprint() bool
        }
        class NvsFingerprintStorage {
            -kNvsNamespace string = "puf"
            -kNvsKey string = "fingerprint"
            +Store(fp Fingerprint) void
            +Load() Fingerprint
            +HasFingerprint() bool
        }
    }

    namespace puf_auth {
        class IAuthenticator {
            <<interface>>
            +Authenticate(candidate Fingerprint) bool
        }
        class HammingAuthenticator {
            -reference_ Fingerprint
            -thresholdPct_ double
            +HammingAuthenticator(reference, thresholdPct)
            +Authenticate(candidate Fingerprint) bool
            +HammingDistance(a, b Fingerprint) size_t
            +FractionalHD(a, b Fingerprint) double
        }
    }

    namespace puf_postprocess {
        class VonNeumannDebias {
            -inner_ IPufGenerator
            -targetBits_ size_t
            +VonNeumannDebias(inner, targetBits)
            +Generate() Fingerprint
            +FingerprintBits() size_t
        }
        class MajorityVoter {
            -inner_ IPufGenerator
            -rounds_ size_t
            +MajorityVoter(inner, rounds)
            +Generate() Fingerprint
            +FingerprintBits() size_t
        }
    }

    namespace puf_metrics {
        class PufMetrics {
            <<utility>>
            +HammingDistance(a, b Fingerprint) size_t
            +FractionalHD(a, b Fingerprint) double
            +IntraHD(samples vector) double
            +InterHD(devices vector) double
            +Uniformity(fp Fingerprint) double
        }
    }

    IPufGenerator <|.. SramPuf
    IPufGenerator <|.. VonNeumannDebias
    IPufGenerator <|.. MajorityVoter
    IPufFactory <|.. Esp32PufFactory
    IFingerprintStorage <|.. NvsFingerprintStorage
    IAuthenticator <|.. HammingAuthenticator

    VonNeumannDebias o-- IPufGenerator
    MajorityVoter o-- IPufGenerator
    Esp32PufFactory ..> SramPuf : creates
```

## Компоненты

| Компонент | Зависимости | Описание |
|---|---|---|
| `puf_core` | — | Интерфейс `IPufGenerator`, тип `Fingerprint` |
| `sram_puf` | `puf_core` | Генератор отпечатка по начальному состоянию SRAM; платформонезависим |
| `puf_factory` | `puf_core`, `sram_puf` | Интерфейс `IPufFactory` и реализация `Esp32PufFactory` |
| `nvs_storage` | `puf_core` | Хранение эталонного отпечатка в ESP32 NVS |
| `puf_auth` | `puf_core` | Аутентификация по расстоянию Хэмминга с настраиваемым порогом |
| `puf_postprocess` | `puf_core` | Декораторы `VonNeumannDebias` и `MajorityVoter` для повышения качества |
| `puf_metrics` | `puf_core` | Метрики оценки PUF: intra-HD, inter-HD, uniformity |
| `puf_log` | `esp_common`, `freertos` | Кольцевой буфер логов; дамп по UART-команде `LOGS` |

Новый тип устройства — новая фабрика. Новый источник энтропии — новый компонент рядом с `sram_puf`, метод `Create*` в `IPufFactory`. Постобработка подключается декораторами без изменения генератора.
