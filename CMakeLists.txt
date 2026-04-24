cmake_minimum_required(VERSION 3.16)
project(multi_otsu VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Header-only library interface target ─────────────────────────────────────
# Other projects can do: add_subdirectory(multi-otsu)
#                        target_link_libraries(foo PRIVATE multi_otsu::multi_otsu)
add_library(multi_otsu INTERFACE)
add_library(multi_otsu::multi_otsu ALIAS multi_otsu)
target_include_directories(multi_otsu INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ── CLI demo (optional, requires no extra deps) ───────────────────────────────
option(BUILD_DEMO "Build the CLI demo executable" ON)
if(BUILD_DEMO)
    add_executable(multi_otsu_demo src/main.cpp)
    target_link_libraries(multi_otsu_demo PRIVATE multi_otsu::multi_otsu)
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────
option(BUILD_TESTS "Build the self-contained unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_executable(test_multi_otsu tests/test_multi_otsu.cpp)
    target_link_libraries(test_multi_otsu PRIVATE multi_otsu::multi_otsu)
    add_test(NAME multi_otsu_tests COMMAND test_multi_otsu)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
include(GNUInstallDirs)
install(FILES include/multi_otsu.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
