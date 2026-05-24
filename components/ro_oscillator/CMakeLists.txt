# Host tests not provided: oscillator bodies use IRAM_ATTR and
# esp_cpu_get_cycle_count (ESP32 cycle counter). The Measure() range
# guard (throws on windowCycles > kMaxWindowCycles) is exercised
# indirectly via on-device tests; a host port would need a fake
# cycle source.
idf_component_register(
    SRCS "src/ro_oscillator.cpp"
    INCLUDE_DIRS "."
    REQUIRES puf_core
)
target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
