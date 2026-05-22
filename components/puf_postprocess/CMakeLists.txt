if(ESP_PLATFORM)
    idf_component_register(
        SRCS "src/von_neumann_debias.cpp" "src/majority_voter.cpp"
        INCLUDE_DIRS "."
        REQUIRES puf_core
    )
else()
    add_library(puf_postprocess
        src/von_neumann_debias.cpp
        src/majority_voter.cpp
    )
    target_include_directories(puf_postprocess PUBLIC .)
    target_link_libraries(puf_postprocess PUBLIC puf_core)
endif()
