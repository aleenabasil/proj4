#include "DSVReader.h"
#include <sstream>
#include <iostream>

// this structure handles reading delimiter-separated values from a data source
struct CDSVReader::SImplementation {
    std::shared_ptr<CDataSource> Source;  // holds our data source
    char Delimiter; // the character that splits the data into columns
    
    // constructor sets up the data source and the delimiter
    SImplementation(std::shared_ptr<CDataSource> src, char delimiter)
        : Source(std::move(src)), Delimiter(delimiter) {}
    
    // reads a row of data, splitting it by delimiter and handling quotes
    bool ReadRow(std::vector<std::string> &row) {
        row.clear(); // start with a fresh row
        std::string right; // collects the characters between delimiters
        char c; // the current character being read
        bool quotes = false; // inside quoted text
        bool data = false; // read any data
        
        while (!Source->End()) {
            if (!Source->Get(c)) return false; // if we can't read anymore, we are done
            data = true;
            
            if (c == '"') { // handle quotes
                char next;
                if (!Source->End() && Source->Peek(next)) {
                    if (next == '"') { // two quotes in a row means add one quote to the data
                        Source->Get(next);
                        right += '"';
                    } else {
                        quotes = !quotes; // flip  quote boool
                    }
                } else {
                    quotes = !quotes; // flip  quote bool
                }
            } else if (c == Delimiter && !quotes) {
                row.push_back(std::move(right)); // end of a column
                right.clear();
            } else if ((c == '\n' || c == '\r') && !quotes) {
                if (!right.empty() || !row.empty()) {
                    row.push_back(std::move(right)); // end of a row
                }
                
                if (c == '\r' && !Source->End()) {  // handle windows line endings
                    char next;
                    if (Source->Peek(next) && next == '\n') {
                        Source->Get(next);
                    }
                }
                return true; // we read a full row
            } else {
                right += c; // just another character in the current column
            }
        }
        
        if (!right.empty() || !row.empty()) {
            row.push_back(std::move(right)); // make sure to capture the last column
        }
        return data; // return whether we read any data at all
    }
};

// constructor for initializing the DSV reader with a source and delimiter
CDSVReader::CDSVReader(std::shared_ptr<CDataSource> src, char delimiter)
    : DImplementation(std::make_unique<SImplementation>(src, delimiter)) {}

// simple destructor
CDSVReader::~CDSVReader() = default;

// checks if all data has been read
bool CDSVReader::End() const {
    return DImplementation->Source->End();
}

// tries to read a row into the provided vector, each element represents a column
bool CDSVReader::ReadRow(std::vector<std::string> &row) {
    return DImplementation->ReadRow(row);
}
