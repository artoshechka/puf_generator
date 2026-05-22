cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{IDF_PATH})
    set(EXTRA_COMPONENT_DIRS components)
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(puf_generator)
else()
    # Хост-сборка: тесты компонентов без ESP32-железа.
    # Запуск: conan install . --output-folder=build_host --build=missing -pr=profiles/host
    #         cmake -B build_host -DCMAKE_TOOLCHAIN_FILE=build_host/conan_toolchain.cmake
    #         cmake --build build_host && ctest --test-dir build_host
    project(puf_generator CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    enable_testing()

    include(cmake/ctest_cmake.txt)

    add_subdirectory(components/puf_core)
    add_subdirectory(components/ro_puf)
    add_subdirectory(components/puf_postprocess)
    add_subdirectory(components/puf_metrics)
endif()
