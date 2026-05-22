# puf_generator

Аппаратный идентификатор устройства на основе Ring Oscillator PUF для ESP32.  
Генерирует 256-битный отпечаток, уникальный для каждого экземпляра чипа,
на основе вариаций времени выполнения программных осцилляторов в IRAM.

---

## Архитектура

```mermaid
classDiagram
    namespace puf_core {
        class IOscillator {
            <<interface>>
            +Measure(windowCycles uint32_t) uint32_t
        }
        class IPufGenerator {
            <<interface>>
            +Generate() Fingerprint
            +FingerprintBits() size_t
        }
    }

    namespace ro_oscillator {
        class RoOscillator {
            +kMaxIndex size_t = 31
            -index_ size_t
            +RoOscillator(index size_t)
            +Measure(windowCycles uint32_t) uint32_t
        }
    }

    namespace ro_puf {
        class RoPuf {
            -oscillators_ vector~IOscillator~
            -bits_ size_t
            -window_ uint32_t
            +RoPuf(oscillators, bits, windowCycles)
            +Generate() Fingerprint
            +FingerprintBits() size_t
        }
    }

    namespace puf_factory {
        class IPufFactory {
            <<interface>>
            +CreateRoPuf(bits size_t) unique_ptr~IPufGenerator~
        }
        class Esp32PufFactory {
            +CreateRoPuf(bits size_t) unique_ptr~IPufGenerator~
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

    IOscillator <|.. RoOscillator
    IPufGenerator <|.. RoPuf
    IPufGenerator <|.. VonNeumannDebias
    IPufGenerator <|.. MajorityVoter
    IPufFactory <|.. Esp32PufFactory
    IFingerprintStorage <|.. NvsFingerprintStorage
    IAuthenticator <|.. HammingAuthenticator

    RoPuf o-- IOscillator
    VonNeumannDebias o-- IPufGenerator
    MajorityVoter o-- IPufGenerator
    Esp32PufFactory ..> RoOscillator : creates
    Esp32PufFactory ..> RoPuf : creates
```

### Компоненты

| Компонент | Зависимости | Описание |
|---|---|---|
| `puf_core` | — | Интерфейсы `IOscillator`, `IPufGenerator`, тип `Fingerprint` |
| `ro_oscillator` | `puf_core` | 32 IRAM-осциллятора для ESP32 (Xtensa LX6) |
| `ro_puf` | `puf_core` | Генератор отпечатка попарным сравнением счётчиков; платформонезависим |
| `puf_factory` | `puf_core`, `ro_oscillator`, `ro_puf` | Интерфейс `IPufFactory` и реализация `Esp32PufFactory` |
| `nvs_storage` | `puf_core` | Хранение эталонного отпечатка в ESP32 NVS |
| `puf_auth` | `puf_core` | Аутентификация по расстоянию Хэмминга с настраиваемым порогом |
| `puf_postprocess` | `puf_core` | Декораторы `VonNeumannDebias` и `MajorityVoter` для повышения качества |
| `puf_metrics` | `puf_core` | Метрики оценки PUF: intra-HD, inter-HD, uniformity |

Новый тип устройства — новая фабрика. Новый тип осциллятора — новый компонент рядом с `ro_oscillator`. Постобработка подключается декораторами без изменения генератора.

---

## Структура проекта

```
puf_generator/
├── conanfile.py
├── cmake/
│   └── ctest_cmake.txt        # puf_add_test() + PUF_BUILD_TESTS флаг
├── profiles/
│   ├── esp32                  # Conan-профиль для кросс-компиляции
│   └── host                   # Conan-профиль для хост-тестов
├── components/
│   ├── puf_core/
│   │   ├── fingerprint.hpp
│   │   ├── i_oscillator.hpp
│   │   ├── i_puf_generator.hpp
│   │   └── CMakeLists.txt
│   ├── ro_oscillator/
│   │   ├── ro_oscillator.hpp
│   │   ├── CMakeLists.txt
│   │   └── src/ro_oscillator.cpp
│   ├── ro_puf/
│   │   ├── ro_puf.hpp
│   │   ├── CMakeLists.txt
│   │   ├── src/ro_puf.cpp
│   │   └── test/
│   │       ├── mock_oscillator.hpp
│   │       └── ro_puf_test.cpp
│   ├── puf_factory/
│   │   ├── ipuf_factory.hpp
│   │   ├── esp32_puf_factory.hpp
│   │   ├── CMakeLists.txt
│   │   └── src/esp32_puf_factory.cpp
│   ├── nvs_storage/
│   │   ├── i_fingerprint_storage.hpp
│   │   ├── nvs_fingerprint_storage.hpp
│   │   ├── CMakeLists.txt
│   │   └── src/nvs_fingerprint_storage.cpp
│   ├── puf_auth/
│   │   ├── i_authenticator.hpp
│   │   ├── hamming_authenticator.hpp
│   │   ├── CMakeLists.txt
│   │   └── src/hamming_authenticator.cpp
│   ├── puf_postprocess/
│   │   ├── von_neumann_debias.hpp
│   │   ├── majority_voter.hpp
│   │   ├── CMakeLists.txt
│   │   └── src/
│   │       ├── von_neumann_debias.cpp
│   │       └── majority_voter.cpp
│   └── puf_metrics/
│       ├── puf_metrics.hpp
│       ├── CMakeLists.txt
│       └── src/puf_metrics.cpp
├── main/
│   ├── main.cpp
│   ├── Kconfig.projbuild
│   └── CMakeLists.txt
└── server/                        # Go-сервер верификации
    ├── go.mod
    ├── main.go
    ├── config.go
    ├── puf.go
    ├── store.go
    └── handler.go
```

---

## Требования

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) v5.x
- [Conan](https://conan.io/) 2.x (`pip install conan`)
- CMake 3.16+
- [Doxygen](https://www.doxygen.nl/) 1.9+ (для генерации документации)

---

## Документация

```bash
doxygen Doxyfile
# HTML-документация генерируется в docs/html/index.html
```

---

## Сборка

### ESP32

#### Установка ESP-IDF

```bash
git clone --recursive --depth 1 --branch v5.4.1 https://github.com/espressif/esp-idf.git ~/esp/esp-idf
cd ~/esp/esp-idf && ./install.sh esp32
```

> `install.sh` must be run before `export.sh`. Re-run it after any ESP-IDF update or if `export.sh` reports missing Python dependencies.

#### Прошивка

```bash
# Activate environment (required in every new terminal)
. ~/esp/esp-idf/export.sh

idf.py set-target esp32

# Linux: /dev/ttyUSB0, macOS: /dev/cu.usbmodem101
idf.py -p /dev/cu.usbmodem101 flash monitor
```

#### Автоматизация (scripts/flash.py)

```bash
# Auto-detect port, install IDF if missing, build and flash
python3 scripts/flash.py

# Explicit port
python3 scripts/flash.py --port /dev/cu.usbmodem101

# Build only
python3 scripts/flash.py --build-only
```

> Requires Python 3.9+. The script clones and installs ESP-IDF automatically on first run.

### Конфигурация (menuconfig)

```bash
idf.py menuconfig   # PUF Generator → длина отпечатка, окно, число раундов
```

| Параметр | По умолчанию | Описание |
|---|---|---|
| `PUF_FINGERPRINT_BITS` | 256 | Длина отпечатка в битах (64–496) |
| `PUF_WINDOW_CYCLES` | 200 000 | Окно измерения в тактах CPU |
| `PUF_MAJORITY_ROUNDS` | 3 | Число раундов голосования (нечётное) |

### Хост-тесты (через Conan)

```bash
# Сгенерировать хост-профиль (один раз)
conan profile detect --name host

# Установить зависимости и настроить сборку
conan install . --output-folder=build_host --build=missing -pr=profiles/host

# Собрать и запустить тесты
cmake -B build_host -DCMAKE_TOOLCHAIN_FILE=build_host/conan_toolchain.cmake \
      -DPUF_BUILD_TESTS=ON
cmake --build build_host
ctest --test-dir build_host --output-on-failure
```

---

## Использование

### Базовый отпечаток

```cpp
#include <esp32_puf_factory.hpp>

puf::Esp32PufFactory factory;
const auto generator = factory.CreateRoPuf(256);
const puf::Fingerprint fp = generator->Generate();
// fp — std::vector<uint8_t>, 32 байта (256 бит)
```

### С постобработкой

```cpp
#include <esp32_puf_factory.hpp>
#include <majority_voter.hpp>
#include <von_neumann_debias.hpp>

puf::Esp32PufFactory factory;
auto raw = factory.CreateRoPuf(512);

// Стабилизация голосованием большинством (3 измерения)
auto stable = std::make_unique<puf::MajorityVoter>(std::move(raw), 3);

// Устранение смещения методом фон Неймана
auto debiased = std::make_unique<puf::VonNeumannDebias>(std::move(stable), 256);

const puf::Fingerprint fp = debiased->Generate();
```

### Хранение и аутентификация

```cpp
#include <nvs_fingerprint_storage.hpp>
#include <hamming_authenticator.hpp>

puf::NvsFingerprintStorage storage;

// Энролмент (один раз при производстве)
if (!storage.HasFingerprint()) {
    storage.Store(fp);
}

// Аутентификация (порог 10% intra-HD)
puf::HammingAuthenticator auth(storage.Load(), 10.0);
const bool ok = auth.Authenticate(fp);
```

### Принцип генерации отпечатка

1. `Esp32PufFactory` создаёт N экземпляров `RoOscillator` (N×(N−1)/2 пар ≥ bits).
2. `RoPuf::Generate()` запускает каждый осциллятор на `windowCycles` тактов.
3. Для каждой пары `(i, j)`: `counts[i] > counts[j]` → бит `1`, иначе `0`.
4. Результат — `ceil(bits/8)` байт, уникальных для данного экземпляра чипа.

---

## Сервер верификации (`server/`)

Go-сервер для регистрации устройств и верификации их PUF-отпечатков по расстоянию Хэмминга.

### Быстрый старт (Docker)

```bash
# Собрать и запустить сервер + PostgreSQL одной командой
ADMIN_TOKEN=secret docker compose up --build
```

Сервер доступен на `http://localhost:8080`.  
Данные PostgreSQL сохраняются в Docker volume `pgdata` между перезапусками.

Переменные окружения можно переопределить через `.env` в корне:

```bash
# .env
ADMIN_TOKEN=my-secret-token
PUF_THRESHOLD_PCT=8.0
```

#### Как это работает

```
┌─────────────────────────────────────┐
│         docker compose up           │
│                                     │
│  ┌──────────────┐  ┌─────────────┐  │
│  │   postgres   │  │   server    │  │
│  │  (healthck)  │──│  (Go bin)   │  │
│  └──────────────┘  └─────────────┘  │
│         pgdata volume               │
└─────────────────────────────────────┘
```

1. `postgres` стартует первым; сервер ждёт `pg_isready` (healthcheck).
2. `server` собирается двухэтапным Dockerfile: Go-тулчейн только в build-стадии, финальный образ — `scratch` (~5 МБ).
3. При старте сервер выполняет `CREATE TABLE IF NOT EXISTS devices` — миграция накатывается автоматически.
4. `pgdata` volume переживает `docker compose down`; для полной очистки: `docker compose down -v`.

### Ручной запуск (без Docker)

#### Требования

- Go 1.22+
- PostgreSQL 14+

```bash
cd server
DATABASE_URL=postgres://user:pass@localhost/pufdb \
ADMIN_TOKEN=secret \
PUF_THRESHOLD_PCT=10.0 \
go run .
```

| Переменная окружения | По умолчанию | Описание |
|---|---|---|
| `DATABASE_URL` | — (обязательная) | DSN PostgreSQL |
| `LISTEN_ADDR` | `:8080` | Адрес и порт сервера |
| `ADMIN_TOKEN` | — | Bearer-токен для админ-операций; если пустой — без защиты |
| `PUF_THRESHOLD_PCT` | `10.0` | Максимальный допустимый intra-HD в процентах |

### REST API

```
# Регистрация устройства (admin)
POST /devices/{id}/enroll
Authorization: Bearer <ADMIN_TOKEN>
{"fingerprint": "a1b2c3..."}

# Верификация устройства по PUF-отпечатку
POST /devices/{id}/verify
Authorization: PUF <hex-fingerprint>
→ {"ok": true, "hamming_pct": 4.2, "threshold_pct": 10.0}

# Список устройств (admin)
GET /devices
Authorization: Bearer <ADMIN_TOKEN>

# Удаление устройства (admin)
DELETE /devices/{id}
Authorization: Bearer <ADMIN_TOKEN>
```

### Механизм аутентификации

Устройство передаёт свежий PUF-отпечаток в заголовке `Authorization: PUF <hex>`.
Сервер вычисляет расстояние Хэмминга между переданным и эталонным отпечатком.
Если дробное HD (в процентах) не превышает `PUF_THRESHOLD_PCT` — устройство считается подлинным.

---

## Метрики оценки (`puf_metrics`)

| Метрика | Идеал | Описание |
|---|---|---|
| `IntraHD` | 0.0 | Среднее HD между повторными измерениями одного устройства (цель: < 5%) |
| `InterHD` | 0.5 | Среднее HD между отпечатками разных устройств (максимальная различимость) |
| `Uniformity` | 0.5 | Доля единичных бит (равномерность распределения) |

```cpp
#include <puf_metrics.hpp>

const double intra = puf::metrics::IntraHD({fp1, fp2, fp3});
const double inter = puf::metrics::InterHD({fpDevice1, fpDevice2});
const double uni   = puf::metrics::Uniformity(fp);
```
