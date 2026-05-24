idf_component_register(
    SRCS "src/ro_oscillator.cpp"
    INCLUDE_DIRS "."
    REQUIRES puf_core
)
target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
