#!/bin/bash

mkdir build && cd build
conan install .. --build=missing
# если не хватает каких-то зависимостей то командой ниже можно разрешить конану самому их поставить 
# НО ЛУЧШЕ ТАК ДЕЛАТЬ
#conan install .. --build=missing -c tools.system.package_manager:mode=install
# Я ждал очень долго, чтобы в итоге не дали скачать за русскость

cmake --build .

