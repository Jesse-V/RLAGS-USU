#include <stdio.h>
#include <sys/types.h>
#include "hidapi.h"

#define MAX_STR 65
#define TRUE 1
#define FALSE 0

unsigned char buf[MAX_STR];
int toggleLeds;
int potentiometerValue;
int pushbuttonStatus;

hid_device* dev_open() {

	hid_device *device;

	device = hid_open(0x04D8, 0x003F, NULL);

	if(device) {
		printf("HID Device opened\n");
		hid_set_nonblocking(device, TRUE);
	} else
		printf("HID Device open failed\n");

	return device;
}

void closeDevice(hid_device *device) {
	hid_close(device);
}

void checkmSec(int msec) {
	struct timeval ts, te;
	long tdiff, sec, ms;

	gettimeofday(&ts, NULL);

	while(1) {
		gettimeofday(&te, NULL);

		tdiff = (1000000*(te.tv_sec - ts.tv_sec)) + (te.tv_usec - ts.tv_usec);

		ms = tdiff / 1000;

		if(ms > msec)
			break;
	}
}

void rwUSB(hid_device *device) {

	unsigned char cho = buf[10];
	float volt = 0;
	int i = 0;

	if(toggleLeds == TRUE) {
		
		toggleLeds = FALSE;

		buf[1] = 0x80;

		if (hid_write(device, buf, sizeof(buf)) == -1) {
			closeDevice(device);
			return;
		}

		buf[0] = 0x00;
		buf[1] = 0x37;
		memset((void*)&buf[2], 0x00, sizeof(buf) - 2);

		return ;
	}
	
	do {
		if(hid_write(device, buf, sizeof(buf)) == -1) {
			closeDevice(device);
			return ;
		}

		checkmSec(50);

		if(hid_read(device, buf, sizeof(buf)) == -1) {
			closeDevice(device);
			return;
		}

		if(buf[0] == 0x37) {
			printf("buf 1 %d\n",buf[1]);
			potentiometerValue = (buf[1]<<2) + ((buf[2]&0xF0)>>2);
			buf[1] = 0x81;
		
			printf("potentiometerValue = %d\n", potentiometerValue);
		}

		if(buf[0] == 0x81) {
			printf("Pushbutton State : %s\n", buf[1]?"Not Pressed":"Pressed");
		}

	} while(0);
}

int main() {

	int i = 0;
	unsigned char s[2] = {0, 0};
	hid_device *device;

	device = dev_open();

	if(!device)
		return -1;

	pushbuttonStatus = FALSE;
	potentiometerValue = 0;
	toggleLeds = 0;
	
	system("clear");
					
	while(1) {
			
			printf("\n");
			printf("a. Toggle LED\n");
			printf("b. AN0/POT Value\n");
			printf("c. Button status\n");
			printf("q. Exit\n");
			printf("\n");

			scanf("%s", s);

			buf[0] = 0x00;
			memset((void*)&buf[2], 0x00, sizeof(buf) - 2);

			buf[10] = s[0];

			switch(s[0]) {
				case 'a':
					toggleLeds = TRUE;
					break;

				case 'b':
					buf[0] = 0x37;
					break;

				case 'c':
					buf[0] = 0x81;
					break;

				case 'q':
				default: 
					printf("Bye~~\n");
					closeDevice(device);
					return 0;
		}
		rwUSB(device);
	}

	closeDevice(device);

	return 0;
}
