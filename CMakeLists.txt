cmake_minimum_required(VERSION 3.21)

project(VampireSurvLike
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

if(TARGET sfml-graphics AND NOT TARGET SFML::Graphics)
    add_library(SFML::Graphics ALIAS sfml-graphics)
endif()

if(TARGET sfml-window AND NOT TARGET SFML::Window)
    add_library(SFML::Window ALIAS sfml-window)
endif()

if(TARGET sfml-system AND NOT TARGET SFML::System)
    add_library(SFML::System ALIAS sfml-system)
endif()

add_executable(VampireSurvLike
    src/main.cpp
    src/app/Application.cpp
    src/assets/AssetManager.cpp
    src/ecs/World.cpp
    src/game/Balance.cpp
    src/game/DefeatPassives.cpp
    src/game/Perks.cpp
    src/game/PersistentState.cpp
    src/screens/BaseScreen.cpp
    src/screens/RunScreen.cpp
)

# Smoke test executable. Covers the SFML-free modules: ECS, PersistentState
# + save/load, Perks, DefeatPassives, Balance. RunScreen / BaseScreen are
# excluded because they pull in SFML and a font dependency.
add_executable(VampireSurvLikeTests
    tests/test_main.cpp
    tests/test_world.cpp
    tests/test_persistent_state.cpp
    tests/test_save_system.cpp
    tests/test_perks.cpp
    tests/test_defeat_passives.cpp
    tests/test_balance.cpp
    src/ecs/World.cpp
    src/game/Balance.cpp
    src/game/DefeatPassives.cpp
    src/game/Perks.cpp
    src/game/PersistentState.cpp
)

target_include_directories(VampireSurvLikeTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

if(MSVC)
    target_compile_options(VampireSurvLikeTests PRIVATE /W4 /permissive-)
else()
    target_compile_options(VampireSurvLikeTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(VampireSurvLike
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(VampireSurvLike
    PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
)

if(MSVC)
    target_compile_options(VampireSurvLike PRIVATE /W4 /permissive-)
else()
    target_compile_options(VampireSurvLike PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    add_custom_command(TARGET VampireSurvLike POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:VampireSurvLike>
            $<TARGET_FILE_DIR:VampireSurvLike>
        COMMAND_EXPAND_LISTS
    )
endif()
