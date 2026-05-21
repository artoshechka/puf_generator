idf_component_register(
    SRCS "src/esp32_puf_factory.cpp"
    INCLUDE_DIRS "."
    REQUIRES puf_core ro_oscillator ro_puf
)
