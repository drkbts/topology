# Topology

A C++ library for creating and manipulating network topologies using Boost Graph Library.

## Overview

This library provides a high-level interface for working with network topologies, built on top of the Boost Graph Library. It includes a generic graph class and specialized topology implementations like unidirectional rings.

## Features

- **Generic Graph Class**: A flexible graph implementation with integer vertex IDs
- **Specialized Topologies**: Pre-built topology classes like `URing` for unidirectional rings, `BRing` for bidirectional rings, `UMesh` for unidirectional linear chains, `BMesh` for bidirectional linear chains, `BGrid` for multidimensional bidirectional grids, and `BTorus` for multidimensional bidirectional tori
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
- `g.dimension` - Dimension size for specialized topologies (URing, BRing, UMesh, BMesh) or multidimensional access for BGrid
- `g.num_dimensions` - Number of dimensions (1 for specialized topologies, varies for BGrid, 0 for generic graphs)

### URing Class
Specialized topology for unidirectional rings:
- Constructor takes ring size N
- Creates vertices 0, 1, ..., N-1
- Connects them in a ring: 0→1→2→...→(N-1)→0
- Allows modification via `add_vertex`/`add_edge`, but converts to generic graph (name changes to "Generic")
- Optimized diameter calculation: floor(N/2)

### BRing Class (alias: Ring)
Specialized topology for bidirectional rings:
- Constructor takes ring size N
- Creates vertices 0, 1, ..., N-1
- Connects them bidirectionally in a ring: 0↔1↔2↔...↔(N-1)↔0
- Has 2×N edges (twice as many as URing due to bidirectional connections)
- Allows modification via `add_vertex`/`add_edge`, but converts to generic graph (name changes to "Generic")
- Optimized diameter calculation: floor(N/2) (same as URing despite bidirectional edges)

### UMesh Class
Specialized topology for unidirectional linear chains (1D mesh):
- Constructor takes mesh size N
- Creates vertices 0, 1, ..., N-1
- Connects them in a linear chain: 0→1→2→...→(N-1) (no wrap-around)
- Allows modification via `add_vertex`/`add_edge`, but converts to generic graph (name changes to "Generic")
- Optimized diameter calculation: N-1

### BMesh Class (alias: Mesh)
Specialized topology for bidirectional linear chains (1D mesh):
- Constructor takes mesh size N
- Creates vertices 0, 1, ..., N-1
- Connects them bidirectionally: 0↔1↔2↔...↔(N-1) (no wrap-around)
- Has 2×(N-1) edges (twice as many as UMesh due to bidirectional connections)
- Allows modification via `add_vertex`/`add_edge`, but converts to generic graph (name changes to "Generic")
- Optimized diameter calculation: N-1 (same as UMesh despite bidirectional edges)

### OPG Class
Specialized topology for one-point graphs (single vertex, no edges):
- Default constructor creates a single vertex (vertex 0)
- Contains exactly one vertex and zero edges
- Allows modification via `add_vertex`/`add_edge`, but converts to generic graph (name changes to "Generic")
- Optimized diameter calculation: 0 (single vertex has zero diameter)
- Dimension proxy returns 1 (representing the single vertex)

### BGrid Class (alias: Grid)
Specialized topology for multidimensional bidirectional grids (Cartesian products of BMesh):
- Constructor takes vector of positive dimensions `{N1, N2, ..., Nk}`
- **Preprocessing**: Dimensions are sorted in descending order, dimensions equal to 1 are filtered out
- Empty vector `{}` or all dimensions = 1 → Creates OPG (single vertex), reports dimension `{1}`
- Single filtered dimension `{N}` → Creates BMesh(N)
- Multiple filtered dimensions → Left associative gproduct: `((BMesh(N1) ⊗ BMesh(N2)) ⊗ ...)`
- Vertex count: product of all filtered dimensions (N1 × N2 × ... × Nk)
- Edge count: calculated using iterative Cartesian product formulas
- Diameter: sum of individual mesh diameters (∑(Ni - 1))
- Multidimensional proxy access: `grid.dimensions[i]`, `grid.dimensions.size()` (returns filtered, sorted dimensions)
- Number of dimensions: `grid.num_dimensions` (returns count of filtered dimensions)
- Allows modification via `add_vertex`/`add_edge`, but converts to generic graph (name changes to "Generic")

### BTorus Class (alias: Torus)
Specialized topology for multidimensional bidirectional tori (Cartesian products of BRing):
- Constructor takes vector of positive dimensions `{N1, N2, ..., Nk}`
- **Preprocessing**: Dimensions are sorted in descending order, dimensions equal to 1 are filtered out
- Empty vector `{}` or all dimensions = 1 → Creates OPG (single vertex), reports dimension `{1}`
- Single filtered dimension `{N}` → Creates BRing(N)
- Multiple filtered dimensions → Left associative gproduct: `((BRing(N1) ⊗ BRing(N2)) ⊗ ...)`
- Vertex count: product of all filtered dimensions (N1 × N2 × ... × Nk)
- Edge count: calculated using iterative Cartesian product formulas
- Diameter: sum of individual ring diameters (∑⌊Ni/2⌋)
- Multidimensional proxy access: `torus.dimensions[i]`, `torus.dimensions.size()` (returns filtered, sorted dimensions)
- Number of dimensions: `torus.num_dimensions` (returns count of filtered dimensions)
- Allows modification via `add_vertex`/`add_edge`, but converts to generic graph (name changes to "Generic")

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

### Creating a Bidirectional Ring
```cpp
BRing ring(5);  // Creates bidirectional ring with vertices 0↔1↔2↔3↔4↔0
// Or using the type alias:
Ring ring_alias(5);  // Same as BRing(5)

std::cout << "Ring size: " << ring.dimension << std::endl;   // 5
std::cout << "Vertices: " << ring.num_vertices << std::endl;  // 5
std::cout << "Edges: " << ring.num_edges << std::endl;       // 2*N = 10
std::cout << "Diameter: " << ring.diameter << std::endl;     // floor(5/2) = 2

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

### Creating a Bidirectional Mesh (Linear Chain)
```cpp
BMesh mesh(5);  // Creates bidirectional chain with vertices 0↔1↔2↔3↔4
// Or using the type alias:
Mesh mesh_alias(5);  // Same as BMesh(5)

std::cout << "Mesh size: " << mesh.dimension << std::endl;  // 5
std::cout << "Vertices: " << mesh.num_vertices << std::endl;  // 5
std::cout << "Edges: " << mesh.num_edges << std::endl;       // 2*(N-1) = 8
std::cout << "Diameter: " << mesh.diameter << std::endl;     // N-1 = 4

// Modification is allowed but converts to generic graph:
mesh.add_vertex(10);
std::cout << "Graph name: " << mesh[boost::graph_bundle].name << std::endl;  // "Generic"
```

### Creating a One-Point Graph
```cpp
OPG opg;  // Creates single vertex graph with vertex 0

std::cout << "OPG size: " << opg.dimension << std::endl;     // 1
std::cout << "Vertices: " << opg.num_vertices << std::endl;  // 1
std::cout << "Edges: " << opg.num_edges << std::endl;       // 0
std::cout << "Diameter: " << opg.diameter << std::endl;     // 0

// Modification is allowed but converts to generic graph:
opg.add_vertex(1);
std::cout << "Graph name: " << opg[boost::graph_bundle].name << std::endl;  // "Generic"
```

### Creating Multidimensional Grids
```cpp
// 0D grid (point) - using type alias Grid
Grid point({});         // Equivalent to OPG (Grid is alias for BGrid)
std::cout << "Vertices: " << point.num_vertices << std::endl;  // 1
std::cout << "Edges: " << point.num_edges << std::endl;        // 0
std::cout << "Dimensions: " << point.dimensions.size() << std::endl; // 0

// 1D grid (line) - mixing BGrid and Grid usage
Grid line({5});         // Equivalent to BMesh(5)
std::cout << "Vertices: " << line.num_vertices << std::endl;   // 5
std::cout << "Edges: " << line.num_edges << std::endl;         // 8
std::cout << "Diameter: " << line.diameter << std::endl;       // 4

// 2D grid (rectangle)
BGrid rect({3, 4});     // Sorted to {4, 3} - 4×3 bidirectional grid
std::cout << "Vertices: " << rect.num_vertices << std::endl;   // 12
std::cout << "Edges: " << rect.num_edges << std::endl;         // 34
std::cout << "Diameter: " << rect.diameter << std::endl;       // 5
std::cout << "Dimensions: " << rect.dimensions[0] << "×" << rect.dimensions[1] << std::endl; // 4×3
std::cout << "Name: " << rect[boost::graph_bundle].name << std::endl;  // "BGrid[4,3]"

// 3D grid (box)
BGrid box({2, 3, 4});   // 2×3×4 bidirectional grid
std::cout << "Vertices: " << box.num_vertices << std::endl;    // 24
std::cout << "Diameter: " << box.diameter << std::endl;        // 7 (1+2+3+1)

// Access individual dimensions
for (size_t i = 0; i < box.dimensions.size(); ++i) {
    std::cout << "Dim " << i << ": " << box.dimensions[i] << std::endl;
}

// 4D grid (hypercube variation)
BGrid hyper({2, 2, 2, 2});  // 2^4 vertices
std::cout << "Vertices: " << hyper.num_vertices << std::endl;  // 16
std::cout << "Diameter: " << hyper.diameter << std::endl;      // 4

// Dimension filtering and sorting examples
BGrid filtered({3, 1, 5, 1, 2});  // Filters out 1s, sorts: {5, 3, 2}
std::cout << "Filtered dimensions: ";
for (size_t i = 0; i < filtered.dimensions.size(); ++i) {
    std::cout << filtered.dimensions[i];
    if (i < filtered.dimensions.size() - 1) std::cout << "×";
}
std::cout << std::endl;  // "5×3×2"

BGrid all_ones({1, 1, 1});  // All filtered out → OPG with dimension {1}
std::cout << "All ones vertices: " << all_ones.num_vertices << std::endl;  // 1
std::cout << "All ones dimension: " << all_ones.dimensions[0] << std::endl; // 1

// Modification converts to generic graph
rect.add_vertex(100);
std::cout << "Graph name: " << rect[boost::graph_bundle].name << std::endl;  // "Generic"
```

### Creating Multidimensional Tori
```cpp
// 0D torus (point) - using type alias Torus
Torus point({});         // Equivalent to OPG (Torus is alias for BTorus)
std::cout << "Vertices: " << point.num_vertices << std::endl;  // 1
std::cout << "Edges: " << point.num_edges << std::endl;        // 0
std::cout << "Dimensions: " << point.dimensions.size() << std::endl; // 0

// 1D torus (ring) - mixing BTorus and Torus usage
Torus ring({5});         // Equivalent to BRing(5)
std::cout << "Vertices: " << ring.num_vertices << std::endl;   // 5
std::cout << "Edges: " << ring.num_edges << std::endl;         // 10
std::cout << "Diameter: " << ring.diameter << std::endl;       // 2

// 2D torus (traditional torus)
BTorus torus2d({3, 4});  // Sorted to {4, 3} - 4×3 bidirectional torus
std::cout << "Vertices: " << torus2d.num_vertices << std::endl;   // 12
std::cout << "Edges: " << torus2d.num_edges << std::endl;         // 48
std::cout << "Diameter: " << torus2d.diameter << std::endl;       // 3 (floor(4/2) + floor(3/2) = 2+1)
std::cout << "Dimensions: " << torus2d.dimensions[0] << "×" << torus2d.dimensions[1] << std::endl; // 4×3
std::cout << "Name: " << torus2d[boost::graph_bundle].name << std::endl;  // "BTorus[4,3]"

// 3D torus (3-torus)
BTorus torus3d({2, 3, 4}); // 4×3×2 bidirectional torus
std::cout << "Vertices: " << torus3d.num_vertices << std::endl;    // 24
std::cout << "Diameter: " << torus3d.diameter << std::endl;        // 4 (floor(4/2)+floor(3/2)+floor(2/2) = 2+1+1)

// Access individual dimensions
for (size_t i = 0; i < torus3d.dimensions.size(); ++i) {
    std::cout << "Dim " << i << ": " << torus3d.dimensions[i] << std::endl;
}

// 4D torus (hypertorus)
BTorus hypertorus({2, 2, 2, 2});  // 2^4 vertices
std::cout << "Vertices: " << hypertorus.num_vertices << std::endl;  // 16
std::cout << "Diameter: " << hypertorus.diameter << std::endl;      // 4

// Dimension filtering and sorting examples
BTorus filtered({3, 1, 5, 1, 2});  // Filters out 1s, sorts: {5, 3, 2}
std::cout << "Filtered dimensions: ";
for (size_t i = 0; i < filtered.dimensions.size(); ++i) {
    std::cout << filtered.dimensions[i];
    if (i < filtered.dimensions.size() - 1) std::cout << "×";
}
std::cout << std::endl;  // "5×3×2"

// Compare with BGrid - same vertices, more edges, smaller diameter
BGrid grid_compare({3, 4});
BTorus torus_compare({3, 4});
std::cout << "Grid edges: " << grid_compare.num_edges << std::endl;    // Grid has fewer edges
std::cout << "Torus edges: " << torus_compare.num_edges << std::endl;  // Torus has more edges (wrap-around)
std::cout << "Grid diameter: " << grid_compare.diameter << std::endl;  // Grid has larger diameter
std::cout << "Torus diameter: " << torus_compare.diameter << std::endl; // Torus has smaller diameter

// Modification converts to generic graph
torus2d.add_vertex(100);
std::cout << "Graph name: " << torus2d[boost::graph_bundle].name << std::endl;  // "Generic"
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

#### Creating a Bidirectional Torus
```cpp
BRing ring1(4);  // 0↔1↔2↔3↔0 (bidirectional)
BRing ring2(4);  // 0↔1↔2↔3↔0 (bidirectional)
// Or using the Ring alias:
Ring ring3(4), ring4(4);

Graph btorus = ring1 * ring2;  // Bidirectional torus
// Creates 4×4 bidirectional torus with 16 vertices and 64 edges

std::cout << "BRing vertices: " << btorus.num_vertices << std::endl;  // 16
std::cout << "BRing edges: " << btorus.num_edges << std::endl;        // 64
std::cout << "BRing name: " << btorus[boost::graph_bundle].name << std::endl;  // "BRing ⊗ BRing"
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

// Mixing bidirectional and unidirectional topologies
BRing bring(4);  // Bidirectional ring
UMesh umesh(3);  // Unidirectional mesh

Graph hybrid = gproduct(bring, umesh);  // 4×3 hybrid topology
// Result: 4×3 = 12 vertices, 4×2 + 8×3 = 8 + 24 = 32 edges
```

#### Using OPG in Cartesian Products
```cpp
OPG point;      // Single vertex graph
UMesh path(5);  // Linear chain 0→1→2→3→4

Graph star = gproduct(point, path);  // Creates path graph (OPG acts as identity)
// Result: 1×5 = 5 vertices, 1×4 + 0×5 = 4 edges (same as original path)

std::cout << "Star vertices: " << star.num_vertices << std::endl;  // 5
std::cout << "Star edges: " << star.num_edges << std::endl;        // 4
```

#### BMesh in Cartesian Products
```cpp
BMesh bmesh(3);  // Bidirectional chain 0↔1↔2 (3 vertices, 4 edges)
UMesh umesh(3);  // Unidirectional chain 0→1→2 (3 vertices, 2 edges)
// Or using aliases:
Mesh bmesh_alias(3);

Graph hybrid = gproduct(bmesh, umesh);  // Mixed bidirectional/unidirectional product
// Result: 3×3 = 9 vertices, 3×2 + 4×3 = 6 + 12 = 18 edges

std::cout << "Hybrid vertices: " << hybrid.num_vertices << std::endl;  // 9
std::cout << "Hybrid edges: " << hybrid.num_edges << std::endl;        // 18
std::cout << "Hybrid name: " << hybrid[boost::graph_bundle].name << std::endl;  // "BMesh ⊗ UMesh"
```

#### BGrid/Grid in Cartesian Products
```cpp
Grid grid2d({3, 3});    // 2D bidirectional grid (9 vertices, 24 edges) - using Grid alias
URing ring(4);          // Unidirectional ring (4 vertices, 4 edges)

Graph complex3d = gproduct(grid2d, ring);  // Creates 3D structure
// Result: 9×4 = 36 vertices, 9×4 + 24×4 = 36 + 96 = 132 edges

std::cout << "3D vertices: " << complex3d.num_vertices << std::endl;  // 36
std::cout << "3D edges: " << complex3d.num_edges << std::endl;        // 132
std::cout << "3D name: " << complex3d[boost::graph_bundle].name << std::endl;  // "BGrid[3,3] ⊗ URing"

// BGrid can also be used with OPG (identity)
OPG point;
Graph same_grid = gproduct(grid2d, point);  // Results in same structure as grid2d
std::cout << "Same vertices: " << same_grid.num_vertices << std::endl;  // 9
std::cout << "Same edges: " << same_grid.num_edges << std::endl;        // 24
```

#### BTorus/Torus in Cartesian Products
```cpp
Torus torus2d({3, 3});  // 2D bidirectional torus (9 vertices, 36 edges) - using Torus alias
URing ring(4);          // Unidirectional ring (4 vertices, 4 edges)

Graph toroidal3d = gproduct(torus2d, ring);  // Creates 3D toroidal structure
// Result: 9×4 = 36 vertices, 9×4 + 36×4 = 36 + 144 = 180 edges

std::cout << "Toroidal 3D vertices: " << toroidal3d.num_vertices << std::endl;  // 36
std::cout << "Toroidal 3D edges: " << toroidal3d.num_edges << std::endl;        // 180
std::cout << "Toroidal 3D name: " << toroidal3d[boost::graph_bundle].name << std::endl;  // "BTorus[3,3] ⊗ URing"

// BTorus can also be used with OPG (identity)
Graph same_torus = gproduct(torus2d, point);  // Results in same structure as torus2d
std::cout << "Same torus vertices: " << same_torus.num_vertices << std::endl;  // 9
std::cout << "Same torus edges: " << same_torus.num_edges << std::endl;        // 36

// Compare grid vs torus in products
Grid grid_compare({2, 3});
BTorus torus_compare({2, 3});
BMesh mesh(4);

Graph grid_mesh = gproduct(grid_compare, mesh);
Graph torus_mesh = gproduct(torus_compare, mesh);
std::cout << "Grid×Mesh edges: " << grid_mesh.num_edges << std::endl;   // Fewer edges
std::cout << "Torus×Mesh edges: " << torus_mesh.num_edges << std::endl; // More edges (torus has wrap-around)
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
- BRing topology behavior
- UMesh topology behavior
- BMesh topology behavior
- BGrid topology behavior
- BTorus topology behavior
- OPG topology behavior
- Cartesian product operations
- Diameter calculations
- Type safety enforcement

## Dependencies

- Boost Graph Library
- Google Test (for testing)
- Bazel (build system)

## License

MIT License

Copyright (c) 2025 Topology Library Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
