#include "DijkstraPathRouter.h"
#include <unordered_map>
#include <vector>
#include <any>
#include <limits>
#include <queue>
#include <algorithm>
#include <memory>
#include <optional>

//implemetation of the Dijkstra path router
struct CDijkstraPathRouter::SImplementation {
    using TVertexID = CPathRouter::TVertexID;

    //struct for vertex in graph
    struct Vertex {
        std::any tag;
        std::unordered_map<TVertexID, double> edges;
        
        //construct for intializign vertec with a tag
        Vertex(std::any t) : tag(t) {}
    };

    //list of all vertices
    std::vector<std::shared_ptr<Vertex>> vertices;
    //next avaible ID
    std::size_t nextID = 0;

    //num of vertices
    std::size_t VertexCount() const noexcept {
        return vertices.size();
    }

    //add a vertec usign the tag and returns ID
    TVertexID AddVertex(std::any tag) noexcept {
        vertices.push_back(std::make_shared<Vertex>(tag));
        return nextID++;
    }

    //gets tag using ID
    std::any GetVertexTag(TVertexID id) const noexcept {
        if (id < vertices.size()) {
            return vertices[id]->tag;
        }
        return std::any();
    }

    //adds a edge between to vertices, with given weight
    bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept {
        if (src >= vertices.size() || dest >= vertices.size() || weight <= 0) {
            return false;
        }
        //creates directed edge
        vertices[src]->edges[dest] = weight;
        //adds reverse edge if true
        if (bidir) {
            vertices[dest]->edges[src] = weight;
        }
        return true;
    }

    bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept {
        return true;
    }

    //Dijkstra's algorithm to find shortest path
    double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID>& path) noexcept {
        //path is emoty
        path.clear();
    
        if (vertices.empty() || src >= vertices.size() || dest >= vertices.size()) {
            return NoPathExists;
        }
        //if same return 0
        if (src == dest) {
            path.push_back(src);
            return 0.0;
        }
        //priotirty queue
        std::priority_queue<std::pair<double, TVertexID>, 
                            std::vector<std::pair<double, TVertexID>>, 
                            std::greater<>> pq;
    
        std::vector<double> distances;
        std::vector<std::optional<TVertexID>> previous;
        //distances to infinity
        distances.assign(vertices.size(), std::numeric_limits<double>::infinity());
        //track path taken
        previous.assign(vertices.size(), std::nullopt);
    
        //dist to source is 0
        distances[src] = 0;
        //push to priority queue
        pq.push({0, src});
    
        while (!pq.empty()) {
            //smallest distance vertex
            auto [dist, u] = pq.top();
            pq.pop();
    
            if (dist > distances[u]) {
                //skip bigger distances
                continue; }
            if (u == dest) {
                //if it is the destination alreay leave the loop
                break; }
    
            for (const auto& [v, weight] : vertices[u]->edges) {
                double new_dist = distances[u] + weight;
                //find the shorter path
                if (new_dist < distances[v]) {
                    distances[v] = new_dist;
                    previous[v] = u;
                    //update the distance intot he queue
                    pq.push({new_dist, v});
                }
            }
        }
    
        if (distances[dest] == std::numeric_limits<double>::infinity()) {
            return NoPathExists;
        }
    
        //back tracks from the destination for shortest path
        for (std::optional<TVertexID> at = dest; at.has_value(); at = previous[*at]) {
            if (!previous[*at].has_value() && *at != src) {
                path.clear();
                return NoPathExists;
            }
            path.push_back(*at);
        }
        
        //reverse to get right order (staar to finish)
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
bool CDijkstraPathRouter::AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir) noexcept{
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
