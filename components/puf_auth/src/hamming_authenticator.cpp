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
    : reference_(std::move(reference)), thresholdPct_(thresholdPct)
{
    if (thresholdPct < 0.0 || thresholdPct > 100.0)
    {
        throw std::invalid_argument("thresholdPct must be in [0, 100]");
    }
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
    if (candidate.size() != reference_.size())
    {
        return false;
    }
    const double hd = FractionalHD(reference_, candidate) * 100.0;
    return hd <= thresholdPct_;
}

}  // namespace puf
