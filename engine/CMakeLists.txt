# engine/CMakeLists.txt

add_library(ke_engine SHARED)

# Устанавливаем свойство для экспорта/импорта
target_compile_definitions(ke_engine PRIVATE KE_ENGINE_BUILD)

# Инклюд пути
target_include_directories(ke_engine 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Vulkan/Managers
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Собираем все исходники
file(GLOB_RECURSE ENGINE_SOURCES 
    "Source/*.cpp"
    "Source/*.c"
    "Source/*.cxx"
)

target_include_directories(ke_engine PUBLIC
    ${rapidjson_SOURCE_DIR}/include
)

target_sources(ke_engine PRIVATE ${ENGINE_SOURCES})

# Линковка с внешними библиотеками
target_link_libraries(ke_engine 
    PUBLIC
        Vulkan::Vulkan
        glfw
        RapidJSON
)

# Для Windows экспорт символов
if(WIN32)
    target_compile_definitions(ke_engine PRIVATE KE_ENGINE_EXPORTS)
endif()

# Настройка выходных директорий
set_target_properties(ke_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)