#include "core.h"
#include <gtest/gtest.h>

namespace topology {
namespace {

class GraphTest : public ::testing::Test {
 protected:
  void SetUp() override {
    graph_ = std::make_unique<Graph>();
  }

  std::unique_ptr<Graph> graph_;
};

TEST_F(GraphTest, EmptyGraphHasNoVertices) {
  EXPECT_EQ(graph_->GetVertexCount(), 0);
  EXPECT_EQ(graph_->GetEdgeCount(), 0);
  EXPECT_TRUE(graph_->GetVertices().empty());
}

TEST_F(GraphTest, AddSingleVertex) {
  graph_->AddVertex("A");
  
  EXPECT_EQ(graph_->GetVertexCount(), 1);
  EXPECT_EQ(graph_->GetEdgeCount(), 0);
  EXPECT_TRUE(graph_->HasVertex("A"));
  EXPECT_FALSE(graph_->HasVertex("B"));
  
  auto vertices = graph_->GetVertices();
  EXPECT_EQ(vertices.size(), 1);
  EXPECT_EQ(vertices[0], "A");
}

TEST_F(GraphTest, AddMultipleVertices) {
  graph_->AddVertex("A");
  graph_->AddVertex("B");
  graph_->AddVertex("C");
  
  EXPECT_EQ(graph_->GetVertexCount(), 3);
  EXPECT_TRUE(graph_->HasVertex("A"));
  EXPECT_TRUE(graph_->HasVertex("B"));
  EXPECT_TRUE(graph_->HasVertex("C"));
}

TEST_F(GraphTest, AddEdge) {
  graph_->AddEdge("A", "B");
  
  EXPECT_EQ(graph_->GetVertexCount(), 2);
  EXPECT_EQ(graph_->GetEdgeCount(), 1);
  EXPECT_TRUE(graph_->HasVertex("A"));
  EXPECT_TRUE(graph_->HasVertex("B"));
  EXPECT_TRUE(graph_->HasEdge("A", "B"));
  EXPECT_FALSE(graph_->HasEdge("B", "A"));
}

TEST_F(GraphTest, GetNeighbors) {
  graph_->AddEdge("A", "B");
  graph_->AddEdge("A", "C");
  
  auto neighbors = graph_->GetNeighbors("A");
  EXPECT_EQ(neighbors.size(), 2);
  EXPECT_TRUE(std::find(neighbors.begin(), neighbors.end(), "B") != neighbors.end());
  EXPECT_TRUE(std::find(neighbors.begin(), neighbors.end(), "C") != neighbors.end());
  
  auto empty_neighbors = graph_->GetNeighbors("B");
  EXPECT_TRUE(empty_neighbors.empty());
}

TEST_F(GraphTest, RemoveVertex) {
  graph_->AddEdge("A", "B");
  graph_->AddEdge("B", "C");
  graph_->AddEdge("C", "A");
  
  graph_->RemoveVertex("B");
  
  EXPECT_EQ(graph_->GetVertexCount(), 2);
  EXPECT_EQ(graph_->GetEdgeCount(), 1);
  EXPECT_FALSE(graph_->HasVertex("B"));
  EXPECT_FALSE(graph_->HasEdge("A", "B"));
  EXPECT_FALSE(graph_->HasEdge("B", "C"));
  EXPECT_TRUE(graph_->HasEdge("C", "A"));
}

TEST_F(GraphTest, RemoveEdge) {
  graph_->AddEdge("A", "B");
  graph_->AddEdge("A", "C");
  
  graph_->RemoveEdge("A", "B");
  
  EXPECT_EQ(graph_->GetVertexCount(), 3);
  EXPECT_EQ(graph_->GetEdgeCount(), 1);
  EXPECT_FALSE(graph_->HasEdge("A", "B"));
  EXPECT_TRUE(graph_->HasEdge("A", "C"));
}

TEST_F(GraphTest, NonExistentVertexOperations) {
  EXPECT_FALSE(graph_->HasVertex("NonExistent"));
  EXPECT_FALSE(graph_->HasEdge("A", "B"));
  EXPECT_TRUE(graph_->GetNeighbors("NonExistent").empty());
  
  // These should not crash
  graph_->RemoveVertex("NonExistent");
  graph_->RemoveEdge("A", "B");
}

}  // namespace
}  // namespace topology
