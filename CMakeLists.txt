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
    src/screens/BaseScreen.cpp
    src/screens/RunScreen.cpp
)

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
