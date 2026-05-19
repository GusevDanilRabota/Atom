cmake_minimum_required(VERSION 3.14)
project(OrbitalVisualizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(orb3d
    quantum_numbers.cpp
    special_functions.cpp
    orbital.cpp
    renderer.cpp
    volume_exporter.cpp
    multirenderer.cpp
    main.cpp
)

target_compile_options(orb3d PRIVATE -O2 -pthread)
target_link_libraries(orb3d PRIVATE pthread)