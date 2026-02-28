include ../../makefile.toolchain

TARGET = usbmassstorage.elf
SRCS = usbmassstorage.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS) -lmsettings -lSDL2 -lSDL2_image -lSDL2_ttf -lusb-1.0

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
