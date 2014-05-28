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
 * i3dmgx3_Cmd.c
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
#include "i3dmgx3_Cmd.h"
#include "i3dmgx3_Utils.h"
#include "i3dmgx3_Errors.h"
#include "i3dmgx3_Serial.h"


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
       portNum = 0;
 }
/*----------------------------------------------------------------------
 * i3dmgx3_RawSensor  0xC1
 *
 * parameters   portNum : the number of the sensor device (1..n)
 *              pI3Record : struct to receive floating point values for
 *                          raw Accel (x,y,z) and raw Ang (x,y,z)
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *---------------------------------------------------------------------*/
int i3dmgx3_RawSensor(int portNum, unsigned char *pRecord) {
    int responseLength = 31;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_RAW_ACCELEROMETER; /* value is 0xC1 */
	int i = 0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
   		status = receiveData(portNum, &pRecord[0], responseLength);
        if (DEBUG) printf("Raw Sensor i3dmgx3_send: rx status : %d and responseLength %d\n", status, responseLength);
			
    	if (status==I3DMGX3_COMM_OK) {
            wChecksum = convert2ushort(&pRecord[responseLength-2]);
	        wCalculatedCheckSum = i3dmgx3_Checksum(&pRecord[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)		
				return	status = I3DMGX3_CHECKSUM_ERROR; 
		}else
			return status = I3DMGX3_COMM_READ_ERROR;	
	}else
		status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
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
int i3dmgx3_AccelAndAngRate(int portNum, unsigned char* pRecord) {
    int responseLength = 31;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_ACCELERATION_ANGU; /* value is 0xC2 */
	int i = 0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
    	status = receiveData(portNum, &pRecord[0], responseLength);
        if (DEBUG) printf("Accel Ang i3dmgx3_send: rx status : %d  and responseLength %d\n", status, responseLength);  
    	if (status==I3DMGX3_COMM_OK) {
            wChecksum = convert2ushort(&pRecord[responseLength-2]);
	        wCalculatedCheckSum = i3dmgx3_Checksum(&pRecord[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)
				return	status = I3DMGX3_CHECKSUM_ERROR;
		}else
		    return status = I3DMGX3_COMM_READ_ERROR;			
	}else
		status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
}
/*----------------------------------------------------------------------
 * i3dmgx3_DeltaAngAndVelocity 0xC3
 *
 * parameters   portNum : the number of the sensor device (1..n)
 *              pI3Record : struct to receive floating point values for
 *							Accel (x,y,z) and Ang (x,y,z)
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_DeltaAngAndVelocity(int portNum, unsigned char* Bresponse) {
    int responseLength = 31;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_DELTA_ANGLE_VELOC; /* value is 0xC3 */
	int i = 0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("DeltaAngAndVelocity i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
     	status = receiveData(portNum, &Bresponse[0], responseLength);
        if (DEBUG) printf("DeltaAngAndVelocity i3dmgx3_send: rx status : %d  and responseLength %d\n", status, responseLength);		   
    	if (status==I3DMGX3_COMM_OK) {
            wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	        wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)
			   return	status = I3DMGX3_CHECKSUM_ERROR;
		}else
		   return status = I3DMGX3_COMM_READ_ERROR;
	}else
		status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
}
/*----------------------------------------------------------------------
 * i3dmgx3_OrientMatrix	0xC5
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *              pI3Record    : pointer to a  float matrix
 *                             which will contain the Orientation data
 *                             upon return.
 *              
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_OrientMatrix(int portNum, unsigned char* Bresponse) {
    int responseLength = 43;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_ORRIENTATION_MAT; /* value is 0xC5 */
	int i = 0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("Orient Matrix i3dmgx3_send: tx status : %d\n", status);
	if (status == I3DMGX3_COMM_OK) {
    		status = receiveData(portNum, &Bresponse[0], responseLength);
            if (DEBUG) printf("Orient Matrix i3dmgx3_send: rx status : %d  and responseLength %d\n", status, responseLength);
					   
    		if (status==I3DMGX3_COMM_OK) {
                wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	            wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
					if(wChecksum != wCalculatedCheckSum)
						return	status = I3DMGX3_CHECKSUM_ERROR;
			}else
			    return status = I3DMGX3_COMM_READ_ERROR;  
	}else
		status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
}
/*----------------------------------------------------------------------
 * i3dmgx3_OrientUpMatrix 0xC6
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *              pI3Record    : pointer to a  float matrix
 *                             which will contain the Orientation Update
 *                             data upon return.
 *              
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns 
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_OrientUpMatrix(int portNum, unsigned char* Bresponse) {
    int responseLength = 43;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_ATTITUDE_UP_MATRIX; /* value is 0xC6 */
	int i = 0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("Orient Update i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
      		status = receiveData(portNum, &Bresponse[0], responseLength);
            if (DEBUG) printf("Orient Update i3dmgx3_send: rx status : %d  and responseLength %d\n", status, responseLength);
					   
    		if (status==I3DMGX3_COMM_OK) {
                wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	            wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
					if(wChecksum != wCalculatedCheckSum)
						return	status = I3DMGX3_CHECKSUM_ERROR;
			}else
			    return status = I3DMGX3_COMM_READ_ERROR;
	}else
		status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
}
/*----------------------------------------------------------------------
 * i3dmgx3_ScaledMagVec	0xC7
 *
 * parameters   portNum : the number of the sensor device (1..n)
 *              pI3Record : struct to receive floating point values for
 *                          scaled Magnetometer Vectors (x,y,z)
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns 
 *                          an error code.
 *---------------------------------------------------------------------*/
int i3dmgx3_ScaledMagVec(int portNum, unsigned char* Bresponse) {
    int responseLength = 19;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_MAGNETROMETER_VECT; /* value is 0xC7 */
	int i = 0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("Scaled Mag Vector i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
      		status = receiveData(portNum, &Bresponse[0], responseLength);
            if (DEBUG) printf("Scaled Mag Vectorr i3dmgx3_send: rx status : %d and responseLength %d\n", status, responseLength);
    		if (status==I3DMGX3_COMM_OK) {
                wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	            wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
					if(wChecksum != wCalculatedCheckSum)
						return	status = I3DMGX3_CHECKSUM_ERROR;
			}else
					return status = I3DMGX3_COMM_READ_ERROR;			
		}else
			status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
}
/*----------------------------------------------------------------------
 * i3dmgx3_AccelAngOreint	0xC8
 *
 * parameters   portNum : the number of the sensor device (1..n)
 *              pI3Record : struct to receive floating point values for
 *                          Acceleration and Angular Rates (x,y,z)
 *                          and Orientation Matrix M1.1 - M1.2 ... M3.3
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns 
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_AccelAngOreint(int portNum, unsigned char* Bresponse) {
    int responseLength = 67;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_ACCEL_ANG_ORIENT; /* value is 0xC8 */
	int i = 0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("Accel AngRate OrientMatx i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
       		status = receiveData(portNum, &Bresponse[0], responseLength);
            if (DEBUG) printf("Accel Ang Orient i3dmgx3_send: rx status : %d and responseLength %d\n", status, responseLength);
    		if (status==I3DMGX3_COMM_OK) {
                wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	            wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
					if(wChecksum != wCalculatedCheckSum)
						return	status = I3DMGX3_CHECKSUM_ERROR;
			}else
					return status = I3DMGX3_COMM_READ_ERROR;  	
	}else
			status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
}
/*----------------------------------------------------------------------
 * i3dmgx3_AccAngMagRate	0xCB
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *              mag       : array which will contain mag data 
 *                         (3 elements X Y and Z)
 *              accel     : array which will contain acceleration data 
 *                         (3 elements X Y and Z)
 *              angRate   : array which will contain angular rate data 
 *                         (3 elements X Y and Z)
 *              
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_AccAngMagRate(int portNum, unsigned char* Bresponse) {
	int responseLength = 43;
	int status = 0, i =0;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_ACCEL_ANG_MAG_VECTO;  //0xCB
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("Get Acc ang mag i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
    		status = receiveData(portNum, &Bresponse[0], responseLength);
			if (DEBUG) printf("Accel ang mag rate i3dmgx3_send: rx status : %d and responseLength %d\n", status, responseLength); /* temp adj 8-1-08 */
    		if (status==I3DMGX3_COMM_OK) {
                wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	            wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
	            if(wChecksum != wCalculatedCheckSum){
                    status = I3DMGX3_CHECKSUM_ERROR;
                }
            }else
                status = I3DMGX3_COMM_READ_ERROR;
    }else
		status = I3DMGX3_COMM_WRITE_ERROR;       
	return status;
}
/*----------------------------------------------------------------------
 * i3dmgx3_GetFullMatrix 0xCC
 *  	
 * parameters   portNum    : the number of the sensor device (1..n)
 *              I3Record   : 18 positional stucture of defined type 
 *                           I3Record to capture data in floating point
 *							   
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_GetFullMatrix(int portNum, unsigned char* Bresponse) {
	int responseLength = 79;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_ACEL_ANG_MAG_VEC_OR; /* value is 0xCC */
	int i=0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
    		status = receiveData(portNum, &Bresponse[0], responseLength);
            if (DEBUG) printf("i3dmgx3_send: rx status : %d and responseLength %d\n", status, responseLength); /* temp adj 8-1-08 */
    		if (status==I3DMGX3_COMM_OK) {
                wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	            wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
				if(wChecksum != wCalculatedCheckSum)
                   return status = I3DMGX3_CHECKSUM_ERROR;
				}else
				return status = I3DMGX3_COMM_READ_ERROR;
    }else
		status = I3DMGX3_COMM_WRITE_ERROR;  
	return status;
}
/*----------------------------------------------------------------------
 * i3dmgx3_captureGyroBias	0xCD
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *              Bresponse    : pointer to a buffer containing returned values
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns 
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_captureGyroBias(int portNum, short sampt, BYTE *BiasBuff, unsigned char *Bresponse) {
    unsigned char cmd = CMD_CAPTURE_GYRO_BIAS; //0xCD
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	int responseLength = 19;
    int status;
	int i = 0;
	unsigned char Coutbuff[5];
	
	/* initialize send buffer to obtain gyro bias */
	Coutbuff[0] = 0xCD;   //Command code for Gyro Bias
	Coutbuff[1] = 0xC1;   // Required identifier
    Coutbuff[2] = 0x29;   // Required identifier
    Coutbuff[3] = (sampt & MSB_MASK) >> 8;  //sample time with mask
	Coutbuff[4] =  sampt & LSB_MASK; 
	
	printf("...one %d/ms moment while gyro bias values are retrieved\n", sampt);

	status = sendBuffData(portNum, &Coutbuff[0], 5);
    if (DEBUG) printf(" Capture Gyro Bias  Send struct status is %d\n", status);
	if (status == I3DMGX3_COMM_OK) {
		    status = receiveData(portNum, &Bresponse[0], responseLength);
			if (DEBUG) printf(" Capture Gyro Bias Receive  status is %d\n", status);
			if (status == I3DMGX3_COMM_OK) {
			    wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	            wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
				if(wChecksum != wCalculatedCheckSum)
					return status = I3DMGX3_CHECKSUM_ERROR;
	        }else
			    return status = I3DMGX3_COMM_READ_ERROR;        			
			
			for (i=0; i<12; i++)
				BiasBuff[i] = Bresponse[1 +i];
	}else
		return status = I3DMGX3_COMM_WRITE_ERROR;
	return status;

}
/*----------------------------------------------------------------------
 * i3dmgx3_WriteGyroBias	0xCA
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *              BiasBuff     : a buffer containing 3 floating point bias(es)
 *              PI3Record    : pointer to a buffer containing returned values
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns 
 *                          an error code.
 *--------------------------------------------------------------------*/

int i3dmgx3_WriteGyroBias(int portNum, BYTE *BiasBuff, unsigned char* Bresponse) {
    unsigned char cmd = CMD_WRITE_GYRO_BIAS; //0xCD
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	int responseLength = 19;
    int status;
	int i = 0;
    unsigned char BiasoutBuff[15];

    BiasoutBuff[0] = 0xCA; //CMD_WRITE_GYRO_BIAS;
	BiasoutBuff[1] = 0x12; // Required identifier
	BiasoutBuff[2] = 0xA5; // Required identifier
	for (i=0; i<12; i++)   //repacks the float values in big endian format
		BiasoutBuff[3 + i] = BiasBuff[i];
		
	status = sendBuffData(portNum, &BiasoutBuff[0], 15); // 15 equals command byte + 14 bytes of data
     
	if (DEBUG) printf(" Write Gyro Bias Send struct status is %d\n", status);
        if (status == I3DMGX3_COMM_OK) {
	    status = receiveData(portNum, &Bresponse[0], responseLength);
	    if (DEBUG) printf(" Write  Gyro Bias Receive  status is %d\n", status);
        if (status == I3DMGX3_COMM_OK) {
			wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	        wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); 
		    if(wChecksum != wCalculatedCheckSum)
			   return status = I3DMGX3_CHECKSUM_ERROR;
	    }else			   
 		   return status = I3DMGX3_COMM_READ_ERROR;
	}else
	    return status = I3DMGX3_COMM_WRITE_ERROR;

	return status;
 }
/*----------------------------------------------------------------------
 * i3dmgx3_EulerAngles	0xCE
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *             I3dmgx3Set    : pointer to a struct containing the values 
 *                             pitch angle in degrees
 *              
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns 
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_EulerAngles(int portNum, unsigned char* Bresponse) {
    int responseLength = 19;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_EULER_ANGLES; /* value is 0xCE */
	int i = 0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("Euler Ang i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
    	status = receiveData(portNum, &Bresponse[0], responseLength);
        if (DEBUG) printf("Euler Ang i3dmgx3_send: rx status : %d and responseLength %d\n", status);
    	if (status==I3DMGX3_COMM_OK) {
            wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	        wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)
				return status = I3DMGX3_CHECKSUM_ERROR;
		}else
			return status = I3DMGX3_COMM_READ_ERROR;
    	
	}else
		return status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
 }
/*----------------------------------------------------------------------
 * i3dmgx3_EulerAngRate	0xCF
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *             I3dmgx3Set    : pointer to a struct containing the values 
 *                             for pitch angles in degrees and
 *							   Angle rates x y and z
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_EulerAngRate(int portNum, unsigned char* Bresponse) {
    int responseLength = 31;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_EULER_ANGLES_ANG_RT; /* value is 0xCF */
	int i = 0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("i3dmgx3_send Euler and Ang Rate: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) { 
	   	status = receiveData(portNum, &Bresponse[0], responseLength);
        if (DEBUG) printf("Euler Ang Rate i3dmgx3_send: rx status : %d and responseLength %d\n", status, responseLength);
        if (status==I3DMGX3_COMM_OK) {
            wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	        wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)
				return status = I3DMGX3_CHECKSUM_ERROR;
		}else
			return status = I3DMGX3_COMM_READ_ERROR;
	} else
		return status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
}
/*----------------------------------------------------------------------
 * i3dmgx3_TransferNonVolatile	0xD0
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *              transfer     : numeric value identifying which quantity 
 *                             to Transfer: 1 == Accel Bias						   
 *                                          2 == Gyro Bias
 *                            Any other value results in no action taken
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_TransferNonVolatile(int portNum, int transfer, unsigned char *Bresponse) {
    int responseLength = 9;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_TRANSFER_NONV_MEM; /* value is 0xD0 */
	int i = 0;
	unsigned char outbuff[5];
			 
    outbuff[0] = 0xD0; // Command Code Identifier
	outbuff[1] = 0xC1; // Required identifier
	outbuff[2] = 0x29; // Required identifier
	outbuff[3] =  (transfer & MSB_MASK) >> 8; //1st byte of quantity to transfer
	outbuff[4] =  transfer & LSB_MASK;       //2ond byte of quantity to transfer
	                  // 1==Accel Bias 2==Gyro Bias, any other value results in no action 

	status = sendBuffData(portNum, &outbuff[0], 5);
   	if (DEBUG) printf(" Transfer Non-V MEM Send struct status is %d\n", status);
        if (status == I3DMGX3_COMM_OK) {
	    status = receiveData(portNum, &Bresponse[0], responseLength);
	     if (DEBUG) printf(" Transfer Non-V MEM Receive  status is %d\n", status);
		     if (status == I3DMGX3_COMM_OK) {
			     wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	             wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2);
                  
			    if(wChecksum != wCalculatedCheckSum)
				   return status = I3DMGX3_CHECKSUM_ERROR;
	        }else			   
 			    return status = I3DMGX3_COMM_READ_ERROR;   
		}else
	       return status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
}
/*----------------------------------------------------------------------
 * i3dmgx3_Tempatures	0xD1
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *              pI3Temps     : pointer to a struct containing Temperature
 *                             Accel and Temp gyro x y and z values.
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_Tempatures(int portNum, unsigned char* Bresponse) {
    int responseLength = 15;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_TEMPERATURES; /* value is 0xD1 */
	int i = 0;
	WORD zetemp = 0;
	
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("Temperatures i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
    	status = receiveData(portNum, &Bresponse[0], responseLength);
        if (DEBUG) printf("Temperatures i3dmgx3_send: rx status : %d and responseLength %d\n", status);
    	if (status==I3DMGX3_COMM_OK) {
            wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	        wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)
				return status = I3DMGX3_CHECKSUM_ERROR;
		}else
			return status = I3DMGX3_COMM_READ_ERROR;		
	}else
		return status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
 }
/*----------------------------------------------------------------------
 * i3dmgx3_GyroStab	0xD2
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *             I3dmgx3Set    : pointer to a struct containing the values 
 *                             gyro stab accel angR and Mag             
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_GyroStab(int portNum, unsigned char* Bresponse) {
    int responseLength = 43;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_GYRO_STAB_A_AR_MG; /* value is 0xD2 */
	int i = 0;
		
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("GyroStab i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
    	status = receiveData(portNum, &Bresponse[0], responseLength);
        if (DEBUG) printf("GyroStab i3dmgx3_send: rx status : %d and responseLength %d\n", status);
        if (status==I3DMGX3_COMM_OK) {
            wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	        wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)
			return status = I3DMGX3_CHECKSUM_ERROR;
		}else
			return status = I3DMGX3_COMM_READ_ERROR;
   	}else
		return status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
 }
/*----------------------------------------------------------------------
 * i3dmgx3_DeltaAngVel	0xD3
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *             I3dmgx3Set    : pointer to a struct containing the values 
 *                             gyro stab accel angR and Mag
 *              
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_DeltaAngVel(int portNum, unsigned char* Bresponse) {
    int responseLength = 43;
	int status;
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	unsigned char cmd = CMD_DELTA_ANGVEL_MAGV; /* value is 0xD3 */
	int i = 0;
		
	status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("DeltaAngVel i3dmgx3_send: tx status : %d\n", status);
    if (status == I3DMGX3_COMM_OK) {
    	status = receiveData(portNum, &Bresponse[0], responseLength);
        if (DEBUG) printf("DeltaAngVel i3dmgx3_send: rx status : %d and responseLength %d\n", status);
        if (status==I3DMGX3_COMM_OK) {
            wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	        wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)
			  return status = I3DMGX3_CHECKSUM_ERROR;
		}else
			return status = I3DMGX3_COMM_READ_ERROR;
    }else
		return status = I3DMGX3_COMM_WRITE_ERROR;
	return status;
 }
/*----------------------------------------------------------------------
 * i3dmgx3_getEEPROMValue	0xE5
 *
 * parameters   portNum    : the number of the sensor device (1..n)
 *              address      : the EEPROM address location
 *              readFlag     : specifies the number of reads required.
 *                             and identifies data type long or float.
 *              value        : the value to get at the address specified
 *
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int i3dmgx3_getEEPROMValue(int portNum, char address, int readFlag, int *readval) {
	int status = 0;
	long bytesRead = 0;
	unsigned char Bresponse[5]  = {0};
	unsigned char BFresponse[5]  = {0};
	unsigned char ConvertBuff[4] = {0};
	int responseLength = 5;
	unsigned long nvalL=0;
	unsigned short nvalA=0, nvalB=0;
	float nvalF=0.0;
	unsigned char zoutbuff[4];
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;

	zoutbuff[0] = 0xE5;     // EEPROM command identifier
	zoutbuff[1] = 0x00;     // Required identifier
	zoutbuff[2] = 0xFC;     // Required identifier
	zoutbuff[3] = address;  // EEPROM address location

	status = sendBuffData(portNum, &zoutbuff[0], 4);
	if (DEBUG) printf(" eeprom read status is %d\n", status);
    if (status == I3DMGX3_COMM_OK) { 
		status = receiveData(portNum, &Bresponse[0], 5);
        if (status == I3DMGX3_COMM_OK) {
            status = I3DMGX3_OK;
			  wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	          wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)
				return status = I3DMGX3_CHECKSUM_ERROR;
			
            nvalA = convert2ushort(&Bresponse[1]);
			
			if (readFlag > 0) {
				ConvertBuff[3] = Bresponse[2];
				ConvertBuff[2] = Bresponse[1];
				zoutbuff[3] = address + 2;

				status = sendBuffData(portNum, &zoutbuff[0], 4);
				if (status != I3DMGX3_COMM_OK)
				   return status;
		
				status = receiveData(portNum, &BFresponse[0], 5);

				if (status == I3DMGX3_COMM_OK) {
					status = I3DMGX3_OK;
					wChecksum = convert2ushort(&Bresponse[responseLength-2]);
					wCalculatedCheckSum = i3dmgx3_Checksum(&BFresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
					if(wChecksum != wCalculatedCheckSum)
						return status = I3DMGX3_CHECKSUM_ERROR;
					nvalB = convert2ushort(&BFresponse[1]);
					
					ConvertBuff[1] = BFresponse[2];
				    ConvertBuff[0] = BFresponse[1];
					nvalF = FloatFromBytes(&ConvertBuff[0]);
					nvalL = convert2ulong(&ConvertBuff[0]);
					if (readFlag == 1){
						printf("  At addr.:0x%X where value is:   %ld \n", zoutbuff[3], nvalL);
						*readval = nvalL;
					}
					else{
						printf("  At addr.:0x%X where value is: %f \n", zoutbuff[3], nvalF);
						*readval = nvalF;
					}
				}else
					return status = I3DMGX3_COMM_READ_ERROR;
			}else{
					printf("  At addr.:0x%X where value is: %d \t0x%X\n", address, nvalA, nvalA);
					*readval = nvalA;
			}
        }else
            return status = I3DMGX3_COMM_READ_ERROR;
    }
    return status;
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
 *
 * WARNING - does not check to see if you have allocated enough space
 *           12 bytes for the string to contain the firmware version.
 *--------------------------------------------------------------------*/
int i3dmgx3_getDeviceIdentiy(int portNum, char flag, unsigned char* Bresponse) {
    char cmd = (char) CMD_GET_DEVICE_ID;		//0xEA
    unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
    int status; 
	int responseLength = 20; 
	unsigned char iden_buff[2];
	
	iden_buff[0] = 0xEA; // Device Identity Command Code
	iden_buff[1] = flag; // Identifier of specific device identity component to obtain

	status = sendBuffData(portNum, &iden_buff[0], 2);
	if (DEBUG) printf("Get Identity_send: tx status : %d\n", status);
	if (status == I3DMGX3_COMM_OK) {
    	status = receiveData(portNum, &Bresponse[0], responseLength);
		if (DEBUG) printf("Get Identity i3dmgx3_send: rx status : %d and responseLength %d\n", status, responseLength);
    	if (status==I3DMGX3_COMM_OK) {
			wChecksum = convert2ushort(&Bresponse[responseLength-2]);
			wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
			if(wChecksum != wCalculatedCheckSum)
				return	status = I3DMGX3_CHECKSUM_ERROR;
		}else
			return status = I3DMGX3_COMM_READ_ERROR;
	}else return status;
   	
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
    unsigned char cmd = (char) CMD_FIRWARE_VERSION;
    unsigned char Bresponse[7] = {0}; 
	unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
	
	short firmwareNum=0;
    short majorNum, minorNum, buildNum;
    int status;
	int responseLength = 7; 
	  	
    status = sendBuffData(portNum, &cmd, 1);
    if (DEBUG) printf("FirmWare_send: tx status : %d\n", status);
	if (status == I3DMGX3_COMM_OK) {
		if (responseLength>0) {
    		status = receiveData(portNum, &Bresponse[0], responseLength);
            if (DEBUG) printf("FirmWare i3dmgx3_send: rx status : %d and responseLength %d\n", status, responseLength);
    		if (status==I3DMGX3_COMM_OK) {
                wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	            wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); //calculate the checkusm, 29 = 31-2 don't include the checksum bytes
					if(wChecksum != wCalculatedCheckSum)
						return	status = I3DMGX3_CHECKSUM_ERROR;
			}else
					return status = I3DMGX3_COMM_READ_ERROR;
		}

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
