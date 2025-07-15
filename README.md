# Topology

A C++ library for creating and manipulating network topologies using Boost Graph Library.

## Overview

This library provides a high-level interface for working with network topologies, built on top of the Boost Graph Library. It includes a generic graph class and specialized topology implementations like unidirectional rings.

## Features

- **Generic Graph Class**: A flexible graph implementation with integer vertex IDs
- **Specialized Topologies**: Pre-built topology classes like `URing` for unidirectional rings
- **Proxy Access Patterns**: Convenient access to graph properties using proxy objects
- **Property Support**: Built-in support for vertex, edge, and graph properties
- **Type Safety**: Compile-time prevention of invalid operations on specialized topologies

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

### URing Class
Specialized topology for unidirectional rings:
- Constructor takes ring size N
- Creates vertices 0, 1, ..., N-1
- Connects them in a ring: 0→1→2→...→(N-1)→0
- Prevents modification after construction (disabled `add_vertex`/`add_edge`)
- Optimized diameter calculation: floor(N/2)

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

std::cout << "Ring size: " << ring.GetRingSize() << std::endl;
std::cout << "Diameter: " << ring.diameter << std::endl;  // floor(5/2) = 2

// These operations are disabled at compile time:
// ring.add_vertex(10);  // Compilation error
// ring.add_edge(0, 10); // Compilation error
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
- Diameter calculations
- Type safety enforcement

## Dependencies

- Boost Graph Library
- Google Test (for testing)
- Bazel (build system)

## License

[Add your license information here]
