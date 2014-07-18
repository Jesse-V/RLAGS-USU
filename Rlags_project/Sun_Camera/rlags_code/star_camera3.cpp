#include "highgui/highgui_c.h"
#include "ASICamera.h"
#include <thread>
#include <iostream>

int main()
{
	const int WIDTH = 1280, HEIGHT = 960;

	if (!openCamera(0))
	{
		std::cout << "openCamera failed!" << std::endl;
		return true;
	}

	if (!initCamera())
	{
		std::cout << "initCamera failed!" << std::endl;
		return true;
	}

	int exposureTime;
	bool autoExpose;
	std::cin >> exposureTime;
	std::cin >> autoExpose;

	bool autov;
	setImageFormat(WIDTH, HEIGHT, 1, IMG_RAW8);
	setValue(CONTROL_EXPOSURE, exposureTime, autoExpose);
	setValue(CONTROL_BRIGHTNESS, 5, false);
	setValue(CONTROL_GAMMA, 50, false);
	setValue(CONTROL_GAIN, 10, false);

	//int exposure_us = getValue(CONTROL_EXPOSURE, &autov);
	//int gain = getValue(CONTROL_GAIN, &autov);
	//int max_gain = getMax(CONTROL_GAIN);
	//std::cout << exposure_us << ", " << gain << ", " << max_gain << std::endl;

	IplImage *buffer = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
	startCapture();

	bool captured = false;
	do
	{
		std::chrono::milliseconds(10);
		captured = getImageData((unsigned char*)buffer->imageData, buffer->imageSize, -1);
	} while (!captured);

	cvSaveImage("star_cam.jpg", buffer);
	stopCapture();
	closeCamera();

	return false;
}
