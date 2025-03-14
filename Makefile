CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
LDFLAGS = -lgtest_main -lgtest -pthread -lexpat


SRC_DIR = src
TEST_DIR = testsrc
OBJ_DIR = obj
BIN_DIR = bin


SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
TEST_FILES = $(wildcard $(TEST_DIR)/*.cpp)


OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/src_%.o,$(SRC_FILES))
TEST_OBJ_FILES = $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/test_%.o,$(TEST_FILES))


GTEST_TARGET = $(BIN_DIR)/runtests


all: $(GTEST_TARGET)


$(GTEST_TARGET): $(OBJ_FILES) $(TEST_OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)


$(OBJ_DIR)/src_%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(OBJ_DIR)/test_%.o: $(TEST_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)


clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)


test: all
	./$(GTEST_TARGET)


.PHONY: all clean test
