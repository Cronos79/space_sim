add_executable(sim_server
    src/main.cpp
)
target_link_libraries(sim_server PRIVATE space_core)
