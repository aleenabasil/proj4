#include "XMLWriter.h"
#include <vector>
#include <string>

// internal structure to handle XML writing
struct CXMLWriter::SImplementation {
    std::shared_ptr<CDataSink> OutputSink;  // Output stream for XML data
    std::vector<std::string> OpenElements;  // Stack to track open elements

    // constructor initializes the output sink
    explicit SImplementation(std::shared_ptr<CDataSink> sink) 
        : OutputSink(std::move(sink)) {}

    // helper function to write raw text to the output sink
    bool Write(const std::string &text) {
        for (char ch : text) {
            if (!OutputSink->Put(ch)) {
                return false;
            }
        }
        return true;
    }

    // writes text while escaping XML special characters
    bool WriteEscapedText(const std::string &text) {
        for (char ch : text) {
            switch (ch) {
                case '<':  if (!Write("&lt;")) return false; break;
                case '>':  if (!Write("&gt;")) return false; break;
                case '&':  if (!Write("&amp;")) return false; break;
                case '\'': if (!Write("&apos;")) return false; break;
                case '"':  if (!Write("&quot;")) return false; break;
                default:
                    if (!OutputSink->Put(ch)) return false;
            }
        }
        return true;
    }

    // ensures all open XML elements are closed properly
    bool CloseAllOpenElements() {
        while (!OpenElements.empty()) {
            if (!Write("</") ||
                !Write(OpenElements.back()) ||
                !Write(">")) {
                return false;
            }
            OpenElements.pop_back();
        }
        return true;
    }

    // writes an XML entity based on its type (tag, text, or self-closing tag)
    bool WriteXMLComponent(const SXMLEntity &entity) {
        switch (entity.DType) {
            case SXMLEntity::EType::StartElement:
                if (!Write("<") ||
                    !Write(entity.DNameData)) {
                    return false;
                }
                for (const auto &attr : entity.DAttributes) {
                    if (!Write(" ") ||
                        !Write(attr.first) ||
                        !Write("=\"") ||
                        !WriteEscapedText(attr.second) ||
                        !Write("\"")) {
                        return false;
                    }
                }
                if (!Write(">")) return false;
                OpenElements.push_back(entity.DNameData);
                break;

            case SXMLEntity::EType::EndElement:
                if (!Write("</") ||
                    !Write(entity.DNameData) ||
                    !Write(">")) {
                    return false;
                }
                if (!OpenElements.empty()) {
                    OpenElements.pop_back();
                }
                break;

            case SXMLEntity::EType::CharData:
                if (!WriteEscapedText(entity.DNameData)) return false;
                break;

            case SXMLEntity::EType::CompleteElement:
                if (!Write("<") ||
                    !Write(entity.DNameData)) {
                    return false;
                }
                for (const auto &attr : entity.DAttributes) {
                    if (!Write(" ") ||
                        !Write(attr.first) ||
                        !Write("=\"") ||
                        !WriteEscapedText(attr.second) ||
                        !Write("\"")) {
                        return false;
                    }
                }
                if (!Write("/>")) return false;
                break;
        }
        return true;
    }
};

// constructor for XML writer
CXMLWriter::CXMLWriter(std::shared_ptr<CDataSink> sink)
    : DImplementation(std::make_unique<SImplementation>(std::move(sink))) {}

// destructor
CXMLWriter::~CXMLWriter() = default;

// ensures all remaining open elements are closed before finalizing output
bool CXMLWriter::Flush() {
    return DImplementation->CloseAllOpenElements();
}

// writes an XML entity to the output
bool CXMLWriter::WriteEntity(const SXMLEntity &entity) {
    return DImplementation->WriteXMLComponent(entity);
}
