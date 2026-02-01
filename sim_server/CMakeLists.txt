cmake_minimum_required(VERSION 3.22)

project(space_sim_sim_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sim_server
    src/main.cpp
)

# Good warnings on GCC/Clang
target_compile_options(sim_server PRIVATE
    -Wall -Wextra -Wpedantic
)

# Threads (std::thread etc.)
find_package(Threads REQUIRED)
target_link_libraries(sim_server PRIVATE Threads::Threads)
