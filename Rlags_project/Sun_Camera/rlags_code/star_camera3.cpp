#include "stdio.h"
#include "highgui/highgui_c.h"
#include "ASICamera.h"
#include <sys/time.h>
#include <time.h>

int  main()
{
	int width = 1280;
	int height = 960;
	int CamNum=3;
	openCamera(CamNum);
	initCamera(); //this must be called before camera operation. and it only need init once
	setImageFormat(width, height, 1, IMG_RAW8);
	IplImage *pRgb;
	pRgb=cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 1);
	setValue(CONTROL_EXPOSURE, 1000, true); //auto exposure
	startCapture(); //get image
	getImageData((unsigned char*)pRgb->imageData, pRgb->imageSize, -1);
	stopCapture();
	cvSaveImage("star_cam.jpg",pRgb);
	return 1;
}






