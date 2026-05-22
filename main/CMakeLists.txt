idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES puf_factory nvs_storage puf_auth puf_postprocess
)
