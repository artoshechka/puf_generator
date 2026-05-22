/// @file puf_log.hpp
/// @brief Кольцевой буфер для перехвата ESP_LOG* сообщений и выдачи их по команде.

#ifndef GUID_3A7C1D04_82E9_4F55_B621_9DE0C53A1748
#define GUID_3A7C1D04_82E9_4F55_B621_9DE0C53A1748

#include <cstddef>

namespace puf
{

/// Ёмкость кольцевого буфера логов в байтах.
static constexpr size_t kLogBufferSize = 4096;

/// Инициализирует буфер и перенаправляет ESP_LOG* в него.
/// Оригинальный вывод в UART при этом сохраняется.
void LogInit();

/// Дампит содержимое буфера в UART и очищает его.
/// Вызывается по команде "LOGS" из главного цикла.
void LogDump();

}  // namespace puf

#endif  // GUID_3A7C1D04_82E9_4F55_B621_9DE0C53A1748
