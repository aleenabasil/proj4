#include "DijkstraTransportationPlanner.h"
#include "DijkstraPathRouter.h"
#include "GeographicUtils.h"
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <iostream>


// CDijkstraTransportationPlanner member functions
// Constructor for the Dijkstra Transportation Planner
CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config);

// Destructor for the Dijkstra Transportation Planner
~CDijkstraTransportationPlanner();

// Returns the number of nodes in the street map
std::size_t NodeCount() const noexcept override;

// Returns the street map node specified by index if index is less than the
// NodeCount(). nullptr is returned if index is greater than or equal to
// NodeCount(). The nodes are sorted by Node ID.
std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const
noexcept override;

// Returns the distance in miles between the src and dest nodes of the
// shortest path if one exists. NoPathExists is returned if no path exists.
// The nodes of the shortest path are filled in the path parameter.
double FindShortestPath(TNodeID src, TNodeID dest, std::vector< TNodeID >
&path) override;

// Returns the time in hours for the fastest path between the src and dest
// nodes of the if one exists. NoPathExists is returned if no path exists.
// The transportation mode and nodes of the fastest path are filled in the
// path parameter.
double FindFastestPath(TNodeID src, TNodeID dest, std::vector< TTripStep >
&path) override;

// Returns true if the path description is created. Takes the trip steps path
// and converts it into a human readable set of steps.
bool GetPathDescription(const std::vector< TTripStep > &path, std::vector<
std::string > &desc) const override;
