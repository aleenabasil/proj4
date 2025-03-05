#include "CSVBusSystem.h"
#include "StringDataSource.h"
#include "DSVReader.h"
#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <memory>


// Mock DSVReader class to read data from a CSV-like structure
// This class will mock the fucnction of reading rows from a CSV file
class MockDSVReader : public CDSVReader {
private:
    size_t CurrentRow;
    std::vector<std::vector<std::string>> MockData;
    
public:
    MockDSVReader(const std::vector<std::vector<std::string>>& data) 
        : CDSVReader(std::make_shared<CStringDataSource>(""), ','), 
          CurrentRow(0), MockData(data) {}
    
    bool ReadRow(std::vector<std::string>& row) {
        if (CurrentRow < MockData.size()) {
            row = MockData[CurrentRow++];
            return true;
        }
        return false;
    }
    
    void Reset() { CurrentRow = 0; }
};

// Unit tests for the CCSVBusSystem class
class CSVBusSystemTest : public ::testing::Test {
protected:
    std::vector<std::vector<std::string>> TestStopData = {
        {"1", "100"},
        {"2", "200"}
    };
    
    std::vector<std::vector<std::string>> TestRouteData = {
        {"Route1", "1"},
        {"Route1", "2"},
        {"Route2", "2"},
        {"Route2", "1"}
    };
};

// Test case to check the number of stops and their access by index and ID
TEST_F(CSVBusSystemTest, StopCountAndAccess) {
    auto stopReader = std::make_shared<MockDSVReader>(TestStopData);
    auto routeReader = std::make_shared<MockDSVReader>(TestRouteData);
    CCSVBusSystem busSystem(stopReader, routeReader);
    
    EXPECT_EQ(busSystem.StopCount(), 2);
    
    auto stop1 = busSystem.StopByIndex(0);
    ASSERT_NE(stop1, nullptr);
    EXPECT_EQ(stop1->ID(), 1);
    
    auto stop2 = busSystem.StopByIndex(1);
    ASSERT_NE(stop2, nullptr);
    EXPECT_EQ(stop2->ID(), 2);
    
    EXPECT_EQ(busSystem.StopByID(1)->NodeID(), 100);
    EXPECT_EQ(busSystem.StopByID(2)->NodeID(), 200);
    EXPECT_EQ(busSystem.StopByIndex(2), nullptr);
    EXPECT_EQ(busSystem.StopByID(3), nullptr);
}

// Test case to check the number of routes and their access by index and name
TEST_F(CSVBusSystemTest, RouteCountAndAccess) {
    auto stopReader = std::make_shared<MockDSVReader>(TestStopData);
    auto routeReader = std::make_shared<MockDSVReader>(TestRouteData);
    CCSVBusSystem busSystem(stopReader, routeReader);
    
    EXPECT_EQ(busSystem.RouteCount(), 2);
    
    auto route1 = busSystem.RouteByIndex(0);
    ASSERT_NE(route1, nullptr);
    EXPECT_EQ(route1->Name(), "Route1");
    EXPECT_EQ(route1->StopCount(), 2);
    EXPECT_EQ(route1->GetStopID(0), 1);
    EXPECT_EQ(route1->GetStopID(1), 2);
    
    auto route2 = busSystem.RouteByIndex(1);
    ASSERT_NE(route2, nullptr);
    EXPECT_EQ(route2->Name(), "Route2");
    EXPECT_EQ(route2->StopCount(), 2);
    EXPECT_EQ(route2->GetStopID(0), 2);
    EXPECT_EQ(route2->GetStopID(1), 1);
    
    EXPECT_EQ(busSystem.RouteByName("Route1")->StopCount(), 2);
    EXPECT_EQ(busSystem.RouteByName("Route2")->StopCount(), 2);
    EXPECT_EQ(busSystem.RouteByIndex(2), nullptr);
    EXPECT_EQ(busSystem.RouteByName("InvalidRoute"), nullptr);
}

// Test case to check behavior when no data is provided 
TEST_F(CSVBusSystemTest, EmptyData) {
    auto emptyStopReader = std::make_shared<MockDSVReader>(std::vector<std::vector<std::string>>());
    auto emptyRouteReader = std::make_shared<MockDSVReader>(std::vector<std::vector<std::string>>());
    CCSVBusSystem busSystem(emptyStopReader, emptyRouteReader);
    
    EXPECT_EQ(busSystem.StopCount(), 0);
    EXPECT_EQ(busSystem.RouteCount(), 0);
    EXPECT_EQ(busSystem.StopByIndex(0), nullptr);
    EXPECT_EQ(busSystem.StopByID(1), nullptr);
    EXPECT_EQ(busSystem.RouteByIndex(0), nullptr);
    EXPECT_EQ(busSystem.RouteByName("Route1"), nullptr);
}
