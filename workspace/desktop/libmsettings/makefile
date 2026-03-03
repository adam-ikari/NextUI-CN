# Handle desktop platform which doesn't need CROSS_COMPILE
ifeq ($(PLATFORM),desktop)
    ifeq (,$(CC))
        CC = gcc
    endif
    # Set default PREFIX for desktop platform
    ifeq (,$(PREFIX))
        PREFIX = /usr
    endif
    # Set default PREFIX_LOCAL for desktop platform
    ifeq (,$(PREFIX_LOCAL))
        PREFIX_LOCAL = $(HOME)/.local
    endif
else
    ifeq (,$(CROSS_COMPILE))
        $(error missing CROSS_COMPILE for this toolchain)
    endif
    CC = $(CROSS_COMPILE)gcc
endif

ifeq (,$(PREFIX))
$(error missing PREFIX for this toolchain)
endif

TARGET=msettings

.PHONY: build
.PHONY: clean

#CFLAGS = -g
#LDFLAGS = -ldl -lrt -s
LDFLAGS = -ldl

OPTM=-Ofast

build:
	$(CC) -c -Werror -fpic "$(TARGET).c" $(CFLAGS)
	$(CC) -shared -o "lib$(TARGET).so" "$(TARGET).o" $(LDFLAGS)
	mkdir -p "$(PREFIX_LOCAL)/include"
	mkdir -p "$(PREFIX_LOCAL)/lib"
	cp "$(TARGET).h" "$(PREFIX_LOCAL)/include"
	cp "lib$(TARGET).so" "$(PREFIX_LOCAL)/lib"

clean:
	rm -f *.o
	rm -f "lib$(TARGET).so"
	rm -f $(PREFIX_LOCAL)/include/$(TARGET).h
	rm -f $(PREFIX_LOCAL)/lib/lib$(TARGET).so