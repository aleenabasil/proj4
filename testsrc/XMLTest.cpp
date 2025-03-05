#include <gtest/gtest.h> 
#include "XMLReader.h"
#include "XMLWriter.h"
#include "StringDataSource.h"
#include "StringDataSink.h"

TEST(XMLTest, BasicReadWrite) {
    // create a data source from a string containing XML data
    std::shared_ptr<CStringDataSource> src = std::make_shared<CStringDataSource>("<tag>data</tag>");

    // create a data sink to capture output from the XML writer
    std::shared_ptr<CStringDataSink> sink = std::make_shared<CStringDataSink>();

    // initialize XML reader w/ the string data
    CXMLReader reader(src);

    // initialize XML writer w/ the string data sink
    CXMLWriter writer(sink);

    // define an XML entity to store data read from the XML reader
    SXMLEntity entity;

    // read from the XML reader until the end of data is reached
    while (!reader.End()) {
        // attempt to read entity from XML stream
        if (reader.ReadEntity(entity)) {
            // if an entity is successfully read, write to the XML writer
            writer.WriteEntity(entity);
        }
    }

    // After all entities are processed, check if the output matches the original input
    EXPECT_EQ(sink->String(), "<tag>data</tag>");
}
