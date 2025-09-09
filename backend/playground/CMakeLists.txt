cmake_minimum_required(VERSION 3.20)
project(spread_playground LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(spread_playground
    main.cpp
)

# Add the backend logic as a subdirectory if building from repo root
add_subdirectory(../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

# Link against the logic library
target_link_libraries(spread_playground PRIVATE spread_logic)
