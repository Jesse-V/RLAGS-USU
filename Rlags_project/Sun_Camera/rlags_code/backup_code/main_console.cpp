#include "stdio.h"
#include "highgui/highgui_c.h"
#include "ASICamera.h"
#include <sys/time.h>
#include <time.h>

//#pragma comment(lib,"../OpenCV/lib/highgui.lib") 
//#pragma comment(lib,"../OpenCV/lib/cxcore.lib") 
//#pragma comment(lib,"../../lib/ASIcamera.lib") 


#define  MAX_CONTROL 7

int  main()
{
	int width;
	char* bayer[] = {"RG","BG","GR","GB"};
	char* controls[MAX_CONTROL] = {"Exposure", "Gain", "Gamma", "WB_R", "WB_B", "Brightness", "USB Traffic"};

	int height;
	int i;
	char c;
	bool bresult;
	unsigned char *pdata;

	int time1,time2;
	int count=0;

	char buf[128]={0};

	int CamNum=0;
	///long exposure, exp_min, exp_max, exp_step, exp_flag, exp_default;
	//long gain, gain_min, gain_max,gain_step, gain_flag, gain_default;

	IplImage *pRgb;

	int numDevices = getNumberOfConnectedCameras();
	if(numDevices <= 0)
	{
		printf("no camera connected, press any key to exit\n");
		getchar();
		return -1;
	}
	else
		printf("attached cameras:\n");

	for(i = 0; i < numDevices; i++)
		printf("%d %s\n",i, getCameraModel(i));

	printf("\nselect one to privew\n");
	scanf("%d", &CamNum);


	bresult = openCamera(CamNum);
	if(!bresult)
	{
		printf("OpenCamera error,are you root?,press any key to exit\n");
		getchar();
		return -1;
	}
	initCamera(); //this must be called before camera operation. and it only need init once

	printf("%s information\n",getCameraModel(CamNum));
	printf("resolution:%dX%d\n", getMaxWidth(),getMaxHeight());
	pdata = new unsigned char[getMaxWidth()*getMaxHeight()*3];
	if(isColorCam())
		printf("Color Camera: bayer pattern:%s\n",bayer[getColorBayer()]);
	else
		printf("Mono camera\n");

	for( i = 0; i < MAX_CONTROL; i++)
	{
			if(isAvailable((Control_TYPE)i))
				printf("%s support:Yes\n", controls[i]);
			else
				printf("%s support:No\n", controls[i]);
	}
	printf("sensor temperature:%02f\n", getSensorTemp());
	printf("\nPlease input the width and height with one space, ie. 640 480\n");
	scanf("%d %d", &width, &height);

	while(!setImageFormat(width, height, 1, IMG_RAW8))
	{
		printf("Set format error, please check the width and height\n ASI120's data size(width*height) must be integer multiple of 1024\n");
		printf("Please input the width and height again，ie. 640 480\n");
		scanf("%d %d", &width, &height);
	}
	printf("\nset image format success, start privew, press ESC to stop \n");
//	cvNamedWindow("video", 1);

	pRgb=cvCreateImage(cvSize(getWidth(),getHeight()), IPL_DEPTH_8U, 1);

	setValue(CONTROL_EXPOSURE, 33*1000, true); //auto exposure

	startCapture(); //start privew

/*	time1 = GetTickCount();

	int count2 = 0;
	while(1)
	{

		//getImageData((unsigned char*)pRgb->imageData, pRgb->imageSize, -1);
		getImageData(pdata,width*height , -1);

		count++;
		time2 = GetTickCount();
		if(time2-time1 > 1000 )
		{
			sprintf(buf, "fps:%d dropped frames:%lu",count, getDroppedFrames());
			printf("%s\n",buf);
			count = 0;
			time1=GetTickCount();


		}
		//cvText(pRgb, buf, 40,40 );

		//cvFlip(pRgb,NULL,1);//加上这句就水平翻转画面
*/
/*		char c=cvWaitKey(1);
		switch(c)
		{
		case 27://按ESC退出
			goto END;
		}

		cvShowImage("video", pRgb);

	}*/
	getImageData((unsigned char*)pRgb->imageData, pRgb->imageSize, -1);
	delete pdata;
	stopCapture();
	cvSaveImage("sun_cam_0.jpg",pRgb);
	printf("over\n");
	return 1;
}






