# Usage (C++ API)

## Runtime PUF selection

```cpp
#include <esp32_puf_factory.hpp>
#include <puf_type.hpp>

puf::Esp32PufFactory factory;
puf::PufType type = puf::PufType::Sram; // choose at runtime

const auto generator = factory.Create(type, 256);
const puf::Fingerprint fp = generator->Generate();
// fp is std::vector<uint8_t>, 32 bytes (256 bits)
```

## RO PUF (explicit)

```cpp
#include <esp32_puf_factory.hpp>

puf::Esp32PufFactory factory;
const auto generator = factory.CreateRoPuf(256);
const puf::Fingerprint fp = generator->Generate();
```

## SRAM PUF (explicit)

```cpp
#include <esp32_puf_factory.hpp>

puf::Esp32PufFactory factory;
const auto generator = factory.CreateSramPuf(256);
const puf::Fingerprint fp = generator->Generate();
```

## With post-processing

```cpp
#include <esp32_puf_factory.hpp>
#include <majority_voter.hpp>
#include <von_neumann_debias.hpp>

puf::Esp32PufFactory factory;
auto raw = factory.CreateRoPuf(512);

// Stabilize with majority voting (3 rounds)
auto stable = std::make_unique<puf::MajorityVoter>(std::move(raw), 3);

// Remove bias with Von Neumann debiasing
auto debiased = std::make_unique<puf::VonNeumannDebias>(std::move(stable), 256);

const puf::Fingerprint fp = debiased->Generate();
```

## Storage and authentication

```cpp
#include <nvs_fingerprint_storage.hpp>
#include <hamming_authenticator.hpp>

puf::NvsFingerprintStorage storage;

// Enrollment (once during manufacturing)
if (!storage.HasFingerprint()) {
    storage.Store(fp);
}

// Authentication (10% intra-HD threshold)
puf::HammingAuthenticator auth(storage.Load(), 10.0);
const bool ok = auth.Authenticate(fp);
```

## Fingerprint generation principles

**RO PUF:**
1. `Esp32PufFactory` creates N `RoOscillator` instances (N*(N-1)/2 pairs >= bits).
2. `RoPuf::Generate()` measures each oscillator for `windowCycles` CPU cycles.
3. For each pair `(i, j)`: `counts[i] > counts[j]` yields bit `1`, else `0`.
4. Result is `ceil(bits/8)` bytes, unique per chip.

**SRAM PUF:**
1. `Esp32PufFactory` allocates `s_sram_puf_buf[64]` in a `.noinit` section.
2. `SramPuf::Generate()` copies the first `ceil(bits/8)` bytes from that buffer.
3. Entropy comes from physical SRAM cell variations at power-on.

## Metrics (`puf_metrics`)

| Metric | Ideal | Description |
|---|---|---|
| `IntraHD` | 0.0 | Mean HD between repeated measurements of one device (goal: < 5%) |
| `InterHD` | 0.5 | Mean HD between fingerprints of different devices (maximal separation) |
| `Uniformity` | 0.5 | Fraction of ones (bit balance) |

```cpp
#include <puf_metrics.hpp>

const double intra = puf::metrics::IntraHD({fp1, fp2, fp3});
const double inter = puf::metrics::InterHD({fpDevice1, fpDevice2});
const double uni   = puf::metrics::Uniformity(fp);
```
