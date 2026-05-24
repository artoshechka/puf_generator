idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES puf_factory puf_log
)
target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
