add_executable(editor)  # <-- ИЗМЕНЕНО

target_sources(editor PRIVATE  # <-- ИЗМЕНЕНО
    src/editormain.cpp
)

target_include_directories(editor PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/include
)

target_link_directories(editor PRIVATE ${CMAKE_BINARY_DIR}/lib)
target_link_libraries(editor PRIVATE ke_engine)

add_custom_command(TARGET editor POST_BUILD  # <-- ИЗМЕНЕНО
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:editor>/Assets  # <-- ИЗМЕНЕНО
)
