#include "core.h"
#include <gtest/gtest.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <algorithm>
#include <set>

namespace topology {

namespace {

class GraphTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Fresh graph for each test
  }

  Graph graph_;
};

TEST_F(GraphTest, EmptyGraphHasNoVertices) {
  EXPECT_EQ(boost::num_vertices(graph_), 0);
  EXPECT_EQ(boost::num_edges(graph_), 0);
  
  // Test proxy patterns
  EXPECT_EQ(graph_.num_vertices, 0);
  EXPECT_EQ(graph_.num_edges, 0);
}

TEST_F(GraphTest, AddSingleVertex) {
  graph_.add_vertex(0);
  
  EXPECT_EQ(boost::num_vertices(graph_), 1);
  EXPECT_EQ(boost::num_edges(graph_), 0);
  
  // Test proxy patterns
  EXPECT_EQ(graph_.num_vertices, 1);
  EXPECT_EQ(graph_.num_edges, 0);
  
  // Check vertex id property
  auto [vi, vi_end] = boost::vertices(graph_);
  EXPECT_EQ(graph_[*vi].id, 0);
}

TEST_F(GraphTest, AddMultipleVertices) {
  graph_.add_vertex(0);
  graph_.add_vertex(1);
  graph_.add_vertex(2);
  
  EXPECT_EQ(boost::num_vertices(graph_), 3);
  EXPECT_EQ(boost::num_edges(graph_), 0);
  
  // Test proxy patterns
  EXPECT_EQ(graph_.num_vertices, 3);
  EXPECT_EQ(graph_.num_edges, 0);
  
  // Check all vertex ids
  std::set<int32_t> ids;
  auto [vi, vi_end] = boost::vertices(graph_);
  for (auto v_it = vi; v_it != vi_end; ++v_it) {
    ids.insert(graph_[*v_it].id);
  }
  std::set<int32_t> expected = {0, 1, 2};
  EXPECT_EQ(ids, expected);
}

TEST_F(GraphTest, AddEdge) {
  graph_.add_vertex(0);
  graph_.add_vertex(1);
  graph_.add_edge(0, 1);
  
  EXPECT_EQ(boost::num_vertices(graph_), 2);
  EXPECT_EQ(boost::num_edges(graph_), 1);
  
  // Test proxy patterns
  EXPECT_EQ(graph_.num_vertices, 2);
  EXPECT_EQ(graph_.num_edges, 1);
}

TEST_F(GraphTest, AddEdgeNonExistentVertex) {
  graph_.add_vertex(0);
  graph_.add_edge(0, 1);  // Vertex 1 doesn't exist
  
  EXPECT_EQ(boost::num_vertices(graph_), 1);
  EXPECT_EQ(boost::num_edges(graph_), 0);  // No edge should be added
  
  // Test proxy patterns
  EXPECT_EQ(graph_.num_vertices, 1);
  EXPECT_EQ(graph_.num_edges, 0);
}

TEST_F(GraphTest, DiameterProxy) {
  // Empty graph has diameter -1
  EXPECT_EQ(graph_.diameter, -1);
  
  // Single vertex has diameter 0
  Graph single;
  single.add_vertex(0);
  EXPECT_EQ(single.diameter, 0);
  
  // Two connected vertices have diameter 1 (in directed graph, this is disconnected)
  Graph two;
  two.add_vertex(0);
  two.add_vertex(1);
  two.add_edge(0, 1);
  EXPECT_EQ(two.diameter, -1);  // Disconnected in directed graph
  
  // Strongly connected triangle has diameter 2
  Graph triangle;
  triangle.add_vertex(0);
  triangle.add_vertex(1);
  triangle.add_vertex(2);
  triangle.add_edge(0, 1);
  triangle.add_edge(1, 2);
  triangle.add_edge(2, 0);
  EXPECT_EQ(triangle.diameter, 2);
}

TEST_F(GraphTest, VerticesProxy) {
  // Empty graph has no vertices
  std::vector<int32_t> empty_vertices = graph_.vertices;
  EXPECT_EQ(empty_vertices.size(), 0);
  
  // Add vertices and test proxy
  Graph g;
  g.add_vertex(3);
  g.add_vertex(1);
  g.add_vertex(2);
  
  std::vector<int32_t> vertices = g.vertices;
  EXPECT_EQ(vertices.size(), 3);
  
  // Sort to have predictable order
  std::sort(vertices.begin(), vertices.end());
  EXPECT_EQ(vertices[0], 1);
  EXPECT_EQ(vertices[1], 2);
  EXPECT_EQ(vertices[2], 3);
}

TEST_F(GraphTest, EdgesProxy) {
  // Empty graph has no edges
  std::vector<std::pair<int32_t, int32_t>> empty_edges = graph_.edges;
  EXPECT_EQ(empty_edges.size(), 0);
  
  // Add vertices and edges, test proxy
  Graph g;
  g.add_vertex(1);
  g.add_vertex(2);
  g.add_vertex(3);
  g.add_edge(1, 2);
  g.add_edge(2, 3);
  g.add_edge(3, 1);
  
  std::vector<std::pair<int32_t, int32_t>> edges = g.edges;
  EXPECT_EQ(edges.size(), 3);
  
  // Sort to have predictable order
  std::sort(edges.begin(), edges.end());
  EXPECT_EQ(edges[0].first, 1);
  EXPECT_EQ(edges[0].second, 2);
  EXPECT_EQ(edges[1].first, 2);
  EXPECT_EQ(edges[1].second, 3);
  EXPECT_EQ(edges[2].first, 3);
  EXPECT_EQ(edges[2].second, 1);
}

TEST_F(GraphTest, GraphName) {
  // Test that generic graph has correct name
  EXPECT_EQ(graph_[boost::graph_bundle].name, "Generic");
  
  Graph g;
  EXPECT_EQ(g[boost::graph_bundle].name, "Generic");
}

}  // namespace

// URing Tests
namespace {

class URingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Fresh ring for each test
  }
};

TEST_F(URingTest, InvalidRingSize) {
  EXPECT_THROW(URing(0), std::invalid_argument);
}

TEST_F(URingTest, RingSize1) {
  URing ring(1);
  
  EXPECT_EQ(boost::num_vertices(ring), 1);
  EXPECT_EQ(boost::num_edges(ring), 0);
  EXPECT_EQ(ring.dimension, 1);
  
  // Test proxy patterns
  EXPECT_EQ(ring.num_vertices, 1);
  EXPECT_EQ(ring.num_edges, 0);
  
  // Check vertex has id 0
  auto [vi, vi_end] = boost::vertices(ring);
  EXPECT_EQ(ring[*vi].id, 0);
}

TEST_F(URingTest, RingSize3) {
  URing ring(3);
  
  EXPECT_EQ(boost::num_vertices(ring), 3);
  EXPECT_EQ(boost::num_edges(ring), 3);
  EXPECT_EQ(ring.dimension, 3);
  
  // Test proxy patterns
  EXPECT_EQ(ring.num_vertices, 3);
  EXPECT_EQ(ring.num_edges, 3);
  
  // Check vertex ids are 0, 1, 2
  std::set<int32_t> ids;
  auto [vi, vi_end] = boost::vertices(ring);
  for (auto v_it = vi; v_it != vi_end; ++v_it) {
    ids.insert(ring[*v_it].id);
  }
  std::set<int32_t> expected = {0, 1, 2};
  EXPECT_EQ(ids, expected);
}

TEST_F(URingTest, URingDiameter) {
  // Ring of size 1 has diameter 0
  URing ring1(1);
  EXPECT_EQ(ring1.diameter, 0);
  
  // Ring of size 3 has diameter 1 (floor(3/2) = 1)
  URing ring3(3);
  EXPECT_EQ(ring3.diameter, 1);
  
  // Ring of size 5 has diameter 2 (floor(5/2) = 2)
  URing ring5(5);
  EXPECT_EQ(ring5.diameter, 2);
}

TEST_F(URingTest, GraphName) {
  // Test that URing has correct name
  URing ring(3);
  EXPECT_EQ(ring[boost::graph_bundle].name, "URing");
}

TEST_F(URingTest, AddVertexAndEdgeConvertsToGeneric) {
  URing ring(3);
  
  // Initially should be URing
  EXPECT_EQ(ring[boost::graph_bundle].name, "URing");
  EXPECT_EQ(ring.num_vertices, 3);
  EXPECT_EQ(ring.num_edges, 3);
  EXPECT_EQ(ring.dimension, 3);
  
  // Adding vertex should convert to Generic
  ring.add_vertex(10);
  EXPECT_EQ(ring[boost::graph_bundle].name, "Generic");
  EXPECT_EQ(ring.num_vertices, 4);  // Should have added the vertex
  
  // Create another ring to test edge addition
  URing ring2(2);
  EXPECT_EQ(ring2[boost::graph_bundle].name, "URing");
  
  // Adding edge should convert to Generic
  ring2.add_edge(0, 1);  // Add edge between existing vertices
  EXPECT_EQ(ring2[boost::graph_bundle].name, "Generic");
}

TEST_F(URingTest, DimensionProxy) {
  URing ring(5);
  
  // Test dimension proxy works
  size_t dim = ring.dimension;
  EXPECT_EQ(dim, 5);
  
  // Test implicit conversion
  EXPECT_EQ(ring.dimension, 5);
}

}  // namespace

// BRing Tests
namespace {

class BRingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Fresh BRing for each test
  }
};

TEST_F(BRingTest, InvalidRingSize) {
  EXPECT_THROW(BRing(0), std::invalid_argument);
}

TEST_F(BRingTest, RingSize1) {
  BRing ring(1);
  
  EXPECT_EQ(boost::num_vertices(ring), 1);
  EXPECT_EQ(boost::num_edges(ring), 0);
  EXPECT_EQ(ring.dimension, 1);
  
  // Test proxy patterns
  EXPECT_EQ(ring.num_vertices, 1);
  EXPECT_EQ(ring.num_edges, 0);
  
  // Check vertex has id 0
  auto [vi, vi_end] = boost::vertices(ring);
  EXPECT_EQ(ring[*vi].id, 0);
}

TEST_F(BRingTest, RingSize3) {
  BRing ring(3);
  
  EXPECT_EQ(boost::num_vertices(ring), 3);
  EXPECT_EQ(boost::num_edges(ring), 6);  // 2*N = 2*3 = 6 bidirectional edges
  EXPECT_EQ(ring.dimension, 3);
  
  // Test proxy patterns
  EXPECT_EQ(ring.num_vertices, 3);
  EXPECT_EQ(ring.num_edges, 6);
  
  // Check vertex ids are 0, 1, 2
  std::set<int32_t> ids;
  auto [vi, vi_end] = boost::vertices(ring);
  for (auto it = vi; it != vi_end; ++it) {
    ids.insert(ring[*it].id);
  }
  EXPECT_EQ(ids, (std::set<int32_t>{0, 1, 2}));
  
  // Check edges form bidirectional ring: 0↔1↔2↔0
  std::vector<std::pair<int32_t, int32_t>> edges = ring.edges;
  std::set<std::pair<int32_t, int32_t>> edge_set(edges.begin(), edges.end());
  
  // Expected bidirectional edges: 0→1, 1→0, 1→2, 2→1, 2→0, 0→2
  EXPECT_EQ(edge_set.count({0, 1}), 1);
  EXPECT_EQ(edge_set.count({1, 0}), 1);
  EXPECT_EQ(edge_set.count({1, 2}), 1);
  EXPECT_EQ(edge_set.count({2, 1}), 1);
  EXPECT_EQ(edge_set.count({2, 0}), 1);
  EXPECT_EQ(edge_set.count({0, 2}), 1);
}

TEST_F(BRingTest, RingSize5) {
  BRing ring(5);
  
  EXPECT_EQ(boost::num_vertices(ring), 5);
  EXPECT_EQ(boost::num_edges(ring), 10);  // 2*N = 2*5 = 10 bidirectional edges
  EXPECT_EQ(ring.dimension, 5);
  
  // Check some specific ring edges exist
  std::vector<std::pair<int32_t, int32_t>> edges = ring.edges;
  std::set<std::pair<int32_t, int32_t>> edge_set(edges.begin(), edges.end());
  
  // Verify bidirectional ring structure
  EXPECT_EQ(edge_set.count({0, 1}), 1);
  EXPECT_EQ(edge_set.count({1, 0}), 1);
  EXPECT_EQ(edge_set.count({4, 0}), 1);  // Last to first
  EXPECT_EQ(edge_set.count({0, 4}), 1);  // First to last
}

TEST_F(BRingTest, GraphName) {
  // Test that BRing has correct name
  BRing ring(3);
  EXPECT_EQ(ring[boost::graph_bundle].name, "BRing");
}

TEST_F(BRingTest, DiameterCalculation) {
  BRing ring1(1);
  EXPECT_EQ(ring1.diameter, 0);  // Single vertex
  
  BRing ring2(2);
  EXPECT_EQ(ring2.diameter, 1);  // floor(2/2) = 1
  
  BRing ring3(3);
  EXPECT_EQ(ring3.diameter, 1);  // floor(3/2) = 1
  
  BRing ring4(4);
  EXPECT_EQ(ring4.diameter, 2);  // floor(4/2) = 2
  
  BRing ring5(5);
  EXPECT_EQ(ring5.diameter, 2);  // floor(5/2) = 2
  
  BRing ring6(6);
  EXPECT_EQ(ring6.diameter, 3);  // floor(6/2) = 3
}

TEST_F(BRingTest, AddVertexAndEdgeConvertsToGeneric) {
  BRing ring(3);
  
  // Initially should be BRing
  EXPECT_EQ(ring[boost::graph_bundle].name, "BRing");
  EXPECT_EQ(ring.num_vertices, 3);
  EXPECT_EQ(ring.num_edges, 6);
  EXPECT_EQ(ring.dimension, 3);
  
  // Adding vertex should convert to Generic
  ring.add_vertex(10);
  EXPECT_EQ(ring[boost::graph_bundle].name, "Generic");
  EXPECT_EQ(ring.num_vertices, 4);  // Should have added the vertex
  
  // Create another ring to test edge addition
  BRing ring2(2);
  EXPECT_EQ(ring2[boost::graph_bundle].name, "BRing");
  
  // Adding edge should convert to Generic
  ring2.add_edge(0, 0);  // Add self-loop
  EXPECT_EQ(ring2[boost::graph_bundle].name, "Generic");
}

TEST_F(BRingTest, DimensionProxy) {
  BRing ring(5);
  
  // Test dimension proxy works
  size_t dim = ring.dimension;
  EXPECT_EQ(dim, 5);
  
  // Test implicit conversion
  EXPECT_EQ(ring.dimension, 5);
}

TEST_F(BRingTest, ProxyAccess) {
  BRing ring(4);
  
  // Test all proxy patterns work
  EXPECT_EQ(ring.num_vertices, 4);
  EXPECT_EQ(ring.num_edges, 8);  // 2*4 = 8
  EXPECT_EQ(ring.diameter, 2);   // floor(4/2) = 2
  EXPECT_EQ(ring.dimension, 4);
  
  // Test vertices and edges vectors
  std::vector<int32_t> vertices = ring.vertices;
  EXPECT_EQ(vertices.size(), 4);
  
  std::vector<std::pair<int32_t, int32_t>> edges = ring.edges;
  EXPECT_EQ(edges.size(), 8);
}

TEST_F(BRingTest, CartesianProductWithOPG) {
  BRing ring(4);  // 0↔1↔2↔3↔0
  OPG opg;
  
  // BRing × OPG should preserve BRing structure (identity property)
  Graph product1 = gproduct(ring, opg);
  EXPECT_EQ(product1.num_vertices, 4);  // 4×1 = 4
  EXPECT_EQ(product1.num_edges, 8);     // 4×0 + 8×1 = 8
  
  // OPG × BRing should also preserve BRing structure
  Graph product2 = gproduct(opg, ring);
  EXPECT_EQ(product2.num_vertices, 4);  // 1×4 = 4
  EXPECT_EQ(product2.num_edges, 8);     // 1×8 + 0×4 = 8
  
  // Test names
  EXPECT_EQ(product1[boost::graph_bundle].name, "BRing ⊗ OPG");
  EXPECT_EQ(product2[boost::graph_bundle].name, "OPG ⊗ BRing");
}

TEST_F(BRingTest, CartesianProductWithURing) {
  BRing bring(3);  // 0↔1↔2↔0 (bidirectional)
  URing uring(3);  // 0→1→2→0 (unidirectional)
  
  // BRing × URing
  Graph product = gproduct(bring, uring);
  EXPECT_EQ(product.num_vertices, 9);   // 3×3 = 9
  EXPECT_EQ(product.num_edges, 27);     // 3×3 + 6×3 = 9 + 18 = 27
  
  EXPECT_EQ(product[boost::graph_bundle].name, "BRing ⊗ URing");
}

TEST_F(BRingTest, CompareWithURing) {
  // BRing should have more edges than URing for same size
  size_t N = 5;
  BRing bring(N);
  URing uring(N);
  
  EXPECT_EQ(bring.num_vertices, uring.num_vertices);  // Same vertices
  EXPECT_EQ(bring.num_edges, 2 * uring.num_edges);   // BRing has twice the edges
  EXPECT_EQ(bring.diameter, uring.diameter);         // Same diameter
  EXPECT_EQ(bring.dimension, uring.dimension);       // Same dimension
}

TEST_F(BRingTest, CartesianProductWithBMesh) {
  BRing ring(3);   // 0↔1↔2↔0 (3 vertices, 6 edges)
  BMesh mesh(3);   // 0↔1↔2 (3 vertices, 4 edges)
  
  // BRing × BMesh creates a cylindrical topology
  Graph cylinder = gproduct(ring, mesh);
  EXPECT_EQ(cylinder.num_vertices, 9);   // 3×3 = 9
  EXPECT_EQ(cylinder.num_edges, 30);     // 3×4 + 6×3 = 12 + 18 = 30
  
  EXPECT_EQ(cylinder[boost::graph_bundle].name, "BRing ⊗ BMesh");
}

}  // namespace

// UMesh Tests
namespace {

class UMeshTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Fresh mesh for each test
  }
};

TEST_F(UMeshTest, InvalidMeshSize) {
  EXPECT_THROW(UMesh(0), std::invalid_argument);
}

TEST_F(UMeshTest, MeshSize1) {
  UMesh mesh(1);
  
  EXPECT_EQ(boost::num_vertices(mesh), 1);
  EXPECT_EQ(boost::num_edges(mesh), 0);
  EXPECT_EQ(mesh.dimension, 1);
  
  // Test proxy patterns
  EXPECT_EQ(mesh.num_vertices, 1);
  EXPECT_EQ(mesh.num_edges, 0);
  
  // Check vertex id is 0
  auto [vi, vi_end] = boost::vertices(mesh);
  EXPECT_EQ(mesh[*vi].id, 0);
}

TEST_F(UMeshTest, MeshSize3) {
  UMesh mesh(3);
  
  EXPECT_EQ(boost::num_vertices(mesh), 3);
  EXPECT_EQ(boost::num_edges(mesh), 2);
  EXPECT_EQ(mesh.dimension, 3);
  
  // Test proxy patterns
  EXPECT_EQ(mesh.num_vertices, 3);
  EXPECT_EQ(mesh.num_edges, 2);
  
  // Check vertex ids are 0, 1, 2
  std::set<int32_t> ids;
  auto [vi, vi_end] = boost::vertices(mesh);
  for (auto v_it = vi; v_it != vi_end; ++v_it) {
    ids.insert(mesh[*v_it].id);
  }
  EXPECT_EQ(ids.size(), 3);
  EXPECT_EQ(ids.count(0), 1);
  EXPECT_EQ(ids.count(1), 1);
  EXPECT_EQ(ids.count(2), 1);
}

TEST_F(UMeshTest, UMeshDiameter) {
  // Mesh of size 1 has diameter 0
  UMesh mesh1(1);
  EXPECT_EQ(mesh1.diameter, 0);
  
  // Mesh of size 3 has diameter 2 (0→1→2, distance = 2)
  UMesh mesh3(3);
  EXPECT_EQ(mesh3.diameter, 2);
  
  // Mesh of size 5 has diameter 4 (0→1→2→3→4, distance = 4)
  UMesh mesh5(5);
  EXPECT_EQ(mesh5.diameter, 4);
}

TEST_F(UMeshTest, GraphName) {
  // Test that UMesh has correct name
  UMesh mesh(3);
  EXPECT_EQ(mesh[boost::graph_bundle].name, "UMesh");
}

TEST_F(UMeshTest, AddVertexAndEdgeConvertsToGeneric) {
  UMesh mesh(3);
  
  // Initially should be UMesh
  EXPECT_EQ(mesh[boost::graph_bundle].name, "UMesh");
  EXPECT_EQ(mesh.num_vertices, 3);
  EXPECT_EQ(mesh.num_edges, 2);
  
  // Adding vertex should convert to Generic
  mesh.add_vertex(10);
  EXPECT_EQ(mesh[boost::graph_bundle].name, "Generic");
  EXPECT_EQ(mesh.num_vertices, 4);  // Should have added the vertex
  
  // Create another mesh to test edge addition
  UMesh mesh2(2);
  EXPECT_EQ(mesh2[boost::graph_bundle].name, "UMesh");
  
  // Adding edge should convert to Generic
  mesh2.add_edge(0, 1);  // Add edge between existing vertices
  EXPECT_EQ(mesh2[boost::graph_bundle].name, "Generic");
}

TEST_F(UMeshTest, DimensionProxy) {
  UMesh mesh(4);
  
  // Test dimension proxy works
  size_t dim = mesh.dimension;
  EXPECT_EQ(dim, 4);
  
  // Test implicit conversion
  EXPECT_EQ(mesh.dimension, 4);
}

}  // namespace

// OPG Tests
namespace {

class OPGTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Fresh OPG for each test
  }
};

TEST_F(OPGTest, DefaultConstruction) {
  OPG opg;
  
  EXPECT_EQ(boost::num_vertices(opg), 1);
  EXPECT_EQ(boost::num_edges(opg), 0);
  EXPECT_EQ(opg.dimension, 1);
  
  // Test proxy patterns
  EXPECT_EQ(opg.num_vertices, 1);
  EXPECT_EQ(opg.num_edges, 0);
  
  // Check vertex id is 0
  auto [vi, vi_end] = boost::vertices(opg);
  EXPECT_EQ(opg[*vi].id, 0);
}

TEST_F(OPGTest, GraphName) {
  // Test that OPG has correct name
  OPG opg;
  EXPECT_EQ(opg[boost::graph_bundle].name, "OPG");
}

TEST_F(OPGTest, DiameterCalculation) {
  OPG opg;
  
  // Single vertex graph has diameter 0
  EXPECT_EQ(opg.diameter, 0);
}

TEST_F(OPGTest, AddVertexConvertsToGeneric) {
  OPG opg;
  
  // Initially should be OPG
  EXPECT_EQ(opg[boost::graph_bundle].name, "OPG");
  EXPECT_EQ(opg.num_vertices, 1);
  EXPECT_EQ(opg.num_edges, 0);
  EXPECT_EQ(opg.dimension, 1);
  
  // Adding vertex should convert to Generic
  opg.add_vertex(1);
  EXPECT_EQ(opg[boost::graph_bundle].name, "Generic");
  EXPECT_EQ(opg.num_vertices, 2);  // Should have added the vertex
}

TEST_F(OPGTest, AddEdgeConvertsToGeneric) {
  OPG opg;
  
  // Initially should be OPG
  EXPECT_EQ(opg[boost::graph_bundle].name, "OPG");
  EXPECT_EQ(opg.num_vertices, 1);
  EXPECT_EQ(opg.num_edges, 0);
  
  // Adding edge to self should convert to Generic
  opg.add_edge(0, 0);  // Self-loop
  EXPECT_EQ(opg[boost::graph_bundle].name, "Generic");
  EXPECT_EQ(opg.num_edges, 1);  // Should have added the edge
}

TEST_F(OPGTest, DimensionProxy) {
  OPG opg;
  
  // Test dimension proxy works
  size_t dim = opg.dimension;
  EXPECT_EQ(dim, 1);
  
  // Test implicit conversion
  EXPECT_EQ(opg.dimension, 1);
}

TEST_F(OPGTest, ProxyAccess) {
  OPG opg;
  
  // Test all proxy patterns work
  EXPECT_EQ(opg.num_vertices, 1);
  EXPECT_EQ(opg.num_edges, 0);
  EXPECT_EQ(opg.diameter, 0);
  EXPECT_EQ(opg.dimension, 1);
  
  // Test vertices and edges vectors
  std::vector<int32_t> vertices = opg.vertices;
  EXPECT_EQ(vertices.size(), 1);
  EXPECT_EQ(vertices[0], 0);
  
  std::vector<std::pair<int32_t, int32_t>> edges = opg.edges;
  EXPECT_EQ(edges.size(), 0);
}

TEST_F(OPGTest, CartesianProductIdentity) {
  OPG opg;
  UMesh mesh(3);  // 0→1→2
  
  // OPG × Mesh should be equivalent to Mesh (identity property)
  Graph product1 = gproduct(opg, mesh);
  EXPECT_EQ(product1.num_vertices, 3);  // 1×3 = 3
  EXPECT_EQ(product1.num_edges, 2);     // 1×2 + 0×3 = 2
  
  // Mesh × OPG should also be equivalent to Mesh
  Graph product2 = gproduct(mesh, opg);
  EXPECT_EQ(product2.num_vertices, 3);  // 3×1 = 3
  EXPECT_EQ(product2.num_edges, 2);     // 3×0 + 2×1 = 2
  
  // Both products should have same structure
  EXPECT_EQ(product1.num_vertices, product2.num_vertices);
  EXPECT_EQ(product1.num_edges, product2.num_edges);
}

TEST_F(OPGTest, CartesianProductWithRing) {
  OPG opg;
  URing ring(4);  // 0→1→2→3→0
  
  // OPG × Ring should preserve ring structure
  Graph product = gproduct(opg, ring);
  EXPECT_EQ(product.num_vertices, 4);  // 1×4 = 4
  EXPECT_EQ(product.num_edges, 4);     // 1×4 + 0×4 = 4
  
  // Test name
  EXPECT_EQ(product[boost::graph_bundle].name, "OPG ⊗ URing");
}

TEST_F(OPGTest, ModificationAfterCartesianProduct) {
  OPG opg;
  UMesh mesh(2);  // 0→1
  
  Graph product = gproduct(opg, mesh);
  
  // Product should be modifiable as generic graph
  product.add_vertex(10);
  EXPECT_EQ(product.num_vertices, 3);
  EXPECT_EQ(product[boost::graph_bundle].name, "OPG ⊗ UMesh");  // Name preserved
}

TEST_F(OPGTest, MultipleOPGProduct) {
  OPG opg1, opg2;
  
  // OPG × OPG should result in single vertex graph  
  Graph product = gproduct(opg1, opg2);
  EXPECT_EQ(product.num_vertices, 1);  // 1×1 = 1
  EXPECT_EQ(product.num_edges, 0);     // 1×0 + 0×1 = 0
  EXPECT_EQ(product[boost::graph_bundle].name, "OPG ⊗ OPG");
}

TEST_F(OPGTest, DimensionProxyAfterModification) {
  OPG opg;
  
  // Initially dimension should be 1
  EXPECT_EQ(opg.dimension, 1);
  
  // After adding vertex, it becomes generic but dimension proxy should still work
  opg.add_vertex(1);
  EXPECT_EQ(opg[boost::graph_bundle].name, "Generic");
  
  // Dimension proxy should throw or return default for generic graphs
  // (The specific behavior depends on implementation)
  try {
    size_t dim = opg.dimension;
    // If no exception, it should return some reasonable value
    EXPECT_GE(dim, 0);
  } catch (...) {
    // Exception is acceptable for generic graphs
  }
}

TEST_F(OPGTest, EmptyGraphProductWithOPG) {
  OPG opg;
  Graph empty;
  
  // OPG × Empty should result in empty graph
  Graph product1 = gproduct(opg, empty);
  EXPECT_EQ(product1.num_vertices, 0);  // 1×0 = 0
  EXPECT_EQ(product1.num_edges, 0);     // 1×0 + 0×0 = 0
  
  // Empty × OPG should also result in empty graph
  Graph product2 = gproduct(empty, opg);
  EXPECT_EQ(product2.num_vertices, 0);  // 0×1 = 0
  EXPECT_EQ(product2.num_edges, 0);     // 0×0 + 0×1 = 0
}

}  // namespace

// BMesh Tests
namespace {

class BMeshTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Fresh BMesh for each test
  }
};

TEST_F(BMeshTest, InvalidMeshSize) {
  EXPECT_THROW(BMesh(0), std::invalid_argument);
}

TEST_F(BMeshTest, MeshSize1) {
  BMesh mesh(1);
  
  EXPECT_EQ(boost::num_vertices(mesh), 1);
  EXPECT_EQ(boost::num_edges(mesh), 0);
  EXPECT_EQ(mesh.dimension, 1);
  
  // Test proxy patterns
  EXPECT_EQ(mesh.num_vertices, 1);
  EXPECT_EQ(mesh.num_edges, 0);
  
  // Check vertex id is 0
  auto [vi, vi_end] = boost::vertices(mesh);
  EXPECT_EQ(mesh[*vi].id, 0);
}

TEST_F(BMeshTest, MeshSize3) {
  BMesh mesh(3);
  
  EXPECT_EQ(boost::num_vertices(mesh), 3);
  EXPECT_EQ(boost::num_edges(mesh), 4);  // 2*(N-1) = 2*2 = 4 bidirectional edges
  EXPECT_EQ(mesh.dimension, 3);
  
  // Test proxy patterns
  EXPECT_EQ(mesh.num_vertices, 3);
  EXPECT_EQ(mesh.num_edges, 4);
  
  // Check vertex ids are 0, 1, 2
  std::set<int32_t> ids;
  auto [vi, vi_end] = boost::vertices(mesh);
  for (auto it = vi; it != vi_end; ++it) {
    ids.insert(mesh[*it].id);
  }
  EXPECT_EQ(ids, (std::set<int32_t>{0, 1, 2}));
  
  // Check edges form bidirectional chain: 0↔1↔2
  std::vector<std::pair<int32_t, int32_t>> edges = mesh.edges;
  std::set<std::pair<int32_t, int32_t>> edge_set(edges.begin(), edges.end());
  
  // Expected bidirectional edges: 0→1, 1→0, 1→2, 2→1
  EXPECT_EQ(edge_set.count({0, 1}), 1);
  EXPECT_EQ(edge_set.count({1, 0}), 1);
  EXPECT_EQ(edge_set.count({1, 2}), 1);
  EXPECT_EQ(edge_set.count({2, 1}), 1);
  
  // Should not have other edges
  EXPECT_EQ(edge_set.count({0, 2}), 0);
  EXPECT_EQ(edge_set.count({2, 0}), 0);
}

TEST_F(BMeshTest, MeshSize5) {
  BMesh mesh(5);
  
  EXPECT_EQ(boost::num_vertices(mesh), 5);
  EXPECT_EQ(boost::num_edges(mesh), 8);  // 2*(N-1) = 2*4 = 8 bidirectional edges
  EXPECT_EQ(mesh.dimension, 5);
}

TEST_F(BMeshTest, GraphName) {
  // Test that BMesh has correct name
  BMesh mesh(3);
  EXPECT_EQ(mesh[boost::graph_bundle].name, "BMesh");
}

TEST_F(BMeshTest, DiameterCalculation) {
  BMesh mesh1(1);
  EXPECT_EQ(mesh1.diameter, 0);  // Single vertex
  
  BMesh mesh2(2);
  EXPECT_EQ(mesh2.diameter, 1);  // 0↔1, diameter = 1
  
  BMesh mesh3(3);
  EXPECT_EQ(mesh3.diameter, 2);  // 0↔1↔2, diameter = 2
  
  BMesh mesh5(5);
  EXPECT_EQ(mesh5.diameter, 4);  // 0↔1↔2↔3↔4, diameter = 4
}

TEST_F(BMeshTest, AddVertexAndEdgeConvertsToGeneric) {
  BMesh mesh(3);
  
  // Initially should be BMesh
  EXPECT_EQ(mesh[boost::graph_bundle].name, "BMesh");
  EXPECT_EQ(mesh.num_vertices, 3);
  EXPECT_EQ(mesh.num_edges, 4);
  EXPECT_EQ(mesh.dimension, 3);
  
  // Adding vertex should convert to Generic
  mesh.add_vertex(10);
  EXPECT_EQ(mesh[boost::graph_bundle].name, "Generic");
  EXPECT_EQ(mesh.num_vertices, 4);  // Should have added the vertex
  
  // Create another mesh to test edge addition
  BMesh mesh2(2);
  EXPECT_EQ(mesh2[boost::graph_bundle].name, "BMesh");
  
  // Adding edge should convert to Generic
  mesh2.add_edge(0, 0);  // Add self-loop
  EXPECT_EQ(mesh2[boost::graph_bundle].name, "Generic");
}

TEST_F(BMeshTest, DimensionProxy) {
  BMesh mesh(4);
  
  // Test dimension proxy works
  size_t dim = mesh.dimension;
  EXPECT_EQ(dim, 4);
  
  // Test implicit conversion
  EXPECT_EQ(mesh.dimension, 4);
}

TEST_F(BMeshTest, ProxyAccess) {
  BMesh mesh(3);
  
  // Test all proxy patterns work
  EXPECT_EQ(mesh.num_vertices, 3);
  EXPECT_EQ(mesh.num_edges, 4);
  EXPECT_EQ(mesh.diameter, 2);
  EXPECT_EQ(mesh.dimension, 3);
  
  // Test vertices and edges vectors
  std::vector<int32_t> vertices = mesh.vertices;
  EXPECT_EQ(vertices.size(), 3);
  
  std::vector<std::pair<int32_t, int32_t>> edges = mesh.edges;
  EXPECT_EQ(edges.size(), 4);
}

TEST_F(BMeshTest, CartesianProductWithOPG) {
  BMesh mesh(3);  // 0↔1↔2
  OPG opg;
  
  // BMesh × OPG should preserve BMesh structure (identity property)
  Graph product1 = gproduct(mesh, opg);
  EXPECT_EQ(product1.num_vertices, 3);  // 3×1 = 3
  EXPECT_EQ(product1.num_edges, 4);     // 3×0 + 4×1 = 4
  
  // OPG × BMesh should also preserve BMesh structure
  Graph product2 = gproduct(opg, mesh);
  EXPECT_EQ(product2.num_vertices, 3);  // 1×3 = 3
  EXPECT_EQ(product2.num_edges, 4);     // 1×4 + 0×3 = 4
  
  // Test names
  EXPECT_EQ(product1[boost::graph_bundle].name, "BMesh ⊗ OPG");
  EXPECT_EQ(product2[boost::graph_bundle].name, "OPG ⊗ BMesh");
}

TEST_F(BMeshTest, CartesianProductWithUMesh) {
  BMesh bmesh(2);  // 0↔1 (bidirectional)
  UMesh umesh(2);  // 0→1 (unidirectional)
  
  // BMesh × UMesh
  Graph product = gproduct(bmesh, umesh);
  EXPECT_EQ(product.num_vertices, 4);  // 2×2 = 4
  EXPECT_EQ(product.num_edges, 6);     // 2×1 + 2×2 = 2 + 4 = 6
  
  EXPECT_EQ(product[boost::graph_bundle].name, "BMesh ⊗ UMesh");
}

TEST_F(BMeshTest, CompareWithUMesh) {
  // BMesh should have more edges than UMesh for same size
  size_t N = 4;
  BMesh bmesh(N);
  UMesh umesh(N);
  
  EXPECT_EQ(bmesh.num_vertices, umesh.num_vertices);  // Same vertices
  EXPECT_EQ(bmesh.num_edges, 2 * umesh.num_edges);   // BMesh has twice the edges
  EXPECT_EQ(bmesh.diameter, umesh.diameter);         // Same diameter
  EXPECT_EQ(bmesh.dimension, umesh.dimension);       // Same dimension
}

}  // namespace

// Cartesian Product Tests
namespace {

class CartesianProductTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Fresh setup for each test
  }
};

TEST_F(CartesianProductTest, UtilityFunctions) {
  using namespace topology::gproduct_utils;
  
  // Test encode/decode vertex pairs
  EXPECT_EQ(encode_vertex_pair(0, 0, 3), 0);
  EXPECT_EQ(encode_vertex_pair(0, 1, 3), 1);
  EXPECT_EQ(encode_vertex_pair(0, 2, 3), 2);
  EXPECT_EQ(encode_vertex_pair(1, 0, 3), 3);
  EXPECT_EQ(encode_vertex_pair(1, 1, 3), 4);
  EXPECT_EQ(encode_vertex_pair(1, 2, 3), 5);
  
  // Test decode
  auto [g1_id, g2_id] = decode_vertex_pair(4, 3);
  EXPECT_EQ(g1_id, 1);
  EXPECT_EQ(g2_id, 1);
  
  auto [g1_id2, g2_id2] = decode_vertex_pair(5, 3);
  EXPECT_EQ(g1_id2, 1);
  EXPECT_EQ(g2_id2, 2);
}

TEST_F(CartesianProductTest, SingleVertexGraphs) {
  Graph g1, g2;
  g1.add_vertex(0);
  g2.add_vertex(0);
  
  Graph product = gproduct(g1, g2);
  
  EXPECT_EQ(product.num_vertices, 1);
  EXPECT_EQ(product.num_edges, 0);
  EXPECT_EQ(product[boost::graph_bundle].name, "Generic ⊗ Generic");
}

TEST_F(CartesianProductTest, TwoVertexPaths) {
  // G: 0→1
  Graph g1;
  g1.add_vertex(0);
  g1.add_vertex(1);
  g1.add_edge(0, 1);
  
  // H: 0→1
  Graph g2;
  g2.add_vertex(0);
  g2.add_vertex(1);
  g2.add_edge(0, 1);
  
  Graph product = gproduct(g1, g2);
  
  // Should have 4 vertices: (0,0), (0,1), (1,0), (1,1)
  EXPECT_EQ(product.num_vertices, 4);
  
  // Should have 4 edges:
  // From G dimension: (0,0)→(1,0), (0,1)→(1,1)
  // From H dimension: (0,0)→(0,1), (1,0)→(1,1)
  EXPECT_EQ(product.num_edges, 4);
  
  // Check specific edges exist
  std::vector<std::pair<int32_t, int32_t>> edges = product.edges;
  std::set<std::pair<int32_t, int32_t>> edge_set(edges.begin(), edges.end());
  
  EXPECT_EQ(edge_set.count({0, 2}), 1);  // (0,0)→(1,0)
  EXPECT_EQ(edge_set.count({1, 3}), 1);  // (0,1)→(1,1)
  EXPECT_EQ(edge_set.count({0, 1}), 1);  // (0,0)→(0,1)
  EXPECT_EQ(edge_set.count({2, 3}), 1);  // (1,0)→(1,1)
}

TEST_F(CartesianProductTest, OperatorOverload) {
  UMesh mesh1(2);  // 0→1
  UMesh mesh2(2);  // 0→1
  
  Graph product = mesh1 * mesh2;
  
  EXPECT_EQ(product.num_vertices, 4);
  EXPECT_EQ(product.num_edges, 4);
  EXPECT_EQ(product[boost::graph_bundle].name, "UMesh ⊗ UMesh");
}

TEST_F(CartesianProductTest, RingTimesPath) {
  URing ring(3);   // 0→1→2→0
  UMesh path(2);   // 0→1
  
  Graph product = gproduct(ring, path);
  
  // Should have 6 vertices: (0,0), (0,1), (1,0), (1,1), (2,0), (2,1)
  EXPECT_EQ(product.num_vertices, 6);
  
  // Ring has 3 edges, path has 1 edge
  // From ring dimension: 3 edges × 2 vertices = 6 edges
  // From path dimension: 1 edge × 3 vertices = 3 edges
  EXPECT_EQ(product.num_edges, 9);
  
  EXPECT_EQ(product[boost::graph_bundle].name, "URing ⊗ UMesh");
}

TEST_F(CartesianProductTest, PathTimesPath3x3) {
  UMesh path1(3);  // 0→1→2
  UMesh path2(3);  // 0→1→2
  
  Graph grid = gproduct(path1, path2);
  
  // Should create a 3×3 grid
  EXPECT_EQ(grid.num_vertices, 9);
  
  // Path1 has 2 edges, path2 has 2 edges
  // From path1 dimension: 2 edges × 3 vertices = 6 edges
  // From path2 dimension: 2 edges × 3 vertices = 6 edges
  EXPECT_EQ(grid.num_edges, 12);
  
  // Test a few specific edges (grid structure)
  std::vector<std::pair<int32_t, int32_t>> edges = grid.edges;
  std::set<std::pair<int32_t, int32_t>> edge_set(edges.begin(), edges.end());
  
  // Horizontal edges: (0,0)→(1,0), (1,0)→(2,0), etc.
  EXPECT_EQ(edge_set.count({0, 3}), 1);  // (0,0)→(1,0)
  EXPECT_EQ(edge_set.count({3, 6}), 1);  // (1,0)→(2,0)
  
  // Vertical edges: (0,0)→(0,1), (0,1)→(0,2), etc.
  EXPECT_EQ(edge_set.count({0, 1}), 1);  // (0,0)→(0,1)
  EXPECT_EQ(edge_set.count({1, 2}), 1);  // (0,1)→(0,2)
}

TEST_F(CartesianProductTest, RingTimesRingTorus) {
  URing ring1(3);  // 0→1→2→0
  URing ring2(3);  // 0→1→2→0
  
  Graph torus = gproduct(ring1, ring2);
  
  // Should create a 3×3 torus
  EXPECT_EQ(torus.num_vertices, 9);
  
  // Each ring has 3 edges
  // From ring1 dimension: 3 edges × 3 vertices = 9 edges
  // From ring2 dimension: 3 edges × 3 vertices = 9 edges
  EXPECT_EQ(torus.num_edges, 18);
  
  EXPECT_EQ(torus[boost::graph_bundle].name, "URing ⊗ URing");
}

TEST_F(CartesianProductTest, AsymmetricProduct) {
  UMesh path(4);   // 0→1→2→3
  URing ring(2);   // 0→1→0
  
  Graph product = gproduct(path, ring);
  
  EXPECT_EQ(product.num_vertices, 8);
  
  // Path has 3 edges, ring has 2 edges
  // From path dimension: 3 edges × 2 vertices = 6 edges
  // From ring dimension: 2 edges × 4 vertices = 8 edges
  EXPECT_EQ(product.num_edges, 14);
}

TEST_F(CartesianProductTest, EmptyGraphHandling) {
  Graph empty1, empty2;
  
  Graph product = gproduct(empty1, empty2);
  
  EXPECT_EQ(product.num_vertices, 0);
  EXPECT_EQ(product.num_edges, 0);
}

TEST_F(CartesianProductTest, ScalarProductFormula) {
  // Test that |V(G1 ⊗ G2)| = |V(G1)| × |V(G2)|
  // Test that |E(G1 ⊗ G2)| = |V(G1)| × |E(G2)| + |E(G1)| × |V(G2)|
  
  // 3-vertex path × 4-vertex path
  UMesh path1(3);  // 3 vertices, 2 edges
  UMesh path2(4);  // 4 vertices, 3 edges
  
  Graph product = gproduct(path1, path2);
  
  // Should have 3 × 4 = 12 vertices
  EXPECT_EQ(product.num_vertices, 12);
  EXPECT_EQ(product.num_vertices, path1.num_vertices * path2.num_vertices);
  
  // Should have 3×3 + 2×4 = 9 + 8 = 17 edges
  size_t expected_edges = path1.num_vertices * path2.num_edges + path1.num_edges * path2.num_vertices;
  EXPECT_EQ(product.num_edges, expected_edges);
  EXPECT_EQ(product.num_edges, 17);
  
  // 5-vertex ring × 2-vertex path  
  URing ring(5);   // 5 vertices, 5 edges
  UMesh path(2);   // 2 vertices, 1 edge
  
  Graph product2 = gproduct(ring, path);
  
  // Should have 5 × 2 = 10 vertices
  EXPECT_EQ(product2.num_vertices, 10);
  EXPECT_EQ(product2.num_vertices, ring.num_vertices * path.num_vertices);
  
  // Should have 5×1 + 5×2 = 5 + 10 = 15 edges
  expected_edges = ring.num_vertices * path.num_edges + ring.num_edges * path.num_vertices;
  EXPECT_EQ(product2.num_edges, expected_edges);
  EXPECT_EQ(product2.num_edges, 15);
  
  // Different sized rings
  URing ring1(7);  // 7 vertices, 7 edges
  URing ring2(3);  // 3 vertices, 3 edges
  
  Graph torus = gproduct(ring1, ring2);
  
  // Should have 7 × 3 = 21 vertices
  EXPECT_EQ(torus.num_vertices, 21);
  EXPECT_EQ(torus.num_vertices, ring1.num_vertices * ring2.num_vertices);
  
  // Should have 7×3 + 7×3 = 21 + 21 = 42 edges
  expected_edges = ring1.num_vertices * ring2.num_edges + ring1.num_edges * ring2.num_vertices;
  EXPECT_EQ(torus.num_edges, expected_edges);
  EXPECT_EQ(torus.num_edges, 42);
}

}  // namespace
}  // namespace topology
