#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace universe {

using SystemId = int32_t;

struct RouteResult {
    int jumps{0};
    std::vector<SystemId> path; // includes start and goal
};

class GateNetwork {
public:
    // Node management (optional, but helps validate)
    void add_node(SystemId id);                 // safe to call multiple times
    bool has_node(SystemId id) const;

    // Edges
    bool add_gate(SystemId a, SystemId b);      // undirected
    bool has_gate(SystemId a, SystemId b) const;

    const std::vector<SystemId>& neighbors(SystemId id) const;

    int gate_count() const { return gate_count_; }
    int node_count() const { return static_cast<int>(adj_.size()); }

    std::optional<RouteResult> shortest_route(SystemId start, SystemId goal) const;
    std::vector<SystemId> within(SystemId start, int max_jumps, bool include_start = false) const;

    bool is_connected() const;

private:
    std::unordered_map<SystemId, std::vector<SystemId>> adj_;

    struct Edge { SystemId a, b; };
    struct EdgeHash {
        size_t operator()(const Edge& e) const noexcept {
            return (static_cast<size_t>(e.a) << 32) ^ static_cast<size_t>(e.b);
        }
    };
    struct EdgeEq {
        bool operator()(const Edge& x, const Edge& y) const noexcept {
            return x.a == y.a && x.b == y.b;
        }
    };

    std::unordered_set<Edge, EdgeHash, EdgeEq> edges_;
    int gate_count_ = 0;

    static Edge norm(SystemId a, SystemId b);
    static const std::vector<SystemId>& empty_neighbors();
};

} // namespace universe
