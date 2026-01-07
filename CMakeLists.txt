cmake_minimum_required(VERSION 3.16)
project(Gomoku LANGUAGES CXX)

if (NOT WIN32)
    message(FATAL_ERROR "This project is Windows-only and requires the MSVC toolchain.")
endif()

add_executable(gomoku WIN32
    src/win32_main.cpp
    src/gomoku.cpp
    src/gomoku_ai.cpp
)

target_compile_features(gomoku PRIVATE cxx_std_17)

target_link_libraries(gomoku PRIVATE user32 gdi32)
