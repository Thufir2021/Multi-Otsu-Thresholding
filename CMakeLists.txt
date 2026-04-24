cmake_minimum_required(VERSION 3.16)
project(multi_otsu VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Header-only library ──────────────────────────────────────────────────────
add_library(multi_otsu INTERFACE)
add_library(multi_otsu::multi_otsu ALIAS multi_otsu)

target_include_directories(multi_otsu INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ── Dependency ───────────────────────────────────────────────────────────────
find_package(TIFF REQUIRED)

# ── CLI Tool ─────────────────────────────────────────────────────────────────
add_executable(multi_otsu_cli
    src/main.cpp
    src/tiff_io.cpp
)

target_link_libraries(multi_otsu_cli
    PRIVATE
        multi_otsu::multi_otsu
        TIFF::TIFF
)

# ── Install ──────────────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(FILES include/multi_otsu.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS multi_otsu_cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)