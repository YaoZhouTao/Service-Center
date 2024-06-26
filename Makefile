# 定义编译器
CXX = g++
# 定义编译器标志
CXXFLAGS = -Wall -std=c++11 -I. -I/usr/include/mysql
LDFLAGS = -L/usr/lib/mysql -lmysqlclient -lpthread

# 定义目录
SRC_DIR = .
OBJ_DIR = obj
BIN_DIR = bin

# 定义源文件和头文件
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# 定义目标文件
TARGET = $(BIN_DIR)/Begin.out

# 生成目标文件的规则
all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 创建目录规则
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# 清理规则
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

