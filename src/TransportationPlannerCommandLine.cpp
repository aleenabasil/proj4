#include "TransportationPlannerCommandLine.h"

struct CTransportationPlannerCommandLine::SImplementation{
    SImplementation() {
    }

    ~SImplementation() {
    }

    bool ProcessCommands() {
        return true;
    }
};

CTransportationPlannerCommandLine::CTransportationPlannerCommandLine(std::shared_ptr<CDataSource> cmdsrc, std::shared_ptr<CDataSink> outsink, std::shared_ptr<CDataSink> errsink, std::shared_ptr<CDataFactory> results, std::shared_ptr<CTransportationPlanner> planner) {
    DImplementation = std::make_unique<SImplementation>();
}

CTransportationPlannerCommandLine::~CTransportationPlannerCommandLine() {

}

bool CTransportationPlannerCommandLine::ProcessCommands() {
    return DImplementation->ProcessCommands();
}

