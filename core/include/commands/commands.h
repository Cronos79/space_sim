#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>

namespace commands {

struct Context {
    int32_t user_id = 0;
};

struct Result {
    bool ok = true;
    std::string text;
    std::string error_code;
    std::vector<std::string> lines;
};

struct Command {
    std::string verb;
    std::vector<std::string> args;
};

using Handler = std::function<Result(const Context&, const Command&)>;

enum class Access : uint8_t { Read, Write };

class Router {
public:
    // Default = Read (keeps existing call sites working)
    bool add(std::string verb, Handler handler, Access access = Access::Read);

    Result execute_line(const Context& ctx, const std::string& line) const;
    Result execute(const Context& ctx, const Command& cmd) const;

    // Query whether a verb is read/write (for lock selection)
    std::optional<Access> access_for(const std::string& verb) const;

private:
    struct Entry {
        Handler handler;
        Access access = Access::Read;
    };

    std::unordered_map<std::string, Entry> handlers_;

    static std::optional<Command> parse_line(const std::string& line);
    static std::string normalize_verb(std::string v);
};

} // namespace commands
