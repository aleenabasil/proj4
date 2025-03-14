#include "DijkstraPathRouter.h"
#include <unordered_map>
#include <vector>
#include <any>
#include <limits>
#include <queue>
#include <algorithm>
#include <memory>

struct CDijkstraPathRouter::SImplementation {
    using TVertexID = CPathRouter::TVertexID;
    static constexpr double NoPathExists = std::numeric_limits<double>::infinity();

    struct Vertex {
        std::any tag;
        std::unordered_map<TVertexID, double> edges;
        
        Vertex(std::any t) : tag(t) {}
    };

    std::vector<std::shared_ptr<Vertex>> vertices;
    std::size_t nextID = 0;

    std::size_t VertexCount() const noexcept {
        return vertices.size();
    }

    TVertexID AddVertex(std::any tag) noexcept {
        vertices.push_back(std::make_shared<Vertex>(tag));
        return nextID++;
    }

    std::any GetVertexTag(TVertexID id) const noexcept {
        if (id < vertices.size()) {
            return vertices[id]->tag;
        }
        return std::any();
    }

    bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept {
        if (src >= vertices.size() || dest >= vertices.size() || weight <= 0) {
            return false;
        }
        vertices[src]->edges[dest] = weight;
        if (bidir) {
            vertices[dest]->edges[src] = weight;
        }
        return true;
    }

    bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept {
        return true;
    }

    double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID>& path) noexcept {
        path.clear();

        if (src >= vertices.size() || dest >= vertices.size()) {
            return NoPathExists;
        }
        
        if (src == dest) {
            path.push_back(src);
            return 0.0;
        }

        std::priority_queue<std::pair<double, TVertexID>, 
                            std::vector<std::pair<double, TVertexID>>, 
                            std::greater<>> pq;
        std::vector<double> distances(vertices.size(), NoPathExists);
        std::vector<TVertexID> previous(vertices.size(), static_cast<TVertexID>(-1));

        distances[src] = 0;
        pq.push({0, src});

        while (!pq.empty()) {
            auto [dist, u] = pq.top();
            pq.pop();

            if (u == dest) {
                break;
            }

            if (dist > distances[u]) {
                continue;
            }

            for (auto [v, weight] : vertices[u]->edges) {
                double new_dist = dist + weight;
                if (new_dist < distances[v]) {
                    distances[v] = new_dist;
                    previous[v] = u;
                    pq.push({new_dist, v});
                }
            }
        }

        if (distances[dest] == NoPathExists) {
            return NoPathExists;
        }

        // Reconstruct path
        for (TVertexID at = dest; at != src; at = previous[at]) {
            if (previous[at] == static_cast<TVertexID>(-1)) {
                path.clear();
                return NoPathExists;
            }
            path.push_back(at);
        }
        path.push_back(src);
        std::reverse(path.begin(), path.end());

        return distances[dest];
    }
};


// CDijkstraPathRouter member functions
// Constructor for the Dijkstra Path Router
CDijkstraPathRouter::CDijkstraPathRouter(){
    DImplementation = std::make_unique<SImplementation>();
}

// Destructor for the Dijkstra Path Router
CDijkstraPathRouter::~CDijkstraPathRouter() = default;

// Returns the number of vertices in the path router
std::size_t CDijkstraPathRouter::VertexCount() const noexcept{
    return DImplementation->VertexCount();
}

// Adds a vertex with the tag provided. The tag can be of any type.
CPathRouter::TVertexID CDijkstraPathRouter::AddVertex(std::any tag) noexcept{
    return DImplementation->AddVertex(tag);
}

// Gets the tag of the vertex specified by id if id is in the path router.
// A std::any() is returned if id is not a valid vertex ID.
std::any CDijkstraPathRouter::GetVertexTag(TVertexID id) const noexcept{
    return DImplementation->GetVertexTag(id);
}

// Adds an edge between src and dest vertices with a weight of weight. If
// bidir is set to true an additional edge between dest and src is added. If
// src or dest nodes do not exist, or if the weight is negative the AddEdge
// will return false, otherwise it returns true.
bool CDijkstraPathRouter::AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir =
false) noexcept{
    return DImplementation->AddEdge(src,dest,weight,bidir);
}

// Allows the path router to do any desired precomputation up to the deadline
bool CDijkstraPathRouter::Precompute(std::chrono::steady_clock::time_point deadline) noexcept{
    return DImplementation->Precompute(deadline);
}

// Returns the path distance of the path from src to dest, and fills out path
// with vertices. If no path exists NoPathExists is returned.
double CDijkstraPathRouter::FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID>
&path) noexcept{
    return DImplementation->FindShortestPath(src,dest,path);
}


