/// @file puf_metrics.cpp
/// @author Artemenko Anton
/// @brief Implementation of PUF evaluation metrics

#include <algorithm>
#include <bit>
#include <puf_metrics.hpp>
#include <stdexcept>

namespace puf::metrics
{

size_t HammingDistance(const Fingerprint& a, const Fingerprint& b)
{
    size_t dist = 0;
    const size_t len = std::min(a.size(), b.size());
    for (size_t i = 0; i < len; ++i)
    {
        dist += static_cast<size_t>(std::popcount(static_cast<uint8_t>(a[i] ^ b[i])));
    }
    return dist;
}

double FractionalHD(const Fingerprint& a, const Fingerprint& b)
{
    if (a.empty() || b.empty()) return 0.0;
    const size_t bits = std::min(a.size(), b.size()) * 8;
    return static_cast<double>(HammingDistance(a, b)) / static_cast<double>(bits);
}

double IntraHD(const std::vector<Fingerprint>& samples)
{
    if (samples.size() < 2) throw std::invalid_argument("need at least 2 samples");

    double sum = 0.0;
    size_t pairs = 0;
    for (size_t i = 0; i < samples.size(); ++i)
    {
        for (size_t j = i + 1; j < samples.size(); ++j)
        {
            sum += FractionalHD(samples[i], samples[j]);
            ++pairs;
        }
    }
    return sum / static_cast<double>(pairs);
}

double InterHD(const std::vector<Fingerprint>& deviceFingerprints)
{
    if (deviceFingerprints.size() < 2) throw std::invalid_argument("need at least 2 devices");

    double sum = 0.0;
    size_t pairs = 0;
    for (size_t i = 0; i < deviceFingerprints.size(); ++i)
    {
        for (size_t j = i + 1; j < deviceFingerprints.size(); ++j)
        {
            sum += FractionalHD(deviceFingerprints[i], deviceFingerprints[j]);
            ++pairs;
        }
    }
    return sum / static_cast<double>(pairs);
}

double Uniformity(const Fingerprint& fp)
{
    if (fp.empty()) return 0.0;
    size_t ones = 0;
    for (uint8_t byte : fp)
    {
        ones += static_cast<size_t>(std::popcount(byte));
    }
    return static_cast<double>(ones) / static_cast<double>(fp.size() * 8);
}

}  // namespace puf::metrics
