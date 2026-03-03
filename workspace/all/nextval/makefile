###########################################################

ifeq (,$(PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. PLATFORM=trimui make)
endif

# Set default CROSS_COMPILE for desktop platform
ifeq (,$(CROSS_COMPILE))
ifeq ($(PLATFORM),desktop)
CROSS_COMPILE=/usr/bin/
else
$(error missing CROSS_COMPILE for this toolchain)
endif
endif

###########################################################

include ../../$(PLATFORM)/platform/makefile.env

###########################################################

TARGET = nextval
INCDIR = -I. -I../common/ -I../../$(PLATFORM)/platform/
SOURCE = $(TARGET).c ../common/utils.c ../common/config.c

CC = $(CROSS_COMPILE)gcc
CFLAGS  += $(OPT) -fomit-frame-pointer
CFLAGS  += $(INCDIR) -DPLATFORM=\"$(PLATFORM)\" -std=gnu99

PRODUCT= build/$(PLATFORM)/$(TARGET).elf

all: 
	mkdir -p build/$(PLATFORM)
	$(CC) $(SOURCE) -o $(PRODUCT) $(CFLAGS) $(LDFLAGS)
clean:
	rm -f $(PRODUCT)
