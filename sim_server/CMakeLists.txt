add_executable(sim_server
    src/main.cpp
    src/dev_universe.cpp
    src/internal_cmd_api.cpp
    src/universe_generator.cpp
    src/cmd_time.cpp
    src/cmd_time_utils.cpp
)
target_link_libraries(sim_server PRIVATE space_core)
target_include_directories(sim_server PRIVATE
    ${PROJECT_SOURCE_DIR}/core/include
    ${PROJECT_SOURCE_DIR}/sim_server/include
    ${CMAKE_CURRENT_LIST_DIR}/../third_party
)

target_compile_definitions(sim_server PRIVATE
    SPACE_SIM_INTERNAL_KEY_DEFAULT="dev123"
)
