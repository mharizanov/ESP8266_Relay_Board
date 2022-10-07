
CFLAGS=-I../lib/heatshrink -std=gnu99 -I~/.platformio/packages/framework-esp8266-nonos-sdk/include
OBJS=main.o heatshrink_encoder.o
TARGET=mkespfsimage

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
