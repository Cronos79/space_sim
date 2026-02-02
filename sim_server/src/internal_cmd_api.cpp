#include "internal_cmd_api.h"
#include "json.hpp"

using json = nlohmann::json;

namespace sim {

void register_internal_cmd_api(
    httplib::Server& server,
    const std::string& internal_key,
    commands::Router& router
) {
    server.Get("/health", [&](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
        res.set_content(R"({"ok":true})", "application/json");
    });

    server.Post("/cmd", [&](const httplib::Request& req, httplib::Response& res) {
        // Shared secret between api_server and sim_server
        const std::string key = req.get_header_value("X-Internal-Key");
        if (key != internal_key) {
            res.status = 403;
            res.set_content(R"({"ok":false,"error":"forbidden"})", "application/json");
            return;
        }

        json in = json::parse(req.body, nullptr, false);
        if (in.is_discarded()
            || !in.contains("user_id") || !in["user_id"].is_number_integer()
            || !in.contains("cmd") || !in["cmd"].is_string()) {
            res.status = 400;
            res.set_content(R"({"ok":false,"error":"bad_request"})", "application/json");
            return;
        }

        commands::Context ctx;
        ctx.user_id = in["user_id"].get<int32_t>();

        auto result = router.execute_line(ctx, in["cmd"].get<std::string>());

        json out;
        out["ok"] = result.ok;
        out["text"] = result.text;
        if (!result.error_code.empty()) out["error"] = result.error_code;

        res.status = result.ok ? 200 : 400;
        res.set_content(out.dump(), "application/json");
    });
}

} // namespace sim
