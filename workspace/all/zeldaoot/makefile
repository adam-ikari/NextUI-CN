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

BUILD_DIR = build/$(PLATFORM)
TARGET = zeldaoot
CC = $(CROSS_COMPILE)gcc

RT64ES_DIR = ../rt64es
RT64ES_LIB = $(RT64ES_DIR)/build/librt64es.so

SOURCES = main.c
OBJECTS = $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))
PRODUCT = $(BUILD_DIR)/$(TARGET)

# Use platform-specific flags from makefile.env
CFLAGS += $(OPT) -fomit-frame-pointer -std=c11
CFLAGS += -I$(RT64ES_DIR) -I$(RT64ES_DIR)/src -I$(RT64ES_DIR)/src/n64 -I$(RT64ES_DIR)/src/zelda

LDFLAGS += -L$(RT64ES_DIR)/build -lrt64es -lpthread

all: $(PRODUCT)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(RT64ES_LIB):
	@$(MAKE) -C $(RT64ES_DIR)

$(PRODUCT): $(OBJECTS) $(RT64ES_LIB) | $(BUILD_DIR)
	@echo "LD $@"
	@$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning..."
	@rm -rf build
	@$(MAKE) -C $(RT64ES_DIR) clean

.PHONY: all clean
