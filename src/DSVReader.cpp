#include "DSVReader.h"
#include <sstream>
#include <iostream>

// handles parsing delimiter-separated values from a data source
struct CDSVReader::SImplementation {
    std::shared_ptr<CDataSource> InputSource;  // data input stream
    char ColumnSeparator; // character that separates columns

    // constructor initializes source and delimiter
    SImplementation(std::shared_ptr<CDataSource> src, char delimiter)
        : InputSource(std::move(src)), ColumnSeparator(delimiter) {}

    // helper function to handle quoted text logic
    bool ProcessQuotes(std::string &columnData, bool &insideQuotes) {
        char nextChar;
        if (!InputSource->End() && InputSource->Peek(nextChar)) {
            if (nextChar == '"') {  
                InputSource->Get(nextChar); // consume one quote, keep one
                columnData += '"';
                return false;
            }
        }
        insideQuotes = !insideQuotes; // toggle state
        return true;
    }

    // reads a single row of data from the input source
    bool ReadRow(std::vector<std::string> &row) {
        row.clear();
        std::string columnData;
        bool insideQuotes = false, dataRead = false;

        for (;;) {  // infinite loop, breaks when row is complete
            char currentChar;
            if (InputSource->End() || !InputSource->Get(currentChar)) break;
            dataRead = true;

            if (currentChar == '"') {
                if (ProcessQuotes(columnData, insideQuotes)) continue;
            } 
            else if (currentChar == ColumnSeparator && !insideQuotes) {
                row.push_back(std::move(columnData));
                columnData.clear();
            } 
            else if ((currentChar == '\n' || currentChar == '\r') && !insideQuotes) {
                if (!columnData.empty() || !row.empty()) {
                    row.push_back(std::move(columnData));
                }
                if (currentChar == '\r' && !InputSource->End()) {
                    char nextChar;
                    if (InputSource->Peek(nextChar) && nextChar == '\n') {
                        InputSource->Get(nextChar);
                    }
                }
                return true;  // row completed
            } 
            else {
                columnData += currentChar;
            }
        }

        if (!columnData.empty() || !row.empty()) {
            row.push_back(std::move(columnData));
        }
        return dataRead;
    }
};

// constructor initializes the DSV reader with a source and delimiter
CDSVReader::CDSVReader(std::shared_ptr<CDataSource> src, char delimiter)
    : DImplementation(std::make_unique<SImplementation>(std::move(src), delimiter)) {}

// destructor automatically cleans up unique_ptr
CDSVReader::~CDSVReader() = default;

// checks if all data has been read
bool CDSVReader::End() const {
    return DImplementation->InputSource->End();
}

// reads a row into the provided vector
bool CDSVReader::ReadRow(std::vector<std::string> &row) {
    return DImplementation->ReadRow(row);
}

