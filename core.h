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
        void add_vertex(int32_t id);

        // Add edge between integer vertex ids
        void add_edge(int32_t i, int32_t j);

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
    };

    // URing - Unidirectional Ring topology
    class URing : public Graph
    {
    public:
        // Constructor: creates a unidirectional ring with N vertices
        // N=1: single vertex with no edges
        // N>1: vertices "0","1",...,"N-1" connected in ring: 0→1→2→...→(N-1)→0
        explicit URing(size_t N);

        // Get the size of the ring
        size_t GetRingSize() const;

    protected:
        // Override diameter calculation for ring topology
        int getDiameter() const override;

    private:
        size_t ring_size_;

        // Disable add_vertex and add_edge for URing - ring structure is fixed
        void add_vertex(int32_t id) = delete;
        void add_edge(int32_t i, int32_t j) = delete;
    };

    // UMesh - Unidirectional Mesh topology (1D linear chain without wrap-around)
    class UMesh : public Graph
    {
    public:
        // Constructor: creates a unidirectional mesh with N vertices
        // N=1: single vertex with no edges
        // N>1: vertices "0","1",...,"N-1" connected in chain: 0→1→2→...→(N-1)
        explicit UMesh(size_t N);

        // Get the size of the mesh
        size_t GetMeshSize() const;

    protected:
        // Override diameter calculation for mesh topology
        int getDiameter() const override;

    private:
        size_t mesh_size_;

        // Disable add_vertex and add_edge for UMesh - mesh structure is fixed
        void add_vertex(int32_t id) = delete;
        void add_edge(int32_t i, int32_t j) = delete;
    };

} // namespace topology

#endif // TOPOLOGY_CORE_H_
