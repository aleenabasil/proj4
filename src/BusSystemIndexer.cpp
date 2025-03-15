#include "BusSystemIndexer.h"
#include "BusSystem.h"
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cstddef>

struct CBusSystemIndexer::SImplementation{
    std::shared_ptr<CBusSystem> BusSystem;
    //list of stop IDS
    mutable std::vector<CBusSystem::TStopID> StopID;
    //list of route Names
    mutable std::vector<std::string> RouteNames;
    //checking if Stop IDs and Route Names are already sorted
    mutable bool SSorted = false;
    mutable bool RSorted = false;

    
    SImplementation(std::shared_ptr<CBusSystem> bussytem)
        : BusSystem(bussytem){}

    std::size_t StopCount() const{
        return BusSystem ? BusSystem->StopCount() : 0;
    }

    std::size_t RouteCount() const{
        return BusSystem ? BusSystem->RouteCount() : 0;
    }


    std::shared_ptr<SStop> SortedStopByIndex(std::size_t index) const {
        size_t count = BusSystem->StopCount();
        //returns null if the index is out of bounds
        if(index >= count) {
        return nullptr;
        }

        //checking if it is sorted (only sort once)
        if (!SSorted) {  
            //clear if anything is in there
            StopID.clear();
            //goes through all the stops
            for (size_t i = 0; i < count; i++) {
                //the stop
                auto stop = BusSystem->StopByIndex(i);
                //if there is a stop add it to the list of ids
                if (stop) {
                    StopID.push_back(stop->ID());}
            }
            //sort all the stop IDs
            std::sort(StopID.begin(), StopID.end());
            SSorted = true;
        }
        return (index < StopID.size()) ? BusSystem->StopByID(StopID[index]) : nullptr;
    }
    

    std::shared_ptr<SRoute> SortedRouteByIndex(std::size_t index) const {
        size_t count = BusSystem->StopCount();
        //returns null if the index is out of bounds
        if(index >= count) {
        return nullptr;
        }
        //checking if it is sorted (only sort once)
        if (!RSorted) {  
            //clear if anything is in there
            RouteNames.clear();
            //goes through all the routes
            for (size_t i = 0; i < count; i++) {
                //getting route
                auto route = BusSystem->RouteByIndex(i);
                //if there is a route add the name to the lsit
                if (route) RouteNames.push_back(route->Name());
            }
            //sorting the route Names
            std::sort(RouteNames.begin(), RouteNames.end());
            RSorted = true;
        }
    
        return (index < RouteNames.size()) ? BusSystem->RouteByName(RouteNames[index]) : nullptr;
    }



    std::shared_ptr<SStop> StopByNodeID(TNodeID id) const{
        size_t count = BusSystem->StopCount();
        //going through each stop
        for(size_t i = 0; i < count; i++){
            auto stop = BusSystem->StopByIndex(i);
            //if it exists
            if(stop){
                //checking if it is the right node ID
                if(stop->NodeID() == id){
                    return stop;
                }
            }
        }
        return nullptr;
    }

    //checks if there is a route hetween the start and destinations
    bool RoutesByNodeIDs(TNodeID src, TNodeID dest,
        std::unordered_set<std::shared_ptr<SRoute> > &routes) const {
        // Getting the start and end node
        auto start = StopByNodeID(src);
        auto end = StopByNodeID(dest);
        
        // Checking if it exists
        if (!start || !end) return false;
        
        // Getting the ID of both nodes
        CBusSystem::TStopID startID = start->ID();
        CBusSystem::TStopID endID = end->ID();
    
        for (size_t routeIndex = 0, allroutes = BusSystem->RouteCount(); 
            routeIndex < allroutes; ++routeIndex) {
            auto route = BusSystem->RouteByIndex(routeIndex);
            if (!route) continue;
    
            //if starrt stop is found
            CBusSystem::TStopID first = 0;  
    
            for (size_t stopIndex = 0, allstops = route->StopCount(); 
                stopIndex < allstops; ++stopIndex) {
                CBusSystem::TStopID currentStopID = route->GetStopID(stopIndex);
                //if we foudn the start stop
                if (currentStopID == startID) {
                    first = startID; 
                }
                //check for end
                if (first && currentStopID == endID) {  
                    routes.insert(route);
                    break;
                }
            }
        }
        
        return !routes.empty();
    }
    
    //check if route exists between two node IDS
    bool RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const{
        std::unordered_set<std::shared_ptr<CBusSystem::SRoute> > route;
        return RoutesByNodeIDs(src, dest, route);
    }
    
};


// CBusSystemIndexer member functions
// Constructor for the Bus System Indexer
CBusSystemIndexer::CBusSystemIndexer(std::shared_ptr<CBusSystem> bussystem){
    DImplementation = std::make_unique<SImplementation>(bussystem);
}

// Destructor for the Bus System Indexer
CBusSystemIndexer::~CBusSystemIndexer() = default;

// Returns the number of stops in the CBusSystem being indexed
std::size_t CBusSystemIndexer::StopCount() const noexcept{
    return DImplementation->StopCount();
}

// Returns the number of routes in the CBusSystem being indexed
std::size_t CBusSystemIndexer::RouteCount() const noexcept{
    return DImplementation->RouteCount();
}

// Returns the SStop specified by the index where the stops are sorted by
// their ID, nullptr is returned if index is greater than equal to
// StopCount()
std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::SortedStopByIndex(std::size_t index) const noexcept{
    return DImplementation->SortedStopByIndex(index);
}

// Returns the SRoute specified by the index where the routes are sorted by
// their Name, nullptr is returned if index is greater than equal to
// RouteCount()
std::shared_ptr<CBusSystem::SRoute> CBusSystemIndexer::SortedRouteByIndex(std::size_t index) const noexcept{
    return DImplementation->SortedRouteByIndex(index);
}

// Returns the SStop associated with the specified node ID, nullptr is
// returned if no SStop associated with the node ID exists
std::shared_ptr<CBusSystem::SStop> CBusSystemIndexer::StopByNodeID(TNodeID id) const noexcept{
    return DImplementation->StopByNodeID(id);
}

// Returns true if at least one route exists between the stops at the src and
// dest node IDs. All routes that have a route segment between the stops at
// the src and dest nodes will be placed in routes unordered set.
bool CBusSystemIndexer::RoutesByNodeIDs(TNodeID src, TNodeID dest,
std::unordered_set<std::shared_ptr<CBusSystem::SRoute> > &routes) const noexcept{
    return DImplementation->RoutesByNodeIDs(src, dest, routes);
}

// Returns true if at least one route exists between the stops at the src and
// dest node IDs.
bool CBusSystemIndexer::RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept{
    return DImplementation->RouteBetweenNodeIDs(src, dest);
}

