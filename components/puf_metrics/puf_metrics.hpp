/// @file puf_metrics.hpp
/// @author Artemenko Anton
/// @brief Метрики оценки качества PUF-отпечатков

#ifndef GUID_D68368E0_2ED9_49ED_BA95_B2099115DCDD
#define GUID_D68368E0_2ED9_49ED_BA95_B2099115DCDD

#include <fingerprint.hpp>

#include <vector>

namespace puf::metrics {

/// @brief Расстояние Хэмминга между двумя отпечатками в битах
size_t HammingDistance(const Fingerprint& a, const Fingerprint& b);

/// @brief Дробное расстояние Хэмминга [0.0, 1.0]
double FractionalHD(const Fingerprint& a, const Fingerprint& b);

/// @brief Intra-device HD: среднее HD между повторными измерениями одного устройства.
///
/// Идеальное значение: 0.0 (полная стабильность).
/// Приемлемое: < 0.05
/// @param samples Множество измерений одного устройства
double IntraHD(const std::vector<Fingerprint>& samples);

/// @brief Inter-device HD: среднее HD между отпечатками разных устройств.
///
/// Идеальное значение: 0.5 (максимальная различимость).
/// @param deviceFingerprints По одному отпечатку от каждого устройства
double InterHD(const std::vector<Fingerprint>& deviceFingerprints);

/// @brief Доля бит равных 1 — оценка равномерности распределения.
///
/// Идеальное значение: 0.5
double Uniformity(const Fingerprint& fp);

} // namespace puf::metrics

#endif // GUID_D68368E0_2ED9_49ED_BA95_B2099115DCDD
