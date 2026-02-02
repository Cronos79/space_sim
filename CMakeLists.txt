cmake_minimum_required(VERSION 3.22)
project(space_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(core)
add_subdirectory(sim_server)
add_subdirectory(api_server)
add_subdirectory(tools/db_tool)
