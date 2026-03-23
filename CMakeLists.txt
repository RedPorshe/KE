cmake_minimum_required(VERSION 4.1.2)
project(KE VERSION 0.1.0)

if(POLICY CMP0126)
    cmake_policy(SET CMP0126 OLD)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_GAME "Build the game executable" ON)
option(BUILD_EDITOR "Build the editor executable" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

# ============================================================================
# Автоматическая загрузка зависимостей
# ============================================================================

# 1. GLFW
message(STATUS "Downloading GLFW...")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

# 2. RapidJSON (header-only)
message(STATUS "Downloading RapidJSON...")
FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(rapidjson)

# 3. Vulkan SDK (должен быть установлен в системе)
find_package(Vulkan REQUIRED)

# ============================================================================
# Настройка целей для линковки
# ============================================================================

# Создаём интерфейсную библиотеку для RapidJSON
if(NOT TARGET RapidJSON)
    add_library(RapidJSON INTERFACE)
    target_include_directories(RapidJSON INTERFACE ${rapidjson_SOURCE_DIR}/include)
    message(STATUS "RapidJSON configured: ${rapidjson_SOURCE_DIR}/include")
endif()

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

message(STATUS "========================================")
message(STATUS "Dependencies downloaded successfully!")
message(STATUS "GLFW: ${glfw_SOURCE_DIR}")
message(STATUS "RapidJSON: ${rapidjson_SOURCE_DIR}")
message(STATUS "========================================")
message(STATUS "Note: Vulkan SDK must be installed separately")
message(STATUS "========================================")

# ============================================================================
# Подпроекты
# ============================================================================
add_subdirectory(engine)

if(BUILD_GAME)
    add_subdirectory(game)
endif()

if(BUILD_EDITOR)
    add_subdirectory(editor)
endif()