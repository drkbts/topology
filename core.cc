#include "core.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <thread>

namespace topology
{

    // DiameterProxy implementation

    DiameterProxy::operator int() const
    {
        // Need to cast to Graph to call the virtual getDiameter method
        const Graph *graph_ptr = static_cast<const Graph *>(&graph_);
        return graph_ptr->getDiameter();
    }

    // VerticesProxy implementation
    VerticesProxy::operator std::vector<int32_t>() const
    {
        std::vector<int32_t> result;
        auto vertices_range = boost::vertices(graph_);
        for (auto v_iter = vertices_range.first; v_iter != vertices_range.second; ++v_iter)
        {
            result.push_back(graph_[*v_iter].id);
        }
        return result;
    }

    // EdgesProxy implementation
    EdgesProxy::operator std::vector<std::pair<int32_t, int32_t>>() const
    {
        std::vector<std::pair<int32_t, int32_t>> result;
        auto edges_range = boost::edges(graph_);
        for (auto e_iter = edges_range.first; e_iter != edges_range.second; ++e_iter)
        {
            auto src = boost::source(*e_iter, graph_);
            auto dst = boost::target(*e_iter, graph_);
            result.push_back({graph_[src].id, graph_[dst].id});
        }
        return result;
    }

    // Graph class implementation

    Graph::Graph() : diameter(*this), num_vertices(*this), num_edges(*this), vertices(*this), edges(*this)
    {
        (*this)[boost::graph_bundle].name = "Generic";
    }

    Graph::Graph(const BaseGraph &other) : BaseGraph(other), diameter(*this), num_vertices(*this), num_edges(*this), vertices(*this), edges(*this)
    {
        (*this)[boost::graph_bundle].name = "Generic";
    }

    void Graph::add_vertex(int32_t id)
    {
        // Add vertex to boost graph
        BaseGraph &bg = static_cast<BaseGraph &>(*this);
        auto v = boost::add_vertex(bg);

        // Set the vertex id property
        (*this)[v].id = id;
    }

    void Graph::add_edge(int32_t i, int32_t j)
    {
        // Find vertices with given ids
        BaseGraph &bg = static_cast<BaseGraph &>(*this);
        boost::graph_traits<BaseGraph>::vertex_descriptor v_i = boost::graph_traits<BaseGraph>::null_vertex();
        boost::graph_traits<BaseGraph>::vertex_descriptor v_j = boost::graph_traits<BaseGraph>::null_vertex();

        // Find vertices by id
        auto [vi, vi_end] = boost::vertices(bg);
        for (auto v_it = vi; v_it != vi_end; ++v_it)
        {
            if ((*this)[*v_it].id == i)
            {
                v_i = *v_it;
            }
            if ((*this)[*v_it].id == j)
            {
                v_j = *v_it;
            }
        }

        // Add edge if both vertices exist
        if (v_i != boost::graph_traits<BaseGraph>::null_vertex() &&
            v_j != boost::graph_traits<BaseGraph>::null_vertex())
        {
            boost::add_edge(v_i, v_j, bg);
        }
    }

    int Graph::getDiameter() const
    {
        return getDiameter_impl(*this);
    }

    int Graph::getDiameter_impl(const BaseGraph &g)
    {

        // Empty graph has no diameter
        if (boost::num_vertices(g) == 0)
        {
            return -1;
        }

        // Single vertex has diameter 0
        if (boost::num_vertices(g) == 1)
        {
            return 0;
        }

        int max_distance = 0;

        // For each vertex, find shortest distances to all other vertices
        auto [vi, vi_end] = boost::vertices(g);
        for (auto source = vi; source != vi_end; ++source)
        {
            // BFS to find shortest distances from source
            std::vector<int> distances(boost::num_vertices(g), -1);
            std::queue<boost::graph_traits<BaseGraph>::vertex_descriptor> queue;

            distances[*source] = 0;
            queue.push(*source);

            while (!queue.empty())
            {
                auto current = queue.front();
                queue.pop();

                auto [ei, ei_end] = boost::out_edges(current, g);
                for (auto edge = ei; edge != ei_end; ++edge)
                {
                    auto target = boost::target(*edge, g);

                    if (distances[target] == -1)
                    { // Not visited
                        distances[target] = distances[current] + 1;
                        queue.push(target);
                    }
                }
            }

            // Find maximum distance from this source
            for (size_t i = 0; i < distances.size(); ++i)
            {
                if (distances[i] == -1)
                {
                    // Graph is disconnected
                    return -1;
                }
                max_distance = std::max(max_distance, distances[i]);
            }
        }

        return max_distance;
    }

    // URing implementation

    URing::URing(size_t N) : ring_size_(N)
    {
        if (N == 0)
        {
            throw std::invalid_argument("Ring size must be positive");
        }

        // Set graph name
        (*this)[boost::graph_bundle].name = "URing";

        // Add vertices with integer ids 0, 1, ..., N-1
        for (size_t i = 0; i < N; ++i)
        {
            Graph::add_vertex(static_cast<int32_t>(i));
        }

        // Add edges to form a ring: 0→1→2→...→(N-1)→0
        // Special case: N=1 has no edges
        if (N > 1)
        {
            for (size_t i = 0; i < N; ++i)
            {
                size_t next = (i + 1) % N;
                Graph::add_edge(static_cast<int32_t>(i), static_cast<int32_t>(next));
            }
        }
    }

    size_t URing::GetRingSize() const
    {
        return ring_size_;
    }

    int URing::getDiameter() const
    {
        if (ring_size_ == 0)
        {
            return -1; // Empty ring
        }
        if (ring_size_ == 1)
        {
            return 0; // Single vertex
        }
        return static_cast<int>(ring_size_ / 2); // Diameter is floor(N/2) for ring
    }

} // namespace topology
