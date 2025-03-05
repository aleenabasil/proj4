#include "XMLWriter.h"
#include <stack>
#include <string>

// internal implementation of the CXMLWriter using a stack to manage open XML elements
struct CXMLWriter::SImplementation {
    std::shared_ptr<CDataSink> Sink;  // destination for XML output
    std::stack<std::string> Stack;    // stack to manage the tags for proper nesting and closure

    // constructor that takes a data sink
    explicit SImplementation(std::shared_ptr<CDataSink> sink) 
        : Sink(std::move(sink)) {}

    // writes a string to the output possibly escaping XML special characters
    bool WriteText(const std::string &str, bool escape) {
        for (char c : str) {
            if (escape) {
                // escaping XML special characters to prevent malformation
                switch (c) {
                    case '<':  if (!WriteText("&lt;", false)) return false; break;
                    case '>':  if (!WriteText("&gt;", false)) return false; break;
                    case '&':  if (!WriteText("&amp;", false)) return false; break;
                    case '\'': if (!WriteText("&apos;", false)) return false; break;
                    case '"':  if (!WriteText("&quot;", false)) return false; break;
                    default:   if (!Sink->Put(c)) return false;
                }
            } else {
                if (!Sink->Put(c)) return false;
            }
        }
        return true;
    }

    // closes all open xml elements ensuring proper xml structure before ending the document
    bool Flush() {
        while (!Stack.empty()) {
            if (!WriteText("</" + Stack.top() + ">", false)) {
                return false;
            }
            Stack.pop();
        }
        return true;
    }

    // writes an xml entity based on its type (tag, data, or self-closing element)
    bool WriteEntity(const SXMLEntity &entity) {
        switch (entity.DType) {
            // handle opening tags
            case SXMLEntity::EType::StartElement:
                if (!WriteText("<" + entity.DNameData, false)) return false;

                // write attributes if any
                for (const auto &attr : entity.DAttributes) {
                    if (!WriteText(" " + attr.first + "=\"", false)) return false;
                    if (!WriteText(attr.second, true)) return false;  
                    if (!WriteText("\"", false)) return false;
                }

                if (!WriteText(">", false)) return false;
                Stack.push(entity.DNameData);  // remember this tag to close it later
                break;

            // handle closing tags
            case SXMLEntity::EType::EndElement:
                if (!WriteText("</" + entity.DNameData + ">", false)) return false;
                if (!Stack.empty()) {
                    Stack.pop();
                }
                break;

            // handle character data within tags
            case SXMLEntity::EType::CharData:
                if (!WriteText(entity.DNameData, true)) return false;  
                break;

            // handle self-closing tags
            case SXMLEntity::EType::CompleteElement:
                if (!WriteText("<" + entity.DNameData, false)) return false;

                for (const auto &attr : entity.DAttributes) {
                    if (!WriteText(" " + attr.first + "=\"", false)) return false;
                    if (!WriteText(attr.second, true)) return false;  
                    if (!WriteText("\"", false)) return false;
                }

                if (!WriteText("/>", false)) return false;
                break;
        }
        return true;
    }
};

// sets up the xml writer with a specified data destination
CXMLWriter::CXMLWriter(std::shared_ptr<CDataSink> sink)
    : DImplementation(std::make_unique<SImplementation>(std::move(sink))) {}

// destructor ensures resources are cleaned up properly
CXMLWriter::~CXMLWriter() = default;

// flush method to make sure all opened tags are closed
bool CXMLWriter::Flush() {
    return DImplementation->Flush();
}

// writes an xml entity to the output based on the provided description and attributes
bool CXMLWriter::WriteEntity(const SXMLEntity &entity) {
    return DImplementation->WriteEntity(entity);
}

