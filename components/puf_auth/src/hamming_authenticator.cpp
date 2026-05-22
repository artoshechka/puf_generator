/// @file hamming_authenticator.cpp
/// @author Artemenko Anton
/// @brief Implementation of the Hamming distance authenticator

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
    size_t dist = 0;
    const size_t len = std::min(a.size(), b.size());
    for (size_t i = 0; i < len; ++i)
    {
        dist += static_cast<size_t>(std::popcount(static_cast<uint8_t>(a[i] ^ b[i])));
    }
    return dist;
}

double HammingAuthenticator::FractionalHD(const Fingerprint& a, const Fingerprint& b)
{
    if (a.empty() || b.empty()) return 0.0;
    const size_t bits = std::min(a.size(), b.size()) * 8;
    return static_cast<double>(HammingDistance(a, b)) / static_cast<double>(bits);
}

bool HammingAuthenticator::Authenticate(const Fingerprint& candidate)
{
    const double hd = FractionalHD(reference_, candidate) * 100.0;
    return hd <= thresholdPct_;
}

}  // namespace puf
