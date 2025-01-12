cmake_minimum_required(VERSION 3.5)

project(TCP_File_System)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

enable_testing()
add_subdirectory(tests/googletest)

# Directories
set(SERVER_DIR server)
set(CLIENT_DIR client)
set(SERVER_DB_DIR ${PROJECT_SOURCE_DIR}/server_directory)
set(CLIENT_DB_DIR ${PROJECT_SOURCE_DIR}/client_directory)

set(INCLUDE_DIR include)
set(TESTS_DIR tests)
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

set(CLIENT_TEST_SOURCES
    ${TESTS_DIR}/clientTests.cpp
)

set(CLIENT_TEST_TEM_SOURCES
    ${TESTS_DIR}/clientTestsTemplate.cpp
)

# Add the include directory (globally for all targets)
include_directories(${INCLUDE_DIR})

# Output directory
# Bare in mind that relative paths 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${BUILD_DIR})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Targets
add_executable(server ${SERVER_SOURCES})
add_executable(client ${CLIENT_SOURCES})
add_executable(clientTests ${CLIENT_TEST_SOURCES})
add_executable(clientTestsTem ${CLIENT_TEST_TEM_SOURCES})

# Link pthread
target_link_libraries(server pthread)
target_link_libraries(client pthread)
target_link_libraries(clientTests gtest gtest_main pthread)

target_include_directories(server PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(client PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(clientTests PRIVATE "${PROJECT_SOURCE_DIR}/include")

target_compile_definitions(client PRIVATE 
    CLIENT_DB_PATH="${CLIENT_DB_DIR}"
)

target_compile_definitions(server PUBLIC 
    SERVER_DB_PATH="${SERVER_DB_DIR}"
)

target_compile_definitions(clientTests PRIVATE 
    SERVER_EXECUTABLE="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server"
    SERVER_DB_PATH="${SERVER_DB_DIR}"
    CLIENT_DB_PATH="${CLIENT_DB_DIR}"
)

target_compile_definitions(clientTestsTem PRIVATE 
    SERVER_EXECUTABLE="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server"
    SERVER_DB_PATH="${SERVER_DB_DIR}"
    CLIENT_DB_PATH="${CLIENT_DB_DIR}"
)

add_test(NAME ClientTests COMMAND clientTests)