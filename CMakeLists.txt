cmake_minimum_required(VERSION 3.30)
project(Playerr)

# форс 17 (пока не знаю зачем), без фоллбека, без gnu
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# нужно для линковки
set(CMAKE_AUTOMOC ON)

# Ищем нужные модули Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_subdirectory(src)
