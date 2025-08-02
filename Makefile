CXX = clang++

INCLUDE_DIRS := $(shell find includes -type d; find libs -type d -name "includes" -o -type d -name "src")
CXXFLAGS += $(patsubst %,-I%,$(INCLUDE_DIRS))

LDFLAGS = -lsimdjson -lssl -lcrypto

SRC_DIR = src
BUILD_DIR = build
BIN = server

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
LIB_SRCS := $(shell find libs -type f -name '*.cpp')

OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
LIB_OBJS := $(patsubst libs/%.cpp,$(BUILD_DIR)/lib/%.o,$(LIB_SRCS))


LIB_NAME = $(BUILD_DIR)/lib/libnerva.so

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/%.o: libs/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

$(BIN): $(OBJS) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean run lib install

run: $(BIN)
	LD_PRELOAD=/usr/lib/libtcmalloc.so.4 ./$(BIN)

clean:
	rm -rf $(BUILD_DIR) $(BIN) $(LIB_NAME)

lib: $(LIB_OBJS)
	$(CXX) -shared -o $(LIB_NAME) $^ $(LDFLAGS)

install: lib
	mkdir -p /usr/local/lib
	cp $(LIB_NAME) /usr/local/lib/
	mkdir -p /usr/local/include/nerva
	cp $(shell find includes -name '*.hpp') /usr/local/include/nerva/
	# Install library headers too
	find libs -name '*.hpp' -exec cp --parents {} /usr/local/include/nerva/ \;
	ldconfig