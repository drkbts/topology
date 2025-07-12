#include "core.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace topology {

class Graph::Impl {
 public:
  std::unordered_map<std::string, std::unordered_set<std::string>> adjacency_list_;
};

Graph::Graph() : impl_(std::make_unique<Impl>()) {}

Graph::~Graph() = default;

void Graph::AddVertex(const std::string& id) {
  impl_->adjacency_list_[id];  // Creates empty set if not exists
}

void Graph::AddEdge(const std::string& from, const std::string& to) {
  AddVertex(from);
  AddVertex(to);
  impl_->adjacency_list_[from].insert(to);
}

void Graph::RemoveVertex(const std::string& id) {
  // Remove all edges pointing to this vertex
  for (auto& [vertex, neighbors] : impl_->adjacency_list_) {
    neighbors.erase(id);
  }
  // Remove the vertex itself
  impl_->adjacency_list_.erase(id);
}

void Graph::RemoveEdge(const std::string& from, const std::string& to) {
  auto it = impl_->adjacency_list_.find(from);
  if (it != impl_->adjacency_list_.end()) {
    it->second.erase(to);
  }
}

bool Graph::HasVertex(const std::string& id) const {
  return impl_->adjacency_list_.find(id) != impl_->adjacency_list_.end();
}

bool Graph::HasEdge(const std::string& from, const std::string& to) const {
  auto it = impl_->adjacency_list_.find(from);
  if (it == impl_->adjacency_list_.end()) {
    return false;
  }
  return it->second.find(to) != it->second.end();
}

std::vector<std::string> Graph::GetVertices() const {
  std::vector<std::string> vertices;
  vertices.reserve(impl_->adjacency_list_.size());
  for (const auto& [vertex, _] : impl_->adjacency_list_) {
    vertices.push_back(vertex);
  }
  return vertices;
}

std::vector<std::string> Graph::GetNeighbors(const std::string& id) const {
  auto it = impl_->adjacency_list_.find(id);
  if (it == impl_->adjacency_list_.end()) {
    return {};
  }
  
  std::vector<std::string> neighbors;
  neighbors.reserve(it->second.size());
  for (const auto& neighbor : it->second) {
    neighbors.push_back(neighbor);
  }
  return neighbors;
}

size_t Graph::GetVertexCount() const {
  return impl_->adjacency_list_.size();
}

size_t Graph::GetEdgeCount() const {
  size_t count = 0;
  for (const auto& [_, neighbors] : impl_->adjacency_list_) {
    count += neighbors.size();
  }
  return count;
}

}  // namespace topology
