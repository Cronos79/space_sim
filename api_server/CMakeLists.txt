add_executable(api_server
    src/main.cpp
)

target_include_directories(api_server PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../third_party
)

target_link_libraries(api_server PRIVATE
    space_core
)
