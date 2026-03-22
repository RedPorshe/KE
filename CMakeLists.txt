cmake_minimum_required(VERSION 4.1.2)
project(KE VERSION 0.1.0)

# Устанавливаем политику для совместимости со старыми проектами
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

# Включаем FetchContent
include(FetchContent)

# ============================================================================
# Автоматическая загрузка всех зависимостей из GitHub
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

# 2. RapidJSON - используем master ветку (более новая, совместима с CMake 4.x)
message(STATUS "Downloading RapidJSON...")
FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# Отключаем сборку лишних компонентов
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Устанавливаем минимальную версию CMake для совместимости
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

FetchContent_MakeAvailable(rapidjson)



# 4. Vulkan SDK (должен быть установлен в системе)
find_package(Vulkan REQUIRED)

# ============================================================================
# Настройка целей для линковки
# ============================================================================

# Vulkan цель
if(NOT TARGET Vulkan::Vulkan)
    add_library(Vulkan::Vulkan INTERFACE IMPORTED)
    target_include_directories(Vulkan::Vulkan INTERFACE ${vulkan_headers_SOURCE_DIR}/include)
    if(Vulkan_LIBRARY)
        target_link_libraries(Vulkan::Vulkan INTERFACE ${Vulkan_LIBRARY})
    endif()
endif()

# Создаем alias для RapidJSON если нужно
if(TARGET rapidjson AND NOT TARGET RapidJSON)
    add_library(RapidJSON INTERFACE)
    target_include_directories(RapidJSON INTERFACE ${rapidjson_SOURCE_DIR}/include)
endif()



message(STATUS "========================================")
message(STATUS "All dependencies downloaded successfully!")
message(STATUS "GLFW: ${glfw_SOURCE_DIR}")
message(STATUS "RapidJSON: ${rapidjson_SOURCE_DIR}")
message(STATUS "Vulkan-Headers: ${vulkan_headers_SOURCE_DIR}")
message(STATUS "========================================")
message(STATUS "Note: Vulkan SDK must be installed separately")
message(STATUS "Download from: https://vulkan.lunarg.com/")
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