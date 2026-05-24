/// @file hamming_authenticator.cpp
/// @author Artemenko Anton
/// @brief Реализация аутентификатора на основе расстояния Хэмминга

#include <algorithm>
#include <bit>
#include <hamming_authenticator.hpp>
#include <stdexcept>

namespace puf
{

HammingAuthenticator::HammingAuthenticator(Fingerprint reference, double thresholdPct)
    : reference_(std::move(reference)), thresholdPct_(thresholdPct), thresholdBitsCached_(0U)
{
    if (thresholdPct < 0.0 || thresholdPct > 100.0)
    {
        throw std::invalid_argument("thresholdPct must be in [0, 100]");
    }
    // Кэшируем целочисленный порог в битах: дальше Authenticate сравнивает
    // популярное расстояние с константой без double-арифметики, что снимает
    // зависимость времени от значения порога/расстояния (FP-операции на ряде
    // ядер ESP32 проходят программно, время варьируется по операндам).
    const size_t refBits = reference_.size() * 8U;
    thresholdBitsCached_ = static_cast<size_t>((static_cast<double>(refBits) * thresholdPct_) / 100.0);
}

size_t HammingAuthenticator::HammingDistance(const Fingerprint& a, const Fingerprint& b)
{
    if (a.size() != b.size())
    {
        throw std::invalid_argument("HammingDistance: fingerprint length mismatch");
    }
    size_t dist = 0;
    for (size_t i = 0; i < a.size(); ++i)
    {
        dist += static_cast<size_t>(std::popcount(static_cast<uint8_t>(a[i] ^ b[i])));
    }
    return dist;
}

double HammingAuthenticator::FractionalHD(const Fingerprint& a, const Fingerprint& b)
{
    if (a.empty() || b.empty())
    {
        throw std::invalid_argument("fingerprints must not be empty");
    }
    if (a.size() != b.size())
    {
        throw std::invalid_argument("fingerprint length mismatch");
    }
    const size_t bits = a.size() * 8U;
    return static_cast<double>(HammingDistance(a, b)) / static_cast<double>(bits);
}

bool HammingAuthenticator::Authenticate(const Fingerprint& candidate)
{
    const size_t refSize = reference_.size();
    // Маска несовпадения длин: 0xFF при несовпадении, 0x00 при совпадении.
    // Подмешивается в XOR ниже, чтобы при mismatch дистанция принудительно
    // равнялась refSize*8 — это убирает раннее ветвление по длине candidate
    // и делает время выполнения зависящим только от refSize.
    const uint8_t mismatchMask = (candidate.size() == refSize) ? 0x00U : 0xFFU;
    const size_t candSize = candidate.size();

    size_t dist = 0;
    for (size_t i = 0; i < refSize; ++i)
    {
        const uint8_t cv = (i < candSize) ? candidate[i] : 0x00U;
        const uint8_t diff = static_cast<uint8_t>((reference_[i] ^ cv) | mismatchMask);
        dist += static_cast<size_t>(std::popcount(diff));
    }

    return dist <= thresholdBitsCached_;
}

}  // namespace puf
