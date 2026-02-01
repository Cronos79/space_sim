#pragma once

#include <sqlite3.h>
#include <string>
#include <optional>
#include <cstdint>

namespace db {

    struct UserAuthRow {
    int64_t id;
    std::string pass_hash;
    int is_active;
};

struct UserProfile {
    int64_t id;
    std::string username;
    std::string email;
};

// Opens (or creates) the database file.
// Returns nullptr on failure.
sqlite3* open(const std::string& path);

// Applies basic PRAGMAs for server use (WAL, busy_timeout, etc.)
bool configure(sqlite3* conn);

// Creates the schema_version table if it doesn't exist.
bool ensure_schema_version_table(sqlite3* conn);

// Update sql from migration
bool apply_sql_file(sqlite3* conn, const std::string& sql_file_path);

int get_schema_version(sqlite3* conn);          // returns 0 if unset / empty
bool set_schema_version(sqlite3* conn, int v);  // sets to v (single-row)

bool apply_migrations(sqlite3* conn);           // applies known migrations

// Inserts a user. Returns true if created, false if username/email already exists or error.
bool create_user(sqlite3* conn, const std::string& username, const std::string& email, const std::string& pass_hash);

// Creates user from plaintext password (hashes internally)
bool create_user_with_password(sqlite3* conn,
                               const std::string& username,
                               const std::string& email,
                               const std::string& password,
                               const std::string& pepper);

std::optional<UserAuthRow> get_user_auth(sqlite3* conn, const std::string& user_or_email);

bool update_last_login(sqlite3* conn, int64_t user_id);

std::optional<UserProfile> get_user_profile(sqlite3* conn, int64_t user_id);

// Close DB (safe on nullptr)
void close(sqlite3* conn);

} // namespace db
