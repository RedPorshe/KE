
add_library(ke_engine SHARED)

# Устанавливаем свойство для экспорта/импорта
target_compile_definitions(ke_engine PRIVATE KE_ENGINE_BUILD)

# Инклюд пути - PUBLIC, чтобы были доступны игре/редактору
target_include_directories(ke_engine 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include    
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Vulkan/Managers    
)

# Собираем все исходники (вам нужно будет перечислить все .cpp файлы)
# Временное решение - можно использовать GLOB, но для больших проектов лучше явный список
file(GLOB_RECURSE ENGINE_SOURCES 
    "Source/*.cpp"
    "Source/*.c"
    "Source/*.cxx"
)

target_sources(ke_engine PRIVATE ${ENGINE_SOURCES} "Include/KE/EngineObject.h" "Include/CoreMinimal.h" "Include/Core/EngineTypes.h" "Include/KE/Systems/WindowSystem.h" "Source/KE/Systems/WindowSystem.cpp" "Include/KE/Systems/RenderSystem.h" "Source/KE/Systems/RenderSystem.cpp"  )

# Линковка с внешними библиотеками
target_link_libraries(ke_engine 
    PUBLIC
        Vulkan::Vulkan
        glfw
         rapidjson
    PRIVATE
        unofficial::shaderc::shaderc
        # Доп. библиотеки только для движка
)
