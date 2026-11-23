cmake_minimum_required(VERSION 3.24)
project(IRanianMultiTheft VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(ENET_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_Declare(enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG v1.3.18
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib enet)

add_executable(irantheft
    src/main.cpp
)
target_link_libraries(irantheft PRIVATE raylib enet)
target_include_directories(irantheft PRIVATE ${enet_SOURCE_DIR}/include)

if(WIN32)
    target_link_libraries(irantheft PRIVATE winmm)
endif()

if(MSVC)
    target_compile_options(irantheft PRIVATE /W4 /permissive-)
else()
    target_compile_options(irantheft PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS irantheft RUNTIME DESTINATION .)
