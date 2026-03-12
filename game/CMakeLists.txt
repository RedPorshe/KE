add_executable(game) 

target_sources(game PRIVATE 
    src/gamemain.cpp
 "src/test.h" "src/test.cpp")

target_include_directories(game PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/include
)

target_link_libraries(game PRIVATE ke_engine)  

add_custom_command(TARGET game POST_BUILD  
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:game>/Assets  
)