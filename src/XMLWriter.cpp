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
                    case '<':  // less than sign
                        if (!WriteText("&lt;", false)) {
                            return false; 
                            break;
                        }
                    case '>':  // greater than sign
                        if (!WriteText("&gt;", false)) {
                            return false; 
                            break;
                        }
                    case '&':
                        if (!WriteText("&amp;", false)) {
                        return false; 
                        break;
                        }
                    case '\'': // single quote
                        if (!WriteText("&apos;", false)) {
                            return false; 
                            break;
                        }
                    case '"':  // double quote
                        if (!WriteText("&quot;", false)) { 
                            return false; 
                            break;
                        }
                    default:   // all other characters
                        if (!Sink->Put(c)) {
                            return false;
                        }
                }
            } else { // no need to escape
                if (!Sink->Put(c)) {
                    return false;
                }
            }
        }
        return true;
    }

    // closes all open xml elements ensuring proper xml structure before ending the document
    bool Flush() {
        while (!Stack.empty()) { // close all open tags
            if (!WriteText("</" + Stack.top() + ">", false)) { // close the tag
                return false;
            }
            Stack.pop(); // remove the tag from the stack
        }
        return true;
    }

    // writes an xml entity based on its type (tag, data, or self-closing element)
    bool WriteEntity(const SXMLEntity &entity) {
        switch (entity.DType) { // determine the type of entity
            // handle opening tags
            case SXMLEntity::EType::StartElement: // start tag
                if (!WriteText("<" + entity.DNameData, false)) { // write the tag name
                    return false;
                }
                // write the attributes
                for (const auto &attr : entity.DAttributes) {
                    if (!WriteText(" " + attr.first + "=\"", false)) { // write the attribute name
                        return false;
                    }
                    if (!WriteText(attr.second, true)) { // write the attribute value
                        return false;
                    }  
                    if (!WriteText("\"", false)) { // close the attribute value
                        return false;
                    }
                }

                if (!WriteText(">", false)) { // close the tag
                    return false;
                }
                Stack.push(entity.DNameData);  // remember this tag to close it later
                break;

            // handle closing tags
            case SXMLEntity::EType::EndElement:
                if (!WriteText("</" + entity.DNameData + ">", false)) { // close the tag
                    return false;
                }
                if (!Stack.empty()) { // remove the tag from the stack
                    Stack.pop(); // remove the tag from the stack
                }
                break;

            // handle character data within tags
            case SXMLEntity::EType::CharData:
                if (!WriteText(entity.DNameData, true)) { // write the character data
                    return false; // return false if writing fails
                }
                break;

            // handle self-closing tags
            case SXMLEntity::EType::CompleteElement:
                if (!WriteText("<" + entity.DNameData, false)) { // write the tag name
                    return false;
                }

                for (const auto &attr : entity.DAttributes) { // write the attributes
                    if (!WriteText(" " + attr.first + "=\"", false)) { // write the attribute name
                        return false;
                    }
                    if (!WriteText(attr.second, true)) { // write the attribute value
                        return false;  
                    }
                    if (!WriteText("\"", false)) { // close the attribute value
                        return false;
                    }
                }

                if (!WriteText("/>", false)) { // close the tag
                    return false;
                }
                break;
        }
        return true; 
    }
};

// sets up the xml writer with a specified data destination
CXMLWriter::CXMLWriter(std::shared_ptr<CDataSink> sink) : DImplementation(std::make_unique<SImplementation>(std::move(sink))) {}

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


