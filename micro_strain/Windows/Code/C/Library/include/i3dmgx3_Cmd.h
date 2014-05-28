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
 * i3dmgx3_Cmd.h
 *
 * Definitions for the 3dm-gx3 inertia sensor devices
 * The continuous mode functions are supported by this adapter.
 *----------------------------------------------------------------------*/
 
#include <windows.h>

/* START CMD SET FOR 3DM_gx3 INERTIA DEVICES */
#define CMD_WIRELESS_PING       0x02	//TODO
#define CMD_RAW_ACCELEROMETER   0xC1
#define CMD_ACCELERATION_ANGU   0xC2
#define CMD_DELTA_ANGLE_VELOC   0xC3
#define CMD_SET_CONTINIOUS      0xC4	
#define CMD_ORRIENTATION_MAT    0xC5
#define CMD_ATTITUDE_UP_MATRIX  0xC6
#define CMD_MAGNETROMETER_VECT  0xC7
#define CMD_ACCEL_ANG_ORIENT    0xC8
#define CMD_WRITE_ACEL_BIAS_COR 0xC9	
#define CMD_WRITE_GYRO_BIAS     0xCA
#define CMD_ACCEL_ANG_MAG_VECTO 0xCB
#define CMD_ACEL_ANG_MAG_VEC_OR 0xCC
#define CMD_CAPTURE_GYRO_BIAS   0xCD	
#define CMD_EULER_ANGLES        0xCE
#define CMD_EULER_ANGLES_ANG_RT 0xCF
#define CMD_TRANSFER_NONV_MEM   0xD0	
#define CMD_TEMPERATURES		0xD1	
#define CMD_GYRO_STAB_A_AR_MG   0xD2	
#define CMD_DELTA_ANGVEL_MAGV   0xD3
#define CMD_GET_DEVICE_ID		0xEA
#define CMD_WRITE_WORD_EEPROM   0xE4	
#define CMD_READ_WORD_EEPROM    0xE5	
#define CMD_FIRWARE_VERSION     0xE9	
#define CMD_STOP_CONTINIOUS     0xFA	
/* END CMD SET FOR 3DM_gx3 INERTIA DEVICES */

#define I3DMgx3_INSTANT    1
#define I3DMgx3_STABILIZED 2
#define I3DMgx3_GYROSCALE_ADDRESS 130
#define I3DMgx3_GYROGAINSCALE 64

#define MAX_DEVICENUM 512

typedef struct _C1RawRecord /* struct for \xC1 6 position float and dword timer */
{
	float Accel[3]; /*Accel  x y z  */
	float AngRt[3]; /*Ang Rate x y z  */
	DWORD timer;
} C1RawRecord;

typedef struct _C2Accel_AngRecord /* struct for \xC1 6 position float and dword timer */
{
	float Accel[3]; /*Accel  x y z  */
	float AngRt[3]; /*Ang Rate x y z  */
	DWORD timer;
} C2Accel_AngRecord;

typedef struct _C3Delta_AngRecord /* struct for \xC1 6 position float and dword timer */
{
	float Accel[3]; /*Accel  x y z  */
	float AngRt[3]; /*Ang Rate x y z  */
	DWORD timer;
} C3Delta_AngRecord;

typedef struct _C5OrientRecord /* struct for \xC5 9 position float and dword timer */
{
	float MatrixX[3]; /*Orientation Matrix X */
	float MatrixY[3]; /*Orientation Matrix Y */
	float MatrixZ[3]; /*Orientation Matrix Z  */
	DWORD timer;
} C5OrientRecord;

typedef struct _C6OrientUPRecord /* struct for \xC6 9 position float and dword timer */
{
	float MatrixX[3]; /*Orientation Matrix X */
	float MatrixY[3]; /*Orientation Matrix Y */
	float MatrixZ[3]; /*Orientation Matrix Z  */
	DWORD timer;
} C6OrientUPRecord;

typedef struct _C7ScaledMagRecord /* struct for \xC7 3 position float and dword timer */
{
	float Vector[3]; /*Orientation Matrix X */
	DWORD timer;
} C7ScaledMagRecord;

typedef struct _C8AclAngMagRecord /* struct for \xC8 15 position float and dword timer */
{
	float Accel[3];   /*Accel  x y z  */
	float AngRt[3];   /*Ang Rate x y z  */
	float MatrixX[3]; /*Orientation Matrix X */
	float MatrixY[3]; /*Orientation Matrix Y */
	float MatrixZ[3]; /*Orientation Matrix Z */
	DWORD timer;
}C8AclAngMagRecord;

typedef struct _CAWriteGyroRecord
{
	float Gyro[3];   /*Gyro  x y z  */
	DWORD timer;
}CAWriteGyroRecord;

typedef struct _CBAclAngMagVecRecord /* struct for \xCB 18 position float and dword timer */
{
	float Accel[3];   /*Accel  x y z  */
	float AngRt[3];   /*Ang Rate x y z  */
	float MagRt[3];   /*Mag Rate x y z  */
	float MatrixX[3]; /*Orientation Matrix X */
	float MatrixY[3]; /*Orientation Matrix Y */
	float MatrixZ[3]; /*Orientation Matrix Z */
	DWORD timer;
}CBAclAngMagVecRecord;

typedef struct _C8AclAngMagMaxRecord /* struct for \xCC 18 position float and dword timer */
{
	float Accel[3];   /*Accel  x y z  */
	float AngRt[3];   /*Ang Rate x y z  */
	float MagRt[3];   /*Mag Rate x y z  */
	float MatrixX[3]; /*Orientation Matrix X */
	float MatrixY[3]; /*Orientation Matrix Y */
	float MatrixZ[3]; /*Orientation Matrix Z */
	DWORD timer;
}CCAclAngMagMaxRecord;

typedef struct _CDCaptureGyroRecord
{
	float Gyro[3];   /*Gyro  x y z  */
	DWORD timer;
}CDCaptureGyroRecord;

typedef struct _CEEulerRecord
{
	float Euler[3];   /*Euler x y z  */
	DWORD timer;
}CEEulerRecord;

typedef struct _CFEulerAngRecord
{
	float Euler[3];   /*Euler x y z  */
	float AngRt[3];   /*Euler Angle rate x y z  */
	DWORD timer;
}CFEulerAngRecord;

typedef struct _D1TempRecord /* struct for \xD1  4 position 2Byte temp readings and timer  */
{
	short tempa;
	short tempx;
	short tempy;
	short tempz;
	DWORD timer;
} D1TempRecord;

typedef struct _D2GyroDeltaRecord  /* struct for \xD1 9 position float and dword timer */
{
	float Accel[3];   /*Accel  x y z  */
	float AngRt[3];   /*Ang Rate x y z  */
	float MagRt[3];   /*Mag Rate x y z  */
	DWORD timer;
}D2GyroDeltaRecord;

typedef struct _D3DeltaAngVelRecord  /* struct for \xD1 9 position float and dword timer */
{
	float Angle[3];   /*Delta Angle  x y z  */
	float Veloc[3];   /*Delta Velocity Rate x y z  */
	float MagVe[3];   /*Delta Mag vector Rate x y z  */
	DWORD timer;
}D3DeltaAngVelRecord;
	  
typedef struct _I3dmgx3set /* struct for 18 position float and dword timer */
{
	float setA[3]; /*First  x y z  -or- First  Matrix set m1.1 m1.2 m1.3 */
	float setB[3]; /*Second x y z  -or- Second Matrix set m2.1 m2.2 m2.3 */
	float setC[3]; /*Third  x y z  -or- Third  Matrix set m3.1 m3.2 m3.3 */
	float setD[3]; /*First  Matrix set m1.1 m1.2 m1.3 */
	float setE[3]; /*Second Matrix set m2.1 m2.2 m2.3 */
	float setF[3]; /*Third  Matrix set m3.1 m3.2 m3.3 */
	DWORD timer;
} I3dmgx3Set;
/*----------------------------------------------------------------------
 * Sensor communication function prototypes.
 *----------------------------------------------------------------------*/

int i3dmgx3_mapDevice(int, int);
int i3dmgx3_sendCommand(int , char, char *, int);
int ReadNextRecord(int, I3dmgx3Set* pRecord);  /* reads in continuous mode */
void i3dmgx3_closeDevice(int);
int i3dmgx3_openPort(int, int, int, int, int, int, int);

/*----------------------------------------------------------------------
 * 3DM-gx3 Command Function prototypes
 *
 *----------------------------------------------------------------------*/

int i3dmgx3_initGyroScale(int);
int i3dmgx3_RawSensor(int, unsigned char *);				                //0xC1
int i3dmgx3_AccelAndAngRate(int, unsigned char* );		        //0xC2
int i3dmgx3_DeltaAngAndVelocity(int, unsigned char* );	        //0xC3
int i3dmgx3_OrientMatrix(int, unsigned char* );			        //0xC5
int i3dmgx3_OrientUpMatrix(int, unsigned char* );		        //0xC6
int i3dmgx3_ScaledMagVec(int, unsigned char* );		 	        //0xC7
int i3dmgx3_AccelAngOreint(int, unsigned char* );			    //0xC8
int i3dmgx3_WriteBias(int, BYTE *pBuff, unsigned char*);        //0xCA
int i3dmgx3_AccAngMagRate(int, unsigned char* );                //0xCB
int i3dmgx3_GetFullMatrix(int, unsigned char* );		   	    //0xCC
int i3dmgx3_captureGyroBias(int, short, unsigned char *, unsigned char* );//0xCD pBuff
int i3dmgx3_EulerAngles(int, unsigned char* );			        //0xCE
int i3dmgx3_EulerAngRate(int, unsigned char*);			        //0xCF
int i3dmgx3_TransferNonVolatile(int, int, unsigned char *);		//0xD0
int i3dmgx3_Tempatures(int, unsigned char*);				    //0xD1
int i3dmgx3_GyroStab(int, unsigned char*);			  	        //0xD2
int i3dmgx3_DeltaAngVel(int, unsigned char*);			        //0xD3
int i3dmgx3_getDeviceIdentiy(int, char, unsigned char*);		//0xEA
int i3dmgx3_writeEEPROMValue(int, char, int, int, int *);		//0xE4
int i3dmgx3_getEEPROMValue(int, char, int, int *);				//0xE5
int i3dmgx3_getFirmwareVersion(int, char *);			        //0xE9
  
/*-------------- end of i3dmgx3_Cmd.h ----------------------*/
