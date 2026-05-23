/// @file puf_metrics.hpp
/// @author Artemenko Anton
/// @brief Метрики оценки качества отпечатка PUF

#ifndef GUID_D68368E0_2ED9_49ED_BA95_B2099115DCDD
#define GUID_D68368E0_2ED9_49ED_BA95_B2099115DCDD

#include <fingerprint.hpp>
#include <vector>

namespace puf::metrics
{

/// @brief Подсчитывает количество различающихся битов между двумя отпечатками
/// @param[in] a Первый отпечаток
/// @param[in] b Второй отпечаток
/// @return Расстояние Хэмминга в битах
size_t HammingDistance(const Fingerprint& a, const Fingerprint& b);

/// @brief Нормализует расстояние Хэмминга по длине отпечатка
/// @param[in] a Первый отпечаток
/// @param[in] b Второй отпечаток
/// @return Относительное расстояние Хэмминга [0.0, 1.0]
double FractionalHD(const Fingerprint& a, const Fingerprint& b);

/// @brief Внутреннее расстояние Хэмминга: среднее расстояние между повторными измерениями одного устройства.
/// Идеальное значение: 0.0 (полная стабильность). Допустимое: < 0.05.
/// @param[in] samples Набор измерений с одного устройства (минимум 2)
/// @return Среднее относительное расстояние Хэмминга по всем парам измерений
/// @throws std::invalid_argument если samples содержит менее 2 элементов
double IntraHD(const std::vector<Fingerprint>& samples);

/// @brief Межустройственное расстояние Хэмминга: среднее расстояние между отпечатками разных устройств.
/// Идеальное значение: 0.5 (максимальная различимость).
/// @param[in] deviceFingerprints По одному отпечатку на устройство (минимум 2)
/// @return Среднее относительное расстояние Хэмминга по всем парам устройств
/// @throws std::invalid_argument если deviceFingerprints содержит менее 2 элементов
double InterHD(const std::vector<Fingerprint>& deviceFingerprints);

/// @brief Доля битов, равных 1 — мера равномерности распределения.
/// Идеальное значение: 0.5.
/// @param[in] fp Анализируемый отпечаток
/// @return Доля установленных битов [0.0, 1.0]
double Uniformity(const Fingerprint& fp);

}  // namespace puf::metrics

#endif  // GUID_D68368E0_2ED9_49ED_BA95_B2099115DCDD
