#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct sqlite3;

namespace session {

// Creates a new session and returns the token.
// ttl_seconds controls expiry (e.g. 7 days).
std::optional<std::string> create(sqlite3* conn, int64_t user_id, int ttl_seconds);

// Returns user_id if session exists and is not expired.
std::optional<int64_t> get_user_id(sqlite3* conn, const std::string& token);

// Updates last_seen_at (optional but nice).
bool touch(sqlite3* conn, const std::string& token);

// Deletes a session token.
bool remove(sqlite3* conn, const std::string& token);

// Deletes expired sessions. Returns number of rows deleted, or -1 on error.
int cleanup_expired(sqlite3* conn);

} // namespace session
