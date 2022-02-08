cmake_minimum_required(VERSION 3.16)
project(task_7)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libswscale
    libavutil
)

add_executable(program program.c)

target_link_libraries( program
    PkgConfig::LIBAV
)
