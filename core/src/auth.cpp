#include "auth.h"

#include <argon2.h>
#include <fstream>
#include <iostream>

namespace auth {

static void read_urandom(void* out, size_t n) {
    std::ifstream ur("/dev/urandom", std::ios::binary);
    if (!ur) {
        std::cerr << "failed to open /dev/urandom\n";
        std::exit(1);
    }
    ur.read(reinterpret_cast<char*>(out), static_cast<std::streamsize>(n));
    if (!ur) {
        std::cerr << "failed to read /dev/urandom\n";
        std::exit(1);
    }
}

std::string hash_password(const std::string& password, const std::string& pepper) {
    // Reasonable dev defaults. We can tune later.
    const uint32_t time_cost = 3;
    const uint32_t memory_cost_kib = 1 << 16; // 64 MiB
    const uint32_t parallelism = 1;

    unsigned char salt[16];
    read_urandom(salt, sizeof(salt));

    // encoded includes params + salt + hash
    char encoded[256];

    std::string pw = password + pepper;

    int rc = argon2id_hash_encoded(
        time_cost,
        memory_cost_kib,
        parallelism,
        pw.data(),
        pw.size(),
        salt,
        sizeof(salt),
        32, // hash length
        encoded,
        sizeof(encoded));

    if (rc != ARGON2_OK) {
        std::cerr << "argon2id_hash_encoded failed: " << argon2_error_message(rc) << "\n";
        std::exit(1);
    }

    return std::string(encoded);
}

bool verify_password(const std::string& encoded_hash,
                     const std::string& password,
                     const std::string& pepper) {
    std::string pw = password + pepper;
    int rc = argon2id_verify(encoded_hash.c_str(), pw.data(), pw.size());
    return rc == ARGON2_OK;
}

} // namespace auth
