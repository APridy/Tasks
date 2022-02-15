cmake_minimum_required(VERSION 3.10.2)
project(task_7)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libswscale
    libswresample
    libavutil
)

add_executable(program program.c)

target_link_libraries( program
    avformat
    avcodec
    avutil
    swscale
    swresample
)
