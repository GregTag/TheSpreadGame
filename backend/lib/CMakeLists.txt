add_library(spread_logic STATIC
    src/field.cpp
    src/game.cpp
)

target_include_directories(spread_logic PUBLIC
    include
)
