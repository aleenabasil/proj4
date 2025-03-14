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

    SImplementation(std::shared_ptr<SConfiguration> config)
        : DConfig(config) {
    }

    ~SImplementation(){
    }

    std::size_t NodeCount() const noexcept {
        return DConfig->StreetMap()->NodeCount();
    }
    
    std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept {
        auto streetMap = DConfig->StreetMap();
        if (!streetMap || index >= streetMap->NodeCount()) {
            return nullptr;
        }
    
        std::vector<std::shared_ptr<CStreetMap::SNode>> Nodes;
        for (size_t i = 0; i < streetMap->NodeCount(); i++) {
            Nodes.push_back(streetMap->NodeByIndex(i));
        }
    
        std::sort(Nodes.begin(), Nodes.end(), [](const auto &a, const auto &b) {
            return a->ID() < b->ID(); // Sort nodes by increasing ID
        });
    
        return Nodes[index]; // Return the sorted node at the requested index        
    }
        
    double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path) {
        auto streetMap = DConfig->StreetMap();
        if (!streetMap || streetMap->NodeCount() == 0) return CPathRouter::NoPathExists;

        std::unordered_map<TNodeID, double> distances;
        std::unordered_map<TNodeID, TNodeID> parents;
        std::priority_queue<std::pair<double, TNodeID>, std::vector<std::pair<double, TNodeID>>, std::greater<>> pq;

        // Initialize structures
        for (size_t i = 0; i < streetMap->NodeCount(); ++i) {
            TNodeID nodeID = streetMap->NodeByIndex(i)->ID();

            // Initialize the nodes to inifinity
            distances[nodeID] = std::numeric_limits<double>::max();
        }

        // Initialize starting point distance
        distances[src] = 0;
        pq.push({0, src});

        // Calculate distances to all nodes
        while (!pq.empty()) {
            // Extract node with shortest distance
            auto [cost, node] = pq.top();
            pq.pop();

            if (node == dest) break;

            auto currentNode = streetMap->NodeByID(node);
            if (!currentNode) continue;

            for (size_t i = 0; i < streetMap->WayCount(); ++i) {
                auto way = streetMap->WayByIndex(i);
                if (!way) continue;

                for (size_t j = 0; j < way->NodeCount() - 1; ++j) {
                    TNodeID u = way->GetNodeID(j);
                    TNodeID v = way->GetNodeID(j + 1);
                    double weight = SGeographicUtils::HaversineDistanceInMiles(
                        streetMap->NodeByID(u)->Location(), streetMap->NodeByID(v)->Location()
                    );

                    if (distances[u] + weight < distances[v]) {
                        distances[v] = distances[u] + weight;
                        parents[v] = u;
                        pq.push({distances[v], v});
                    }
                }
            }
        }

        if (distances[dest] == std::numeric_limits<double>::max()) 
            return CPathRouter::NoPathExists;

        // Reconstruct the path from source to destination
        TNodeID current = dest;
        while (current != src) {
            path.push_back(current);
            current = parents[current];
        }
        path.push_back(src);
        std::reverse(path.begin(), path.end());

        return distances[dest];
    }

    std::shared_ptr<CBusSystem::SStop> FindStopByNodeID(TNodeID nodeID, std::shared_ptr<CBusSystem> busSystem) {
        for (size_t i = 0; i < busSystem->StopCount(); ++i) {
            auto stop = busSystem->StopByIndex(i);
            if (stop && stop->NodeID() == nodeID) {
                return stop;  // Return the bus stop if found
            }
        }
        return nullptr; // No stop found at this node
    }

    double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path) {
        auto streetMap = DConfig->StreetMap();
        auto busSystem = DConfig->BusSystem();
        if (!streetMap || !busSystem || streetMap->NodeCount() == 0) return CPathRouter::NoPathExists;
    
        std::unordered_map<TNodeID, double> times;
        std::unordered_map<TNodeID, std::pair<ETransportationMode, TNodeID>> parents;
        std::priority_queue<std::pair<double, TNodeID>, std::vector<std::pair<double, TNodeID>>, std::greater<>> pq;
    
        // Initialize structures
        for (size_t i = 0; i < streetMap->NodeCount(); ++i) {
            TNodeID nodeID = streetMap->NodeByIndex(i)->ID();
            times[nodeID] = std::numeric_limits<double>::max();
        }
    
        // Initialize source node to 0
        times[src] = 0;
        pq.push({0, src});
    
        while (!pq.empty()) {
            // Get node with the shortest known travel time
            auto [time, node] = pq.top();
            pq.pop();
    
            if (node == dest) break;
    
            auto currentNode = streetMap->NodeByID(node);
            if (!currentNode) continue;
    
            // Speed Extraction**
            for (size_t i = 0; i < streetMap->WayCount(); ++i) {
                auto way = streetMap->WayByIndex(i);
                if (!way) continue;
        
                for (size_t j = 0; j < way->NodeCount() - 1; ++j) {
                    TNodeID u = way->GetNodeID(j);
                    TNodeID v = way->GetNodeID(j + 1);
                    double distance = SGeographicUtils::HaversineDistanceInMiles(
                        streetMap->NodeByID(u)->Location(), streetMap->NodeByID(v)->Location()
                    );
    
                    // Walking speed
                    double walkTime = distance / DConfig->WalkSpeed();
                    if (times[u] + walkTime < times[v]) {
                        times[v] = times[u] + walkTime;
                        parents[v] = {ETransportationMode::Walk, u};
                        pq.push({times[v], v});
                    }
    
                    // Biking speed
                    double bikeTime = distance / DConfig->BikeSpeed();
                    if (times[u] + bikeTime < times[v]) {
                        times[v] = times[u] + bikeTime;
                        parents[v] = {ETransportationMode::Bike, u};
                        pq.push({times[v], v});
                    }
                }
            }
    
            // Correct Bus Time Calculation**
            auto stop = FindStopByNodeID(node, busSystem);
            if (stop) {
                for (size_t i = 0; i < busSystem->RouteCount(); ++i) {
                    auto route = busSystem->RouteByIndex(i);
                    if (!route) continue;
    
                    for (size_t j = 0; j < route->StopCount() - 1; ++j) {
                        TNodeID busSrc = route->GetStopID(j);
                        TNodeID busDest = route->GetStopID(j + 1);
                        
                        // Correct bus travel time computation**
                        double distance = SGeographicUtils::HaversineDistanceInMiles(
                            streetMap->NodeByID(busSrc)->Location(), 
                            streetMap->NodeByID(busDest)->Location()
                        );
                        
                        double busTime = (distance / DConfig->DefaultSpeedLimit()) + (DConfig->BusStopTime() / 3600.0);
                        if (times[busSrc] + busTime < times[busDest]) {
                            times[busDest] = times[busSrc] + busTime;
                            parents[busDest] = {ETransportationMode::Bus, busSrc};
                            pq.push({times[busDest], busDest});
                        }
                    }
                }
            }
        }
    
        if (times[dest] == std::numeric_limits<double>::max()) return CPathRouter::NoPathExists;
    
        // Correct Path Reconstruction**
        TNodeID current = dest;
        while (current != src) {
            path.push_back({parents[current].first, current});
            current = parents[current].second;
        }
        path.push_back({parents[src].first, src});
        std::reverse(path.begin(), path.end());
    
        return times[dest];
    }

    bool GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const {
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
