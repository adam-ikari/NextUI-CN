BUILD_DIR = build
PREFIX = $(NEXTUI_PREFIX)
CXX = $(CROSS_COMPILE)g++
CXXFLAGS = -march=armv7-a -mfpu=neon-vfpv4 -ftree-vectorize -Ofast -flto -fomit-frame-pointer -funroll-loops -fno-math-errno -fno-trapping-math -Wall -std=c++17 -fPIC
LDFLAGS = -lm -lGLESv3 -lEGL
TARGET = $(BUILD_DIR)/librt64es.so
SOURCES = src/rt64es_gles32.cpp src/n64/rt64es_n64.cpp src/zelda/rt64es_zelda.cpp

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

TEST_TARGET = $(BUILD_DIR)/test_basic
test: $(TEST_TARGET)
	@echo "Running tests..."
	@$(TEST_TARGET)

$(TEST_TARGET): test/test_basic.cpp $(TARGET)
	@$(CXX) $(CXXFLAGS) -I. $< -o $@ -L$(BUILD_DIR) -lrt64es $(LDFLAGS)

.PHONY: all clean install test
