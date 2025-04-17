cmake_minimum_required(VERSION 3.8)
project(Playerr)

# подключение conan toolchain
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

# Ищем нужные модули Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Заголовки
include_directories(include)

# Добавляем исполняемый файл
add_executable(QtApp
    src/main.cpp
    src/MainWindow.cpp
)

# Линкуем с Qt
target_link_libraries(QtApp PRIVATE Qt6::Widgets)
