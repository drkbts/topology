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

    // NumDimensionsProxy implementation
    NumDimensionsProxy::operator size_t() const
    {
        // Get the graph name to determine type
        const std::string& name = graph_[boost::graph_bundle].name;
        
        // Check if it's a BGrid (multiple dimensions possible)
        if (name.substr(0, 6) == "BGrid[") {
            // Count commas in the name to determine dimensions
            // BGrid[3,4,5] has 3 dimensions, BGrid[5] has 1 dimension, BGrid[] has 1 dimension
            if (name == "BGrid[]") {
                return 1; // Special case for empty BGrid (OPG)
            }
            
            size_t dimensions = 1; // At least one dimension
            for (char c : name) {
                if (c == ',') dimensions++;
            }
            return dimensions;
        }
        
        // Check if it's a BTorus (multiple dimensions possible)
        if (name.substr(0, 7) == "BTorus[") {
            // Count commas in the name to determine dimensions
            // BTorus[3,4,5] has 3 dimensions, BTorus[5] has 1 dimension, BTorus[] has 1 dimension
            if (name == "BTorus[]") {
                return 1; // Special case for empty BTorus (OPG)
            }
            
            size_t dimensions = 1; // At least one dimension
            for (char c : name) {
                if (c == ',') dimensions++;
            }
            return dimensions;
        }
        
        // For other specialized topologies, return 1 (single dimension)
        if (name == "URing" || name == "BRing" || name == "UMesh" || name == "BMesh" || name == "OPG") {
            return 1;
        }
        
        // Generic graph, return 0
        return 0;
    }

    // Graph class implementation

    Graph::Graph() : diameter(*this), num_vertices(*this), num_edges(*this), vertices(*this), edges(*this), num_dimensions(*this)
    {
        (*this)[boost::graph_bundle].name = "Generic";
    }

    Graph::Graph(const BaseGraph &other) : BaseGraph(other), diameter(*this), num_vertices(*this), num_edges(*this), vertices(*this), edges(*this), num_dimensions(*this)
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

    // MultiDimensionProxy implementation
    MultiDimensionProxy::MultiDimensionProxy(const BGrid &grid) : ptr_(&grid), is_torus_(false) {}
    
    MultiDimensionProxy::MultiDimensionProxy(const BTorus &torus) : ptr_(&torus), is_torus_(true) {}

    size_t MultiDimensionProxy::operator[](size_t index) const
    {
        const auto& dims = is_torus_ ? 
            static_cast<const BTorus*>(ptr_)->GetDimensions() :
            static_cast<const BGrid*>(ptr_)->GetDimensions();
        if (index >= dims.size()) {
            throw std::out_of_range("Dimension index out of range");
        }
        return dims[index];
    }

    const std::vector<size_t>& MultiDimensionProxy::get() const
    {
        return is_torus_ ? 
            static_cast<const BTorus*>(ptr_)->GetDimensions() :
            static_cast<const BGrid*>(ptr_)->GetDimensions();
    }

    size_t MultiDimensionProxy::size() const
    {
        return is_torus_ ? 
            static_cast<const BTorus*>(ptr_)->GetNumDimensions() :
            static_cast<const BGrid*>(ptr_)->GetNumDimensions();
    }

    // BGrid implementation
    BGrid::BGrid(const std::vector<size_t>& dimensions) : dimensions(*this)
    {
        // Validate all dimensions are positive
        for (size_t dim : dimensions) {
            if (dim == 0) {
                throw std::invalid_argument("All grid dimensions must be positive");
            }
        }

        // Sort dimensions in descending order and filter out dimensions equal to 1
        std::vector<size_t> filtered_dims;
        for (size_t dim : dimensions) {
            if (dim > 1) {
                filtered_dims.push_back(dim);
            }
        }
        
        // Sort in descending order
        std::sort(filtered_dims.begin(), filtered_dims.end(), std::greater<size_t>());
        
        // If empty after filtering, report as length 1 with entry 1
        if (filtered_dims.empty()) {
            dimensions_ = {1};
        } else {
            dimensions_ = filtered_dims;
        }

        if (dimensions_.empty() || (dimensions_.size() == 1 && dimensions_[0] == 1)) {
            // Case 1: Empty list → OPG
            OPG opg;
            // Copy the OPG structure to this BGrid
            for (auto [vi, vi_end] = boost::vertices(opg); vi != vi_end; ++vi) {
                Graph::add_vertex(opg[*vi].id);
            }
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "BGrid[]";

        } else if (dimensions_.size() == 1) {
            // Case 2: Single dimension → BMesh
            BMesh mesh(dimensions_[0]);
            // Copy the BMesh structure to this BGrid
            for (auto [vi, vi_end] = boost::vertices(mesh); vi != vi_end; ++vi) {
                Graph::add_vertex(mesh[*vi].id);
            }
            for (auto [ei, ei_end] = boost::edges(mesh); ei != ei_end; ++ei) {
                auto src = boost::source(*ei, mesh);
                auto tgt = boost::target(*ei, mesh);
                Graph::add_edge(mesh[src].id, mesh[tgt].id);
            }
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "BGrid[" + std::to_string(dimensions_[0]) + "]";

        } else {
            // Case 3: Multiple dimensions → Left associative gproduct of BMesh's
            buildGrid(dimensions_);

            // Build name: BGrid[d1,d2,d3,...] using filtered and sorted dimensions
            std::string name = "BGrid[";
            for (size_t i = 0; i < dimensions_.size(); ++i) {
                if (i > 0) name += ",";
                name += std::to_string(dimensions_[i]);
            }
            name += "]";
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = name;
        }
    }

    void BGrid::buildGrid(const std::vector<size_t>& dims)
    {
        if (dims.size() < 2) return;

        // Build the grid using recursive gproducts
        // Create a helper function to compute the left-associative product
        std::function<void(Graph&, const std::vector<size_t>&, size_t)> build_product = 
            [&](Graph& result, const std::vector<size_t>& dimensions, size_t index) {
                if (index == 0) {
                    // Base case: copy first BMesh to result
                    BMesh mesh(dimensions[0]);
                    for (auto [vi, vi_end] = boost::vertices(mesh); vi != vi_end; ++vi) {
                        result.add_vertex(mesh[*vi].id);
                    }
                    for (auto [ei, ei_end] = boost::edges(mesh); ei != ei_end; ++ei) {
                        auto src = boost::source(*ei, mesh);
                        auto tgt = boost::target(*ei, mesh);
                        result.add_edge(mesh[src].id, mesh[tgt].id);
                    }
                    return;
                }
                
                // Recursive case: compute product with previous results
                Graph prev_result;
                build_product(prev_result, dimensions, index - 1);
                
                BMesh current_mesh(dimensions[index]);
                Graph product_result = gproduct(prev_result, static_cast<Graph&>(current_mesh));
                
                // Copy product_result to result
                for (auto [vi, vi_end] = boost::vertices(product_result); vi != vi_end; ++vi) {
                    result.add_vertex(product_result[*vi].id);
                }
                for (auto [ei, ei_end] = boost::edges(product_result); ei != ei_end; ++ei) {
                    auto src = boost::source(*ei, product_result);
                    auto tgt = boost::target(*ei, product_result);
                    result.add_edge(product_result[src].id, product_result[tgt].id);
                }
            };

        // Build the final grid
        Graph final_result;
        build_product(final_result, dims, dims.size() - 1);

        // Copy to this BGrid
        for (auto [vi, vi_end] = boost::vertices(final_result); vi != vi_end; ++vi) {
            Graph::add_vertex(final_result[*vi].id);
        }
        for (auto [ei, ei_end] = boost::edges(final_result); ei != ei_end; ++ei) {
            auto src = boost::source(*ei, final_result);
            auto tgt = boost::target(*ei, final_result);
            Graph::add_edge(final_result[src].id, final_result[tgt].id);
        }
    }

    size_t BGrid::calculateGridEdges(const std::vector<size_t>& dims) const
    {
        if (dims.empty()) return 0;
        if (dims.size() == 1) {
            return 2 * (dims[0] - 1); // BMesh edge count
        }

        // Calculate iteratively using gproduct formula
        size_t vertices1 = dims[0];
        size_t edges1 = 2 * (dims[0] - 1);

        for (size_t i = 1; i < dims.size(); ++i) {
            size_t vertices2 = dims[i];
            size_t edges2 = 2 * (dims[i] - 1);

            // Apply gproduct formula: |E(G1 ⊗ G2)| = |V(G1)| × |E(G2)| + |E(G1)| × |V(G2)|
            size_t new_edges = vertices1 * edges2 + edges1 * vertices2;
            size_t new_vertices = vertices1 * vertices2;

            vertices1 = new_vertices;
            edges1 = new_edges;
        }

        return edges1;
    }

    const std::vector<size_t>& BGrid::GetDimensions() const
    {
        return dimensions_;
    }

    size_t BGrid::GetNumDimensions() const
    {
        return dimensions_.size();
    }

    int BGrid::getDiameter() const
    {
        if (dimensions_.empty()) {
            return 0; // OPG case
        }

        // For BMesh grid, diameter is sum of individual mesh diameters
        // Each BMesh(n) has diameter n-1
        int total_diameter = 0;
        for (size_t dim : dimensions_) {
            total_diameter += static_cast<int>(dim - 1);
        }
        return total_diameter;
    }

    void BGrid::add_vertex(int32_t id)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_vertex(id);
    }

    void BGrid::add_edge(int32_t i, int32_t j)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_edge(i, j);
    }

    // BTorus implementation
    BTorus::BTorus(const std::vector<size_t>& dimensions) : dimensions(*this)
    {
        // Validate all dimensions are positive
        for (size_t dim : dimensions) {
            if (dim == 0) {
                throw std::invalid_argument("All torus dimensions must be positive");
            }
        }

        // Sort dimensions in descending order and filter out dimensions equal to 1
        std::vector<size_t> filtered_dims;
        for (size_t dim : dimensions) {
            if (dim > 1) {
                filtered_dims.push_back(dim);
            }
        }
        
        // Sort in descending order
        std::sort(filtered_dims.begin(), filtered_dims.end(), std::greater<size_t>());
        
        // If empty after filtering, report as length 1 with entry 1
        if (filtered_dims.empty()) {
            dimensions_ = {1};
        } else {
            dimensions_ = filtered_dims;
        }

        if (dimensions_.empty() || (dimensions_.size() == 1 && dimensions_[0] == 1)) {
            // Case 1: Empty list → OPG
            OPG opg;
            // Copy the OPG structure to this BTorus
            for (auto [vi, vi_end] = boost::vertices(opg); vi != vi_end; ++vi) {
                Graph::add_vertex(opg[*vi].id);
            }
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "BTorus[]";

        } else if (dimensions_.size() == 1) {
            // Case 2: Single dimension → BRing
            BRing ring(dimensions_[0]);
            // Copy the BRing structure to this BTorus
            for (auto [vi, vi_end] = boost::vertices(ring); vi != vi_end; ++vi) {
                Graph::add_vertex(ring[*vi].id);
            }
            for (auto [ei, ei_end] = boost::edges(ring); ei != ei_end; ++ei) {
                auto src = boost::source(*ei, ring);
                auto tgt = boost::target(*ei, ring);
                Graph::add_edge(ring[src].id, ring[tgt].id);
            }
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "BTorus[" + std::to_string(dimensions_[0]) + "]";

        } else {
            // Case 3: Multiple dimensions → Left associative gproduct of BRing's
            buildTorus(dimensions_);

            // Build name: BTorus[d1,d2,d3,...] using filtered and sorted dimensions
            std::string name = "BTorus[";
            for (size_t i = 0; i < dimensions_.size(); ++i) {
                if (i > 0) name += ",";
                name += std::to_string(dimensions_[i]);
            }
            name += "]";
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = name;
        }
    }

    void BTorus::buildTorus(const std::vector<size_t>& dims)
    {
        if (dims.size() < 2) return;

        // Build the torus using recursive gproducts
        // Create a helper function to compute the left-associative product
        std::function<void(Graph&, const std::vector<size_t>&, size_t)> build_product = 
            [&](Graph& result, const std::vector<size_t>& dimensions, size_t index) {
                if (index == 0) {
                    // Base case: copy first BRing to result
                    BRing ring(dimensions[0]);
                    for (auto [vi, vi_end] = boost::vertices(ring); vi != vi_end; ++vi) {
                        result.add_vertex(ring[*vi].id);
                    }
                    for (auto [ei, ei_end] = boost::edges(ring); ei != ei_end; ++ei) {
                        auto src = boost::source(*ei, ring);
                        auto tgt = boost::target(*ei, ring);
                        result.add_edge(ring[src].id, ring[tgt].id);
                    }
                    return;
                }
                
                // Recursive case: compute product with previous results
                Graph prev_result;
                build_product(prev_result, dimensions, index - 1);
                
                BRing current_ring(dimensions[index]);
                Graph product_result = gproduct(prev_result, static_cast<Graph&>(current_ring));
                
                // Copy product_result to result
                for (auto [vi, vi_end] = boost::vertices(product_result); vi != vi_end; ++vi) {
                    result.add_vertex(product_result[*vi].id);
                }
                for (auto [ei, ei_end] = boost::edges(product_result); ei != ei_end; ++ei) {
                    auto src = boost::source(*ei, product_result);
                    auto tgt = boost::target(*ei, product_result);
                    result.add_edge(product_result[src].id, product_result[tgt].id);
                }
            };

        // Build the final torus
        Graph final_result;
        build_product(final_result, dims, dims.size() - 1);

        // Copy to this BTorus
        for (auto [vi, vi_end] = boost::vertices(final_result); vi != vi_end; ++vi) {
            Graph::add_vertex(final_result[*vi].id);
        }
        for (auto [ei, ei_end] = boost::edges(final_result); ei != ei_end; ++ei) {
            auto src = boost::source(*ei, final_result);
            auto tgt = boost::target(*ei, final_result);
            Graph::add_edge(final_result[src].id, final_result[tgt].id);
        }
    }

    size_t BTorus::calculateTorusEdges(const std::vector<size_t>& dims) const
    {
        if (dims.empty()) return 0;
        if (dims.size() == 1) {
            return 2 * dims[0]; // BRing edge count (N rings have N edges each way = 2N total)
        }

        // Calculate iteratively using gproduct formula
        size_t vertices1 = dims[0];
        size_t edges1 = 2 * dims[0]; // BRing has 2*N edges

        for (size_t i = 1; i < dims.size(); ++i) {
            size_t vertices2 = dims[i];
            size_t edges2 = 2 * dims[i]; // BRing has 2*N edges

            // Apply gproduct formula: |E(G1 ⊗ G2)| = |V(G1)| × |E(G2)| + |E(G1)| × |V(G2)|
            size_t new_edges = vertices1 * edges2 + edges1 * vertices2;
            size_t new_vertices = vertices1 * vertices2;

            vertices1 = new_vertices;
            edges1 = new_edges;
        }

        return edges1;
    }

    const std::vector<size_t>& BTorus::GetDimensions() const
    {
        return dimensions_;
    }

    size_t BTorus::GetNumDimensions() const
    {
        return dimensions_.size();
    }

    int BTorus::getDiameter() const
    {
        if (dimensions_.empty() || (dimensions_.size() == 1 && dimensions_[0] == 1)) {
            // OPG case
            return 0;
        }
        
        if (dimensions_.size() == 1) {
            // Single BRing case: diameter is floor(N/2)
            return static_cast<int>(dimensions_[0] / 2);
        }
        
        // Multiple dimensions: sum of individual ring diameters
        int total_diameter = 0;
        for (size_t dim : dimensions_) {
            total_diameter += static_cast<int>(dim / 2); // floor(dim/2)
        }
        return total_diameter;
    }

    void BTorus::add_vertex(int32_t id)
    {
        // Convert to generic graph when modified
        if (static_cast<BaseGraph&>(*this)[boost::graph_bundle].name != "Generic")
        {
            static_cast<BaseGraph&>(*this)[boost::graph_bundle].name = "Generic";
        }
        Graph::add_vertex(id);
    }

    void BTorus::add_edge(int32_t i, int32_t j)
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
