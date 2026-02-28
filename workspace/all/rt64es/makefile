BUILD_DIR = build
PREFIX = $(NEXTUI_PREFIX)
CXX = $(CROSS_COMPILE)g++
CXXFLAGS = -march=armv7-a -mfpu=neon -ftree-vectorize -O2 -Wall -std=c++17 -fPIC
LDFLAGS = -lm -lGLESv3 -lEGL
TARGET = $(BUILD_DIR)/librt64es.so
SOURCES = src/rt64es_gles32.cpp

all: $(TARGET)
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
$(TARGET): $(SOURCES) | $(BUILD_DIR)
	@$(CXX) $(CXXFLAGS) -shared -I. $(SOURCES) -o $@ $(LDFLAGS)
clean:
	@rm -rf $(BUILD_DIR)
install: $(TARGET)
	@install -d $(PREFIX)/usr/lib $(PREFIX)/usr/include
	@install -m 755 $(TARGET) $(PREFIX)/usr/lib/
	@install -m 644 rt64es.h $(PREFIX)/usr/include/
.PHONY: all clean install
