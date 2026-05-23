# Использование (C++ API)

## Выбор PUF во время выполнения

```cpp
#include <esp32_puf_factory.hpp>
#include <puf_type.hpp>

puf::Esp32PufFactory factory;
puf::PufType type = puf::PufType::Sram; // выбор во время выполнения

const auto generator = factory.Create(type, 256);
const puf::Fingerprint fp = generator->Generate();
// fp — это std::vector<uint8_t> длиной 32 байта (256 бит)
```

## RO PUF (явное создание)

```cpp
#include <esp32_puf_factory.hpp>

puf::Esp32PufFactory factory;
const auto generator = factory.CreateRoPuf(256);
const puf::Fingerprint fp = generator->Generate();
```

## SRAM PUF (явное создание)

```cpp
#include <esp32_puf_factory.hpp>

puf::Esp32PufFactory factory;
const auto generator = factory.CreateSramPuf(256);
const puf::Fingerprint fp = generator->Generate();
```

## С пост-обработкой

```cpp
#include <esp32_puf_factory.hpp>
#include <majority_voter.hpp>
#include <von_neumann_debias.hpp>

puf::Esp32PufFactory factory;
auto raw = factory.CreateRoPuf(512);

// Стабилизация мажоритарным голосованием (3 раунда)
auto stable = std::make_unique<puf::MajorityVoter>(std::move(raw), 3);

// Устранение смещения дебиасингом фон Неймана
auto debiased = std::make_unique<puf::VonNeumannDebias>(std::move(stable), 256);

const puf::Fingerprint fp = debiased->Generate();
```

## Хранение и аутентификация

```cpp
#include <nvs_fingerprint_storage.hpp>
#include <hamming_authenticator.hpp>

puf::NvsFingerprintStorage storage;

// Регистрация (однократно при производстве)
if (!storage.HasFingerprint()) {
    storage.Store(fp);
}

// Аутентификация (порог внутриустройственного расстояния Хэмминга 10%)
puf::HammingAuthenticator auth(storage.Load(), 10.0);
const bool ok = auth.Authenticate(fp);
```

## Принципы генерации отпечатка

**RO PUF:**
1. `Esp32PufFactory` создаёт N экземпляров `RoOscillator` (N*(N-1)/2 пар >= bits).
2. `RoPuf::Generate()` измеряет каждый осциллятор в течение `windowCycles` тактов процессора.
3. Для каждой пары `(i, j)`: `counts[i] > counts[j]` даёт бит `1`, иначе `0`.
4. Результат — `ceil(bits/8)` байт, уникальных для каждого чипа.

**SRAM PUF:**
1. `Esp32PufFactory` размещает `s_sram_puf_buf[64]` в секции `.noinit`.
2. `SramPuf::Generate()` копирует первые `ceil(bits/8)` байт из этого буфера.
3. Источник энтропии — физические разбросы ячеек SRAM при подаче питания.

## Метрики (`puf_metrics`)

| Метрика | Идеал | Описание |
|---|---|---|
| `IntraHD` | 0.0 | Среднее расстояние Хэмминга между повторными измерениями одного устройства (цель: < 5%) |
| `InterHD` | 0.5 | Среднее расстояние Хэмминга между отпечатками разных устройств (максимальное разделение) |
| `Uniformity` | 0.5 | Доля единиц (баланс битов) |

```cpp
#include <puf_metrics.hpp>

const double intra = puf::metrics::IntraHD({fp1, fp2, fp3});
const double inter = puf::metrics::InterHD({fpDevice1, fpDevice2});
const double uni   = puf::metrics::Uniformity(fp);
```
