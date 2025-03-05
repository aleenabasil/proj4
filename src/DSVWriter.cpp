#include "DSVWriter.h"
#include "DataSink.h"

// implementation structure for CDSVWriter, which handles writing to a data sink
struct CDSVWriter::SImplementation {
    std::shared_ptr<CDataSink> Sink; // data sink for writing
    char Delimiter; // character used as delimiter
    bool QuoteAll; // determines if all fields should be quoted

    // constructor for SImplementation, initializes the data sink, delimiter, and quote
    SImplementation(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall)
        : Sink(sink), Delimiter(delimiter), QuoteAll(quoteall) {}

    // writes a row of data to the sink
    bool WriteRow(const std::vector<std::string>& row) {
        // sink is valid
        if (!Sink) return false; 
        // write only a newline for an empty row
        if (row.empty()) {
            return Sink->Put('\n');
        }
        // iterate over each field in the row
        for (size_t i = 0; i < row.size(); ++i) {
            bool quote = QuoteAll || row[i].find(Delimiter) != std::string::npos ||
                              row[i].find('"') != std::string::npos || row[i].find('\n') != std::string::npos;
            
            if (quote) {
                // start quoted field
                if (!Sink->Put('"')) return false; 
                // iterate over each character in the field
                for (char c : row[i]) {
                    if (c == '"') {
                        // escape double quotes by doubling them
                        if (!Sink->Put('"') || !Sink->Put('"')) return false;
                    } else {
                        // write the character as is
                        if (!Sink->Put(c)) return false;
                    }
                }
                // end quoted field
                if (!Sink->Put('"')) return false; 
            // if no quoting is needed, write the field character by character
            } else {
                for (char c : row[i]) {
                    if (!Sink->Put(c)) return false;
                }
            }
            // add delimiter between fields, but not after the last field
            if (i < row.size() - 1) {
                if (!Sink->Put(Delimiter)) return false; 
            }
        }
        // end the row with a newline character
        return Sink->Put('\n'); 
    }
};
// constructor for DSV writer, sink specifies the data destination, delimiter
// specifies the delimiting character, and quoteall specifies if all values
// should be quoted or only those that contain the delimiter, a double quote,
// or a newline
CDSVWriter::CDSVWriter(std::shared_ptr<CDataSink> sink, char delimiter, bool quoteall)
    : DImplementation(std::make_unique<SImplementation>(sink, delimiter, quoteall)) {}

// destructor for CDSVWriter
CDSVWriter::~CDSVWriter() = default;

// returns true if the row is successfully written, one string per column
// should be put in the row vector
bool CDSVWriter::WriteRow(const std::vector<std::string>& row) {
    return DImplementation->WriteRow(row);
}
