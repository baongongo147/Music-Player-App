# Compiler và flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Tên file thực thi và thư mục source
TARGET = main.exe
SRC_DIR = src
BUILD_DIR = build

# Các file source
SRC = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/controller/*.cpp) $(wildcard $(SRC_DIR)/model/*.cpp) $(wildcard $(SRC_DIR)/view/*.cpp)

# Các file object (.o)
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

# Quy tắc build
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

# Compile từng file .cpp thành .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Chạy chương trình
run: $(TARGET)
	./$(TARGET)

# Dọn dẹp
clean:
	rm -f $(OBJ) $(TARGET)
