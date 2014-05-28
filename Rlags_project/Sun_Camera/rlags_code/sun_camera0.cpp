#include "stdio.h"
#include "highgui/highgui_c.h"
#include "ASICamera.h"
#include <sys/time.h>
#include <time.h>

int  main()
{
	int width = 1280;
	int height = 960;
	int CamNum=0;
	openCamera(CamNum);
	initCamera(); //this must be called before camera operation. and it only need init once
	setImageFormat(width, height, 1, IMG_RAW8);
	IplImage *pRgb;
	pRgb=cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 1);
	setValue(CONTROL_EXPOSURE, 1000, true); //auto exposure
	startCapture(); //get image
	getImageData((unsigned char*)pRgb->imageData, pRgb->imageSize, -1);
	getImageData((unsigned char*)pRgb->imageData, pRgb->imageSize, -1);
	getImageData((unsigned char*)pRgb->imageData, pRgb->imageSize, -1);
	getImageData((unsigned char*)pRgb->imageData, pRgb->imageSize, -1);
	cvSaveImage("/home/linaro/Rlags_project/scripts/sun_cameras/sun_cam_0.jpg",pRgb);
	stopCapture();
	return 1;
}






