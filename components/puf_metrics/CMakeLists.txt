if(ESP_PLATFORM)
    idf_component_register(
        SRCS "src/puf_metrics.cpp"
        INCLUDE_DIRS "."
        REQUIRES puf_core
    )
else()
    add_library(puf_metrics src/puf_metrics.cpp)
    target_include_directories(puf_metrics PUBLIC .)
    target_link_libraries(puf_metrics PUBLIC puf_core)

    puf_add_test(
        TARGET      puf_metrics_test
        SRCS        test/puf_metrics_test.cpp
        INCLUDE_DIRS test
        LINK_LIBS   puf_metrics
    )
endif()
