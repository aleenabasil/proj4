#include "XMLReader.h"
#include <expat.h>
#include <queue>
#include <memory>
#include <vector>

struct CXMLReader::SImplementation {
    std::shared_ptr<CDataSource> Source;  // source for XML data stream
    XML_Parser Parser; // parser object from the Expat library
    std::queue<SXMLEntity> Queue; // queue to hold parsed XML entities
    bool Data; // flag to check if data parsing is complete
    std::string Buffer; // buffer to accumulate text data between XML tags

    // handles both start and end element events in one unified function
    static void ElementHandler(void *userData, const char *name, const char **element, bool isStart) {
        auto *impl = static_cast<SImplementation *>(userData);
        impl->FlushCharData();  // flush out any accumulated character data

        SXMLEntity entity;
        entity.DType = isStart ? SXMLEntity::EType::StartElement : SXMLEntity::EType::EndElement;
        entity.DNameData = name;

        // if it is a start element and it has attributes, parse them
        if (isStart && element) {
            for (int i = 0; element[i] != nullptr; i += 2) {
                if (element[i + 1] != nullptr) {
                    entity.DAttributes.emplace_back(element[i], element[i + 1]);
                }
            }
        }

        impl->Queue.push(entity);  // add the constructed entity to the queue
    }

    // wrapper to handle the start of an XML element
    static void StartElementHandler(void *userData, const char *name, const char **element) {
        ElementHandler(userData, name, element, true);
    }

    // wrapper to handle the end of an XML element
    static void EndElementHandler(void *userData, const char *name) {
        ElementHandler(userData, name, nullptr, false);
    }

    // processes character data found within XML elements
    static void CharDataHandler(void *userData, const char *j, int len) {
        if (j && len > 0) {
            static_cast<SImplementation *>(userData)->Buffer.append(j, len);  // Append text to the buffer.
        }
    }

    // constructor sets up the parser and registers handlers for parsing events
    SImplementation(std::shared_ptr<CDataSource> src) : Source(std::move(src)), Data(false) {
        Parser = XML_ParserCreate(nullptr);
        XML_SetUserData(Parser, this);
        XML_SetElementHandler(Parser, StartElementHandler, EndElementHandler);
        XML_SetCharacterDataHandler(Parser, CharDataHandler);
    }

    // destructor cleans up the parser to prevent memory leaks
    ~SImplementation() {
        XML_ParserFree(Parser);
    }

    // flushes accumulated character data into the queue as an entity
    void FlushCharData() {
        if (!Buffer.empty()) {
            Queue.push({SXMLEntity::EType::CharData, Buffer, {}});
            Buffer.clear();  // Clear the buffer for new data.
        }
    }

    // reads and parses XML data from the source, processing entities into the queue
    bool ReadEntity(SXMLEntity &entity, bool skipcdata) {
        while (Queue.empty() && !Data) {
            std::vector<char> buffer(4096);
            size_t length = 0;
            while (length < buffer.size() && !Source->End()) {
                char ch;
                if (Source->Get(ch)) {
                    buffer[length++] = ch;  // fill buffer with data from the source
                } else {
                    break;
                }
            }

            if (length == 0) {  // no more data to read indicates the end of the data source
                Data = true;
                XML_Parse(Parser, nullptr, 0, 1);  // signal the parser that parsing is complete
                break;
            }

            if (XML_Parse(Parser, buffer.data(), length, 0) == XML_STATUS_ERROR) {
                return false;  // handle parsing errors
            }
        }

        if (!Queue.empty()) {
            entity = Queue.front();
            Queue.pop();
            return !(skipcdata && entity.DType == SXMLEntity::EType::CharData) || ReadEntity(entity, skipcdata);
        }

        return false;  // return false if no more entities are available
    }
};

// interface for creating an XML reader with a specific data source
CXMLReader::CXMLReader(std::shared_ptr<CDataSource> src)
    : DImplementation(std::make_unique<SImplementation>(std::move(src))) {}

CXMLReader::~CXMLReader() = default; // destructor is straightforward because the unique_ptr takes care of cleanup

// returns true if all data has been parsed and the entity queue is empty
bool CXMLReader::End() const {
    return DImplementation->Data && DImplementation->Queue.empty();
}

// method to read an XML entity, with an option to skip character data
bool CXMLReader::ReadEntity(SXMLEntity &entity, bool skipcdata) {
    return DImplementation->ReadEntity(entity, skipcdata);
}

