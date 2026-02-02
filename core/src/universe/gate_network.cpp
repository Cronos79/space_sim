#include "universe/gate_network.h"
#include <queue>
#include <algorithm>

namespace universe {

static std::vector<SystemId> kEmpty;

const std::vector<SystemId>& GateNetwork::empty_neighbors() { return kEmpty; }

GateNetwork::Edge GateNetwork::norm(SystemId a, SystemId b) {
    return (a < b) ? Edge{a, b} : Edge{b, a};
}

void GateNetwork::add_node(SystemId id) {
    adj_.try_emplace(id, std::vector<SystemId>{});
}

bool GateNetwork::has_node(SystemId id) const {
    return adj_.contains(id);
}

bool GateNetwork::add_gate(SystemId a, SystemId b) {
    if (a == b) return false;
    add_node(a);
    add_node(b);

    Edge e = norm(a, b);
    if (edges_.contains(e)) return false;

    edges_.insert(e);
    adj_[a].push_back(b);
    adj_[b].push_back(a);
    gate_count_++;
    return true;
}

bool GateNetwork::has_gate(SystemId a, SystemId b) const {
    if (a == b) return false;
    return edges_.contains(norm(a, b));
}

const std::vector<SystemId>& GateNetwork::neighbors(SystemId id) const {
    auto it = adj_.find(id);
    return (it == adj_.end()) ? empty_neighbors() : it->second;
}

std::optional<RouteResult> GateNetwork::shortest_route(SystemId start, SystemId goal) const {
    if (!has_node(start) || !has_node(goal)) return std::nullopt;
    if (start == goal) return RouteResult{0, {start}};

    std::queue<SystemId> q;
    std::unordered_map<SystemId, SystemId> prev;
    std::unordered_map<SystemId, int> dist;

    q.push(start);
    dist[start] = 0;

    while (!q.empty()) {
        SystemId cur = q.front();
        q.pop();

        for (SystemId nxt : neighbors(cur)) {
            if (dist.contains(nxt)) continue;

            dist[nxt] = dist[cur] + 1;
            prev[nxt] = cur;

            if (nxt == goal) {
                std::vector<SystemId> path;
                for (SystemId p = goal;; p = prev[p]) {
                    path.push_back(p);
                    if (p == start) break;
                }
                std::reverse(path.begin(), path.end());
                return RouteResult{dist[goal], std::move(path)};
            }

            q.push(nxt);
        }
    }

    return std::nullopt;
}

std::vector<SystemId> GateNetwork::within(SystemId start, int max_jumps, bool include_start) const {
    std::vector<SystemId> out;
    if (!has_node(start) || max_jumps < 0) return out;

    std::queue<SystemId> q;
    std::unordered_map<SystemId, int> dist;

    q.push(start);
    dist[start] = 0;

    while (!q.empty()) {
        SystemId cur = q.front();
        q.pop();

        int d = dist[cur];
        if (d > max_jumps) continue;

        if (include_start || cur != start) out.push_back(cur);

        for (SystemId nxt : neighbors(cur)) {
            if (dist.contains(nxt)) continue;
            dist[nxt] = d + 1;
            if (dist[nxt] <= max_jumps) q.push(nxt);
        }
    }

    return out;
}

bool GateNetwork::is_connected() const {
    if (adj_.empty()) return true;

    SystemId start = adj_.begin()->first;
    std::queue<SystemId> q;
    std::unordered_set<SystemId> seen;

    q.push(start);
    seen.insert(start);

    while (!q.empty()) {
        SystemId cur = q.front();
        q.pop();
        for (SystemId nxt : neighbors(cur)) {
            if (seen.contains(nxt)) continue;
            seen.insert(nxt);
            q.push(nxt);
        }
    }

    return seen.size() == adj_.size();
}

} // namespace universe
