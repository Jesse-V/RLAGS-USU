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
 * i3dmgx3_Cont.h
 *
 * Definitions for the 3dm-gx3 inertia sensor devices
 * The continuous mode functions are supported by this adapter.
 *----------------------------------------------------------------------*/
#include <stdio.h>

/* START CMD SET FOR 3DM_GX3 INERTIA DEVICES */
#define CMD_WIRELESS_PING       0x02
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
#define D4CHECK   0xD4
#define D5CHECK   0xD5

/* END CMD SET FOR 3DM_GX3 INERTIA DEVICES */

#define I3DMGX3_INSTANT    1
#define I3DMGX3_STABILIZED 2

#define MAX_DEVICENUM 16
typedef unsigned short WORD;
typedef unsigned char  BYTE;
typedef unsigned long  DWORD;

typedef struct _C2Accel_AngRecord /* struct for \xC1 6 position float and dword timer */
{
	float Accel[3]; /*Accel  x y z  */
	float AngRt[3]; /*Ang Rate x y z  */
	DWORD timer;
} C2Accel_AngRecord;

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
int ReadNextRecord(int, I3dmgx3Set* pRecord, BYTE *);  /* reads in continuous mode */
int SendCommand(int, const char*, int);
void i3dmgx3_closeDevice(int);
int i3dmgx3_openPort(int, int, int, int, int, int, int);

/*----------------------------------------------------------------------
 * 3DM-GX3 Command Function prototypes
 *
 *----------------------------------------------------------------------*/
int i3dmgx3_AccelAndAngRate(int, BYTE*);		                //0xC2
int i3dmgx3_getDeviceIdentiy(int, char, char *);				//0xEA
int i3dmgx3_writeEEPROMValue(int, char, int, int, int *);		//0xE4
int i3dmgx3_getEEPROMValue(int, char, int, int *);				//0xE5
int i3dmgx3_getFirmwareVersion(int, char *);					//0xE9
int SetContinuousMode(int, unsigned char);
  
/*-------------- end of i3dmgx3_Cmd.h ----------------------*/
