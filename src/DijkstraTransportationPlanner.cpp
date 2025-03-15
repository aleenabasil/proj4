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
    std::shared_ptr<SConfiguration> DConfig; // configuration object

    // constructor
    SImplementation(std::shared_ptr<SConfiguration> config) 
        : DConfig(config) {
    }

    // destructor
    ~SImplementation(){
    }

    // returns the number of nodes in the street map
    std::size_t NodeCount() const noexcept {
        return DConfig->StreetMap()->NodeCount();
    }
    
    // returns the street map node specified by index
    std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept {
        auto streetMap = DConfig->StreetMap(); // get the street map from the configuration
        if (!streetMap || index >= streetMap->NodeCount()) { // check if the street map is valid and the index is within bounds
            return nullptr; // return nullptr if the street map is invalid or the index is out of bounds
        }
    
        std::vector<std::shared_ptr<CStreetMap::SNode>> Nodes; // create a vector of nodes
        for (size_t i = 0; i < streetMap->NodeCount(); i++) { // iterate through the nodes in the street map
            Nodes.push_back(streetMap->NodeByIndex(i)); // add the node to the vector
        }
    
        std::sort(Nodes.begin(), Nodes.end(), [](const auto &a, const auto &b) {
            return a->ID() < b->ID(); // sort nodes by increasing ID
        });
    
        return Nodes[index]; // return the sorted node at the requested index        
    }
    
    double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path) {
        auto streetMap = DConfig->StreetMap(); // get the street map from the configuration
        if (!streetMap || streetMap->NodeCount() == 0) { // check if the street map is valid and not empty
            return CPathRouter::NoPathExists; // return NoPathExists if the street map is invalid or empty
        }
        
        std::unordered_map<TNodeID, double> distances; // create a map to store distances
        std::unordered_map<TNodeID, TNodeID> parents; // create a map to store parent nodes
        std::priority_queue<std::pair<double, TNodeID>, std::vector<std::pair<double, TNodeID>>, std::greater<>> pq; // create a priority queue

        // initialize structures
        for (size_t i = 0; i < streetMap->NodeCount(); ++i) {
            TNodeID nodeID = streetMap->NodeByIndex(i)->ID(); // get the node ID

            // initialize the nodes to inifinity
            distances[nodeID] = std::numeric_limits<double>::max();
        }

        // initialize starting point distance
        distances[src] = 0;
        pq.push({0, src}); // push the starting point to the priority queue

        // calculate distances to all nodes
        while (!pq.empty()) {
            // extract node with shortest distance
            auto [cost, node] = pq.top();
            pq.pop(); // remove the node from the priority queue

            if (node == dest) {
                break; // break if the destination node is reached
            }
            auto currentNode = streetMap->NodeByID(node); // get the current node
            if (!currentNode) { // check if the current node is valid
                continue;
            }

            for (size_t i = 0; i < streetMap->WayCount(); ++i) { // iterate through the ways in the street map
                auto way = streetMap->WayByIndex(i); // get the way
                if (!way) { // check if the way is valid
                    continue;
                }

                for (size_t j = 0; j < way->NodeCount() - 1; ++j) { // iterate through the nodes in the way
                    TNodeID u = way->GetNodeID(j); // get the start node
                    TNodeID v = way->GetNodeID(j + 1); // get the end node
                    double weight = SGeographicUtils::HaversineDistanceInMiles( // calculate the weight of the edge
                        streetMap->NodeByID(u)->Location(), streetMap->NodeByID(v)->Location()
                    );

                    if (distances[u] + weight < distances[v]) { // check if the new distance is shorter
                        distances[v] = distances[u] + weight; // update the distance
                        parents[v] = u; // update the parent node
                        pq.push({distances[v], v}); // push the node to the priority queue
                    }
                }
            }
        }

        if (distances[dest] == std::numeric_limits<double>::max()) // check if the destination node is unreachable
            return CPathRouter::NoPathExists; // return NoPathExists if the destination node is unreachable

        // Reconstruct the path from source to destination
        TNodeID current = dest;
        while (current != src) { // iterate from destination to source
            path.push_back(current); // add the node to the path
            current = parents[current]; // move to the parent node
        }
        path.push_back(src); // add the source node to the path
        std::reverse(path.begin(), path.end()); // reverse the path

        return distances[dest]; // return the distance to the destination
    }

    std::shared_ptr<CBusSystem::SStop> FindStopByNodeID(TNodeID nodeID, std::shared_ptr<CBusSystem> busSystem) { 
        for (size_t i = 0; i < busSystem->StopCount(); ++i) { // iterate through the stops in the bus system
            auto stop = busSystem->StopByIndex(i); // get the stop
            if (stop && stop->NodeID() == nodeID) {
                return stop;  // return the bus stop if found
            }
        }
        return nullptr; // no stop found at this node
    }

    double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path) {
        auto streetMap = DConfig->StreetMap(); // get the street map from the configuration
        auto busSystem = DConfig->BusSystem(); // get the bus system from the configuration
        if (!streetMap || !busSystem || streetMap->NodeCount() == 0) {
            return CPathRouter::NoPathExists; // return NoPathExists if the street map or bus system is invalid or empty
        }

        std::unordered_map<TNodeID, double> times; // create a map to store times
        std::unordered_map<TNodeID, std::pair<ETransportationMode, TNodeID>> parents; // create a map to store parent nodes
        std::priority_queue<std::pair<double, TNodeID>, std::vector<std::pair<double, TNodeID>>, std::greater<>> pq; // create a priority queue
    
        // initialize structures
        for (size_t i = 0; i < streetMap->NodeCount(); ++i) { // iterate through the nodes in the street map
            TNodeID nodeID = streetMap->NodeByIndex(i)->ID(); // get the node ID
            times[nodeID] = std::numeric_limits<double>::max(); // initialize the times to infinity
        }
    
        // initialize source node
        times[src] = 0;
        pq.push({0, src}); // push the source node to the priority queue
    
        while (!pq.empty()) { // iterate while the priority queue is not empty
            auto [time, node] = pq.top(); // extract node with shortest time
            pq.pop(); // remove the node from the priority queue
    
            if (node == dest) { // check if the destination node is reached
                break;
            }
            auto currentNode = streetMap->NodeByID(node); // get the current node
            if (!currentNode) { // check if the current node is valid
                continue;
            }
            // extract Speed from OSM Ways
            for (size_t i = 0; i < streetMap->WayCount(); ++i) {
                auto way = streetMap->WayByIndex(i); // get the way
                if (!way) {
                    continue;
                }
                double speed = DConfig->DefaultSpeedLimit(); // get the default speed limit
                if (way->HasAttribute("maxspeed")) { // check if the way has a maxspeed attribute
                    std::string speed_str = way->GetAttribute("maxspeed"); // get the maxspeed attribute
                    if (speed_str.find("mph") != std::string::npos) { // check if the speed is in mph
                        speed = std::stod(speed_str.substr(0, speed_str.find("mph"))); // convert the speed to double
                    }
                }
    
                for (size_t j = 0; j < way->NodeCount() - 1; ++j) { // iterate through the nodes in the way
                    TNodeID u = way->GetNodeID(j); // get the start node
                    TNodeID v = way->GetNodeID(j + 1); // get the end node
                    double distance = SGeographicUtils::HaversineDistanceInMiles(
                        streetMap->NodeByID(u)->Location(), streetMap->NodeByID(v)->Location() // calculate the distance between the nodes
                    );
    
                    double roadTime = distance / speed; // calculate the time to travel the road
                
                    if (times[u] + roadTime < times[v]) { // check if the new time is shorter
                        times[v] = times[u] + roadTime; // update the time
                        parents[v] = {ETransportationMode::Walk, u};  // update the parent node
                        pq.push({times[v], v}); // push the node to the priority queue
                    }
                }
            }
    
            // Ensure stop is valid before using it
            auto stop = FindStopByNodeID(node, busSystem); // find the bus stop at the current node
            if (stop) { // check if the bus stop is valid
                for (size_t i = 0; i < busSystem->RouteCount(); ++i) { // iterate through the routes in the bus system
                    auto route = busSystem->RouteByIndex(i); // get the route
                    if (!route) { // check if the route is valid
                        continue;
                    }

                    for (size_t j = 0; j < route->StopCount() - 1; ++j) { // iterate through the stops in the route
                        TNodeID busSrc = route->GetStopID(j); // get the start stop
                        TNodeID busDest = route->GetStopID(j + 1); // get the end stop
                        
                        double distance = SGeographicUtils::HaversineDistanceInMiles(
                            streetMap->NodeByID(busSrc)->Location(), 
                            streetMap->NodeByID(busDest)->Location()
                        );
                        
                        double busSpeed = DConfig->DefaultSpeedLimit(); // get the default speed limit
                        double busTime = (distance / busSpeed) + (DConfig->BusStopTime() / 3600.0); // calculate the time to travel the road and stop time
    
                        if (times[busSrc] + busTime < times[busDest]) {
                            times[busDest] = times[busSrc] + busTime; // update the time
                            parents[busDest] = {ETransportationMode::Bus, busSrc}; // update the parent node
                            pq.push({times[busDest], busDest}); // push the node to the priority queue
                        }
                    }
                }
            }
        }
    
        // ensure dest was updated in times before path reconstruction
        if (times[dest] == std::numeric_limits<double>::max()) {
            return CPathRouter::NoPathExists;
        }
        // ensure parents[current] exists before accessing .second
        TNodeID current = dest;
        if (parents.find(current) == parents.end()) { // check if the parent node exists
            return CPathRouter::NoPathExists; // return NoPathExists if the parent node does not exist
        }
    
        while (current != src) { // iterate from destination to source
            if (parents.find(current) == parents.end()) { // check if the parent node exists  
                return CPathRouter::NoPathExists; // return NoPathExists if the parent node does not exist
            }
    
            path.push_back({parents[current].first, current}); // add the node to the path
            current = parents[current].second; // move to the parent node
        }
    
        path.push_back({ETransportationMode::Walk, src}); // add the source node to the path
        std::reverse(path.begin(), path.end()); // reverse the path
    
        return times[dest]; // return the time to the destination
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
    DImplementation = std::make_unique<SImplementation>(config); // create the implementation object
}

// destructor
CDijkstraTransportationPlanner::~CDijkstraTransportationPlanner() {

}

// returns the number of nodes in the street map
std::size_t CDijkstraTransportationPlanner::NodeCount() const noexcept {
    return DImplementation->NodeCount();
}

// returns the street map node specified by index
std::shared_ptr<CStreetMap::SNode> CDijkstraTransportationPlanner::SortedNodeByIndex(std::size_t index) const noexcept {
    return DImplementation->SortedNodeByIndex(index);
}

// finds the shortest path
double CDijkstraTransportationPlanner::FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path) {
    return DImplementation->FindShortestPath(src, dest, path);
}

// finds the fastest path
double CDijkstraTransportationPlanner::FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path) {
    return DImplementation->FindFastestPath(src, dest, path);
}

// converts path to readable format
bool CDijkstraTransportationPlanner::GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const {
    return DImplementation->GetPathDescription(path, desc);
}
