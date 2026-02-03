// api_server/src/main.cpp
//
// Minimal C++ HTTP API server for Space Sim
// - Uses cpp-httplib (header-only) + nlohmann::json (header-only)
// - Uses shared core library for DB/auth/session logic
//
// Endpoints:
//   GET  /health
//   GET  /db_ping
//   POST /register
//   POST /login        (sets HttpOnly cookie "sid")
//   GET  /me           (reads cookie "sid")
//   POST /logout       (deletes session + clears cookie)

#include <cstdlib>
#include <iostream>
#include <string>
#include <optional>

#include <sqlite3.h>

#include "httplib.h"
#include "json.hpp"

#include "auth.h"
#include "db.h"
#include "session.h"

using json = nlohmann::json;

// Repo-root relative DB path (we run server from repo root via ./build/api_server/api_server)
static const char* kDbPath = "./sim_server/data/game.db";

// -------------------------
// Small helpers
// -------------------------

// Very small cookie parser: finds "name=value" in the Cookie header.
static std::string get_cookie_value(const httplib::Request& req, const std::string& name) {
    const std::string cookie = req.get_header_value("Cookie");
    if (cookie.empty()) return {};

    const std::string needle = name + "=";
    size_t pos = cookie.find(needle);
    if (pos == std::string::npos) return {};
    pos += needle.size();

    size_t end = cookie.find(';', pos);
    std::string val = (end == std::string::npos) ? cookie.substr(pos) : cookie.substr(pos, end - pos);

    // trim leading spaces
    while (!val.empty() && val.front() == ' ') val.erase(val.begin());
    return val;
}

// Open DB + basic configure + ensure schema + apply migrations.
// Returns nullptr on failure.
static sqlite3* open_db_ready() {
    sqlite3* conn = db::open(kDbPath);
    if (!conn) return nullptr;

    bool ok = db::configure(conn)
           && db::ensure_schema_version_table(conn)
           && db::apply_migrations(conn);

    if (!ok) {
        db::close(conn);
        return nullptr;
    }
    return conn;
}

static void json_reply(httplib::Response& res, int status, const json& body) {
    res.status = status;
    res.set_content(body.dump(), "application/json");
}

static std::optional<int64_t> require_login(const httplib::Request& req, httplib::Response& res) {
    const std::string sid = get_cookie_value(req, "sid");
    if (sid.empty()) {
        json_reply(res, 401, {{"ok", false}, {"error", "not_logged_in"}});
        return std::nullopt;
    }

    sqlite3* conn = open_db_ready();
    if (!conn) {
        json_reply(res, 500, {{"ok", false}, {"error", "db_init_failed"}});
        return std::nullopt;
    }

    auto uid = session::get_user_id(conn, sid);
    if (!uid) {
        db::close(conn);
        json_reply(res, 401, {{"ok", false}, {"error", "not_logged_in"}});
        return std::nullopt;
    }

    session::touch(conn, sid);
    db::close(conn);
    return uid;
}

// -------------------------
// main
// -------------------------

int main() {
    httplib::Server server;

    // Pepper used with password hashing/verifying (keep stable per environment).
    // For production: set SPACE_SIM_PEPPER in systemd Environment= or env file.
    std::string pepper = "dev-pepper-change-me";
    if (const char* env = std::getenv("SPACE_SIM_PEPPER"); env) {
        pepper = env;
    }

    // GET /health -> {"ok":true}
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json out;
        out["ok"] = true;
        json_reply(res, 200, out);
    });

    // GET /db_ping -> verifies DB open/init works
    server.Get("/db_ping", [](const httplib::Request&, httplib::Response& res) {
        sqlite3* conn = open_db_ready();
        if (!conn) {
            json out;
            out["ok"] = false;
            out["db"] = "fail";
            out["error"] = "db_init_failed";
            json_reply(res, 500, out);
            return;
        }
        db::close(conn);

        json out;
        out["ok"] = true;
        out["db"] = "ok";
        json_reply(res, 200, out);
    });
    
server.Post("/admin/cleanup_sessions", [](const httplib::Request&, httplib::Response& res) {
    sqlite3* conn = open_db_ready();
    if (!conn) {
        json_reply(res, 500, {{"ok", false}, {"error", "db_init_failed"}});
        return;
    }

    int n = session::cleanup_expired(conn);
    db::close(conn);

    if (n < 0) {
        json_reply(res, 500, {{"ok", false}, {"error", "cleanup_failed"}});
        return;
    }

    json out;
    out["ok"] = true;
    out["deleted"] = n;
    json_reply(res, 200, out);
});

    // POST /register
    // Body: {"username":"...","email":"...","password":"..."}
    server.Post("/register", [&pepper](const httplib::Request& req, httplib::Response& res) {
        json in;
        try {
            in = json::parse(req.body);
        } catch (...) {
            json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}});
            return;
        }

        auto get_str = [&](const char* key) -> std::string {
            if (!in.contains(key) || !in[key].is_string()) return {};
            return in[key].get<std::string>();
        };

        const std::string username = get_str("username");
        const std::string email    = get_str("email");
        const std::string password = get_str("password");

        if (username.empty() || email.empty() || password.empty()) {
            json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}});
            return;
        }

        sqlite3* conn = open_db_ready();
        if (!conn) {
            json_reply(res, 500, {{"ok", false}, {"error", "db_init_failed"}});
            return;
        }

        const bool created = db::create_user_with_password(conn, username, email, password, pepper);
        db::close(conn);

        if (!created) {
            json_reply(res, 409, {{"ok", false}, {"error", "username_or_email_taken"}});
            return;
        }

        json_reply(res, 200, {{"ok", true}});
    });

    // POST /login
    // Body: {"user_or_email":"...","password":"..."}
    // Success: sets cookie "sid"
    server.Post("/login", [&pepper](const httplib::Request& req, httplib::Response& res) {
        json in;
        try {
            in = json::parse(req.body);
        } catch (...) {
            json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}});
            return;
        }

        auto get_str = [&](const char* key) -> std::string {
            if (!in.contains(key) || !in[key].is_string()) return {};
            return in[key].get<std::string>();
        };

        const std::string user_or_email = get_str("user_or_email");
        const std::string password      = get_str("password");

        if (user_or_email.empty() || password.empty()) {
            json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}});
            return;
        }

        sqlite3* conn = open_db_ready();
        if (!conn) {
            json_reply(res, 500, {{"ok", false}, {"error", "db_init_failed"}});
            return;
        }

        auto row = db::get_user_auth(conn, user_or_email);
        if (!row || row->is_active == 0) {
            db::close(conn);
            json_reply(res, 401, {{"ok", false}, {"error", "invalid_credentials"}});
            return;
        }

        if (!auth::verify_password(row->pass_hash, password, pepper)) {
            db::close(conn);
            json_reply(res, 401, {{"ok", false}, {"error", "invalid_credentials"}});
            return;
        }

        // Update last login time
        db::update_last_login(conn, row->id);

        // Create session (7 days)
        constexpr int ttl_seconds = 7 * 24 * 60 * 60;
        auto token = session::create(conn, row->id, ttl_seconds);
        if (!token) {
            db::close(conn);
            json_reply(res, 500, {{"ok", false}, {"error", "session_create_failed"}});
            return;
        }

        // Touch session immediately (optional, but fine)
        session::touch(conn, *token);

        // Close DB now that session is created
        db::close(conn);

        // Set cookie (HttpOnly prevents JS from reading it)
        // For HTTPS later, add "; Secure"
        const std::string set_cookie =
            "sid=" + *token +
            "; Path=/; Max-Age=" + std::to_string(ttl_seconds) +
            "; HttpOnly; SameSite=Lax";
        res.set_header("Set-Cookie", set_cookie);

        json out;
        out["ok"] = true;
        out["user_id"] = row->id;
        json_reply(res, 200, out);
    });

    // GET /me
    // Reads cookie "sid" and returns {"ok":true,"user_id":...}
    server.Get("/me", [](const httplib::Request& req, httplib::Response& res) {
        auto uid = require_login(req, res);
        if (!uid) return;

        json out;
        out["ok"] = true;
        out["user_id"] = *uid;
        json_reply(res, 200, out);
    });

    // POST /logout
    // Deletes session token and clears cookie
    server.Post("/logout", [](const httplib::Request& req, httplib::Response& res) {
        const std::string sid = get_cookie_value(req, "sid");

        if (!sid.empty()) {
            sqlite3* conn = open_db_ready();
            if (conn) {
                session::remove(conn, sid);
                db::close(conn);
            }
        }

        // Clear cookie
        res.set_header("Set-Cookie", "sid=; Path=/; Max-Age=0; HttpOnly; SameSite=Lax");
        json_reply(res, 200, {{"ok", true}});
    });

    server.Get("/whoami", [](const httplib::Request& req, httplib::Response& res) {
        auto uid = require_login(req, res);
        if (!uid) return;

        sqlite3* conn = open_db_ready();
        if (!conn) {
            json_reply(res, 500, {{"ok", false}, {"error", "db_init_failed"}});
            return;
        }

        auto prof = db::get_user_profile(conn, *uid);
        db::close(conn);

        if (!prof) {
            json_reply(res, 404, {{"ok", false}, {"error", "user_not_found"}});
            return;
        }

        json out;
        out["ok"] = true;
        out["user"] = {{"id", prof->id}, {"username", prof->username}, {"email", prof->email}};
        json_reply(res, 200, out);
    });

    // -------------------------
// /api/* aliases (for nginx + frontend)
// Keeps old routes working too.
// -------------------------

server.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
    json_reply(res, 200, {{"ok", true}});
});

// Alias /api/me -> /me
server.Get("/api/me", [](const httplib::Request& req, httplib::Response& res) {
    // Same body as /me
    auto uid = require_login(req, res);
    if (!uid) return;
    json_reply(res, 200, {{"ok", true}, {"user_id", *uid}});
});

// Alias /api/whoami -> /whoami
server.Get("/api/whoami", [](const httplib::Request& req, httplib::Response& res) {
    // Same body as /whoami
    auto uid = require_login(req, res);
    if (!uid) return;

    sqlite3* conn = open_db_ready();
    if (!conn) {
        json_reply(res, 500, {{"ok", false}, {"error", "db_init_failed"}});
        return;
    }

    auto prof = db::get_user_profile(conn, *uid);
    db::close(conn);

    if (!prof) {
        json_reply(res, 404, {{"ok", false}, {"error", "user_not_found"}});
        return;
    }

    json out;
    out["ok"] = true;
    out["user"] = {{"id", prof->id}, {"username", prof->username}, {"email", prof->email}};
    json_reply(res, 200, out);
});

// Alias /api/register -> /register
server.Post("/api/register", [&pepper](const httplib::Request& req, httplib::Response& res) {
    // Reuse exact logic from /register by calling same code inline:
    // (This is a minimal duplication approach.)
    json in;
    try { in = json::parse(req.body); }
    catch (...) { json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}}); return; }

    auto get_str = [&](const char* key) -> std::string {
        if (!in.contains(key) || !in[key].is_string()) return {};
        return in[key].get<std::string>();
    };

    const std::string username = get_str("username");
    const std::string email    = get_str("email");
    const std::string password = get_str("password");
    if (username.empty() || email.empty() || password.empty()) {
        json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}});
        return;
    }

    sqlite3* conn = open_db_ready();
    if (!conn) { json_reply(res, 500, {{"ok", false}, {"error", "db_init_failed"}}); return; }

    const bool created = db::create_user_with_password(conn, username, email, password, pepper);
    db::close(conn);

    if (!created) { json_reply(res, 409, {{"ok", false}, {"error", "username_or_email_taken"}}); return; }
    json_reply(res, 200, {{"ok", true}});
});

// Alias /api/login -> /login
server.Post("/api/login", [&pepper](const httplib::Request& req, httplib::Response& res) {
    // Copy of /login logic (same as your existing handler)
    json in;
    try { in = json::parse(req.body); }
    catch (...) { json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}}); return; }

    auto get_str = [&](const char* key) -> std::string {
        if (!in.contains(key) || !in[key].is_string()) return {};
        return in[key].get<std::string>();
    };

    const std::string user_or_email = get_str("user_or_email");
    const std::string password      = get_str("password");
    if (user_or_email.empty() || password.empty()) {
        json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}});
        return;
    }

    sqlite3* conn = open_db_ready();
    if (!conn) { json_reply(res, 500, {{"ok", false}, {"error", "db_init_failed"}}); return; }

    auto row = db::get_user_auth(conn, user_or_email);
    if (!row || row->is_active == 0) {
        db::close(conn);
        json_reply(res, 401, {{"ok", false}, {"error", "invalid_credentials"}});
        return;
    }

    if (!auth::verify_password(row->pass_hash, password, pepper)) {
        db::close(conn);
        json_reply(res, 401, {{"ok", false}, {"error", "invalid_credentials"}});
        return;
    }

    db::update_last_login(conn, row->id);

    constexpr int ttl_seconds = 7 * 24 * 60 * 60;
    auto token = session::create(conn, row->id, ttl_seconds);
    if (!token) {
        db::close(conn);
        json_reply(res, 500, {{"ok", false}, {"error", "session_create_failed"}});
        return;
    }

    session::touch(conn, *token);
    db::close(conn);

    const std::string set_cookie =
        "sid=" + *token +
        "; Path=/; Max-Age=" + std::to_string(ttl_seconds) +
        "; HttpOnly; SameSite=Lax";
    res.set_header("Set-Cookie", set_cookie);

    json_reply(res, 200, {{"ok", true}, {"user_id", row->id}});
});

// Alias /api/logout -> /logout
server.Post("/api/logout", [](const httplib::Request& req, httplib::Response& res) {
    const std::string sid = get_cookie_value(req, "sid");
    if (!sid.empty()) {
        sqlite3* conn = open_db_ready();
        if (conn) { session::remove(conn, sid); db::close(conn); }
    }
    res.set_header("Set-Cookie", "sid=; Path=/; Max-Age=0; HttpOnly; SameSite=Lax");
    json_reply(res, 200, {{"ok", true}});
});

// NEW: /api/cmd (frontend-friendly)
// Accepts either {"line":"time"} OR {"cmd":"time"}.
server.Post("/api/cmd", [&](const httplib::Request& req, httplib::Response& res) {
    auto uid = require_login(req, res);
    if (!uid) return;

    json in = json::parse(req.body, nullptr, false);
    if (in.is_discarded()) {
        json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}});
        return;
    }

    std::string cmd;
    if (in.contains("line") && in["line"].is_string()) cmd = in["line"].get<std::string>();
    else if (in.contains("cmd") && in["cmd"].is_string()) cmd = in["cmd"].get<std::string>();

    if (cmd.empty()) {
        json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}, {"text", "Missing field: line/cmd"}});
        return;
    }

    std::string internal_key = "dev-internal-key-change-me";
    if (const char* env = std::getenv("SPACE_SIM_INTERNAL_KEY"); env) internal_key = env;

    httplib::Client cli("127.0.0.1", 8090);
    cli.set_read_timeout(5, 0);
    cli.set_write_timeout(5, 0);

    json forward;
    forward["user_id"] = *uid;
    forward["cmd"] = cmd;

    httplib::Headers headers = {{"X-Internal-Key", internal_key}};
    auto r = cli.Post("/cmd", headers, forward.dump(), "application/json");
    if (!r) {
        json_reply(res, 502, {{"ok", false}, {"error", "sim_unreachable"}});
        return;
    }

    res.status = r->status;
    res.set_content(r->body, "application/json");
});

    server.Post("/cmd", [&](const httplib::Request& req, httplib::Response& res) {
        auto uid = require_login(req, res);
        if (!uid) return;

        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded() || !in.contains("cmd") || !in["cmd"].is_string()) {
            json_reply(res, 400, {{"ok", false}, {"error", "bad_request"}});
            return;
        }

        std::string internal_key = "dev-internal-key-change-me";
        if (const char* env = std::getenv("SPACE_SIM_INTERNAL_KEY"); env) {
            internal_key = env;
        }

        httplib::Client cli("127.0.0.1", 8090);
        cli.set_read_timeout(5, 0);
        cli.set_write_timeout(5, 0);

        json forward;
        forward["user_id"] = *uid;
        forward["cmd"] = in["cmd"].get<std::string>();

        httplib::Headers headers = {{"X-Internal-Key", internal_key}};

        auto r = cli.Post("/cmd", headers, forward.dump(), "application/json");
        if (!r) {
            json_reply(res, 502, {{"ok", false}, {"error", "sim_unreachable"}});
            return;
        }

        res.status = r->status;
        res.set_content(r->body, "application/json");
    });

    std::cout << "API server listening on 0.0.0.0:8080\n";
    server.listen("0.0.0.0", 8080);
    return 0;
}
