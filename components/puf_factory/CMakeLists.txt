# Host tests not provided: factory pulls in ro_oscillator (IRAM_ATTR,
# esp_cpu) and a .noinit-sectioned SRAM buffer, neither of which links
# on a host build. RequiredOscillators() is pure C++ and could be
# extracted into a testable helper; left as a follow-up.
idf_component_register(
    SRCS "src/esp32_puf_factory.cpp"
    INCLUDE_DIRS "."
    REQUIRES puf_core ro_oscillator ro_puf sram_puf
)
target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
