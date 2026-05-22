# Использование (C++ API)

## Базовый отпечаток

```cpp
#include <esp32_puf_factory.hpp>

puf::Esp32PufFactory factory;
const auto generator = factory.CreateRoPuf(256);
const puf::Fingerprint fp = generator->Generate();
// fp — std::vector<uint8_t>, 32 байта (256 бит)
```

## С постобработкой

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

## Хранение и аутентификация

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

## Принцип генерации отпечатка

1. `Esp32PufFactory` создаёт N экземпляров `RoOscillator` (N×(N−1)/2 пар ≥ bits).
2. `RoPuf::Generate()` запускает каждый осциллятор на `windowCycles` тактов.
3. Для каждой пары `(i, j)`: `counts[i] > counts[j]` → бит `1`, иначе `0`.
4. Результат — `ceil(bits/8)` байт, уникальных для данного экземпляра чипа.

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
