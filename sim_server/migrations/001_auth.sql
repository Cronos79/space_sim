-- 001_auth.sql
PRAGMA foreign_keys=ON;

CREATE TABLE IF NOT EXISTS users (
  id            INTEGER PRIMARY KEY,
  username      TEXT NOT NULL COLLATE NOCASE,
  email         TEXT NOT NULL COLLATE NOCASE,
  pass_hash     TEXT NOT NULL,
  created_at    INTEGER NOT NULL,
  last_login_at INTEGER,
  is_active     INTEGER NOT NULL DEFAULT 1
);

CREATE UNIQUE INDEX IF NOT EXISTS ux_users_username ON users(username);
CREATE UNIQUE INDEX IF NOT EXISTS ux_users_email    ON users(email);
