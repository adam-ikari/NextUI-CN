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
SDL?=SDL

###########################################################

TARGET = lvgl9demo
INCDIR = -I. -I../common/ -I../../$(PLATFORM)/platform/ -I../../i18n/
SOURCE = -c ../common/utils.c ../common/api.c ../common/config.c ../common/scaler.c ../../$(PLATFORM)/platform/platform.c ../../i18n/i18n.c
CSOURCE = $(TARGET).c lv_port_nextui.c build/$(PLATFORM)/utils.o build/$(PLATFORM)/api.o build/$(PLATFORM)/config.o build/$(PLATFORM)/scaler.o build/$(PLATFORM)/platform.o build/$(PLATFORM)/i18n.o 

# LVGL source files
LVGL_DIR = src
LVGL_SOURCES = $(shell find $(LVGL_DIR) -name "*.c" | sort)
LVGL_OBJECTS = $(addprefix build/$(PLATFORM)/lvgl/, $(LVGL_SOURCES:.c=.o))

CC = $(CROSS_COMPILE)gcc
CFLAGS += $(OPT) -fomit-frame-pointer
CFLAGS += $(INCDIR) -DPLATFORM=\"$(PLATFORM)\"
CFLAGS += -DLV_CONF_INCLUDE_SIMPLE
CFLAGS += -fpermissive
# Enable GPU optimizations
CFLAGS += -march=armv7-a -mfpu=neon -ftree-vectorize
LDFLAGS += -lmsettings -lSDL2 -lSDL2_image -lSDL2_ttf

PRODUCT= build/$(PLATFORM)/$(TARGET).elf

all: $(PREFIX_LOCAL)/include/msettings.h
	mkdir -p build/$(PLATFORM)/lvgl
	$(CC) $(SOURCE) $(CFLAGS) $(LDFLAGS)
	mv utils.o api.o config.o scaler.o platform.o i18n.o build/$(PLATFORM)
	
	# Compile LVGL source files with GPU optimizations
	@for src in $(LVGL_SOURCES); do \
		obj=$$(echo $$src | sed 's|^$(LVGL_DIR)/||'); \
		$(CC) $(CFLAGS) -I. -c $$src -o build/$(PLATFORM)/lvgl/$$obj; \
	done
	
	# Link everything
	$(CC) $(CSOURCE) $(LVGL_OBJECTS) -o $(PRODUCT) $(CFLAGS) $(LDFLAGS)
	@echo "LVGL 9 Demo built with GPU acceleration"

clean:
	rm -rf build/$(PLATFORM)

$(PREFIX_LOCAL)/include/msettings.h:
	cd ../../$(PLATFORM)/libmsettings && make
EOF
