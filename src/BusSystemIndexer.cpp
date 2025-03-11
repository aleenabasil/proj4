#include "BusSystemIndexer.h"
#include "BusSystem.h"
#include <memory>
#include <string>
#include <iostream>
#include <vector>


// CBusSystemIndexer member functions
// Constructor for the Bus System Indexer
CBusSystemIndexer(std::shared_ptr<CBusSystem> bussystem);

// Destructor for the Bus System Indexer
~CBusSystemIndexer();

// Returns the number of stops in the CBusSystem being indexed
std::size_t StopCount() const noexcept;

// Returns the number of routes in the CBusSystem being indexed
std::size_t RouteCount() const noexcept;

// Returns the SStop specified by the index where the stops are sorted by
// their ID, nullptr is returned if index is greater than equal to
// StopCount()
std::shared_ptr<SStop> SortedStopByIndex(std::size_t index) const noexcept;

// Returns the SRoute specified by the index where the routes are sorted by
// their Name, nullptr is returned if index is greater than equal to
// RouteCount()
std::shared_ptr<SRoute> SortedRouteByIndex(std::size_t index) const noexcept;

// Returns the SStop associated with the specified node ID, nullptr is
// returned if no SStop associated with the node ID exists
std::shared_ptr<SStop> StopByNodeID(TNodeID id) const noexcept;

// Returns true if at least one route exists between the stops at the src and
// dest node IDs. All routes that have a route segment between the stops at
// the src and dest nodes will be placed in routes unordered set.
bool RoutesByNodeIDs(TNodeID src, TNodeID dest,
std::unordered_set<std::shared_ptr<SRoute> > &routes) const noexcept;

// Returns true if at least one route exists between the stops at the src and
// dest node IDs.
bool RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept;
