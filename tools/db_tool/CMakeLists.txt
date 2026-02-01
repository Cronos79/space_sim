add_executable(db_tool
    src/main.cpp
)

target_link_libraries(db_tool PRIVATE space_core)
