/*----------------------------------------------------------------------
* (c) 2009 Microstrain, Inc.
*----------------------------------------------------------------------
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING 
* CUSTOMERS WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER 
* FOR THEM TO SAVE TIME. AS A RESULT, MICROSTRAIN SHALL NOT BE HELD LIABLE 
* FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY 
* CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY 
* CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH 
* THEIR PRODUCTS.
*---------------------------------------------------------------------*/

/*----------------------------------------------------------------------
* Read Acceleration and Angular Rate
*
* 9/12/2008	fpm added this comment
*
* This command line application demonstrates how to use the 3DM-GX3 SDK to 
* retrieve an Acceleration and Angular Rate data sample from a MicroStrain 
* orientation sensor.  The only argument is the port number for the sensor.
* This can be obtained by using the Windows Device Manager. Under the 
* "Ports (COM & LPT)" listing, the MicroStrain device will be listed as
* either a "CP210x USB to UART Bridge Controller" or a "MicroStrain Virtual
* COM Port".  If no port argument is specified, then a scanport function is 
* called which will look for an attached sensor by scanning the serial ports.
*----------------------------------------------------------------------*/

#include <stdio.h>
#include "ms_basic_type.h"
#include "i3dmgx3_Errors.h"
#include "i3dmgx3_Serial.h"
#include "i3dmgx3_Cmd.h"
#include "i3dmgx3_Utils.h"
#include "win_scanports.h"

int GetComPort(); //prompt user for comport and opens it
int OnGetSerialPorts();

/*--------------------------------------------------------------------*/
int main(int argc, char **argv) {

        s32 zvert=0;
        BOOL endloopy = FALSE;

	s16 portNum;
	s16 deviceNum = 0;
	s16 i;
        s16 Ccount=0;
	u16 value=0;
	s16 id_flag = 0;
	s16 errorCode;
	s16 tryPortNum = 1;
	unsigned char Record[79];				//record returned from device read where max size is 79
	C2Accel_AngRecord	Accel_AngRecord;

	printf("\n   3DM-GX3 Read Acceleration and Angular Rate\n");

	/*-------- If user specifies a port, then use it */
	if (argc > 1) {
		tryPortNum = atoi(argv[1]);
		if (tryPortNum < 2 || tryPortNum > 256) {
			printf("   usage:  i3dmgx3 <portNumber>\n");
			printf("        valid ports are 2..256\n");
			exit(1);
		}

	        /*-------- open a port, map a device */
	        portNum = i3dmgx3_openPort(tryPortNum, 115200, 8, 0, 1, 1024, 1024);
	        if (portNum<0) {
		    printf("   port open failed.\n");
		    printf("   Comm error %d, %s: ", portNum, explainError(portNum));
		   goto Exit;
	        }

        }else{
          portNum=OnGetSerialPorts();
          if(portNum<0)
             goto Exit;

        }
	printf("\n   Using COM Port #%d \n", portNum);

	/*-------- Set Comm Timeout values */
	errorCode = setCommTimeouts(portNum, 50, 50); /* Read & Write timeout values */
	if (errorCode!=I3DMGX3_COMM_OK) {
		printf("   setCommTimeouts failed on port:%d with errorcode:%d\n",portNum,errorCode);
		goto Exit;
	} 

	/*-------- Disclose the byte order of host */
	if( TestByteOrder() !=BIG_ENDIAN)
		printf("   (Local Host is in Little Endian format)\n");
	else
		printf("   (Local Host is in Big Endian format)\n");
	printf("\n");  

	/*-------- 0xC2 Accel and Ang rate Output --- Accel x y z and Ang x y z */
	printf("\n   0xC2  Accel and Ang Output  \n");


	errorCode = i3dmgx3_AccelAndAngRate(portNum, &Record[0]);
	if (errorCode < 0){
		printf("   Error Accel and AngRate - : %s\n", explainError(errorCode));
                endloopy =TRUE;
	}else{
		for (i=0; i<3; i++) {
			Accel_AngRecord.Accel[i] = FloatFromBytes(&Record[1 + i*4]);	// extract float from byte array
			Accel_AngRecord.AngRt[i] = FloatFromBytes(&Record[13 + i*4]);	// extract float from byte array
		}
		printf("\n\tAccel X\t\tAccel Y\t\tAccel Z\n");
		printf("  \t%f\t%f\t%f\n", Accel_AngRecord.Accel[0], Accel_AngRecord.Accel[1], Accel_AngRecord.Accel[2]);
		printf("\n\t  Ang X\t\t Ang Y\t\t Ang Z\n");
		printf("  \t%f\t%f\t%f\n", Accel_AngRecord.AngRt[0], Accel_AngRecord.AngRt[1], Accel_AngRecord.AngRt[2]);

		Accel_AngRecord.timer = convert2ulong(&Record[25]);
		printf("\n   Time Stamp: %u\n", Accel_AngRecord.timer);
        }

Exit:
	/*-------- close device */
	if (portNum >= 0)
		i3dmgx3_closeDevice(portNum);

	/*-------- wait for user to respond before exiting */
	printf("\nHit return to exit...\n");
	while (getchar() == EOF);
	return(0);
}

/*===========================================================================*
** GetComPort
**---------------------------------------------------------------------------
** Description: Prompt user for the comport and then opens it.  The user is 
**              prompted until a valid comport is selected and successfully 
**              opened.
**
** Return: HANDLE - handle to the opened comport
**===========================================================================*/

int GetComPort(){
	s16 vvportNum   = 0;
	int iComPort  = 0;
	int errCount = 0;
	int MaxFail = 5;
	//Get comport number ask user for the comport number and open it
	while(vvportNum == 0)
	{
		printf("Enter Comport to use:");
		scanf("%d", &iComPort);
		
		/* open a port, map a device */
                vvportNum = i3dmgx3_openPort(iComPort, 115200, 8, 0, 1, 1024, 1024);
                if (vvportNum<0) {
                    printf("port open failed. ");
                    printf("Comm error %d, %s:\n", vvportNum, explainError(vvportNum));
			if (errCount++ >= MaxFail)
				exit(-1);
			iComPort = 0;
		}
		else {
                        printf("\n   Using Comm Port #%d \n", vvportNum);
		}
		
	} 

	return vvportNum;
}
/* ***************************************************************************************
**  OnGetSerialPorts                   Checks for a valid 3dm-gx3-25 device and if found
**                                     gets a connection and returns the portnumber
**
*****************************************************************************************/
int OnGetSerialPorts()
{
	int i_count, t_count=0;
	unsigned char c_name[20];
    int  i_errorCode = 0;
    int i_portNum = 0;
	char c_check[]="3DM-GX3";
	char Verity[20];
	
 
	for(i_count=2; i_count<257; i_count++)
	{
		int i_portNum = i3dmgx3_openPort(i_count, 115200, 8, 0, 1, 1024, 1024);
                if (i_portNum>0) {
                    i_errorCode = i3dmgx3_getDeviceIdentiy(i_portNum, 2, &c_name[0]);

                     memcpy(Verity,c_name,20);
                     //if (strstr(c_name,c_check) != NULL){
				
					if (strstr(Verity,c_check) != NULL){
			           return i_portNum;					
	                     } //End string compare
					
                       else i3dmgx3_closeDevice(i_count);          
				
                 }// end open port
				
	}// end for loop
	   printf("No 3DM-GX3 devices found, please check connections\n");
       return -1;                
 }