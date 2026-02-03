#include "commands/commands.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace commands {

static std::string trim(const std::string& s) {
    size_t a = 0;
    while (a < s.size() && std::isspace((unsigned char)s[a])) a++;
    size_t b = s.size();
    while (b > a && std::isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

std::string Router::normalize_verb(std::string v) {
    std::transform(v.begin(), v.end(), v.begin(),
        [](unsigned char c){ return (char)std::tolower(c); });
    return v;
}

bool Router::add(std::string verb, Handler handler, Access access) {
    verb = normalize_verb(std::move(verb));
    if (verb.empty() || !handler) return false;

    Entry e;
    e.handler = std::move(handler);
    e.access = access;

    return handlers_.emplace(std::move(verb), std::move(e)).second;
}

std::optional<Command> Router::parse_line(const std::string& line) {
    std::string s = trim(line);
    if (s.empty()) return std::nullopt;

    std::istringstream iss(s);
    Command cmd;

    if (!(iss >> cmd.verb)) return std::nullopt;
    cmd.verb = normalize_verb(std::move(cmd.verb));

    std::string arg;
    while (iss >> arg) cmd.args.push_back(std::move(arg));

    return cmd;
}

Result Router::execute_line(const Context& ctx, const std::string& line) const {
    auto parsed = parse_line(line);
    if (!parsed) return Result{false, "Empty command.", "empty_command", {}};
    return execute(ctx, *parsed);
}

Result Router::execute(const Context& ctx, const Command& cmd) const {
    auto it = handlers_.find(normalize_verb(cmd.verb));
    if (it == handlers_.end()) {
        Result r;
        r.ok = false;
        r.error_code = "unknown_command";
        r.text = "Unknown command: " + cmd.verb;
        return r;
    }
    return it->second.handler(ctx, cmd);
}

std::optional<Access> Router::access_for(const std::string& verb) const {
    auto it = handlers_.find(normalize_verb(verb));
    if (it == handlers_.end()) return std::nullopt;
    return it->second.access;
}

} // namespace commands
