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

    // DimensionProxy implementation
    DimensionProxy::operator size_t() const
    {
        // Try to cast to URing first
        const URing *ring = dynamic_cast<const URing *>(&graph_);
        if (ring)
        {
            return ring->GetDimensionSize();
        }

        // Try to cast to BRing
        const BRing *bring = dynamic_cast<const BRing *>(&graph_);
        if (bring)
        {
            return bring->GetDimensionSize();
        }

        // Try to cast to UMesh
        const UMesh *mesh = dynamic_cast<const UMesh *>(&graph_);
        if (mesh)
        {
            return mesh->GetDimensionSize();
        }

        // Try to cast to BMesh
        const BMesh *bmesh = dynamic_cast<const BMesh *>(&graph_);
        if (bmesh)
        {
            return bmesh->GetDimensionSize();
        }

        // Try to cast to OPG
        const OPG *opg = dynamic_cast<const OPG *>(&graph_);
        if (opg)
        {
            return opg->GetDimensionSize();
        }

        // Not a specialized topology, return 0 or throw
        return 0;
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

    URing::URing(size_t N) : dimension(*this), dimension_(N)
    {
        if (N == 0)
        {
            throw std::invalid_argument("Ring size must be positive");
        }

        // Set graph name
        static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "URing";

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

    size_t URing::GetDimensionSize() const
    {
        return dimension_;
    }

    int URing::getDiameter() const
    {
        if (dimension_ == 0)
        {
            return -1; // Empty ring
        }
        if (dimension_ == 1)
        {
            return 0; // Single vertex
        }
        return static_cast<int>(dimension_ / 2); // Diameter is floor(N/2) for ring
    }

    void URing::add_vertex(int32_t id)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_vertex(id);
    }

    void URing::add_edge(int32_t i, int32_t j)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_edge(i, j);
    }

    // BRing implementation
    BRing::BRing(size_t N) : dimension(*this), dimension_(N)
    {
        if (N == 0)
        {
            throw std::invalid_argument("Ring size must be positive");
        }

        // Set graph name
        static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "BRing";

        // Add vertices with integer ids 0, 1, ..., N-1
        for (size_t i = 0; i < N; ++i)
        {
            Graph::add_vertex(static_cast<int32_t>(i));
        }

        // Add bidirectional edges to form a ring: 0↔1↔2↔...↔(N-1)↔0
        // Special case: N=1 has no edges
        if (N > 1)
        {
            for (size_t i = 0; i < N; ++i)
            {
                size_t next = (i + 1) % N;
                Graph::add_edge(static_cast<int32_t>(i), static_cast<int32_t>(next));
                Graph::add_edge(static_cast<int32_t>(next), static_cast<int32_t>(i));
            }
        }
    }

    size_t BRing::GetDimensionSize() const
    {
        return dimension_;
    }

    int BRing::getDiameter() const
    {
        if (dimension_ == 0)
        {
            return -1; // Empty ring
        }
        if (dimension_ == 1)
        {
            return 0; // Single vertex
        }
        return static_cast<int>(dimension_ / 2); // Diameter is floor(N/2) for bidirectional ring
    }

    void BRing::add_vertex(int32_t id)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_vertex(id);
    }

    void BRing::add_edge(int32_t i, int32_t j)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_edge(i, j);
    }

    // UMesh implementation

    UMesh::UMesh(size_t N) : dimension(*this), dimension_(N)
    {
        if (N == 0)
        {
            throw std::invalid_argument("Mesh size must be positive");
        }

        // Set graph name
        static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "UMesh";

        // Add vertices with integer ids 0, 1, ..., N-1
        for (size_t i = 0; i < N; ++i)
        {
            Graph::add_vertex(static_cast<int32_t>(i));
        }

        // Add edges to form a linear chain: 0→1→2→...→(N-1)
        // Special case: N=1 has no edges
        if (N > 1)
        {
            for (size_t i = 0; i < N - 1; ++i)
            {
                Graph::add_edge(static_cast<int32_t>(i), static_cast<int32_t>(i + 1));
            }
        }
    }

    size_t UMesh::GetDimensionSize() const
    {
        return dimension_;
    }

    int UMesh::getDiameter() const
    {
        if (dimension_ == 0)
        {
            return -1; // Empty mesh
        }
        if (dimension_ == 1)
        {
            return 0; // Single vertex
        }
        return static_cast<int>(dimension_ - 1); // Diameter is N-1 for linear chain
    }

    void UMesh::add_vertex(int32_t id)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_vertex(id);
    }

    void UMesh::add_edge(int32_t i, int32_t j)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_edge(i, j);
    }

    // OPG implementation

    OPG::OPG() : dimension(*this), dimension_(1)
    {
        // Set graph name
        static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "OPG";

        // Add single vertex with ID 0
        Graph::add_vertex(0);
    }

    size_t OPG::GetDimensionSize() const
    {
        return dimension_;
    }

    int OPG::getDiameter() const
    {
        return 0; // Single vertex always has diameter 0
    }

    void OPG::add_vertex(int32_t id)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_vertex(id);
    }

    void OPG::add_edge(int32_t i, int32_t j)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_edge(i, j);
    }

    // BMesh implementation
    BMesh::BMesh(size_t N) : dimension(*this), dimension_(N)
    {
        if (N == 0)
        {
            throw std::invalid_argument("Mesh size must be positive");
        }

        // Set graph name
        static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "BMesh";

        // Add vertices with integer ids 0, 1, ..., N-1
        for (size_t i = 0; i < N; ++i)
        {
            Graph::add_vertex(static_cast<int32_t>(i));
        }

        // Add bidirectional edges to form a linear chain: 0↔1↔2↔...↔(N-1)
        // Special case: N=1 has no edges
        if (N > 1)
        {
            for (size_t i = 0; i < N - 1; ++i)
            {
                Graph::add_edge(static_cast<int32_t>(i), static_cast<int32_t>(i + 1));
                Graph::add_edge(static_cast<int32_t>(i + 1), static_cast<int32_t>(i));
            }
        }
    }

    size_t BMesh::GetDimensionSize() const
    {
        return dimension_;
    }

    int BMesh::getDiameter() const
    {
        if (dimension_ == 0)
        {
            return -1; // Empty mesh
        }
        if (dimension_ == 1)
        {
            return 0; // Single vertex
        }
        return static_cast<int>(dimension_ - 1); // Diameter is N-1 for bidirectional linear chain
    }

    void BMesh::add_vertex(int32_t id)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_vertex(id);
    }

    void BMesh::add_edge(int32_t i, int32_t j)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_edge(i, j);
    }

    // Cartesian product utility functions
    namespace gproduct_utils
    {
        int32_t encode_vertex_pair(int32_t g1_id, int32_t g2_id, size_t g2_size)
        {
            return g1_id * static_cast<int32_t>(g2_size) + g2_id;
        }

        std::pair<int32_t, int32_t> decode_vertex_pair(int32_t product_id, size_t g2_size)
        {
            return {product_id / static_cast<int32_t>(g2_size), product_id % static_cast<int32_t>(g2_size)};
        }
    }

    // Cartesian product implementation
    Graph gproduct(const Graph &g1, const Graph &g2)
    {
        Graph result;
        result[boost::graph_bundle].name = g1[boost::graph_bundle].name + " ⊗ " + g2[boost::graph_bundle].name;

        std::vector<int32_t> g1_vertices = g1.vertices;
        std::vector<int32_t> g2_vertices = g2.vertices;
        size_t g1_num_vertices = g1.num_vertices;
        size_t g2_num_vertices = g2.num_vertices;

        // Add all vertex pairs using scalar product formula: |V(G1)| × |V(G2)|
        // Total vertices = g1_num_vertices * g2_num_vertices
        for (size_t i = 0; i < g1_num_vertices; ++i)
        {
            for (size_t j = 0; j < g2_num_vertices; ++j)
            {
                int32_t product_id = gproduct_utils::encode_vertex_pair(g1_vertices[i], g2_vertices[j], g2_num_vertices);
                result.add_vertex(product_id);
            }
        }

        // Add edges from G dimension (u1 connects to u2, v1 = v2)
        // This contributes |E(G1)| × |V(G2)| edges
        std::vector<std::pair<int32_t, int32_t>> g1_edges = g1.edges;
        for (auto [u1, u2] : g1_edges)
        {
            for (size_t j = 0; j < g2_num_vertices; ++j)
            {
                int32_t from_id = gproduct_utils::encode_vertex_pair(u1, g2_vertices[j], g2_num_vertices);
                int32_t to_id = gproduct_utils::encode_vertex_pair(u2, g2_vertices[j], g2_num_vertices);
                result.add_edge(from_id, to_id);
            }
        }

        // Add edges from H dimension (u1 = u2, v1 connects to v2)
        // This contributes |V(G1)| × |E(G2)| edges
        std::vector<std::pair<int32_t, int32_t>> g2_edges = g2.edges;
        for (auto [v1, v2] : g2_edges)
        {
            for (size_t i = 0; i < g1_num_vertices; ++i)
            {
                int32_t from_id = gproduct_utils::encode_vertex_pair(g1_vertices[i], v1, g2_num_vertices);
                int32_t to_id = gproduct_utils::encode_vertex_pair(g1_vertices[i], v2, g2_num_vertices);
                result.add_edge(from_id, to_id);
            }
        }

        return result;
    }

    // Operator overload for Cartesian product
    Graph operator*(const Graph &g1, const Graph &g2)
    {
        return gproduct(g1, g2);
    }

} // namespace topology
