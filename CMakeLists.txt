cmake_minimum_required(VERSION 3.24)
project(IRanianMultiTheft VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_library(ires_engine STATIC
    engine/ires_engine.cpp
    engine/asset_formats.cpp
)
target_include_directories(ires_engine PUBLIC engine)
target_link_libraries(ires_engine PUBLIC raylib)

add_executable(irantheft src/main.cpp)
target_link_libraries(irantheft PRIVATE ires_engine)

add_executable(ires_editor tools/ires_editor.cpp)
target_link_libraries(ires_editor PRIVATE ires_engine)

add_executable(ires_packer tools/ires_packer.cpp engine/asset_formats.cpp)
target_include_directories(ires_packer PRIVATE engine)\ntarget_link_libraries(ires_packer PRIVATE raylib)\nadd_dependencies(irantheft ires_packer)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

add_custom_command(TARGET irantheft POST_BUILD
    COMMAND $<TARGET_FILE:ires_packer> ${CMAKE_BINARY_DIR}/assets ${CMAKE_BINARY_DIR}/game.iremg
    COMMENT "Packing IRES game assets into IREMG"
)

if(WIN32)
    target_link_libraries(irantheft PRIVATE winmm)
    target_link_libraries(ires_editor PRIVATE winmm)
endif()

if(MSVC)
    foreach(target ires_engine irantheft ires_editor ires_packer)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    endforeach()
else()
    foreach(target ires_engine irantheft ires_editor ires_packer)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()

install(TARGETS irantheft ires_editor ires_packer RUNTIME DESTINATION .)
