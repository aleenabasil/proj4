#include "OpenStreetMap.h"
#include "XMLReader.h"
#include "StreetMap.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include "DSVReader.h"

struct COpenStreetMap::SImplementation {
    // forward declaration of node and way implementation classes
    class SNodeImpl;
    class SWayImpl;
    
    // containers for nodes and ways
    std::vector<std::shared_ptr<SNodeImpl>> Nodes;
    std::vector<std::shared_ptr<SWayImpl>> Ways;
};

// node implementation, inheriting from CStreetMap::SNode
class COpenStreetMap::SImplementation::SNodeImpl : public CStreetMap::SNode {
public:
    TNodeID NodeID; // node identifier
    TLocation NodeLocation; // coordinates (latitude, longitude)
    std::unordered_map<std::string, std::string> Attributes; // additional attributes

    TNodeID ID() const noexcept override { // override the ID method
        return NodeID; // return the node identifier
    }

    // override the Location method
    TLocation Location() const noexcept override {
        return NodeLocation; // return the node location
    }

    // override the AttributeCount method
    std::size_t AttributeCount() const noexcept override {
        return Attributes.size(); // return the number of attributes
    }

    // override the GetAttributeKey method
    std::string GetAttributeKey(std::size_t index) const noexcept override {
        if (index < Attributes.size()) { // check if the index is within bounds
            auto it = Attributes.begin(); // get an iterator to the beginning of the map
            std::advance(it, index); // advance the iterator by the index
            return it->first; // return the key at the specified index
        }
        return "";
    }

    // override the HasAttribute method
    bool HasAttribute(const std::string &key) const noexcept override {
        return Attributes.find(key) != Attributes.end(); // check if the key exists in the map
    }

    // override the GetAttribute method
    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = Attributes.find(key); // find the key in the map
        return (it != Attributes.end()) ? it->second : ""; // return the value if found, otherwise an empty string
    }
};

// way implementation, inheriting from CStreetMap::SWay
class COpenStreetMap::SImplementation::SWayImpl : public CStreetMap::SWay {
public:
    TWayID WayID; // way identifier
    std::vector<TNodeID> NodeIDs; // list of node IDs forming the way
    std::unordered_map<std::string, std::string> Attributes; // way attributes

    TWayID ID() const noexcept override { // override the ID method
        return WayID; // return the way identifier
    }

    // override the NodeCount method
    std::size_t NodeCount() const noexcept override {
        return NodeIDs.size(); // return the number of nodes in the way
    }

    // override the GetNodeID method
    TNodeID GetNodeID(std::size_t index) const noexcept override {
        return (index < NodeIDs.size()) ? NodeIDs[index] : CStreetMap::InvalidNodeID; // return the node ID if it exists
    }

    // override the AttributeCount method
    std::size_t AttributeCount() const noexcept override {
        return Attributes.size(); // return the number of attributes
    }

    // override the GetAttributeKey method
    std::string GetAttributeKey(std::size_t index) const noexcept override {
        if (index < Attributes.size()) {
            auto it = Attributes.begin(); // get an iterator to the beginning of the map
            std::advance(it, index); // advance the iterator by the index
            return it->first; // return the key at the specified index
        }
        return "";
    }

    // override the HasAttribute method
    bool HasAttribute(const std::string &key) const noexcept override {
        return Attributes.find(key) != Attributes.end(); // check if the key exists in the map
    }

    // override the GetAttribute method
    std::string GetAttribute(const std::string &key) const noexcept override {
        auto it = Attributes.find(key); // find the key in the map
        return (it != Attributes.end()) ? it->second : ""; // return the value if found, otherwise an empty string
    }
};

// constructor parses XML and populates nodes and ways
COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src) {
    DImplementation = std::make_unique<SImplementation>(); // create the implementation object
    SXMLEntity entity; // create an entity object to store XML data
    std::shared_ptr<SImplementation::SNodeImpl> currentNode = nullptr; // create a pointer to the current node
    std::shared_ptr<SImplementation::SWayImpl> currentWay = nullptr; // create a pointer to the current way
    
    // process XML entities from the provided source
    while (src->ReadEntity(entity)) {
        if (entity.DType == SXMLEntity::EType::StartElement) { // check if the entity is a start element
            if (entity.DNameData == "node") { // check if the element is a node
                currentNode = std::make_shared<SImplementation::SNodeImpl>(); // create a new node
                currentWay.reset(); // reset the current way pointer
                // process node attributes
                for (const auto &attr : entity.DAttributes) { // iterate through the attributes
                    if (attr.first == "id") { // check if the attribute is the node ID
                        currentNode->NodeID = std::stoull(attr.second); // set the node ID
                    } else if (attr.first == "lat") { // check if the attribute is the latitude
                        currentNode->NodeLocation.first = std::stod(attr.second); // set the latitude
                    } else if (attr.first == "lon") { // check if the attribute is the longitude
                        currentNode->NodeLocation.second = std::stod(attr.second); // set the longitude
                    } else { // if the attribute is not the node ID, latitude, or longitude
                        currentNode->Attributes[attr.first] = attr.second; // add the attribute to the node
                    }
                }
            } else if (entity.DNameData == "way") { // check if the element is a way
                currentWay = std::make_shared<SImplementation::SWayImpl>(); // create a new way
                currentNode.reset(); // reset the current node pointer
                // process way attributes
                for (const auto &attr : entity.DAttributes) { // iterate through the attributes
                    if (attr.first == "id") { // check if the attribute is the way ID
                        currentWay->WayID = std::stoull(attr.second); // set the way ID
                    } else {
                        currentWay->Attributes[attr.first] = attr.second; // add the attribute to the way
                    }
                }
            } else if (entity.DNameData == "nd" && currentWay) { // check if the element is a node reference inside a way
                // process node reference inside a way
                for (const auto &attr : entity.DAttributes) { // iterate through the attributes
                    if (attr.first == "ref") { // check if the attribute is the node reference
                        currentWay->NodeIDs.push_back(std::stoull(attr.second)); // add the node reference to the way
                    }
                }
            } else if (entity.DNameData == "tag") {
                // process tag elements for both nodes and ways.
                std::string key, value;
                for (const auto &attr : entity.DAttributes) { // iterate through the attributes
                    if (attr.first == "k") { // check if the attribute is the key
                        key = attr.second; // set the key
                    } else if (attr.first == "v") { // check if the attribute is the value
                        value = attr.second; // set the value
                    }
                }
                // add the key-value pair to the node or way attributes
                if (!key.empty()) {
                    if (currentNode) {
                        currentNode->Attributes[key] = value; // add the attribute to the node
                    } else if (currentWay) {
                        currentWay->Attributes[key] = value; // add the attribute to the way
                    }
                }
            }
        } else if (entity.DType == SXMLEntity::EType::EndElement) { // check if the entity is an end element
            if (entity.DNameData == "node" && currentNode) { // check if the element is a node and the current node is valid
                DImplementation->Nodes.push_back(currentNode); // add the current node to the list of nodes
                currentNode.reset(); // reset the current node pointer
            } else if (entity.DNameData == "way" && currentWay) { // check if the element is a way and the current way is valid
                DImplementation->Ways.push_back(currentWay); // add the current way to the list of ways
                currentWay.reset(); // reset the current way pointer
            }
        }
    }
}

COpenStreetMap::~COpenStreetMap() = default; // default destructor

std::size_t COpenStreetMap::NodeCount() const noexcept { // override the NodeCount method
    return DImplementation->Nodes.size(); // return the number of nodes
}

std::size_t COpenStreetMap::WayCount() const noexcept { // override the WayCount method
    return DImplementation->Ways.size(); // return the number of ways
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(std::size_t index) const noexcept { // override the NodeByIndex method
    if (index < DImplementation->Nodes.size()) { // check if the index is within bounds
        return DImplementation->Nodes[index]; // return the node at the specified inswz
    }
    return nullptr;
}

// override the NodeByID method
std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByID(TNodeID id) const noexcept {
    for (const auto &node : DImplementation->Nodes) { // iterate through the nodes
        if (node->ID() == id) { // check if the node ID matches the specified ID
            return node; // return the node if found
        }
    }
    return nullptr;
}

// override the WayByIndex method
std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByIndex(std::size_t index) const noexcept {
    if (index < DImplementation->Ways.size()) { // check if the index is within bounds
        return DImplementation->Ways[index]; // return the way at the specified index
    }
    return nullptr;
}

// override the WayByID method
std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByID(TWayID id) const noexcept {
    for (const auto &way : DImplementation->Ways) {
        if (way->ID() == id) { // check if the way ID matches the specified ID
            return way; // return the way if found
        }
    }
    return nullptr;
}
