###########################################################

ifeq (,$(PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. PLATFORM=trimui make)
endif

###########################################################

include ../../$(PLATFORM)/platform/makefile.env
SDL?=SDL

###########################################################

TARGET = nextui
INCDIR = -I. -I../common/ -I../../$(PLATFORM)/platform/ -I../../i18n/
INCDIR += -Icomponents -Istate -Iscreens
SOURCE = main.c \
	array.c \
	app.c \
	renderer.c \
	screens/screen.c \
	screens/game_list_screen.c \
	screens/quick_menu_screen.c \
	screens/game_switcher_screen.c \
	components/ui_component.c \
	components/pill_component.c \
	components/button_component.c \
	components/list_component.c \
	components/status_component.c \
	state/ui_state.c \
	../common/scaler.c \
	../common/utils.c \
	../common/config.c \
	../common/api.c \
	nextui_api.c \
	../../i18n/i18n.c \
	../../$(PLATFORM)/platform/platform.c \
	desktop_stubs.c

CC = $(CROSS_COMPILE)gcc
CFLAGS  += $(OPT) -fomit-frame-pointer
CFLAGS  += $(INCDIR) -DPLATFORM=\"$(PLATFORM)\" -std=gnu99
# Add stub defines for SDL2 libraries that might not be available on desktop
ifeq ($(PLATFORM),desktop)
    CFLAGS += -DNO_SDL2_IMAGE -DNO_SDL2_TTF
    LDFLAGS += `pkg-config --libs sdl2`
    # macOS doesn't use -lGL or -lGLESv2, it uses -framework OpenGL
else
    LDFLAGS	 += -lmsettings -lGLESv2
endif
ifeq ($(PLATFORM), tg5040)
CFLAGS += -DHAS_WIFIMG -DHAS_BTMG
LDFLAGS +=  -lwifimg -lwifid
LDFLAGS +=  -lbtmg -lglib-2.0 -lgio-2.0 -lshared-mainloop -lbluetooth-internal -lasound -ljson-c
endif

PRODUCT= build/$(PLATFORM)/$(TARGET).elf

all: $(PREFIX_LOCAL)/include/msettings.h
	mkdir -p build/$(PLATFORM)
	$(CC) $(SOURCE) -o $(PRODUCT) $(CFLAGS) $(LDFLAGS)
clean:
	rm -f $(PRODUCT)

$(PREFIX_LOCAL)/include/msettings.h:
	cd ../../$(PLATFORM)/libmsettings && make
