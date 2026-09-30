cmake_minimum_required(VERSION 3.21)

project(DoomLauncher VERSION 1.0.0 LANGUAGES CXX)

add_library(DoomLauncher SHARED
    src/DoomLauncher.cpp
    src/DoomLauncher.def
)

target_compile_features(DoomLauncher PRIVATE cxx_std_20)
target_compile_definitions(DoomLauncher PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(DoomLauncher PRIVATE Comdlg32)

if(MSVC)
    target_compile_options(DoomLauncher PRIVATE /W4 /permissive- /utf-8)
    set_property(TARGET DoomLauncher PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set_target_properties(DoomLauncher PROPERTIES
    PREFIX ""
    OUTPUT_NAME "DoomLauncher"
)

