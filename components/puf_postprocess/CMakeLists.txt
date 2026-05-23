if(ESP_PLATFORM)
    idf_component_register(
        SRCS "src/von_neumann_debias.cpp" "src/majority_voter.cpp"
        INCLUDE_DIRS "."
        REQUIRES puf_core
    )
    target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)
else()
    add_library(puf_postprocess
        src/von_neumann_debias.cpp
        src/majority_voter.cpp
    )
    target_include_directories(puf_postprocess PUBLIC .)
    target_link_libraries(puf_postprocess PUBLIC puf_core)

    puf_add_test(
        TARGET      puf_postprocess_test
        SRCS        test/puf_postprocess_test.cpp
        INCLUDE_DIRS test
        LINK_LIBS   puf_postprocess
    )
endif()
