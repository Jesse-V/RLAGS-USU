# Makefile for usbio

TARGET = usbio
SRCS = main.c hid-libusb.c
OBJS = main.o
CC = gcc
FLAGS = -I/usr/include/libusb-1.0/ -lusb-1.0 -lpthread -DDEBUG_PRINTF

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f $(TARGET) *.o

$(TARGET):$(OBJS)
	$(CC) -o $@ $(SRCS) $(FLAGS)

$(OBJS): $($@:.o=.c) hid-libusb.c Makefile

