# Topology

A C++ library for creating and manipulating network topologies using Boost Graph Library.

## Overview

This library provides a high-level interface for working with network topologies, built on top of the Boost Graph Library. It includes a generic graph class and specialized topology implementations like unidirectional rings.

## Features

- **Generic Graph Class**: A flexible graph implementation with integer vertex IDs
- **Specialized Topologies**: Pre-built topology classes like `URing` for unidirectional rings and `UMesh` for linear chains
- **Cartesian Product Operations**: Create complex topologies by combining simpler graphs using Cartesian products
- **Proxy Access Patterns**: Convenient access to graph properties using proxy objects
- **Property Support**: Built-in support for vertex, edge, and graph properties
- **Flexible Modification**: Specialized topologies can be modified, automatically converting to generic graphs

## Core Components

### Graph Class
The `Graph` class inherits from `boost::adjacency_list` and provides:
- Integer-based vertex IDs (`int32_t`)
- Methods for adding vertices and edges
- Proxy access to graph properties

### Proxy Properties
Access graph information through convenient proxy objects:
- `g.diameter` - Graph diameter (longest shortest path)
- `g.num_vertices` - Number of vertices
- `g.num_edges` - Number of edges  
- `g.vertices` - Vector of all vertex IDs
- `g.edges` - Vector of all edge pairs (source, destination)
- `g.dimension` - Dimension size for specialized topologies (URing, UMesh)

### URing Class
Specialized topology for unidirectional rings:
- Constructor takes ring size N
- Creates vertices 0, 1, ..., N-1
- Connects them in a ring: 0→1→2→...→(N-1)→0
- Allows modification via `add_vertex`/`add_edge`, but converts to generic graph (name changes to "Generic")
- Optimized diameter calculation: floor(N/2)

### UMesh Class
Specialized topology for unidirectional linear chains (1D mesh):
- Constructor takes mesh size N
- Creates vertices 0, 1, ..., N-1
- Connects them in a linear chain: 0→1→2→...→(N-1) (no wrap-around)
- Allows modification via `add_vertex`/`add_edge`, but converts to generic graph (name changes to "Generic")
- Optimized diameter calculation: N-1

### Cartesian Product Operations
Create complex topologies by combining simpler graphs using Cartesian products:
- **Function**: `gproduct(g1, g2)` - Creates the Cartesian product of two graphs
- **Operator**: `g1 * g2` - Convenient syntax for Cartesian products
- **Vertex Count**: `|V(G₁ ⊗ G₂)| = |V(G₁)| × |V(G₂)|` (scalar product formula)
- **Edge Count**: `|E(G₁ ⊗ G₂)| = |V(G₁)| × |E(G₂)| + |E(G₁)| × |V(G₂)|`
- **Edge Rule**: `(u₁,v₁)` connects to `(u₂,v₂)` if and only if:
  - `u₁ = u₂` AND `v₁` connects to `v₂` in the second graph, OR
  - `u₁` connects to `u₂` in the first graph AND `v₁ = v₂`
- **Applications**: Create grids, tori, cylinders, and other complex network topologies

## Data Structures

### Vertex Properties
```cpp
struct VertexProperties {
    int32_t id;
};
```

### Edge Properties
```cpp
struct EdgeProperties {
    double latency;
    double bandwidth;
};
```

### Graph Properties
```cpp
struct GraphProperties {
    std::string name;
};
```

## Usage Examples

### Creating a Generic Graph
```cpp
#include "core.h"
using namespace topology;

Graph g;
g.add_vertex(1);
g.add_vertex(2);
g.add_vertex(3);
g.add_edge(1, 2);
g.add_edge(2, 3);

// Access properties
std::cout << "Vertices: " << g.num_vertices << std::endl;
std::cout << "Edges: " << g.num_edges << std::endl;
std::cout << "Diameter: " << g.diameter << std::endl;

// Get vertex and edge lists
std::vector<int32_t> vertices = g.vertices;
std::vector<std::pair<int32_t, int32_t>> edges = g.edges;
```

### Creating a Unidirectional Ring
```cpp
URing ring(5);  // Creates ring with vertices 0,1,2,3,4

std::cout << "Ring size: " << ring.dimension << std::endl;  // 5
std::cout << "Diameter: " << ring.diameter << std::endl;    // floor(5/2) = 2

// Modification is allowed but converts to generic graph:
ring.add_vertex(10);
std::cout << "Graph name: " << ring[boost::graph_bundle].name << std::endl;  // "Generic"
```

### Creating a Unidirectional Mesh (Linear Chain)
```cpp
UMesh mesh(5);  // Creates linear chain with vertices 0→1→2→3→4

std::cout << "Mesh size: " << mesh.dimension << std::endl;  // 5
std::cout << "Diameter: " << mesh.diameter << std::endl;    // N-1 = 4

// Modification is allowed but converts to generic graph:
mesh.add_vertex(10);
std::cout << "Graph name: " << mesh[boost::graph_bundle].name << std::endl;  // "Generic"
```

### Cartesian Product Operations

#### Creating a 2D Grid
```cpp
UMesh path1(3);  // 0→1→2 (3 vertices, 2 edges)
UMesh path2(3);  // 0→1→2 (3 vertices, 2 edges)

Graph grid = gproduct(path1, path2);
// Creates 3×3 grid with 9 vertices and 12 edges
// Vertices: 3 × 3 = 9
// Edges: 3×2 + 2×3 = 6 + 6 = 12

std::cout << "Grid vertices: " << grid.num_vertices << std::endl;  // 9
std::cout << "Grid edges: " << grid.num_edges << std::endl;        // 12
```

#### Creating a Torus
```cpp
URing ring1(4);  // 0→1→2→3→0
URing ring2(4);  // 0→1→2→3→0

Graph torus = ring1 * ring2;  // Using operator syntax
// Creates 4×4 torus with 16 vertices and 32 edges

std::cout << "Torus name: " << torus[boost::graph_bundle].name << std::endl;  // "URing ⊗ URing"
```

#### Creating a Cylinder
```cpp
URing ring(6);   // 0→1→2→3→4→5→0
UMesh path(4);   // 0→1→2→3

Graph cylinder = gproduct(ring, path);
// Creates cylindrical topology with 24 vertices

std::cout << "Cylinder vertices: " << cylinder.num_vertices << std::endl;  // 24
std::cout << "Cylinder edges: " << cylinder.num_edges << std::endl;        // 39
```

#### Mixed Topology Products
```cpp
// Create complex topologies by combining different base structures
URing ring(5);
UMesh mesh(3);

Graph complex = ring * mesh;  // 5×3 cylindrical mesh
Graph another = gproduct(mesh, ring);  // 3×5 cylindrical mesh (different orientation)
```

## Building

This project uses Bazel for building:

```bash
# Build the library
bazel build //:core

# Run tests
bazel test //:core_test
```

## Testing

The library includes comprehensive tests using Google Test framework:
- Graph creation and manipulation
- Proxy property access
- URing topology behavior
- UMesh topology behavior
- Cartesian product operations
- Diameter calculations
- Type safety enforcement

## Dependencies

- Boost Graph Library
- Google Test (for testing)
- Bazel (build system)

## License

[Add your license information here]
