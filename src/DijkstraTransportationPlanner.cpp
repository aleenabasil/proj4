#include <memory>
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>
#include <limits>
#include <iostream>
#include "DijkstraTransportationPlanner.h"
#include "TransportationPlanner.h"
#include "StreetMap.h" 
#include "GeographicUtils.h"
#include "BusSystem.h"
#include <algorithm>

struct CDijkstraTransportationPlanner::SImplementation{
    std::shared_ptr<SConfiguration> DConfig;
    
    // Map of node ID to adjacency list for distance-based routing
    std::unordered_map<TNodeID, std::vector<std::pair<TNodeID, double>>> DDistanceGraph;
    
    // Map of node ID to adjacency list for time-based routing with transportation mode
    std::unordered_map<TNodeID, std::vector<std::tuple<TNodeID, double, ETransportationMode>>> DTimeGraph;

    SImplementation(std::shared_ptr<SConfiguration> config)
        : DConfig(config) {
        BuildGraphs();
    }

    ~SImplementation(){
    }
    
    void BuildGraphs() {
        auto streetMap = DConfig->StreetMap();
        auto busSystem = DConfig->BusSystem();
        if (!streetMap) return;
        
        // Build distance graph from street map
        for (size_t i = 0; i < streetMap->WayCount(); ++i) {
            auto way = streetMap->WayByIndex(i);
            if (!way) continue;
            
            for (size_t j = 0; j < way->NodeCount() - 1; ++j) {
                TNodeID src = way->GetNodeID(j);
                TNodeID dest = way->GetNodeID(j + 1);
                
                auto srcNode = streetMap->NodeByID(src);
                auto destNode = streetMap->NodeByID(dest);
                if (!srcNode || !destNode) continue;
                
                double distance = SGeographicUtils::HaversineDistanceInMiles(
                    srcNode->Location(), destNode->Location()
                );
                
                // Add bidirectional edge
                DDistanceGraph[src].push_back({dest, distance});
                DDistanceGraph[dest].push_back({src, distance});
                
                // Add walk and bike edges to time graph
                double walkTime = distance / DConfig->WalkSpeed();
                DTimeGraph[src].push_back({dest, walkTime, ETransportationMode::Walk});
                DTimeGraph[dest].push_back({src, walkTime, ETransportationMode::Walk});
                
                double bikeTime = distance / DConfig->BikeSpeed();
                DTimeGraph[src].push_back({dest, bikeTime, ETransportationMode::Bike});
                DTimeGraph[dest].push_back({src, bikeTime, ETransportationMode::Bike});
            }
        }
        
        // Add bus edges to time graph
        if (busSystem) {
            for (size_t i = 0; i < busSystem->RouteCount(); ++i) {
                auto route = busSystem->RouteByIndex(i);
                if (!route) continue;
                
                for (size_t j = 0; j < route->StopCount() - 1; ++j) {
                    auto srcStop = busSystem->StopByID(route->GetStopID(j));
                    auto destStop = busSystem->StopByID(route->GetStopID(j + 1));
                    
                    if (!srcStop || !destStop) continue;
                    
                    TNodeID srcNodeID = srcStop->NodeID();
                    TNodeID destNodeID = destStop->NodeID();
                    
                    // Add bus edge with bus travel time
                    DTimeGraph[srcNodeID].push_back({destNodeID, DConfig->BusStopTime(), ETransportationMode::Bus});
                }
            }
        }
    }

    std::size_t NodeCount() const noexcept {
        return DConfig->StreetMap()->NodeCount();
    }
    
    std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept {
        return DConfig->StreetMap()->NodeByIndex(index);
    }
        
    double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path) {
        path.clear();
        auto streetMap = DConfig->StreetMap();
        if (!streetMap) return -1.0;
        
        // Check if source and destination exist
        if (DDistanceGraph.find(src) == DDistanceGraph.end() || 
            DDistanceGraph.find(dest) == DDistanceGraph.end()) {
            return -1.0;
        }
        
        // Handle trivial case
        if (src == dest) {
            path.push_back(src);
            return 0.0;
        }

        // Standard Dijkstra's algorithm
        std::unordered_map<TNodeID, double> distances;
        std::unordered_map<TNodeID, TNodeID> parents;
        std::priority_queue<std::pair<double, TNodeID>, std::vector<std::pair<double, TNodeID>>, std::greater<>> pq;

        // Initialize distances
        for (const auto& [nodeID, _] : DDistanceGraph) {
            distances[nodeID] = std::numeric_limits<double>::max();
        }
        distances[src] = 0.0;
        pq.push({0.0, src});

        while (!pq.empty()) {
            auto [dist, node] = pq.top();
            pq.pop();
            
            if (node == dest) break;
            
            if (dist > distances[node]) continue; // Skip if we've found a better path
            
            // Check all neighbors
            for (const auto& [neighbor, weight] : DDistanceGraph[node]) {
                double newDist = dist + weight;
                if (newDist < distances[neighbor]) {
                    distances[neighbor] = newDist;
                    parents[neighbor] = node;
                    pq.push({newDist, neighbor});
                }
            }
        }
        
        if (distances[dest] == std::numeric_limits<double>::max()) {
            return -1.0; // No path exists
        }
        
        // Reconstruct path
        TNodeID current = dest;
        while (current != src) {
            path.push_back(current);
            current = parents[current];
        }
        path.push_back(src);
        std::reverse(path.begin(), path.end());
        
        return distances[dest];
    }

    double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path) {
        path.clear();
        auto streetMap = DConfig->StreetMap();
        if (!streetMap) return -1.0;
        
        // Check if source and destination exist
        if (DTimeGraph.find(src) == DTimeGraph.end() || 
            DTimeGraph.find(dest) == DTimeGraph.end()) {
            return -1.0;
        }
        
        // Handle trivial case
        if (src == dest) {
            path.push_back({ETransportationMode::Walk, src});
            return 0.0;
        }

        // Modified Dijkstra's algorithm for time-based routing
        std::unordered_map<TNodeID, double> times;
        std::unordered_map<TNodeID, std::pair<TNodeID, ETransportationMode>> parents;
        std::priority_queue<std::pair<double, TNodeID>, std::vector<std::pair<double, TNodeID>>, std::greater<>> pq;

        // Initialize times
        for (const auto& [nodeID, _] : DTimeGraph) {
            times[nodeID] = std::numeric_limits<double>::max();
        }
        times[src] = 0.0;
        pq.push({0.0, src});

        while (!pq.empty()) {
            auto [time, node] = pq.top();
            pq.pop();
            
            if (node == dest) break;
            
            if (time > times[node]) continue; // Skip if we've found a better path
            
            // Check all neighbors
            for (const auto& [neighbor, travelTime, mode] : DTimeGraph[node]) {
                double newTime = time + travelTime;
                if (newTime < times[neighbor]) {
                    times[neighbor] = newTime;
                    parents[neighbor] = {node, mode};
                    pq.push({newTime, neighbor});
                }
            }
        }
        
        if (times[dest] == std::numeric_limits<double>::max()) {
            return -1.0; // No path exists
        }
        
        // Reconstruct path with transportation modes
        std::vector<TTripStep> reversePath;
        TNodeID current = dest;
        while (current != src) {
            auto [prevNode, mode] = parents[current];
            reversePath.push_back({mode, current});
            current = prevNode;
        }
        reversePath.push_back({ETransportationMode::Walk, src}); // Starting point
        
        // Reverse the path to get source to destination
        for (auto it = reversePath.rbegin(); it != reversePath.rend(); ++it) {
            path.push_back(*it);
        }
        
        return times[dest];
    }

    bool GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const {
        desc.clear();
        for (const auto& step : path) {
            std::string mode;
            switch (step.first) {
                case ETransportationMode::Walk: mode = "Walk"; break;
                case ETransportationMode::Bike: mode = "Bike"; break;
                case ETransportationMode::Bus: mode = "Take bus"; break;
            }
            desc.push_back(mode + " to node " + std::to_string(step.second));
        }
        return true;
    }
};

CDijkstraTransportationPlanner::CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config) {
    DImplementation = std::make_unique<SImplementation>(config);
}

// Destructor
CDijkstraTransportationPlanner::~CDijkstraTransportationPlanner() {
}

// Returns the number of nodes in the street map
std::size_t CDijkstraTransportationPlanner::NodeCount() const noexcept {
    return DImplementation->NodeCount();
}

// Returns the street map node specified by index
std::shared_ptr<CStreetMap::SNode> CDijkstraTransportationPlanner::SortedNodeByIndex(std::size_t index) const noexcept {
    return DImplementation->SortedNodeByIndex(index);
}

// Finds the shortest path
double CDijkstraTransportationPlanner::FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path) {
    return DImplementation->FindShortestPath(src, dest, path);
}

// Finds the fastest path
double CDijkstraTransportationPlanner::FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path) {
    return DImplementation->FindFastestPath(src, dest, path);
}

// Converts path to human-readable format
bool CDijkstraTransportationPlanner::GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const {
    return DImplementation->GetPathDescription(path, desc);
}
