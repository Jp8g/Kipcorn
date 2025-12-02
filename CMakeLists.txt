cmake_minimum_required(VERSION 3.10)

project(KIPCORN C)
set(CMAKE_C_STANDARD 99)

add_library(kipcorn STATIC src/kipcorn.c external/xdg-shell.c external/xdg-decoration-unstable-v1.c)

find_package(OpenGL REQUIRED COMPONENTS EGL)

target_link_libraries(kipcorn PRIVATE wayland-client OpenGL::EGL wayland-egl xkbcommon)

target_include_directories(kipcorn PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)