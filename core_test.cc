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
  EXPECT_EQ(ring.GetRingSize(), 1);
  
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
  EXPECT_EQ(ring.GetRingSize(), 3);
  
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

TEST_F(URingTest, AddVertexAndEdgeDisabled) {
  URing ring(3);
  
  // The following should not compile due to deleted methods:
  // ring.add_vertex(10);  // Would cause compile error
  // ring.add_edge(0, 10); // Would cause compile error
  
  // Verify that the ring structure is preserved
  EXPECT_EQ(ring.num_vertices, 3);
  EXPECT_EQ(ring.num_edges, 3);
  EXPECT_EQ(ring.GetRingSize(), 3);
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
  EXPECT_EQ(mesh.GetMeshSize(), 1);
  
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
  EXPECT_EQ(mesh.GetMeshSize(), 3);
  
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

TEST_F(UMeshTest, AddVertexAndEdgeDisabled) {
  UMesh mesh(3);
  
  // The following should not compile due to deleted methods:
  // mesh.add_vertex(10);  // Would cause compile error
  // mesh.add_edge(0, 10); // Would cause compile error
  
  // Just test that we can create and use the mesh
  EXPECT_EQ(mesh.num_vertices, 3);
  EXPECT_EQ(mesh.num_edges, 2);
}

}  // namespace

// Tensor Product Tests
namespace {

class TensorProductTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Fresh setup for each test
  }
};

TEST_F(TensorProductTest, UtilityFunctions) {
  using namespace topology::tensor_product_utils;
  
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

TEST_F(TensorProductTest, SingleVertexGraphs) {
  Graph g1, g2;
  g1.add_vertex(0);
  g2.add_vertex(0);
  
  Graph product = tensor_product(g1, g2);
  
  EXPECT_EQ(product.num_vertices, 1);
  EXPECT_EQ(product.num_edges, 0);
  EXPECT_EQ(product[boost::graph_bundle].name, "Generic ⊗ Generic");
}

TEST_F(TensorProductTest, TwoVertexPaths) {
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
  
  Graph product = tensor_product(g1, g2);
  
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

TEST_F(TensorProductTest, OperatorOverload) {
  UMesh mesh1(2);  // 0→1
  UMesh mesh2(2);  // 0→1
  
  Graph product = mesh1 * mesh2;
  
  EXPECT_EQ(product.num_vertices, 4);
  EXPECT_EQ(product.num_edges, 4);
  EXPECT_EQ(product[boost::graph_bundle].name, "UMesh ⊗ UMesh");
}

TEST_F(TensorProductTest, RingTimesPath) {
  URing ring(3);   // 0→1→2→0
  UMesh path(2);   // 0→1
  
  Graph product = tensor_product(ring, path);
  
  // Should have 6 vertices: (0,0), (0,1), (1,0), (1,1), (2,0), (2,1)
  EXPECT_EQ(product.num_vertices, 6);
  
  // Ring has 3 edges, path has 1 edge
  // From ring dimension: 3 edges × 2 vertices = 6 edges
  // From path dimension: 1 edge × 3 vertices = 3 edges
  EXPECT_EQ(product.num_edges, 9);
  
  EXPECT_EQ(product[boost::graph_bundle].name, "URing ⊗ UMesh");
}

TEST_F(TensorProductTest, PathTimesPath3x3) {
  UMesh path1(3);  // 0→1→2
  UMesh path2(3);  // 0→1→2
  
  Graph grid = tensor_product(path1, path2);
  
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

TEST_F(TensorProductTest, RingTimesRingTorus) {
  URing ring1(3);  // 0→1→2→0
  URing ring2(3);  // 0→1→2→0
  
  Graph torus = tensor_product(ring1, ring2);
  
  // Should create a 3×3 torus
  EXPECT_EQ(torus.num_vertices, 9);
  
  // Each ring has 3 edges
  // From ring1 dimension: 3 edges × 3 vertices = 9 edges
  // From ring2 dimension: 3 edges × 3 vertices = 9 edges
  EXPECT_EQ(torus.num_edges, 18);
  
  EXPECT_EQ(torus[boost::graph_bundle].name, "URing ⊗ URing");
}

TEST_F(TensorProductTest, AsymmetricProduct) {
  UMesh path(4);   // 0→1→2→3
  URing ring(2);   // 0→1→0
  
  Graph product = tensor_product(path, ring);
  
  EXPECT_EQ(product.num_vertices, 8);
  
  // Path has 3 edges, ring has 2 edges
  // From path dimension: 3 edges × 2 vertices = 6 edges
  // From ring dimension: 2 edges × 4 vertices = 8 edges
  EXPECT_EQ(product.num_edges, 14);
}

TEST_F(TensorProductTest, EmptyGraphHandling) {
  Graph empty1, empty2;
  
  Graph product = tensor_product(empty1, empty2);
  
  EXPECT_EQ(product.num_vertices, 0);
  EXPECT_EQ(product.num_edges, 0);
}

}  // namespace
}  // namespace topology
