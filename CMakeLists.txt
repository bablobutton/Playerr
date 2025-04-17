cmake_minimum_required(VERSION 3.8)

project(Playerr LANGUAGES CXX)

# Находим Qt6 (можно заменить на Qt5 если нужно)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Включаем автоматическое создание moc файлов
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Настройка исходников
set(SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.hpp  # Важно: добавляем .h файл в источники
)

# Создаем исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCES})

# Подключаем Qt модули
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Widgets
    Qt6::Core
)

# Для MacOS специфичные настройки
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.qtmainwindow"
    )
endif()