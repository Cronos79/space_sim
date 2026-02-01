#include "db.h"

#include <filesystem>
#include <iostream>

#include <fstream>
#include <sstream>

#include "auth.h"

namespace db {

static bool exec(sqlite3* conn, const char* sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(conn, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << (err ? err : "(unknown)") << "\n";
        sqlite3_free(err);
        return false;
    }
    return true;
}

sqlite3* open(const std::string& path) {
    // Ensure parent dir exists
    std::filesystem::path p(path);
    if (p.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(p.parent_path(), ec);
    }

    sqlite3* conn = nullptr;
    if (sqlite3_open(path.c_str(), &conn) != SQLITE_OK) {
        std::cerr << "sqlite3_open failed: " << sqlite3_errmsg(conn) << "\n";
        sqlite3_close(conn);
        return nullptr;
    }
    return conn;
}

bool configure(sqlite3* conn) {
    // Solid defaults for a server (good concurrency + fewer lock errors)
    // WAL is good even early on.
    if (!exec(conn, "PRAGMA journal_mode=WAL;")) return false;
    if (!exec(conn, "PRAGMA synchronous=NORMAL;")) return false;
    if (!exec(conn, "PRAGMA foreign_keys=ON;")) return false;
    if (!exec(conn, "PRAGMA busy_timeout=5000;")) return false;
    return true;
}

bool ensure_schema_version_table(sqlite3* conn) {
    // Minimal table for migrations later
    const char* sql =
        "CREATE TABLE IF NOT EXISTS schema_version ("
        "  version INTEGER PRIMARY KEY NOT NULL"
        ");";
    return exec(conn, sql);
}

bool apply_sql_file(sqlite3* conn, const std::string& sql_file_path) {
    std::ifstream f(sql_file_path, std::ios::binary);
    if (!f) {
        std::cerr << "cannot open sql file: " << sql_file_path << "\n";
        return false;
    }

    std::ostringstream ss;
    ss << f.rdbuf();
    std::string sql = ss.str();

    if (sql.empty()) {
        std::cerr << "sql file empty: " << sql_file_path << "\n";
        return false;
    }

    if (!exec(conn, "BEGIN;")) return false;
    if (!exec(conn, sql.c_str())) { exec(conn, "ROLLBACK;"); return false; }
    if (!exec(conn, "COMMIT;")) return false;
    return true;
}

int get_schema_version(sqlite3* conn) {
    // Ensure row exists; if not, treat as version 0
    const char* sql = "SELECT version FROM schema_version LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return 0;
    }

    int version = 0;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        version = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return version;
}

bool set_schema_version(sqlite3* conn, int v) {
    // Keep exactly one row
    if (!exec(conn, "DELETE FROM schema_version;")) return false;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT INTO schema_version(version) VALUES(?);";
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, v);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "insert schema_version failed: " << sqlite3_errmsg(conn) << "\n";
        return false;
    }
    return true;
}

bool apply_migrations(sqlite3* conn) {
    int v = get_schema_version(conn);

    // v==0: fresh DB (or old DB without version row)
    if (v < 1) {
        std::cout << "Applying migration v1: 001_auth.sql\n";
        if (!apply_sql_file(conn, "./migrations/001_auth.sql")) return false;
        if (!set_schema_version(conn, 1)) return false;
        v = 1;
    }

    if (v < 2) {
        std::cout << "Applying migration v2: 002_sessions.sql\n";
        if (!apply_sql_file(conn, "./sim_server/migrations/002_sessions.sql")) return false;
        if (!set_schema_version(conn, 2)) return false;
        v = 2;
    }

    // Later: if (v < 2) { apply 002...; set to 2; }
    return true;
}

bool create_user(sqlite3* conn,
                 const std::string& username,
                 const std::string& email,
                 const std::string& pass_hash) {
    const char* sql =
        "INSERT INTO users(username, email, pass_hash, created_at) "
        "VALUES(?, ?, ?, strftime('%s','now'));";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, pass_hash.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) return true;

    if (rc == SQLITE_CONSTRAINT) {
        // unique username/email violated
        return false;
    }

    std::cerr << "insert user failed: " << sqlite3_errmsg(conn) << "\n";
    return false;
}

bool create_user_with_password(sqlite3* conn,
                               const std::string& username,
                               const std::string& email,
                               const std::string& password,
                               const std::string& pepper) {
    std::string encoded = auth::hash_password(password, pepper);
    return create_user(conn, username, email, encoded);
}

std::optional<UserAuthRow> get_user_auth(sqlite3* conn, const std::string& user_or_email) {
    const char* sql =
        "SELECT id, pass_hash, is_active "
        "FROM users "
        "WHERE username = ? OR email = ? "
        "LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, user_or_email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user_or_email.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    UserAuthRow row{};
    row.id = sqlite3_column_int64(stmt, 0);

    const unsigned char* txt = sqlite3_column_text(stmt, 1);
    row.pass_hash = txt ? reinterpret_cast<const char*>(txt) : "";

    row.is_active = sqlite3_column_int(stmt, 2);

    sqlite3_finalize(stmt);
    return row;
}

bool update_last_login(sqlite3* conn, int64_t user_id) {
    const char* sql = "UPDATE users SET last_login_at = strftime('%s','now') WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return false;
    }

    sqlite3_bind_int64(stmt, 1, user_id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::optional<UserProfile> get_user_profile(sqlite3* conn, int64_t user_id) {
    const char* sql = "SELECT id, username, email FROM users WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return std::nullopt;
    }

    sqlite3_bind_int64(stmt, 1, user_id);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    UserProfile p{};
    p.id = sqlite3_column_int64(stmt, 0);

    const unsigned char* u = sqlite3_column_text(stmt, 1);
    const unsigned char* e = sqlite3_column_text(stmt, 2);

    p.username = u ? reinterpret_cast<const char*>(u) : "";
    p.email    = e ? reinterpret_cast<const char*>(e) : "";

    sqlite3_finalize(stmt);
    return p;
}

void close(sqlite3* conn) {
    if (conn) sqlite3_close(conn);
}

} // namespace db
