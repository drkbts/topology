#ifndef TOPOLOGY_CORE_H_
#define TOPOLOGY_CORE_H_

#include <vector>
#include <string>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

namespace topology {

class Graph {
 public:
  Graph();
  ~Graph();
  
  void AddVertex(const std::string& id);
  void AddEdge(const std::string& from, const std::string& to);
  void RemoveVertex(const std::string& id);
  void RemoveEdge(const std::string& from, const std::string& to);
  
  bool HasVertex(const std::string& id) const;
  bool HasEdge(const std::string& from, const std::string& to) const;
  
  std::vector<std::string> GetVertices() const;
  std::vector<std::string> GetNeighbors(const std::string& id) const;
  
  size_t GetVertexCount() const;
  size_t GetEdgeCount() const;
  
 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace topology

#endif  // TOPOLOGY_CORE_H_
