option(ENABLE_JSON "Enable JSON serialization support" OFF)

add_library(spread_logic STATIC
    src/field.cpp
    src/game.cpp
)

target_include_directories(spread_logic PUBLIC
    include
)

if(ENABLE_JSON)
    find_package(nlohmann_json REQUIRED)
    target_link_libraries(spread_logic PRIVATE nlohmann_json::nlohmann_json)
    target_compile_definitions(spread_logic PUBLIC SPREAD_LOGIC_ENABLE_JSON)
endif()
