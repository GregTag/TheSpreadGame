cmake_minimum_required(VERSION 3.26)
project(SpreadServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build tests" OFF)

# Find dependencies
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

add_subdirectory(lib)

# Find all source files
file(GLOB_RECURSE SOURCE_FILES
    src/*.cpp
)

add_executable(spread_server ${SOURCE_FILES})

target_include_directories(spread_server PRIVATE include)

target_link_libraries(spread_server PRIVATE Boost::system Threads::Threads nlohmann_json::nlohmann_json spdlog::spdlog spread_logic)

target_compile_definitions(spread_server PRIVATE BOOST_ASIO_HAS_STD_COROUTINE)

# Install
install(TARGETS spread_server RUNTIME DESTINATION bin)
