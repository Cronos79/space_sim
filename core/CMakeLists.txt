add_library(space_core STATIC
    src/db.cpp
    src/auth.cpp
    src/session.cpp
)

target_include_directories(space_core PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(space_core PUBLIC
    sqlite3
    argon2
)
