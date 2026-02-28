BUILD_DIR = build
PREFIX = $(NEXTUI_PREFIX)
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++

CFLAGS = -march=armv7-a -mfpu=neon-vfpv4 -ftree-vectorize -O2 -Wall -std=c11
CXXFLAGS = -march=armv7-a -mfpu=neon-vfpv4 -ftree-vectorize -O2 -Wall -std=c++17

LDFLAGS = -lm -lSDL2 -lEGL -lGLESv3 -lpthread

RT64ES_DIR = ../rt64es
RT64ES_LIB = $(RT64ES_DIR)/build/librt64es.so

SOURCES = main.c
OBJECTS = $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))
TARGET = $(BUILD_DIR)/zeldaoot

all: $(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(RT64ES_LIB):
	@$(MAKE) -C $(RT64ES_DIR)

$(TARGET): $(OBJECTS) $(RT64ES_LIB) | $(BUILD_DIR)
	@echo "LD $@"
	@$(CC) $(OBJECTS) -o $@ -L$(RT64ES_DIR)/build -lrt64es $(LDFLAGS)
	@echo "Build complete: $@"

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -I$(RT64ES_DIR) -I$(RT64ES_DIR)/src -I$(RT64ES_DIR)/src/n64 -I$(RT64ES_DIR)/src/zelda -c $< -o $@

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)
	@$(MAKE) -C $(RT64ES_DIR) clean

install: $(TARGET)
	@echo "Installing zeldaoot..."
	@install -d $(PREFIX)/usr/bin
	@install -m 755 $(TARGET) $(PREFIX)/usr/bin/
	@$(MAKE) -C $(RT64ES_DIR) install PREFIX=$(PREFIX)
	@echo "Installation complete"

.PHONY: all clean install
