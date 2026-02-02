add_executable(sim_server
    src/main.cpp
    src/dev_universe.cpp
    src/internal_cmd_api.cpp
)
target_link_libraries(sim_server PRIVATE space_core)
target_include_directories(sim_server PRIVATE ${CMAKE_SOURCE_DIR}/core/include)