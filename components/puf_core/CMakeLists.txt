if(ESP_PLATFORM)
    idf_component_register(
        INCLUDE_DIRS "."
    )
else()
    add_library(puf_core INTERFACE)
    target_include_directories(puf_core INTERFACE .)
    target_compile_features(puf_core INTERFACE cxx_std_17)
endif()
