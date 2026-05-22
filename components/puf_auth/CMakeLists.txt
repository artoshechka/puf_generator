if(ESP_PLATFORM)
    idf_component_register(
        SRCS "src/hamming_authenticator.cpp"
        INCLUDE_DIRS "."
        REQUIRES puf_core
    )
else()
    add_library(puf_auth src/hamming_authenticator.cpp)
    target_include_directories(puf_auth PUBLIC .)
    target_link_libraries(puf_auth PUBLIC puf_core)
    target_compile_features(puf_auth PUBLIC cxx_std_20)

    puf_add_test(
        TARGET      puf_auth_test
        SRCS        test/puf_auth_test.cpp
        INCLUDE_DIRS test
        LINK_LIBS   puf_auth
    )
endif()
