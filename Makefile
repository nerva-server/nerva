CXX = clang++
CXXFLAGS = -fno-omit-frame-pointer -std=c++20 -Wall -Wextra -O2 -I./src -I./includes
LDFLAGS = -lsimdjson

SRC_DIR = src
BUILD_DIR = build
BIN = server

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

LIB_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/lib/%.o,$(SRCS))
LIB_NAME = $(BUILD_DIR)/lib/nerva.so

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean run lib install

run: $(BIN)
	LD_PRELOAD=/usr/lib/libtcmalloc.so.4 ./$(BIN)

clean:
	rm -rf $(BUILD_DIR) $(BIN) $(LIB_NAME)

$(BUILD_DIR)/lib/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

lib: $(LIB_OBJS)
	$(CXX) -shared -o $(LIB_NAME) $^ $(LDFLAGS)

install: lib
	mkdir -p /usr/local/lib
	cp $(LIB_NAME) /usr/local/lib/
	mkdir -p /usr/local/include/nerva
	cp $(shell find includes -name '*.hpp') /usr/local/include/nerva/
	ldconfig
