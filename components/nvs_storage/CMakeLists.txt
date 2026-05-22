idf_component_register(
    SRCS "src/nvs_fingerprint_storage.cpp"
    INCLUDE_DIRS "."
    REQUIRES puf_core nvs_flash
)
target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
