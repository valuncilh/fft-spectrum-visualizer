cmake_minimum_required(VERSION 3.10)
project(FFTVisualizer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.c
    src/graphics.c
    src/fft_processor.c
    # src/kiss_fft.c   # удаляем
)

add_executable(fft_visualizer ${SOURCES})
target_include_directories(fft_visualizer PRIVATE include)

if(MSVC)
    target_compile_definitions(fft_visualizer PRIVATE 
        _USE_MATH_DEFINES 
        _CRT_SECURE_NO_WARNINGS
    )
    target_link_libraries(fft_visualizer comdlg32 user32)
else()
    target_link_libraries(fft_visualizer m)
endif()