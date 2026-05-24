if(ESP_PLATFORM)
    idf_component_register(
        SRCS "src/sram_puf.cpp"
        INCLUDE_DIRS "."
        REQUIRES puf_core
    )
    target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
else()
    add_library(sram_puf src/sram_puf.cpp)
    target_include_directories(sram_puf PUBLIC .)
    target_link_libraries(sram_puf PUBLIC puf_core)

    puf_add_test(
        TARGET      sram_puf_test
        SRCS        test/sram_puf_test.cpp
        LINK_LIBS   sram_puf
    )
endif()
