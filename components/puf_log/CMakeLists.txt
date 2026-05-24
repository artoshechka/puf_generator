# Host tests not provided: this component binds directly to
# esp_log_set_vprintf, FreeRTOS mutex, and xPortInIsrContext.
# A host build would require either ESP-IDF stubs or splitting
# the ring-buffer logic out of the IDF adapter. Tracked as a
# follow-up refactor; for now the only coverage is on hardware.
idf_component_register(
    SRCS "src/puf_log.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_common freertos
)
