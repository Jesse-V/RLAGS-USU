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
 * i3dmgx3_SerialWin.c
 *
 * Serial port interface for the MicroStrain Sensor device. 
 *
 * This module is specific to the Microsoft Windows platform. 
 *   (95,98,NT,2000,XP)
 * 
 *
 *----------------------------------------------------------------------*/
#ifndef DEBUG
#define DEBUG 0
#endif

#include <stdio.h>
#include <windows.h>


#include "i3dmgx3_Errors.h"
#include "i3dmgx3_Serial.h"

// port Handles in use
HANDLE portHandles[MAX_PORT_NUM];

/*----------------------------------------------------------------------
 * openPort
 *
 * parameters:  portNum : a port number from 1 to MAX_PORT_NUM
 *
 * returns:     a port handle or I3DMGX3_COMM_FAILED.
 *----------------------------------------------------------------------*/

int openPort(int portNum, int inputBuff, int outputBuff) {

    HANDLE portHandle;
	char portName[12] = "";
	char portIndex[8] = "";
	int error_code =0;
	
    if (portNum<0 || portNum > MAX_PORT_NUM) {
        
		printf("portnum is %d\n", portNum);
		return I3DMGX3_COMM_INVALID_PORTNUM;
    }
    //strcat(portName, "COM");
	strcat(portName, "\\\\.\\COM");
    _itoa(portNum, portIndex,10);
	strcat(portName, portIndex);
    //portName = strcat("COM", portIndex);
	portHandle = CreateFile(portName, 
							GENERIC_READ | GENERIC_WRITE,
							0, // exclusive access
							NULL, // no security
							OPEN_EXISTING,
							0, // no overlapped I/O
							NULL); // null template

	
    if (DEBUG) printf("trying to open port: %s # %d\n", portName, portNum);
           
	if (portHandle == INVALID_HANDLE_VALUE || portHandle == 0) {
		error_code= GetLastError();
	    if(DEBUG) printf("last_error code %d on port %s\n", error_code, portName);
		CloseHandle(portHandle); 
		return I3DMGX3_COMM_FAILED;
	} else {
        SetupComm(portHandle, inputBuff, outputBuff);	// set buffer sizes
        portHandles[portNum-1] = portHandle;
        return portNum;
	}
}
/*----------------------------------------------------------------------
 * closePort
 *
 * parameters:  portNum : closes the port corresponding to this number.
 *----------------------------------------------------------------------*/

void closePort(int portNum) {
     if (portNum>0 && portNum<=MAX_PORT_NUM) {
    	CloseHandle(portHandles[portNum-1]); 
		//printf("port number is %d\n",portNum);
    }
}

/*----------------------------------------------------------------------
 * setCommParameters
 *
 * parameters:  portNum  : a serial port number
 *              baudrate : the communication flow rate (speed).
 *              charsize : the character size (7,8 bits)
 *              parity   : 0=none, 1=odd, 2=even
 *              stopbits : the number of stop bits (1 or 2)
 *
 * returns:     I3DMGX3_COMM_OK if the settings succeeded.
 *              I3DMGX3_COMM_FAILED if there was an error.
 *----------------------------------------------------------------------*/
int setCommParameters(int portNum, int baudrate, int charsize,
                      int parity, int stopbits ) {

    BOOL ready;
    DCB  dcb;
    HANDLE portHandle;
    portHandle = portHandles[portNum-1];

    ready = GetCommState(portHandle, &dcb);
    if (!ready) {
        return I3DMGX3_COMM_FAILED;
    }
    if (stopbits==1) {
        stopbits=0;    /* in Windows 0 means 1 stopbit */
    }
	dcb.BaudRate = baudrate;  //Baudrate is 115200 
	dcb.ByteSize = charsize;  //Charsize is 8  default for MicroStrain
	dcb.Parity = parity;      //Parity is none default for MicroStrain
	dcb.StopBits = stopbits;  //Stopbits is 1  default for MicroStrain
	dcb.fAbortOnError = TRUE;
	ready = SetCommState(portHandle, &dcb);
    if (!ready)
        return I3DMGX3_COMM_FAILED;
    else
        return I3DMGX3_COMM_OK;
}

/*----------------------------------------------------------------------
 * setCommTimeouts
 *
 * parameters:  portNum      : a serial port number
 *              readTimeOut  : timeout for single char and total read.
 *              writeTimeOut : timeout for single char and total write.
 *----------------------------------------------------------------------*/

int setCommTimeouts( int portNum, 
                     int readTimeout, 
                     int writeTimeout) {
    BOOL ready;
	COMMTIMEOUTS timeOuts;
    HANDLE portHandle;
    portHandle = portHandles[portNum-1];

	ready = GetCommTimeouts (portHandle, &timeOuts);
    if (!ready) {
        return I3DMGX3_COMM_FAILED;
    }
	timeOuts.ReadIntervalTimeout = readTimeout;
	  //Specifies the maximum acceptable time, in milliseconds, to elapse between the arrival of two characters on the communication line.
	timeOuts.ReadTotalTimeoutConstant = readTimeout;
	  //Specifies the constant, in milliseconds, used to calculate the total timeout period for read operations.
	timeOuts.ReadTotalTimeoutMultiplier = 10;
      //Specifies the multiplier, in milliseconds, used to calculate the total timeout period for read operations.
	timeOuts.WriteTotalTimeoutConstant = writeTimeout;
      //Specifies the constant, in milliseconds, used to calculate the total timeout period for write operations.
	timeOuts.WriteTotalTimeoutMultiplier = 10;
	  //Specifies the multiplier, in milliseconds, used to calculate the total timeout period for write operations. 
	ready = SetCommTimeouts (portHandle, &timeOuts);
	
    if (!ready) {
        return I3DMGX3_COMM_FAILED;
    }
    else
        return I3DMGX3_COMM_OK;
}

/*----------------------------------------------------------------------
 * sendBuffData
 *
 * parameters:  portNum  - a serial port number (1..n)
 *              command  - a buffer containing multiple bytes corresponding to 
 *                           the MicroStrain command being issued
 *              commandLength - the length of the command buffer
 *                              which is the number of bytes to send
 *
 * returns:    COMM_OK if write/read succeeded
 *             COMM_WRITE_ERROR if there was an error writing to port
 *--------------------------------------------------------------------*/

int sendBuffData(int portNum, unsigned char *command, int commandLength)
{
	DWORD bytesWritten;
	BOOL status;

    HANDLE portHandle;
    portHandle = portHandles[portNum-1];

    /* write the command. */
	status = WriteFile (portHandle, command, commandLength, &bytesWritten, 0);
	if (!status)
		return I3DMGX3_COMM_WRITE_ERROR;
    else
        return I3DMGX3_COMM_OK;
}

/*---------------------------------------------------------------------
 * receiveData
 *
 * parameters:  portNum  - a serial port number
 *              response - a pointer to a character buffer
 *              responseLength - the # of bytes expected for response.
 *
 * returns:     COMM_OK if write/read succeeded
 *              COMM_READ_ERROR if there was an reading from the port
 *              COMM_RLDLEN_ERROR if the length of the response did not
 *                            match the number of returned bytes.
 *---------------------------------------------------------------------*/
int receiveData(int portNum, unsigned char *response, int responseLength) {
	DWORD bytesRead = 0;
	BOOL status;
    int returnVal;
	int Count = 0;
	int MaxCount = 10;
    HANDLE portHandle;
    portHandle = portHandles[portNum-1];
	

	returnVal = I3DMGX3_COMM_OK;
	if (responseLength>0) {
		while(bytesRead < 1 && Count++ < MaxCount)
		{
		    status = ReadFile(portHandle, &response[0], responseLength, &bytesRead, NULL);
		    if (bytesRead != responseLength)
				if (DEBUG) printf("number of bytes read %d and requested were %d\n", bytesRead, responseLength);
		}
		if (status) { /* check for wrong number of bytes returned. */
			if (bytesRead != responseLength){
				//printf("number of bytes read %d and requested were %d\n", bytesRead, responseLength);
                                returnVal = I3DMGX3_COMM_RDLEN_ERROR;
			} 
		} else
            returnVal = I3DMGX3_COMM_READ_ERROR; 
	}
    return status; //returnVal;
}
/*---------------------------------------------------------------------
 * purge_port
 * parameters:  portNum  - a serial port number
 *
 * returns:	if successful, 0 
 *			if failed, -1 
 *---------------------------------------------------------------------*/
int purge_port( int portNum){
	HANDLE portHandle;
    portHandle = portHandles[portNum-1];
	if (!PurgeComm(portHandle, PURGE_RXCLEAR | PURGE_TXCLEAR)) {// clear the input and output buffers
		return -1;
	}
	return 0;
}
/*-------------- end of i3dmgx3SerialWin.c ----------------------*/
