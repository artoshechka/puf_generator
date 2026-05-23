idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES puf_factory puf_postprocess puf_log ro_oscillator
)
target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
