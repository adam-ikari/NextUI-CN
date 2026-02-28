###########################################################

ifeq (,$(PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
	$(error please specify PLATFORM, eg. PLATFORM=trimui make)
endif

ifeq (,$(CROSS_COMPILE))
	$(error missing CROSS_COMPILE for this toolchain)
endif

###########################################################

include ../../$(PLATFORM)/platform/makefile.env

###########################################################

BUILD_DIR = build
TARGET = $(BUILD_DIR)/librt64es.so
SOURCES = src/rt64es_gles32.cpp src/n64/rt64es_n64.cpp src/zelda/rt64es_zelda.cpp
CXX = $(CROSS_COMPILE)g++

# Use platform-specific flags from makefile.env
CXXFLAGS += $(OPT) -flto -fomit-frame-pointer -funroll-loops -fno-math-errno -fno-trapping-math -Wall -std=c++17 -fPIC
CXXFLAGS += -I.

LDFLAGS += -lm -lGLESv3 -lEGL

all: $(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(SOURCES) | $(BUILD_DIR)
	@$(CXX) $(CXXFLAGS) -shared $(SOURCES) -o $@ $(LDFLAGS)

clean:
	@rm -rf $(BUILD_DIR)

TEST_TARGET = $(BUILD_DIR)/test_basic

test: $(TEST_TARGET)
	@echo "Running tests..."
	@$(TEST_TARGET)

$(TEST_TARGET): test/test_basic.cpp $(TARGET)
	@$(CXX) $(CXXFLAGS) -I. $< -o $@ -L$(BUILD_DIR) -lrt64es $(LDFLAGS)

.PHONY: all clean test
