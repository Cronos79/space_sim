#include "session.h"

#include <sqlite3.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace session {

static bool read_urandom(void* out, size_t n) {
    std::ifstream ur("/dev/urandom", std::ios::binary);
    if (!ur) return false;
    ur.read(reinterpret_cast<char*>(out), static_cast<std::streamsize>(n));
    return static_cast<size_t>(ur.gcount()) == n;
}

static std::string to_hex(const unsigned char* data, size_t n) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    s.resize(n * 2);
    for (size_t i = 0; i < n; ++i) {
        s[i * 2 + 0] = hex[(data[i] >> 4) & 0xF];
        s[i * 2 + 1] = hex[(data[i] >> 0) & 0xF];
    }
    return s;
}

static bool exec_stmt(sqlite3* conn, sqlite3_stmt* stmt) {
    int rc = sqlite3_step(stmt);
    return rc == SQLITE_DONE || rc == SQLITE_ROW;
}

std::optional<std::string> create(sqlite3* conn, int64_t user_id, int ttl_seconds) {
    // 32 bytes -> 64 hex chars
    unsigned char raw[32];
    if (!read_urandom(raw, sizeof(raw))) {
        std::cerr << "session: failed to read /dev/urandom\n";
        return std::nullopt;
    }
    std::string token = to_hex(raw, sizeof(raw));

    const char* sql =
        "INSERT INTO sessions(token, user_id, created_at, expires_at, last_seen_at) "
        "VALUES(?, ?, strftime('%s','now'), strftime('%s','now') + ?, strftime('%s','now'));";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "session: prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, user_id);
    sqlite3_bind_int(stmt, 3, ttl_seconds);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        // Extremely unlikely: token collision
        std::cerr << "session: insert failed: " << sqlite3_errmsg(conn) << "\n";
        return std::nullopt;
    }

    return token;
}

std::optional<int64_t> get_user_id(sqlite3* conn, const std::string& token) {
    const char* sql =
        "SELECT user_id "
        "FROM sessions "
        "WHERE token = ? AND expires_at > strftime('%s','now') "
        "LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "session: prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    int64_t user_id = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);
    return user_id;
}

bool touch(sqlite3* conn, const std::string& token) {
    const char* sql =
        "UPDATE sessions SET last_seen_at = strftime('%s','now') "
        "WHERE token = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "session: prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool remove(sqlite3* conn, const std::string& token) {
    const char* sql = "DELETE FROM sessions WHERE token = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "session: prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

int cleanup_expired(sqlite3* conn) {
    const char* sql = "DELETE FROM sessions WHERE expires_at <= strftime('%s','now');";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "session: cleanup prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return -1;
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "session: cleanup step failed: " << sqlite3_errmsg(conn) << "\n";
        return -1;
    }

    return sqlite3_changes(conn);
}

} // namespace session
