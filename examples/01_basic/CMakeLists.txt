cmake_minimum_required(VERSION 3.15)

project(BasicExample)

add_executable(BasicExample main.cpp)

target_link_libraries(BasicExample PRIVATE TomatoEngine)

# Copy resources if needed
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources)
    add_custom_command(TARGET BasicExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:BasicExample>/resources
    )
endif()
