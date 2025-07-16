#ifndef TOPOLOGY_CORE_H_
#define TOPOLOGY_CORE_H_

#include <vector>
#include <string>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

namespace topology
{

    struct VertexProperties
    {
        int32_t id;
    };

    struct EdgeProperties
    {
        double latency;
        double bandwidth;
    };

    struct GraphProperties
    {
        std::string name;
    };

    // Base graph type
    using BaseGraph = boost::adjacency_list<
        boost::vecS,      // Vertex container type
        boost::vecS,      // Edge container type
        boost::directedS, // Graph type
        VertexProperties, // Vertex properties
        EdgeProperties,   // Edge properties
        GraphProperties,  // Graph properties
        boost::listS      // Edge list type
        >;

    // Proxy class for diameter access
    class DiameterProxy
    {
    public:
        DiameterProxy(const BaseGraph &graph) : graph_(graph) {}

        // Implicit conversion to int for g.diameter usage
        operator int() const;

        // Assignment is not allowed (read-only property)
        DiameterProxy &operator=(int) = delete;

    private:
        const BaseGraph &graph_;
    };

    // Proxy class for num_vertices access
    class NumVerticesProxy
    {
    public:
        NumVerticesProxy(const BaseGraph &graph) : graph_(graph) {}

        // Implicit conversion to size_t for g.num_vertices usage
        operator size_t() const
        {
            return boost::num_vertices(graph_);
        }

        // Assignment is not allowed (read-only property)
        NumVerticesProxy &operator=(size_t) = delete;

    private:
        const BaseGraph &graph_;
    };

    // Proxy class for num_edges access
    class NumEdgesProxy
    {
    public:
        NumEdgesProxy(const BaseGraph &graph) : graph_(graph) {}

        // Implicit conversion to size_t for g.num_edges usage
        operator size_t() const
        {
            return boost::num_edges(graph_);
        }

        // Assignment is not allowed (read-only property)
        NumEdgesProxy &operator=(size_t) = delete;

    private:
        const BaseGraph &graph_;
    };

    // Proxy class for vertices access
    class VerticesProxy
    {
    public:
        VerticesProxy(const BaseGraph &graph) : graph_(graph) {}

        // Implicit conversion to vector<int32_t> for g.vertices usage
        operator std::vector<int32_t>() const;

        // Assignment is not allowed (read-only property)
        VerticesProxy &operator=(const std::vector<int32_t>&) = delete;

    private:
        const BaseGraph &graph_;
    };

    // Proxy class for edges access
    class EdgesProxy
    {
    public:
        EdgesProxy(const BaseGraph &graph) : graph_(graph) {}

        // Implicit conversion to vector<pair<int32_t, int32_t>> for g.edges usage
        operator std::vector<std::pair<int32_t, int32_t>>() const;

        // Assignment is not allowed (read-only property)
        EdgesProxy &operator=(const std::vector<std::pair<int32_t, int32_t>>&) = delete;

    private:
        const BaseGraph &graph_;
    };

    // Proxy class for num_dimensions access
    class NumDimensionsProxy
    {
    public:
        NumDimensionsProxy(const BaseGraph &graph) : graph_(graph) {}

        // Implicit conversion to size_t for g.num_dimensions usage
        operator size_t() const;

        // Assignment is not allowed (read-only property)
        NumDimensionsProxy &operator=(size_t) = delete;

    private:
        const BaseGraph &graph_;
    };

    // Graph class that inherits from boost::adjacency_list
    class Graph : public BaseGraph
    {
    public:
        // Default constructor
        Graph();

        // Copy constructor
        Graph(const BaseGraph &other);

        // Assignment operator
        Graph &operator=(const BaseGraph &other)
        {
            BaseGraph::operator=(other);
            return *this;
        }

        // Add vertex with integer id
        virtual void add_vertex(int32_t id);

        // Add edge between integer vertex ids
        virtual void add_edge(int32_t i, int32_t j);

        // Diameter proxy for g.diameter construct
        DiameterProxy diameter;

        // Proxy for g.num_vertices construct
        NumVerticesProxy num_vertices;

        // Proxy for g.num_edges construct
        NumEdgesProxy num_edges;

        // Proxy for g.vertices construct
        VerticesProxy vertices;

        // Proxy for g.edges construct
        EdgesProxy edges;

        // Proxy for g.num_dimensions construct
        NumDimensionsProxy num_dimensions;

    protected:
        // Get the diameter of the graph (longest shortest path)
        // Returns -1 if graph is disconnected or empty
        virtual int getDiameter() const;

        // Static helper for diameter calculation
        static int getDiameter_impl(const BaseGraph &g);

        // Friend class to access private methods
        friend class DiameterProxy;
        friend class VerticesProxy;
        friend class EdgesProxy;
        friend class NumDimensionsProxy;
    };

    // Forward declarations for specialized topologies
    class URing;
    class BRing;
    class UMesh;
    class BMesh;
    class BGrid;
    class OPG;

    // Proxy class for dimension access (for specialized topologies)
    class DimensionProxy
    {
    public:
        DimensionProxy(const Graph &graph) : graph_(graph) {}

        // Implicit conversion to size_t for g.dimension usage
        operator size_t() const;

        // Assignment is not allowed (read-only property)
        DimensionProxy &operator=(size_t) = delete;

    private:
        const Graph &graph_;
    };

    // Proxy class for multidimensional access (for BGrid)
    class MultiDimensionProxy
    {
    public:
        MultiDimensionProxy(const BGrid &grid);

        // Access individual dimensions
        size_t operator[](size_t index) const;

        // Get all dimensions
        const std::vector<size_t>& get() const;

        // Number of dimensions
        size_t size() const;

        // Assignment is not allowed (read-only property)
        MultiDimensionProxy &operator=(const std::vector<size_t>&) = delete;

    private:
        const BGrid &grid_;
    };

    // URing - Unidirectional Ring topology
    class URing : public Graph
    {
    public:
        // Constructor: creates a unidirectional ring with N vertices
        // N=1: single vertex with no edges
        // N>1: vertices "0","1",...,"N-1" connected in ring: 0→1→2→...→(N-1)→0
        explicit URing(size_t N);

        // Override add_vertex and add_edge to convert to generic graph when modified
        void add_vertex(int32_t id) override;
        void add_edge(int32_t i, int32_t j) override;

        // Proxy for g.dimension construct
        DimensionProxy dimension;

        // Get the size of the ring
        size_t GetDimensionSize() const;

    protected:
        // Override diameter calculation for ring topology
        int getDiameter() const override;

    private:
        size_t dimension_;
    };

    // BRing - Bidirectional Ring topology
    class BRing : public Graph
    {
    public:
        // Constructor: creates a bidirectional ring with N vertices
        // N=1: single vertex with no edges
        // N>1: vertices "0","1",...,"N-1" connected bidirectionally in ring: 0↔1↔2↔...↔(N-1)↔0
        explicit BRing(size_t N);

        // Override add_vertex and add_edge to convert to generic graph when modified
        void add_vertex(int32_t id) override;
        void add_edge(int32_t i, int32_t j) override;

        // Proxy for g.dimension construct
        DimensionProxy dimension;

        // Get the size of the ring
        size_t GetDimensionSize() const;

    protected:
        // Override diameter calculation for bidirectional ring topology
        int getDiameter() const override;

    private:
        size_t dimension_;
    };

    // UMesh - Unidirectional Mesh topology (1D linear chain without wrap-around)
    class UMesh : public Graph
    {
    public:
        // Constructor: creates a unidirectional mesh with N vertices
        // N=1: single vertex with no edges
        // N>1: vertices "0","1",...,"N-1" connected in chain: 0→1→2→...→(N-1)
        explicit UMesh(size_t N);

        // Override add_vertex and add_edge to convert to generic graph when modified
        void add_vertex(int32_t id) override;
        void add_edge(int32_t i, int32_t j) override;

        // Proxy for g.dimension construct
        DimensionProxy dimension;

        // Get the size of the mesh
        size_t GetDimensionSize() const;

    protected:
        // Override diameter calculation for mesh topology
        int getDiameter() const override;

    private:
        size_t dimension_;
    };

    // OPG - One-Point Graph topology (single vertex, no edges)
    class OPG : public Graph
    {
    public:
        // Constructor: creates a single vertex with ID 0
        OPG();

        // Override add_vertex and add_edge to convert to generic graph when modified
        void add_vertex(int32_t id) override;
        void add_edge(int32_t i, int32_t j) override;

        // Proxy for g.dimension construct (always returns 1)
        DimensionProxy dimension;

        // Get the dimension size (always 1)
        size_t GetDimensionSize() const;

    protected:
        // Override diameter calculation (always 0)
        int getDiameter() const override;

    private:
        size_t dimension_;  // Always 1
    };

    // BMesh - Bidirectional Mesh topology (1D linear chain with bidirectional edges)
    class BMesh : public Graph
    {
    public:
        // Constructor: creates a bidirectional mesh with N vertices
        // N=1: single vertex with no edges
        // N>1: vertices "0","1",...,"N-1" connected bidirectionally: 0↔1↔2↔...↔(N-1)
        explicit BMesh(size_t N);

        // Override add_vertex and add_edge to convert to generic graph when modified
        void add_vertex(int32_t id) override;
        void add_edge(int32_t i, int32_t j) override;

        // Proxy for g.dimension construct
        DimensionProxy dimension;

        // Get the size of the mesh
        size_t GetDimensionSize() const;

    protected:
        // Override diameter calculation for bidirectional mesh topology
        int getDiameter() const override;

    private:
        size_t dimension_;
    };

    // BGrid - Multidimensional Bidirectional Grid topology (Cartesian products of BMesh)
    class BGrid : public Graph
    {
    public:
        // Constructor: creates grid from vector of positive dimensions
        // Empty vector {} → OPG (single vertex)
        // Single dimension {N} → BMesh(N)
        // Multiple dimensions {N1, N2, ...} → Left associative gproduct of BMesh's
        explicit BGrid(const std::vector<size_t>& dimensions);

        // Override add_vertex and add_edge to convert to generic graph when modified
        void add_vertex(int32_t id) override;
        void add_edge(int32_t i, int32_t j) override;

        // Dimension access
        const std::vector<size_t>& GetDimensions() const;
        size_t GetNumDimensions() const;

        // Multi-dimensional proxy access
        MultiDimensionProxy dimensions;

    protected:
        // Override diameter calculation for grid topology
        int getDiameter() const override;

    private:
        std::vector<size_t> dimensions_;

        // Helper method to construct the grid using left-associative gproduct
        void buildGrid(const std::vector<size_t>& dims);

        // Helper method to calculate edge count for multidimensional grids
        size_t calculateGridEdges(const std::vector<size_t>& dims) const;
    };

    // Type alias for convenience
    using Grid = BGrid;

    // Cartesian product utility functions
    namespace gproduct_utils
    {
        // Encode vertex pair (g1_id, g2_id) into single ID
        int32_t encode_vertex_pair(int32_t g1_id, int32_t g2_id, size_t g2_size);

        // Decode product vertex ID back to pair
        std::pair<int32_t, int32_t> decode_vertex_pair(int32_t product_id, size_t g2_size);
    }

    // Cartesian product of two graphs
    // Vertex set: |V(G1 ⊗ G2)| = |V(G1)| × |V(G2)| (scalar product)
    // Edge set: |E(G1 ⊗ G2)| = |V(G1)| × |E(G2)| + |E(G1)| × |V(G2)|
    // Edge rule: (u1,v1) connects to (u2,v2) iff:
    // - u1 = u2 AND v1 connects to v2 in H, OR
    // - u1 connects to u2 in G AND v1 = v2
    Graph gproduct(const Graph &g1, const Graph &g2);

    // Operator overload for Cartesian product
    Graph operator*(const Graph &g1, const Graph &g2);

} // namespace topology

#endif // TOPOLOGY_CORE_H_
