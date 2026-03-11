CXX = g++
CC = gcc
CXXFLAGS = -c -Wall -Wextra -g -std=c++17 -I. -I./build -MMD
CFLAGS = -c -Wall -Wextra -g -std=c11 -I. -I./build -MMD
LDFLAGS = -lssl -lcrypto
BUILD_DIR = build
BIN_DIR = bin

C_SOURCES = FileSystem/FileSystem.c FileSystem/Structures.c
CPP_SOURCES = main.cpp Server.cpp SSLSocket.cpp Socket.cpp VSocket.cpp

C_OBJECTS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
CPP_OBJECTS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CPP_SOURCES))

OBJECTS = $(CPP_OBJECTS) $(C_OBJECTS)
DEPS = $(OBJECTS:.o=.d)

all: $(BIN_DIR)/Server

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR)/Server: $(OBJECTS) | $(BIN_DIR)
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

-include $(DEPS)