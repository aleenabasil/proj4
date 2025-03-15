#include "DSVWriter.h"
#include "DataSink.h"

// implementation structure for CDSVWriter, which handles writing to a data sink
struct CDSVWriter::SImplementation {
    // data sink for writing
    std::shared_ptr<CDataSink> Sink; 
    // character used as delimiter
    char Delimiter; 
    // determines if all fields should be quoted                  
    bool QuoteAll;                     

   
   // constructor for SImplementation, initializes the data sink, delimiter, and quot
    SImplementation(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteAllFields)
        : Sink(std::move(sink)), Delimiter(delimiter), QuoteAll(quoteAllFields) {}

    // writes a row of data to the sink
    bool WriteRow(const std::vector<std::string>& row) {
        // sink is valid
        if (!Sink) {
            return false;  }
         // Write just a newline for empty rows
        if (row.empty()) {
            return Sink->Put('\n'); }

        // writes a row of data to the sink
        for (size_t i = 0; i < row.size(); ++i) {
            bool quotes = QuoteAll || row[i].find(Delimiter) != std::string::npos ||
                               row[i].find('"') != std::string::npos || row[i].find('\n') != std::string::npos;

            if (quotes) {
                if (!Sink->Put('"')){
                    return false; } 

                for (char ch : row[i]) {
                    if (ch == '"') {
                        // Escape double quotes by duplicating them
                        if (!Sink->Put('"') || !Sink->Put('"')) {
                            return false;}
                    } else {
                        if (!Sink->Put(ch)) {
                            return false;}
                    }
                }
                // end quoted field
                if (!Sink->Put('"')) {
                    return false;  }
                // if no quoting is needed, write the field character by character
            } else {
                // Write the field as is
                for (char ch : row[i]) {
                    if (!Sink->Put(ch)) {
                        return false;}
                }
            }

            // add delimiter except for last
            if (i < row.size() - 1) {
                if (!Sink->Put(Delimiter)) {
                    return false;}
            }
        }

        return Sink->Put('\n');  // End the row with a newline
    }
};

// constructor for DSV writer, sink specifies the data destination, delimiter
// specifies the delimiting character, and quoteall specifies if all values
// should be quoted or only those that contain the delimiter, a double quote,
// or a newline
CDSVWriter::CDSVWriter(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteAll)
    : DImplementation(std::make_unique<SImplementation>(std::move(sink), delimiter, quoteAll)) {}

// destructor for CDSVWriter
CDSVWriter::~CDSVWriter() = default;


// returns true if the row is successfully written, one string per column
// should be put in the row vector
bool CDSVWriter::WriteRow(const std::vector<std::string>& row) {
    return DImplementation->WriteRow(row);
}
