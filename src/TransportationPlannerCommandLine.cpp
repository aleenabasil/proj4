#include "TransportationPlannerCommandLine.h"
#include "TransportationPlanner.h"
#include "DataSource.h"
#include "DataSink.h"
#include "DataFactory.h"
#include <memory>
#include <string>
#include <vector>

// Constructor for the Transportation Planner Command Line
CTransportationPlannerCommandLine::CTransportationPlannerCommandLine(std::shared_ptr<CDataSource> cmdsrc,
                                                                   std::shared_ptr<CDataSink> outsink,
                                                                   std::shared_ptr<CDataSink> errsink,
                                                                   std::shared_ptr<CDataFactory> results,
                                                                   std::shared_ptr<CTransportationPlanner> planner)
    : DCommandSource(cmdsrc),
      DOutputSink(outsink),
      DErrorSink(errsink),
      DResultsFactory(results),
      DPlanner(planner) {
}

// Destructor for the Transportation Planner Command Line
CTransportationPlannerCommandLine::~CTransportationPlannerCommandLine() {
}

// Processes the commands
bool CTransportationPlannerCommandLine::ProcessCommands() {
    std::string command;
    bool success = true;
    
    // Process commands until end of input or failure
    while(success && DCommandSource->Read(command)) {
        // Parse command
        if(command.empty()) {
            continue; // Skip empty commands
        }
        
        // Basic command parser
        if(command == "help" || command == "?") {
            DOutputSink->Write("Transportation Planner Commands:\n");
            DOutputSink->Write("  help, ? - Display this help message\n");
            DOutputSink->Write("  exit, quit - Exit the program\n");
            DOutputSink->Write("  node <id> - Display information about node with ID\n");
            DOutputSink->Write("  shortest <src> <dest> - Find shortest path between nodes\n");
            DOutputSink->Write("  fastest <src> <dest> - Find fastest path between nodes\n");
        }
        else if(command == "exit" || command == "quit") {
            break; // Exit command loop
        }
        else if(command.substr(0, 5) == "node ") {
            try {
                TNodeID nodeID = std::stoul(command.substr(5));
                auto node = DPlanner->NodeByID(nodeID);
                
                if(node) {
                    DOutputSink->Write("Node ID: " + std::to_string(nodeID) + "\n");
                    DOutputSink->Write("Location: " + std::to_string(node->Location().first) + 
                                      ", " + std::to_string(node->Location().second) + "\n");
                }
                else {
                    DErrorSink->Write("Error: Node " + std::to_string(nodeID) + " not found.\n");
                }
            }
            catch(const std::exception& e) {
                DErrorSink->Write("Error: Invalid node ID format.\n");
            }
        }
        else if(command.substr(0, 9) == "shortest ") {
            // Parse source and destination IDs
            size_t pos = command.find(" ", 9);
            if(pos != std::string::npos) {
                try {
                    TNodeID srcID = std::stoul(command.substr(9, pos - 9));
                    TNodeID destID = std::stoul(command.substr(pos + 1));
                    
                    std::vector<TNodeID> path;
                    double distance = DPlanner->FindShortestPath(srcID, destID, path);
                    
                    if(distance >= 0.0) {
                        DOutputSink->Write("Shortest path from " + std::to_string(srcID) + 
                                          " to " + std::to_string(destID) + ":\n");
                        DOutputSink->Write("Distance: " + std::to_string(distance) + " miles\n");
                        DOutputSink->Write("Path: ");
                        for(size_t i = 0; i < path.size(); ++i) {
                            DOutputSink->Write(std::to_string(path[i]));
                            if(i < path.size() - 1) {
                                DOutputSink->Write(" -> ");
                            }
                        }
                        DOutputSink->Write("\n");
                    }
                    else {
                        DErrorSink->Write("Error: No path found from " + std::to_string(srcID) + 
                                         " to " + std::to_string(destID) + ".\n");
                    }
                }
                catch(const std::exception& e) {
                    DErrorSink->Write("Error: Invalid node ID format.\n");
                }
            }
            else {
                DErrorSink->Write("Error: Invalid shortest command format. Use 'shortest <src> <dest>'.\n");
            }
        }
        else if(command.substr(0, 8) == "fastest ") {
            // Parse source and destination IDs
            size_t pos = command.find(" ", 8);
            if(pos != std::string::npos) {
                try {
                    TNodeID srcID = std::stoul(command.substr(8, pos - 8));
                    TNodeID destID = std::stoul(command.substr(pos + 1));
                    
                    std::vector<TTripStep> path;
                    double time = DPlanner->FindFastestPath(srcID, destID, path);
                    
                    if(time >= 0.0) {
                        DOutputSink->Write("Fastest path from " + std::to_string(srcID) + 
                                          " to " + std::to_string(destID) + ":\n");
                        DOutputSink->Write("Time: " + std::to_string(time) + " hours\n");
                        
                        std::vector<std::string> description;
                        if(DPlanner->GetPathDescription(path, description)) {
                            DOutputSink->Write("Path description:\n");
                            for(const auto& step : description) {
                                DOutputSink->Write("  " + step + "\n");
                            }
                        }
                    }
                    else {
                        DErrorSink->Write("Error: No path found from " + std::to_string(srcID) + 
                                         " to " + std::to_string(destID) + ".\n");
                    }
                }
                catch(const std::exception& e) {
                    DErrorSink->Write("Error: Invalid node ID format.\n");
                }
            }
            else {
                DErrorSink->Write("Error: Invalid fastest command format. Use 'fastest <src> <dest>'.\n");
            }
        }
        else {
            DErrorSink->Write("Error: Unknown command '" + command + "'. Type 'help' for assistance.\n");
        }
    }
    
    return success;
}
