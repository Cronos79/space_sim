#include <unistd.h>
#include "db.h"
#include "auth.h"

#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char** argv) {
    std::string db_path = "./data/game.db";

    std::string pepper = "dev-pepper-change-me";
    if (const char* env = std::getenv("SPACE_SIM_PEPPER"); env) {
        pepper = env;
    }

    // Simple arg: --db path
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--db") {
            db_path = argv[i + 1];
        }
    }

    sqlite3* conn = db::open(db_path);
    if (!conn) return 1;

    if (!db::configure(conn)) {
        db::close(conn);
        return 1;
    }

    if (!db::ensure_schema_version_table(conn)) {
        db::close(conn);
        return 1;
    }

    if (!db::apply_migrations(conn)) {
        db::close(conn);
        return 1;
    }

    std::cout << "DB ready: " << db_path << "\n";

        // Dev command: --login <user_or_email> <password>
    for (int i = 1; i + 2 < argc; ++i) {
        if (std::string(argv[i]) == "--login") {
            std::string user_or_email = argv[i + 1];
            std::string password = argv[i + 2];

            auto row = db::get_user_auth(conn, user_or_email);
            if (!row || row->is_active == 0) {
                std::cout << "login failed\n";
                db::close(conn);
                return 3;
            }

            if (!auth::verify_password(row->pass_hash, password, pepper)) {
                std::cout << "login failed\n";
                db::close(conn);
                return 3;
            }

            db::update_last_login(conn, row->id);
            std::cout << "login ok user_id=" << row->id << "\n";
            db::close(conn);
            return 0;
        }
    }

        // Dev command: --create-user-pw <username> <email> <password>
    for (int i = 1; i + 3 < argc; ++i) {
        if (std::string(argv[i]) == "--create-user-pw") {
            std::string username = argv[i + 1];
            std::string email = argv[i + 2];
            std::string password = argv[i + 3];

            bool ok = db::create_user_with_password(conn, username, email, password, pepper);
            std::cout << (ok ? "user created\n" : "user not created (exists or error)\n");

            db::close(conn);
            return ok ? 0 : 2;
        }
    }

        // Dev command: --create-user <username> <email>
    for (int i = 1; i + 2 < argc; ++i) {
        if (std::string(argv[i]) == "--create-user") {
            std::string username = argv[i + 1];
            std::string email = argv[i + 2];

            bool ok = db::create_user(conn, username, email, "TEMP_HASH");
            std::cout << (ok ? "user created\n" : "user not created (exists or error)\n");

            db::close(conn);
            return ok ? 0 : 2;
        }
    }

    db::close(conn);
    return 0;
}
