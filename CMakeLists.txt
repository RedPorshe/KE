cmake_minimum_required(VERSION 3.20)
project(KE VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_GAME "Build the game executable" ON)
option(BUILD_EDITOR "Build the editor executable" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Поиск библиотек через vcpkg
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED) 
find_package(unofficial-shaderc CONFIG REQUIRED)
message(STATUS "Found shaderc")

# Подпроекты
add_subdirectory(engine)

if(BUILD_GAME)
    add_subdirectory(game)
endif()

if(BUILD_EDITOR)
    add_subdirectory(editor)
endif()



