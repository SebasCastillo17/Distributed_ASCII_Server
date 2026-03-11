# Compiladores y banderas
CC = gcc
CXX = g++
CFLAGS = -c -Wall -Iinclude
CXXFLAGS = -c -Wall -Iinclude

# Directorios
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Archivos fuente y headers
C_SOURCES := $(shell find $(SRC_DIR) -name '*.c')
CPP_SOURCES := $(shell find $(SRC_DIR) -name '*.cc')
CHEADERS := $(shell find include -name '*.h')
CPPHEADERS := $(shell find include -name '*.hh')

# Archivos objeto
C_OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
CPP_OBJECTS := $(patsubst $(SRC_DIR)/%.cc, $(BUILD_DIR)/%.o, $(CPP_SOURCES))
OBJECTS := $(C_OBJECTS) $(CPP_OBJECTS)

# Ejecutables individuales
EXE1 := $(BIN_DIR)/ASCIIServer
EXE2 := $(BIN_DIR)/ForkServer
EXE3 := $(BIN_DIR)/Client
EXE := $(EXE1) $(EXE2) $(EXE3)

# Objetos por ejecutable (excluyen el otro main)
ASCIISERVER_OBJECTS := $(filter-out $(BUILD_DIR)/Client.o $(BUILD_DIR)/ForkServer.o, $(OBJECTS))
FORKSERVER_OBJECTS := $(filter-out $(BUILD_DIR)/ASCIIServer.o $(BUILD_DIR)/Client.o, $(OBJECTS))
CLIENT_OBJECTS := $(filter-out $(BUILD_DIR)/ASCIIServer.o $(BUILD_DIR)/ForkServer.o, $(OBJECTS))

# Regla por defecto
all: $(EXE)

# Enlazar main1
$(EXE1): $(ASCIISERVER_OBJECTS) | $(BIN_DIR)
	$(CXX) $(ASCIISERVER_OBJECTS) -o $@

# Enlazar main2
$(EXE2): $(FORKSERVER_OBJECTS) | $(BIN_DIR)
	$(CXX) $(FORKSERVER_OBJECTS) -o $@

# Enlazar main3
$(EXE3): $(CLIENT_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CLIENT_OBJECTS) -o $@

# Compilar archivos C
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(CHEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

# Compilar archivos C++
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc $(CHEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@

# Crear bin/
$(BIN_DIR):
	mkdir -p $@

# Linter
lint:
	cpplint --extensions=c,h,cc,hh --filter=-build/header_guard $(shell find $(SRC_DIR) include -name '*.[ch]pp' -o -name '*.[ch]')

# Documentación
doc:
	doxygen

# Limpieza
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) latex html disk.dat doc/

