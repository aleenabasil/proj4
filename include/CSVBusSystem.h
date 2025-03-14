#ifndef CSVBUSROUTE_H
#define CSVBUSROUTE_H

#include "BusSystem.h"
#include "DSVReader.h"

class CCSVBusSystem : public CBusSystem{
    private:
        struct SImplementation;
        struct SStop : public CBusSystem::SStop {
            TStopID StopID;
            CStreetMap::TNodeID val;
            TStopID ID() const noexcept override {return StopID;}
    
            CStreetMap::TNodeID NodeID() const noexcept override {return val;}
        };
    
        struct SRoute : public CBusSystem::SRoute {
            std::string RouteName;
            std::vector<TStopID> RouteStops;
    
            std::string Name() const noexcept override {return RouteName;}
    
            std::size_t StopCount() const noexcept override {return RouteStops.size();}
    
            TStopID GetStopID(std::size_t index) const noexcept override {
                return (index < RouteStops.size()) ? RouteStops[index] : CBusSystem::InvalidStopID;}
        };
        std::unique_ptr< SImplementation > DImplementation;
    public:
        CCSVBusSystem(std::shared_ptr< CDSVReader > stopsrc, std::shared_ptr< CDSVReader > routesrc);
        ~CCSVBusSystem();

        std::size_t StopCount() const noexcept override;
        std::size_t RouteCount() const noexcept override;
        std::shared_ptr<CBusSystem::SStop> StopByIndex(std::size_t index) const noexcept override;
        std::shared_ptr<CBusSystem::SStop> StopByID(TStopID id) const noexcept override;
        std::shared_ptr<CBusSystem::SRoute> RouteByIndex(std::size_t index) const noexcept override;
        std::shared_ptr<CBusSystem::SRoute> RouteByName(const std::string &name) const noexcept override;
};

#endif
