CC 	= arm-linux-gcc
CFLAGS 	= -O2 -Wall -Werror -lrt -lpthread
LDFLAGS = -lusb-1.0
SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
TARGET 	= xboxcontroller

.PHONY: all clean

all: $(TARGET)
 
$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

install:
	@scp $(TARGET) root@192.168.1.100:/bin

clean:
	@rm -rf $(TARGET) $(OBJECTS)

