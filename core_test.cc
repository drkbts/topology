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
}  // namespace topology
