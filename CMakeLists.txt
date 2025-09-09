cmake_minimum_required(VERSION 3.16)
project(Fow)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Copy assets to work directory

file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/assets)
file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/Release/assets)
file(REMOVE_RECURSE DESTINATION ${CMAKE_BINARY_DIR}/Debug/assets)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/Release)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/Debug)

# Fetching raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # Disable raylib examples
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)    # Disable raylib example games

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib)

# Fetching raylib-cpp
FetchContent_Declare(
    raylib_cpp
    GIT_REPOSITORY "https://github.com/RobLoach/raylib-cpp.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib_cpp)

file(GLOB_RECURSE RAYLIB_CPP_HEADERS ${CMAKE_BINARY_DIR}/_deps/raylib_cpp-src/include/*.hpp)
add_custom_target(raylib_cpp_sources SOURCES ${RAYLIB_CPP_HEADERS})

# Adding source files
set(SOURCE_LIST

src/core/core.cc
src/core/core.h
src/core/screen/screen.cc
src/core/screen/screen.h
src/core/screen/screen_controller.cc
src/core/screen/screen_controller.h

src/drawable/button/button.cc
src/drawable/button/button.h
src/drawable/button/text_button.cc
src/drawable/button/text_button.h
src/drawable/button/texture_button.cc
src/drawable/button/texture_button.h
src/drawable/complex_drawable.cc
src/drawable/complex_drawable.h
src/drawable/drawable.cc
src/drawable/drawable.h
src/drawable/rectangle.cc
src/drawable/rectangle.h
src/drawable/text.cc
src/drawable/text.h
src/drawable/texture.cc
src/drawable/texture.h
src/drawable/texture_manager.h

src/gui/window.cc
src/gui/window.h

src/input/input.cc
src/input/input.h

src/main.cc

src/match/map/map.cc
src/match/map/map.h
src/match/map/terrain.cc
src/match/map/terrain.h
src/match/map/tile.cc
src/match/map/tile.h
src/match/match.cc
src/match/match.h
src/match/match_screen.cc
src/match/match_screen.h
src/match/player.cc
src/match/player.h
src/match/targets/bridge.cc
src/match/targets/bridge.h
src/match/targets/road.cc
src/match/targets/road.h
src/match/targets/target.cc
src/match/targets/target.h
src/match/targets/units/unit.cc
src/match/targets/units/unit.h

src/menu/menu_screen.cc
src/menu/menu_screen.h

src/structs/texture_states.h
src/structs/vector2i.h

src/utils/math_utils.h
src/utils/set_operations.h
src/utils/astar.h
src/utils/astar.cpp

)

add_executable(Fow ${SOURCE_LIST})

# Organize source files into folders for Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})
source_group(TREE ${CMAKE_BINARY_DIR}/_deps/raylib_cpp-src/include FILES ${RAYLIB_CPP_HEADERS})

# Set the startup project for Visual Studio
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "Fow")

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC raylib raylib_cpp)
