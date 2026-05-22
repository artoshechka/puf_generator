if(ESP_PLATFORM)
    idf_component_register(
        SRCS "src/puf_metrics.cpp"
        INCLUDE_DIRS "."
        REQUIRES puf_core
    )
    target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
else()
    add_library(puf_metrics src/puf_metrics.cpp)
    target_include_directories(puf_metrics PUBLIC .)
    target_link_libraries(puf_metrics PUBLIC puf_core)
    target_compile_features(puf_metrics PUBLIC cxx_std_20)

    puf_add_test(
        TARGET      puf_metrics_test
        SRCS        test/puf_metrics_test.cpp
        INCLUDE_DIRS test
        LINK_LIBS   puf_metrics
    )
endif()
