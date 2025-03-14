#include "DijkstraPathRouter.h"
#include <limits>
#include <queue>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <any>
#include <memory>
#include <chrono>

struct CDijkstraPathRouter::SImplementation{
    
    struct ThisVertex{
        TVertexID ID; 
        std::any Tag; 
        std::unordered_map<TVertexID,double> edges; 
        std::vector<TVertexID> path; 
        
        ~ThisVertex() = default;

        TVertexID GetVertexID() const noexcept{
            return ID;
        }
        std::any GetTag() const noexcept{
            return Tag;
        }
        std::vector<TVertexID> GetNeighbors() const noexcept{
            return path;
        }
        double GetWeight(TVertexID to) const noexcept{
            auto it = edges.find(to);
            if(it != edges.end()){
                return it->second;
            }
            return std::numeric_limits<double>::infinity();
        }


    };

    std::vector<std::shared_ptr<ThisVertex>> vertices; 
    TVertexID nextID; 

    std::size_t VertexCount() const noexcept{
        return vertices.size();
    }

    TVertexID AddVertex(std::any tag) noexcept{
        auto newVertex = std::make_shared<ThisVertex>();
        newVertex->ID = nextID;
        newVertex->Tag = tag;
        vertices.push_back(newVertex);
        nextID++;
        return newVertex->ID;
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

}

// Gets the tag of the vertex specified by id if id is in the path router.
// A std::any() is returned if id is not a valid vertex ID.
std::any CDijkstraPathRouter::GetVertexTag(TVertexID id) const noexcept{

}

// Adds an edge between src and dest vertices with a weight of weight. If
// bidir is set to true an additional edge between dest and src is added. If
// src or dest nodes do not exist, or if the weight is negative the AddEdge
// will return false, otherwise it returns true.
bool CDijkstraPathRouter::AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir =
false) noexcept{

}

// Allows the path router to do any desired precomputation up to the deadline
bool CDijkstraPathRouter::Precompute(std::chrono::steady_clock::time_point deadline) noexcept{

}

// Returns the path distance of the path from src to dest, and fills out path
// with vertices. If no path exists NoPathExists is returned.
double CDijkstraPathRouter::FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID>
&path) noexcept{

}

