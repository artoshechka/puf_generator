if(ESP_PLATFORM)
    idf_component_register(
        SRCS "src/ro_puf.cpp"
        INCLUDE_DIRS "."
        REQUIRES puf_core
    )
    target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
else()
    add_library(ro_puf src/ro_puf.cpp)
    target_include_directories(ro_puf PUBLIC .)
    target_link_libraries(ro_puf PUBLIC puf_core)

    puf_add_test(
        TARGET      ro_puf_test
        SRCS        test/ro_puf_test.cpp
        INCLUDE_DIRS test
        LINK_LIBS   ro_puf
    )
endif()
