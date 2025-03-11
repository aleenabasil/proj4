#include "DijkstraPathRouter.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>



// CDijkstraPathRouter member functions
// Constructor for the Dijkstra Path Router
CDijkstraPathRouter();

// Destructor for the Dijkstra Path Router
~CDijkstraPathRouter();

// Returns the number of vertices in the path router
std::size_t VertexCount() const noexcept;

// Adds a vertex with the tag provided. The tag can be of any type.
TVertexID AddVertex(std::any tag) noexcept;

// Gets the tag of the vertex specified by id if id is in the path router.
// A std::any() is returned if id is not a valid vertex ID.
std::any GetVertexTag(TVertexID id) const noexcept;

// Adds an edge between src and dest vertices with a weight of weight. If
// bidir is set to true an additional edge between dest and src is added. If
// src or dest nodes do not exist, or if the weight is negative the AddEdge
// will return false, otherwise it returns true.
bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir =
false) noexcept;
