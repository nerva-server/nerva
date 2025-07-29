CXX = clang++
CXXFLAGS = -fno-omit-frame-pointer -std=c++20 -Wall -Wextra -O2 -lsimdjson -I./src -I./includes
# -g -fsanitize=address = ASan Debug

SRC_DIR = src
BUILD_DIR = build
BIN = server

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

.PHONY: clean run

run: $(BIN)
	LD_PRELOAD=/usr/lib/libtcmalloc.so.4 ./$(BIN)

clean:
	rm -rf $(BUILD_DIR) $(BIN)
