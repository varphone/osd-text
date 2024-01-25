# Minimum required CMake version
cmake_minimum_required(VERSION 3.12)

# Project name
project(osd-text VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your source files here
set(SOURCES
    src/main.cpp
    src/stb_image_write.h
    src/stb_truetype.h
    src/text.cpp
)

# Add executable target
add_executable(${PROJECT_NAME} ${SOURCES})

# Set include directories
target_include_directories(${PROJECT_NAME} PUBLIC)
