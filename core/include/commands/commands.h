#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>

namespace commands {

// A tiny context you can expand later.
// Keep it "engine-facing" not HTTP-facing.
struct Context {
    int32_t user_id = 0;
    // Later: permissions, current location, etc.
};

// What your text UI wants back every time.
struct Result {
    bool ok = true;
    std::string text;                 // ready to print in UI
    std::string error_code;           // machine readable (optional)
    std::vector<std::string> lines;   // optional: structured text lines
};

// Parsed command: verb + args.
struct Command {
    std::string verb;
    std::vector<std::string> args;
};

// Handler signature
using Handler = std::function<Result(const Context&, const Command&)>;

class Router {
public:
    // register a command verb, e.g. "route"
    bool add(std::string verb, Handler handler);

    // Execute a raw text command line, e.g. "route sol kelnor"
    Result execute_line(const Context& ctx, const std::string& line) const;

    // Optional: execute already-parsed
    Result execute(const Context& ctx, const Command& cmd) const;

private:
    std::unordered_map<std::string, Handler> handlers_;

    static std::optional<Command> parse_line(const std::string& line);
    static std::string normalize_verb(std::string v);
};

} // namespace commands
