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
        return DConfig->StreetMap()->NodeByIndex(index);
    }
        
    double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path) {
        path.clear();
        auto streetMap = DConfig->StreetMap();
        if (!streetMap) return -1.0;
        
        // Check if source and destination exist
        if (!streetMap->NodeByID(src) || !streetMap->NodeByID(dest)) {
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

        // Initialize distances for source
        distances[src] = 0.0;
        pq.push({0.0, src});

        while (!pq.empty()) {
            auto [dist, node] = pq.top();
            pq.pop();
            
            if (node == dest) break;
            
            // Skip if we've already found a shorter path to this node
            if (dist > distances[node]) continue;
            
            // Find all neighboring nodes via ways
            for (size_t i = 0; i < streetMap->WayCount(); ++i) {
                auto way = streetMap->WayByIndex(i);
                if (!way) continue;
                
                for (size_t j = 0; j < way->NodeCount(); ++j) {
                    if (way->GetNodeID(j) == node) {
                        // Found current node in way, now check neighbors
                        if (j > 0) {
                            // Check previous node in way
                            TNodeID neighbor = way->GetNodeID(j-1);
                            double weight = SGeographicUtils::HaversineDistanceInMiles(
                                streetMap->NodeByID(node)->Location(),
                                streetMap->NodeByID(neighbor)->Location()
                            );
                            
                            double newDist = dist + weight;
                            if (!distances.count(neighbor) || newDist < distances[neighbor]) {
                                distances[neighbor] = newDist;
                                parents[neighbor] = node;
                                pq.push({newDist, neighbor});
                            }
                        }
                        
                        if (j < way->NodeCount() - 1) {
                            // Check next node in way
                            TNodeID neighbor = way->GetNodeID(j+1);
                            double weight = SGeographicUtils::HaversineDistanceInMiles(
                                streetMap->NodeByID(node)->Location(),
                                streetMap->NodeByID(neighbor)->Location()
                            );
                            
                            double newDist = dist + weight;
                            if (!distances.count(neighbor) || newDist < distances[neighbor]) {
                                distances[neighbor] = newDist;
                                parents[neighbor] = node;
                                pq.push({newDist, neighbor});
                            }
                        }
                    }
                }
            }
        }
        
        if (!distances.count(dest)) {
            return -1.0; // No path exists
        }
        
        // Reconstruct path
        TNodeID current = dest;
        while (current != src) {
            path.push_back(current);
            if (!parents.count(current)) {
                return -1.0; // Path reconstruction failed
            }
            current = parents[current];
        }
        path.push_back(src);
        std::reverse(path.begin(), path.end());
        
        return distances[dest];
    }

    std::unordered_map<TNodeID, std::vector<std::pair<TNodeID, ETransportationMode>>> FindNeighbors(TNodeID nodeID) {
        auto streetMap = DConfig->StreetMap();
        auto busSystem = DConfig->BusSystem();
        std::unordered_map<TNodeID, std::vector<std::pair<TNodeID, ETransportationMode>>> neighbors;
        
        // Find street neighbors (for walking and biking)
        for (size_t i = 0; i < streetMap->WayCount(); ++i) {
            auto way = streetMap->WayByIndex(i);
            if (!way) continue;
            
            for (size_t j = 0; j < way->NodeCount(); ++j) {
                if (way->GetNodeID(j) == nodeID) {
                    // Found current node in way, now check neighbors
                    if (j > 0) {
                        // Previous node in way
                        TNodeID neighbor = way->GetNodeID(j-1);
                        neighbors[neighbor].push_back({neighbor, ETransportationMode::Walk});
                        neighbors[neighbor].push_back({neighbor, ETransportationMode::Bike});
                    }
                    
                    if (j < way->NodeCount() - 1) {
                        // Next node in way
                        TNodeID neighbor = way->GetNodeID(j+1);
                        neighbors[neighbor].push_back({neighbor, ETransportationMode::Walk});
                        neighbors[neighbor].push_back({neighbor, ETransportationMode::Bike});
                    }
                }
            }
        }
        
        // Find bus neighbors
        if (busSystem) {
            // Check if this node is a bus stop
            std::shared_ptr<CBusSystem::SStop> stop = nullptr;
            for (size_t i = 0; i < busSystem->StopCount(); ++i) {
                auto s = busSystem->StopByIndex(i);
                if (s && s->NodeID() == nodeID) {
                    stop = s;
                    break;
                }
            }
            
            if (stop) {
                for (size_t i = 0; i < busSystem->RouteCount(); ++i) {
                    auto route = busSystem->RouteByIndex(i);
                    if (!route) continue;
                    
                    for (size_t j = 0; j < route->StopCount(); ++j) {
                        if (route->GetStopID(j) == stop->ID()) {
                            // Found stop in route, check next stop
                            if (j < route->StopCount() - 1) {
                                TNodeID nextStopID = route->GetStopID(j+1);
                                auto nextStop = busSystem->StopByID(nextStopID);
                                if (nextStop) {
                                    neighbors[nextStop->NodeID()].push_back({nextStop->NodeID(), ETransportationMode::Bus});
                                }
                            }
                        }
                    }
                }
            }
        }
        
        return neighbors;
    }

    double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path) {
        path.clear();
        auto streetMap = DConfig->StreetMap();
        auto busSystem = DConfig->BusSystem();
        if (!streetMap) return -1.0;
        
        // Check if source and destination exist
        if (!streetMap->NodeByID(src) || !streetMap->NodeByID(dest)) {
            return -1.0;
        }
        
        // Handle trivial case
        if (src == dest) {
            path.push_back({ETransportationMode::Walk, src});
            return 0.0;
        }

        // Modified Dijkstra's algorithm
        std::unordered_map<TNodeID, double> times;
        std::unordered_map<TNodeID, std::pair<TNodeID, ETransportationMode>> parents;
        std::priority_queue<std::pair<double, TNodeID>, std::vector<std::pair<double, TNodeID>>, std::greater<>> pq;

        // Initialize times for source
        times[src] = 0.0;
        pq.push({0.0, src});

        while (!pq.empty()) {
            auto [time, node] = pq.top();
            pq.pop();
            
            if (node == dest) break;
            
            // Skip if we've already found a faster path to this node
            if (time > times[node]) continue;
            
            // Find all adjacent nodes via ways (for walking and biking)
            for (size_t i = 0; i < streetMap->WayCount(); ++i) {
                auto way = streetMap->WayByIndex(i);
                if (!way) continue;
                
                for (size_t j = 0; j < way->NodeCount(); ++j) {
                    if (way->GetNodeID(j) == node) {
                        // Process adjacent nodes
                        if (j > 0) {
                            // Previous node in way
                            TNodeID neighbor = way->GetNodeID(j-1);
                            double distance = SGeographicUtils::HaversineDistanceInMiles(
                                streetMap->NodeByID(node)->Location(),
                                streetMap->NodeByID(neighbor)->Location()
                            );
                            
                            // Try walking
                            double walkTime = distance / DConfig->WalkSpeed();
                            double newTime = time + walkTime;
                            if (!times.count(neighbor) || newTime < times[neighbor]) {
                                times[neighbor] = newTime;
                                parents[neighbor] = {node, ETransportationMode::Walk};
                                pq.push({newTime, neighbor});
                            }
                            
                            // Try biking
                            double bikeTime = distance / DConfig->BikeSpeed();
                            newTime = time + bikeTime;
                            if (!times.count(neighbor) || newTime < times[neighbor]) {
                                times[neighbor] = newTime;
                                parents[neighbor] = {node, ETransportationMode::Bike};
                                pq.push({newTime, neighbor});
                            }
                        }
                        
                        if (j < way->NodeCount() - 1) {
                            // Next node in way
                            TNodeID neighbor = way->GetNodeID(j+1);
                            double distance = SGeographicUtils::HaversineDistanceInMiles(
                                streetMap->NodeByID(node)->Location(),
                                streetMap->NodeByID(neighbor)->Location()
                            );
                            
                            // Try walking
                            double walkTime = distance / DConfig->WalkSpeed();
                            double newTime = time + walkTime;
                            if (!times.count(neighbor) || newTime < times[neighbor]) {
                                times[neighbor] = newTime;
                                parents[neighbor] = {node, ETransportationMode::Walk};
                                pq.push({newTime, neighbor});
                            }
                            
                            // Try biking
                            double bikeTime = distance / DConfig->BikeSpeed();
                            newTime = time + bikeTime;
                            if (!times.count(neighbor) || newTime < times[neighbor]) {
                                times[neighbor] = newTime;
                                parents[neighbor] = {node, ETransportationMode::Bike};
                                pq.push({newTime, neighbor});
                            }
                        }
                    }
                }
            }
            
            // Process bus routes
            if (busSystem) {
                // Check if this node is a bus stop
                std::shared_ptr<CBusSystem::SStop> stop = nullptr;
                for (size_t i = 0; i < busSystem->StopCount(); ++i) {
                    auto s = busSystem->StopByIndex(i);
                    if (s && s->NodeID() == node) {
                        stop = s;
                        break;
                    }
                }
                
                if (stop) {
                    for (size_t i = 0; i < busSystem->RouteCount(); ++i) {
                        auto route = busSystem->RouteByIndex(i);
                        if (!route) continue;
                        
                        for (size_t j = 0; j < route->StopCount(); ++j) {
                            if (route->GetStopID(j) == stop->ID()) {
                                // Found stop in route, check next stop
                                if (j < route->StopCount() - 1) {
                                    TNodeID nextStopID = route->GetStopID(j+1);
                                    auto nextStop = busSystem->StopByID(nextStopID);
                                    if (nextStop) {
                                        TNodeID nextNodeID = nextStop->NodeID();
                                        double busTime = DConfig->BusStopTime();
                                        double newTime = time + busTime;
                                        
                                        if (!times.count(nextNodeID) || newTime < times[nextNodeID]) {
                                            times[nextNodeID] = newTime;
                                            parents[nextNodeID] = {node, ETransportationMode::Bus};
                                            pq.push({newTime, nextNodeID});
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        if (!times.count(dest)) {
            return -1.0; // No path exists
        }
        
        // Reconstruct path with transportation modes
        std::vector<TTripStep> reversePath;
        TNodeID current = dest;
        while (current != src) {
            if (!parents.count(current)) {
                return -1.0; // Path reconstruction failed
            }
            
            auto [prevNode, mode] = parents[current];
            reversePath.push_back({mode, current});
            current = prevNode;
        }
        reversePath.push_back({ETransportationMode::Walk, src}); // Starting point
        
        // Reverse the path to get source to destination
        std::reverse(reversePath.begin(), reversePath.end());
        path = reversePath;
        
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

CDijkstraTransportationPlanner::CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration>
