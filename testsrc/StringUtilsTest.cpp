#include <gtest/gtest.h>
#include "StringUtils.h"

TEST(StringUtilsTest, SliceTest){
    ASSERT_EQ(StringUtils::Slice("hello", 0, 5), "hello");
    ASSERT_EQ(StringUtils::Slice("hello", 1, 3), "ell");
    ASSERT_EQ(StringUtils::Slice("hello", 0, 0), "");
}

TEST(StringUtilsTest, Capitalize){
    ASSERT_EQ(StringUtils::Capitalize("hello anika"), "Hello anika");
    ASSERT_EQ(StringUtils::Capitalize("anika"), "Anika");
}

TEST(StringUtilsTest, Upper){
    ASSERT_EQ(StringUtils::Upper("anika loves cs"), "ANIKA LOVES CS");
    ASSERT_EQ(StringUtils::Upper("i love c"), "I LOVE C");
}

TEST(StringUtilsTest, Lower){
    ASSERT_EQ(StringUtils::Lower("ANIKA LOVES CS"), "anika loves cs");
    ASSERT_EQ(StringUtils::Lower("I LOVE C"), "i love c");
}

TEST(StringUtilsTest, LStrip){
    ASSERT_EQ(StringUtils::LStrip("   anika"), "anika");
    ASSERT_EQ(StringUtils::LStrip("    anika    "), "anika    ");
}

TEST(StringUtilsTest, RStrip){
    ASSERT_EQ(StringUtils::RStrip("anika   "), "anika");
    ASSERT_EQ(StringUtils::RStrip("    anika    "), "    anika");
}

TEST(StringUtilsTest, Strip){
    ASSERT_EQ(StringUtils::Strip("   anika   "), "anika");
    ASSERT_EQ(StringUtils::Strip("anika"), "anika");
}

TEST(StringUtilsTest, Center){
    ASSERT_EQ(StringUtils::Center("anika", 10, ' '), "  anika   ");
    ASSERT_EQ(StringUtils::Center("anika", 5, ' '), "anika");
}

TEST(StringUtilsTest, LJust){
    ASSERT_EQ(StringUtils::LJust("anika", 10, ' '), "anika     ");
    ASSERT_EQ(StringUtils::LJust("anika", 5, ' '), "anika");
}

TEST(StringUtilsTest, RJust){
    ASSERT_EQ(StringUtils::RJust("anika", 10, ' '), "     anika");
    ASSERT_EQ(StringUtils::RJust("anika", 5, ' '), "anika");
}

TEST(StringUtilsTest, Replace){
    ASSERT_EQ(StringUtils::Replace("anika loves cs", "loves", "hates"), "anika hates cs");
    ASSERT_EQ(StringUtils::Replace("anika loves cs", "loves", ""), "anika  cs");
}

TEST(StringUtilsTest, Split){
    ASSERT_EQ(StringUtils::Split("anika loves cs"), (std::vector<std::string>{"anika", "loves", "cs"}));
    ASSERT_EQ(StringUtils::Split("i,hate,cs", ","), (std::vector<std::string>{"i", "hate", "cs"}));
}

TEST(StringUtilsTest, Join){
    ASSERT_EQ(StringUtils::Join(" ", std::vector<std::string>{"anika", "loves", "cs"}), "anika loves cs");
    ASSERT_EQ(StringUtils::Join(",", std::vector<std::string>{"i", "hate", "cs"}), "i,hate,cs");
}

TEST(StringUtilsTest, ExpandTabs){
    ASSERT_EQ(StringUtils::ExpandTabs("anika\tloves\tcs", 4), "anika   loves   cs");
    ASSERT_EQ(StringUtils::ExpandTabs("anika\tbhandarkar", 4), "anika   bhandarkar");
}

TEST(StringUtilsTest, EditDistance){
    ASSERT_EQ(StringUtils::EditDistance("anika", "anika"), 0);
    ASSERT_EQ(StringUtils::EditDistance("anika", "anik"), 1);
}
