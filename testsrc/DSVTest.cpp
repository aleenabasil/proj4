#include "DSVReader.h"
#include "DSVWriter.h"
#include "StringDataSource.h"
#include "StringDataSink.h"
#include <gtest/gtest.h>

TEST(DSVTest, BasicReadWrite) {
    // initialize a shared pointer for data w/ DSV content
    std::shared_ptr<CStringDataSource> src = std::make_shared<CStringDataSource>("hello,anikaandaleena,hi\na,b,c\n");
    // initialize a shared pointer for data sink to see output
    std::shared_ptr<CStringDataSink> sink = std::make_shared<CStringDataSink>();

    // create a DSV reader w/ data and specify delimiter
    CDSVReader reader(src, ',');
    // create a DSV writer w/ data sink and specify delimiter
    CDSVWriter writer(sink, ',');

    // vector to hold each row of data read from source
    std::vector<std::string> row;
    // continue reading rows until there are no more to read
    while (reader.ReadRow(row)) {
        // for each row read and write it to the sink
        writer.WriteRow(row);
    }

    // assert that string output from sink matches expected DSV content
    EXPECT_EQ(sink->String(), "hello,anikaandaleena,hi\na,b,c\n");
}

