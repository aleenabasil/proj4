#include "OpenStreetMap.h"
#include <memory>         
#include <vector>          
#include <string>          
#include <unordered_map>  
#include <iostream> 
#include "DSVReader.h"  
#include "XMLReader.h"

// Define the implementation structure for COpenStreetMap
struct COpenStreetMap::SImplementation {
    class SNodeImpl;
    class SWayImpl;
    
    std::vector<std::shared_ptr<SNodeImpl>> Nodes;
    std::vector<std::shared_ptr<SWayImpl>> Ways;
};

class COpenStreetMap::SImplementation::SNodeImpl : public CStreetMap::SNode {
public:
    TNodeID NodeID;
    TLocation NodeLocation;
    std::unordered_map<std::string, std::string> Attributes;

    TNodeID ID() const noexcept override { return NodeID; }
    TLocation Location() const noexcept override { return NodeLocation; }
    std::size_t AttributeCount() const noexcept override { return Attributes.size(); }
    std::string GetAttributeKey(std::size_t index) const noexcept override {
        if (index < Attributes.size()) {
            auto it = Attributes.begin();
            std::advance(it, index);
            return it->first;
        }
        return "";
    }
    bool HasAttribute(const std::string &key) const noexcept override { return Attributes.find(key) != Attributes.end(); }
    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = Attributes.find(key);
        return (it != Attributes.end()) ? it->second : "";
    }
};

class COpenStreetMap::SImplementation::SWayImpl : public CStreetMap::SWay {
public:
    TWayID WayID;
    std::vector<TNodeID> NodeIDs;
    std::unordered_map<std::string, std::string> Attributes;

    TWayID ID() const noexcept override { return WayID; }
    std::size_t NodeCount() const noexcept override { return NodeIDs.size(); }
    TNodeID GetNodeID(std::size_t index) const noexcept override {
        return (index < NodeIDs.size()) ? NodeIDs[index] : CStreetMap::InvalidNodeID;
    }
    std::size_t AttributeCount() const noexcept override { return Attributes.size(); }
    std::string GetAttributeKey(std::size_t index) const noexcept override {
        if (index < Attributes.size()) {
            auto it = Attributes.begin();
            std::advance(it, index);
            return it->first;
        }
        return "";
    }
    bool HasAttribute(const std::string &key) const noexcept override { return Attributes.find(key) != Attributes.end(); }
    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = Attributes.find(key);
        return (it != Attributes.end()) ? it->second : "";
    }
};

COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src) {
    DImplementation = std::make_unique<SImplementation>();
    SXMLEntity entity;
    std::shared_ptr<SImplementation::SNodeImpl> currentNode = nullptr;
    std::shared_ptr<SImplementation::SWayImpl> currentWay = nullptr;

    while (src->ReadEntity(entity)) {
        if (entity.DType == SXMLEntity::EType::StartElement) {
            if (entity.DNameData == "node") {
                currentNode = std::make_shared<SImplementation::SNodeImpl>();
                currentWay = nullptr;
                for (const auto& attr : entity.DAttributes) {
                    if (attr.first == "id") currentNode->NodeID = std::stoull(attr.second);
                    else if (attr.first == "lat") currentNode->NodeLocation.first = std::stod(attr.second);
                    else if (attr.first == "lon") currentNode->NodeLocation.second = std::stod(attr.second);
                    else currentNode->Attributes[attr.first] = attr.second;
                }
            } else if (entity.DNameData == "way") {
                currentWay = std::make_shared<SImplementation::SWayImpl>();
                currentNode = nullptr;
                for (const auto& attr : entity.DAttributes) {
                    if (attr.first == "id") currentWay->WayID = std::stoull(attr.second);
                    else currentWay->Attributes[attr.first] = attr.second;
                }
            } else if (entity.DNameData == "nd" && currentWay) {
                for (const auto& attr : entity.DAttributes) {
                    if (attr.first == "ref") currentWay->NodeIDs.push_back(std::stoull(attr.second));
                }
            } else if (entity.DNameData == "tag") {
                std::string key, value;
                for (const auto& attr : entity.DAttributes) {
                    if (attr.first == "k") key = attr.second;
                    else if (attr.first == "v") value = attr.second;
                }
                if (!key.empty()) {
                    if (currentNode) currentNode->Attributes[key] = value;
                    else if (currentWay) currentWay->Attributes[key] = value;
                }
            }
        } else if (entity.DType == SXMLEntity::EType::EndElement) {
            if (entity.DNameData == "node" && currentNode) {
                DImplementation->Nodes.push_back(currentNode);
                currentNode = nullptr;
            } else if (entity.DNameData == "way" && currentWay) {
                DImplementation->Ways.push_back(currentWay);
                currentWay = nullptr;
            }
        }
    }
}

COpenStreetMap::~COpenStreetMap() = default;
std::size_t COpenStreetMap::NodeCount() const noexcept { return DImplementation->Nodes.size(); }
std::size_t COpenStreetMap::WayCount() const noexcept { return DImplementation->Ways.size(); }
std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(std::size_t index) const noexcept { return (index < DImplementation->Nodes.size()) ? DImplementation->Nodes[index] : nullptr; }
std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByID(TNodeID id) const noexcept {
    for (auto& node : DImplementation->Nodes) if (node->ID() == id) return node;
    return nullptr;
}
std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByIndex(std::size_t index) const noexcept { return (index < DImplementation->Ways.size()) ? DImplementation->Ways[index] : nullptr; }
std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByID(TWayID id) const noexcept {
    for (auto& way : DImplementation->Ways) if (way->ID() == id) return way;
    return nullptr;
}
