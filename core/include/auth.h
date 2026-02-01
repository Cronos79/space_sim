#pragma once
#include <string>

namespace auth {

// Hash password using Argon2id and return the encoded hash string.
// pepper can be empty; for production you’ll set it from an env var.
std::string hash_password(const std::string& password, const std::string& pepper);

// Verify password against an encoded hash string.
bool verify_password(const std::string& encoded_hash,
                     const std::string& password,
                     const std::string& pepper);

} // namespace auth
