cmake_minimum_required(VERSION 3.5)

project(TCP_File_System)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Directories
set(SERVER_DIR server)
set(CLIENT_DIR client)
set(INCLUDE_DIR include)
set(BUILD_DIR bin)

# Source files
set(SERVER_SOURCES
    ${SERVER_DIR}/server.cpp
    ${SERVER_DIR}/server_upload.cpp
    ${SERVER_DIR}/server_directory.cpp
)

set(CLIENT_SOURCES
    ${CLIENT_DIR}/client.cpp
    ${CLIENT_DIR}/client_upload.cpp
)

# Add the include directory (globally for all targets)
include_directories(${INCLUDE_DIR})

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${BUILD_DIR})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Targets
add_executable(server ${SERVER_SOURCES})
add_executable(client ${CLIENT_SOURCES})

# Link pthread
target_link_libraries(server pthread)
target_link_libraries(client pthread)

# Custom Commands to Copy Directories After Building
add_custom_command(TARGET server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/server_directory
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_directory)

add_custom_command(TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/client_directory
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_directory)