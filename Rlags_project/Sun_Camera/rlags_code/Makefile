ver = debug
platform = x86

CC = g++
INCLIB = /usr/local/include
LDLIB = /usr/local/lib
OPENCV = -I/usr/include/opencv2    -lopencv_highgui -lopencv_core
USB =  -I../libusb/include  -L../libusb/$(platform) -lusb-1.0  


LIBS = -lASICamera 
LIBSPATH = -L../lib/$(platform) -I../include



ifeq ($(ver), debug)
DEFS = -D_LIN -D_DEBUG 
CFLAGS = -g  -I $(INCLIB) -L $(LDLIB) $(DEFS) $(COMMON) $(LIBSPATH) $(LIBS) -lpthread  $(USB) -DGLIBC_20
else
DEFS = -D_LIN 
CFLAGS =  -O3 -I $(INCLIB) -L $(LDLIB) $(DEFS) $(COMMON) $(LIBSPATH) $(LIBS) -lpthread  $(USB) -DGLIBC_20
endif

ifeq ($(platform), x64)
CFLAGS += -m64
endif

ifeq ($(platform), arm)
CC = arm-linux-gnueabi-g++
AR= arm-linux-gnueabi-ar
USB = -I/usr/local/arm/libusb/include/libusb-1.0  -L/usr/local/arm/libusb/lib -lusb-1.0  
endif

all:
	$(CC) sun_camera0.cpp -o get_sun0_image $(CFLAGS) $(OPENCV)
	$(CC) sun_camera1.cpp -o get_sun1_image $(CFLAGS) $(OPENCV)
	$(CC) sun_camera2.cpp -o get_sun2_image $(CFLAGS) $(OPENCV)
	$(CC) star_camera3.cpp -o get_star3_image $(CFLAGS) $(OPENCV)


clean:
	rm  test_gui test_console
#pkg-config libusb-1.0 --cflags --libs
#pkg-config opencv --cflags --libs

