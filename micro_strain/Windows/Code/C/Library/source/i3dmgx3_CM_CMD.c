/*----------------------------------------------------------------------
 *
 * I3DM-GX3 Interface Software
 *
 *----------------------------------------------------------------------
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
 * i3dmgx3_CM_CMD.c
 *
 * 3DM-GX3 Sensor device functions.
 *
 * This platform independent module relies on the communication functions 
 * found in either i3dmgx3SerialWin.c or i3dmgx3SerialLinux.c 
 * (your choice, depending on platform).
 *--------------------------------------------------------------------*/
 
#ifndef DEBUG
#define DEBUG 0
#endif

#include <stdio.h>
#include "i3dmgx3_Cont.h"
#include "i3dmgx3_Utils.h"
#include "i3dmgx3_Errors.h"
#include "i3dmgx3_Serial.h"

HANDLE portHandles[MAX_PORT_NUM];
char *axis[] = { "X", "Y", "Z"};

/*----------------------------------------------------------------------
 * i3dmgx3_openPort
 *
 * Open a serial communications port. (platform independent).
 *
 * parameters   portNum  : a port number (1..n).
 *--------------------------------------------------------------------*/

int i3dmgx3_openPort(int portNum, int baudrate, int size, int parity, 
					 int stopbits, int inputBuff, int outputBuff) {
    int errcheck;
    int porth;

    errcheck = openPort(portNum, inputBuff, outputBuff);
    if (errcheck<0) {
        return errcheck;
    }
    porth = errcheck;  /* no error, so this is the port number. */

    /* set communications parameters */
    errcheck = setCommParameters(porth, baudrate, size, parity, stopbits);
    if (errcheck!=I3DMGX3_COMM_OK) {
        return errcheck;
    }

    /* set timeouts */
    errcheck = setCommTimeouts(porth, 50, 50); /* Read Write */
    if (errcheck!=I3DMGX3_COMM_OK) {
        return errcheck;
    }
    return porth;
}

/*----------------------------------------------------------------------
 * i3dmgx3_closeDevice
 *
 * parameters   portNum      : the number of the sensor device 
 *                                
 * Close a device, and also any underlying port.
 *--------------------------------------------------------------------*/
void i3dmgx3_closeDevice(int portNum) {
    
       closePort(portNum);    
}
/*----------------------------------------------------------------------
 * i3dmgx3_AccelandAngRate 0xC2
 *
 * parameters   portNum : the number of the sensor device (1..n)
 *              pI3Record : struct to receive floating point values for
 *							Accel (x,y,z) and Ang (x,y,z)
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_AccelAndAngRate(int portNum, BYTE* Bresponse) {
    int responseLength = 31;
	int status;
	unsigned short wChecksum = 0;
	unsigned short wCalculatedCheckSum = 0;
	char cmd = CMD_ACCELERATION_ANGU; /* value is 0xC2 */
    		
    sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("i3dmgx3_send: tx status : %d\n", status);
    receiveData(portNum, &Bresponse[0], responseLength);
    if (DEBUG) printf("Accel Ang i3dmgx3_send: rx status : %d  and responseLength %d\n", status, responseLength);
					   
     wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	 wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum chars
	 if(wChecksum != wCalculatedCheckSum)
		return	status = I3DMGX3_CHECKSUM_ERROR;  
		
	 return 0;
}

/*===========================================================================
* SetContinuousMode
*---------------------------------------------------------------------------
* Start continuous mode
*===========================================================================*/
int SetContinuousMode(int portNum, unsigned char dataType)
{
	unsigned char Bresponse[6];
	int status = 0;
      unsigned char GX3command[] = {0xD6, 0xC6, 0x6B, 0x00};
      unsigned char GX3toobuff[] = {0xD4, 0xA3, 0x47, 0x02};
      GX3command[3] = dataType;
	status = sendBuffData(portNum, &GX3command[0], 4);
      if(status == 0){
	       status = receiveData(portNum, &Bresponse[0], 4);
	       status = sendBuffData(portNum, &GX3toobuff[0], 4);
	       if(status == 0)
	          status = receiveData(portNum, &Bresponse[0], 4);
			 
	}
      return status;
}

/*===========================================================================
* StopContinuousMode
*---------------------------------------------------------------------------
* End continuous mode
*=========================================================================== */
int StopContinuousMode(int portNum)  /*stops a node that is in continuous mode */
{
	int status;
	unsigned char GX3Buff[] = {0xFA, 0x75, 0xB4};
	status = sendBuffData(portNum, &GX3Buff[0], 3);
      purge_port(portNum);
	return status;
}
/*===========================================================================
* ReadNextRecord
*---------------------------------------------------------------------------
* Descriptions: reads the next record in continuous mode
* Parameters: hComPort - handle to an open comport connected to a node in 
*                        continuous mode.
*             pRecored - pointer to a struct that will recieve the record
* Return: int - SUCCESS if successful, otherwise error code indicating the
*               failure
*==========================================================================*/
int ReadNextRecord(int portNum, I3dmgx3Set* pRecord, BYTE* cmd_return){
	unsigned long dwcharsRead  = 0;
	int iTry            = 0;
	int iMaxTry         = 75;  //read X chars looking for a start of packet befor giving up
	int i = 0;
	int status = 0;
	unsigned char cmd_chk;
	unsigned char Bresponse[50] = {0}; //Current max buff request is 42
	unsigned short wChecksum = 0;
	unsigned short wCalculatedCheckSum = 0;
	HANDLE portHandle;
	BOOL found = FALSE;
    portHandle = portHandles[portNum-1];
	
	
	while( !found) {
		status = receiveData(portNum, &cmd_chk, 1);
		if (status <0){
			//printf("ReceiveData start of packet failed with status: %d\n", status);
			return I3DMGX3_COMM_READ_ERROR; 
		}
		if (cmd_chk == 0xC2) //Acceleration & Angular rate packet
			found = TRUE;
		if (cmd_chk == 0xCB) //Acceleration & Angular Mag packet
			found = TRUE;
		if (cmd_chk == 0xD3) //Delta Ang Velocity and Mag packet
			found = TRUE;
		if (cmd_chk == 0xC4) //Continuous mode confirmation packet
			found = TRUE;
		if (iTry++ >= iMaxTry);
		    found = TRUE;
		
	}
   		switch(cmd_chk)	{
			case 0xc2:
                  
				  status = receiveData(portNum, &Bresponse[0], 30);
				  pRecord->timer = ((((((Bresponse[24]) << 8) & Bresponse[25]) << 8) & Bresponse[26]) << 8) & Bresponse[27];
				  pRecord->timer = convert2ulong(&Bresponse[24]);
				  wChecksum = convert2ushort(&Bresponse[28]);
	                          wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], 28) + 0xC2; 
				  break; 
		        case 0xcb:
				  status = receiveData(portNum, &Bresponse[0], 42);
				  pRecord->timer = ((((((Bresponse[36]) << 8) & Bresponse[37]) << 8) & Bresponse[38]) << 8) & Bresponse[39];
				  wChecksum = convert2ushort(&Bresponse[40]);
	                          wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], 40) + 0xCB; 			  
				  break;
                        case 0xd3:
				  status = receiveData(portNum, &Bresponse[0], 42);
				  pRecord->timer = ((((((Bresponse[36]) << 8) & Bresponse[37]) << 8) & Bresponse[38]) << 8) & Bresponse[39];
				  wChecksum = convert2ushort(&Bresponse[40]);
	                          wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], 40) + 0xD3; 			  
				  break;
		        case 0xc4:
				  status = receiveData(portNum, &Bresponse[0], 7);
				  pRecord->timer = ((((((Bresponse[1]) << 8) & Bresponse[2]) << 8) & Bresponse[3]) << 8) & Bresponse[4];
				  wChecksum = convert2ushort(&Bresponse[5]);
	                          wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], 5) + 0xC4; 			  
				  break;
		   default: /* printf("Invalid Comand Code %x\n", cmd_chk);  */
			       return -12;
	}
    if (status <0 ){
		//printf("Read failed status is: %d\n",status);
		return status;
	}
    if(wChecksum != wCalculatedCheckSum){
	    return I3DMGX3_CHECKSUM_ERROR;
	}
	if (cmd_chk != 0xc4) {   
	     for (i=0; i<3; i++) {
				pRecord->setA[i] = FloatFromBytes(&Bresponse[0 + i*4]); //  Accel x y z
				pRecord->setB[i] = FloatFromBytes(&Bresponse[12 + i*4]); // Ang rates x y z
				if (cmd_chk == 0xCB || cmd_chk == 0xD3)
                                    pRecord->setC[i] = FloatFromBytes(&Bresponse[24 + i*4]); // Mag rates x y z                               
				  
	    }
	}
	
	*cmd_return = cmd_chk;
	return I3DMGX3_OK;
}
/*----------------------------------------------------------------------
 * i3dmgx3_getDeviceIdentity  0xEA
 *
 * parameters   portNum : the number of the sensor device (1..n)
 *              flag      : identifier for which Identity to obtain.
 *              id        : a pointer to char string, already allocated.
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_getDeviceIdentiy(int portNum, char flag, char *snid) {
    char cmd = (char) CMD_GET_DEVICE_ID;		//0xEA
    unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
    int status; 
	int responseLength = 20; 
	unsigned char iden_buff[2];
	unsigned char Bresponse[20];
	int tempCount = 0;
	int tempCount2 = 0;
	int x = 0;
	
	iden_buff[0] = 0xEA; // Device Identity Command Code
	iden_buff[1] = flag; // Identifier of specific device identity component to obtain

	status = sendBuffData(portNum, &iden_buff[0], 2);
	//if (DEBUG) printf("Get Identity_send: tx status : %d\n", status);
	//if (status == I3DMGX3_COMM_OK) {
    	status = receiveData(portNum, &Bresponse[0], responseLength);
		//if (DEBUG) printf("Get Identity i3dmgx3_send: rx status : %d and responseLength %d\n", status, responseLength);
    	//if (status==I3DMGX3_COMM_OK) {
			wChecksum = convert2ushort(&Bresponse[responseLength-2]);
			wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)
				return	status = I3DMGX3_CHECKSUM_ERROR;
		//}else
			//return status = I3DMGX3_COMM_READ_ERROR;
	//}else return status;
   	
	/*if (flag < 3) {  
	    for(x=2;x<18;x++){ 
		   if (!isspace(Bresponse[x]))
			   snid[tempCount++] = Bresponse[x];
		}
	}
    if (flag == 3) { */
		for (x=2; x<18;x++)
			snid[tempCount2++] = Bresponse[x];
	    //strcpy(Bresponse, tempX);
	//} 

return I3DMGX3_OK;
}
/*----------------------------------------------------------------------
 * i3dmgx3_getFirmwareVersion  0xE9
 *
 * parameters   portNum : the number of the sensor device (1..n)
 *              firmware  : a pointer to char string, already allocated.
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *
 * WARNING - does not check to see if you have allocated enough space
 *           12 bytes for the string to contain the firmware version.
 *---------------------------------------------------------------------*/

int i3dmgx3_getFirmwareVersion(int portNum, char *firmware) {
    unsigned char cmd = ( unsigned char) CMD_FIRWARE_VERSION;
    BYTE Bresponse[7] = {0}; 
    unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	
    short firmwareNum=0;
    short majorNum, minorNum, buildNum;

    int status;
    int responseLength = 7; 
		  	
    status = sendBuffData(portNum, &cmd, 1);
     //printf("FirmWare_send: tx status : %d\n", status);
	if (status == I3DMGX3_COMM_OK) {
		//if (responseLength>0) {
    		    status = receiveData(portNum, &Bresponse[0], responseLength);
                   // printf("FirmWare i3dmgx3_send: rx status : %d and responseLength %d\n", status, responseLength);
    		    //if (status==I3DMGX3_COMM_OK) {
                       wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	               wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
		       if(wChecksum != wCalculatedCheckSum)
			     return status = I3DMGX3_CHECKSUM_ERROR;
		 // }else
		 //    return status = I3DMGX3_COMM_READ_ERROR;
		// }

		firmwareNum = convert2short(&Bresponse[3]);
		if (firmwareNum > 0) {
			/* format for firmware number is #.#.## */
	            majorNum = firmwareNum / 1000;
		    minorNum = (firmwareNum % 1000) / 100;
		    buildNum = firmwareNum % 100;
		    sprintf(firmware, "%d.%d.%d", majorNum, minorNum, buildNum);
		} 
		return I3DMGX3_OK;
	}else return status;
}
/*-------------- end of i3dmgx3Adapter.c ----------------------*/
