#include <memory>         
#include <vector>          
#include <string>          
#include <unordered_map>  
#include <iostream> 
#include "CSVBusSystem.h" 
#include "DSVReader.h"    
#include "XMLReader.h"


// Implementation structure for the CSV Bus System
struct CCSVBusSystem::SImplementation{
    std::unordered_map<TStopID, std::shared_ptr<SStop>> Stops;
    std::unordered_map<std::string, std::shared_ptr<SRoute>> Routes;
    std::vector<std::shared_ptr<SStop>> SList;
    std::vector<std::shared_ptr<SRoute>> RList;
};


// CCSVBusSystem member functions
// Constructor for the CSV Bus System
CCSVBusSystem::CCSVBusSystem(std::shared_ptr< CDSVReader > stopsrc, std::shared_ptr<CDSVReader > routesrc){
    DImplementation = std::make_unique<SImplementation>();
    // Temporary vector to hold data from each row of CSV
    std::vector<std::string> row;  
    
    // If stops CSV is provided, process the stops
    if (stopsrc) {
        // Read each row of the stops CSV file
        while (stopsrc->ReadRow(row)) {
            if (row.size() >= 2) {
                //make sure info is valid  
                try {
                    auto stop = std::make_shared<SStop>();

                    // Convert and store stop ID and node ID from the row data
                    stop->StopID = std::stoul(row[0]);  
                    stop->val = std::stoul(row[1]);

                    // Add the stop to the stops map and the list
                    DImplementation->Stops[stop->StopID] = stop; 
                    DImplementation->SList.push_back(stop);  

                // Handle any exceptions that occur CANNOT REMOVE
                } catch (const std::exception& e) {
                    std::cerr << "Exception caught: " << e.what() << "\n";
                }
            }
        }
    }

    
    if (routesrc) {

        std::unordered_map<std::string, std::shared_ptr<SRoute>> temp;  
        // Read each row from the routes CSV
        while (routesrc->ReadRow(row)) {  
            if (row.size() >= 2) {  
                //make sure info is valid
                try {
                    std::string name = row[0];  
                    TStopID stopID = std::stoul(row[1]);  

                    // Find or create a route for the name
                    auto& route = temp[name];  
                    if (!route) {
                        route = std::make_shared<SRoute>();
                        route->RouteName = name;
                    }

                    // Add the stop ID to the route's list of stops
                    route->RouteStops.push_back(stopID);  

                // Handle any exceptions that occur CANNOT REMOVE
                } catch (const std::exception& e) { 
                    std::cerr << "Exception caught: " << e.what() << "\n";
                }
            }
        }
        // After reading the CSV, add all routes to the system's routes map and list
        for (const auto& pair : temp) {
            DImplementation->Routes[pair.first] = pair.second; 
            DImplementation->RList.push_back(pair.second);  
        }
    }
}


// Destructor for the CSV Bus System
CCSVBusSystem::~CCSVBusSystem() = default;


// Returns the number of stops in the system
std::size_t CCSVBusSystem::StopCount() const noexcept {
    // Get the size of the stop list
    std::size_t count = DImplementation->SList.size();
    return count;
}


// Returns the number of routes in the system
std::size_t CCSVBusSystem::RouteCount() const noexcept {
    // Get the size of the route list
    std::size_t count = DImplementation->RList.size();
    return count;
}


// Returns the SStop specified by the index, nullptr is returned if index is
// greater than equal to StopCount()
std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByIndex(std::size_t index) const noexcept {

    // return the stop at the given index if not
    // Return nullptr if index is out of bounds
    return (index < DImplementation->SList.size()) ? DImplementation->SList[index] : nullptr;
}


// Returns the SStop specified by the stop id, nullptr is returned if id is
// not in the stops
std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByID(TStopID id) const noexcept {
    // Search for the stop in the map
    auto stop = DImplementation->Stops.find(id);
    //returns speccifc stop if not
    //Return nullptr if the stop ID is not found
    return (stop != DImplementation->Stops.end()) ? stop->second : nullptr;
}


// Returns the SRoute specified by the index, nullptr is returned if index is
// greater than equal to RouteCount()
std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByIndex(std::size_t index) const noexcept {
    
    // return the route at the given index if not
    // Return nullptr if index is out of bounds
    return (index < DImplementation->RList.size()) ? DImplementation->RList[index] : nullptr;
}


// Returns the SRoute specified by the name, nullptr is returned if name is
// not in the routes
std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByName(const std::string &name) const noexcept {
     // Search for the route by name
    auto route = DImplementation->Routes.find(name);

    // Return the corresponding route object if not
    // Return nullptr if the route name is not found
    return (route != DImplementation->Routes.end()) ? route->second : nullptr;
}
