//Example code for interfacing with the Microstrain 3DM-GX3-25 Sensor

/* compile using: gcc -o imu_cc imu_cc.c -lm

  Once compiled the desired device can be specified using a command line argument:

./BINFILENAME /dev/ttyACM0

  or the program will scan for attached devices by name. The 3DM-GX3-25 will usually
  show up in /dev/ttyACM0  to ttyACM# where # represents the device number by the
  order the devices were attached

*/

#include <termios.h> // terminal io (serial port) interface
#include <fcntl.h>   // File control definitions
#include <errno.h>   // Error number definitions
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <math.h>
#include <fstream>

#define TRUE 1
#define FALSE 0

typedef int ComPortHandle;
typedef unsigned char Byte;

/*----------------------------------------------------*/
int TestByteOrder();
float Bytes2Float(const unsigned char*);
unsigned long Bytes2Ulong(unsigned char*);
#define PI 3.14159265359

typedef struct _CC_AAMM
{
	float Accel[3]; 	/* Accel        x y z   */
	float AngRate[3]; 	/* Ang Rate     x y z   */
	float Mag[3];		/* Magnetomer   x y z   */
	float M1[3];		/* M(1,1) M(1,2) M(1,3) */
	float M2[3];		/* M(2,1) M(2,2) M(2,3) */
	float M3[3];		/* M(3,1) M(3,2) M(3,3) */
	unsigned long timer;	/* Timer in Seconds     */
} CC_AAMM;
/*----------------------------------------------------*/

// Utility functions for working with a com port in Linux

// Purge
// Clears the com port's read and write buffers

int Purge(ComPortHandle comPortHandle){

  if (tcflush(comPortHandle,TCIOFLUSH)==-1){

    printf("flush failed\n");
    return FALSE;

  }

  return TRUE;

}

// OpenComPort
// Opens a com port with the correct settings for communicating with a MicroStrain 3DM-GX3-25 sensor

ComPortHandle OpenComPort(const char* comPortPath){

  ComPortHandle comPort = open(comPortPath, O_RDWR | O_NOCTTY);

  if (comPort== -1){ //Opening of port failed

    printf("Unable to open com Port %s\n Errno = %i\n", comPortPath, errno);
    return -1;

  }

  //Get the current options for the port...
  struct termios options;
  tcgetattr(comPort, &options);

  //set the baud rate to 115200
  int baudRate = B115200;
  cfsetospeed(&options, baudRate);
  cfsetispeed(&options, baudRate);

  //set the number of data bits.
  options.c_cflag &= ~CSIZE;  // Mask the character size bits
  options.c_cflag |= CS8;

  //set the number of stop bits to 1
  options.c_cflag &= ~CSTOPB;

  //Set parity to None
  options.c_cflag &=~PARENB;

  //set for non-canonical (raw processing, no echo, etc.)
  options.c_iflag = IGNPAR; // ignore parity check close_port(int
  options.c_oflag = 0; // raw output
  options.c_lflag = 0; // raw input

  //Time-Outs -- won't work with NDELAY option in the call to open
  options.c_cc[VMIN]  = 0;   // block reading until RX x characers. If x = 0, it is non-blocking.
  options.c_cc[VTIME] = 100;   // Inter-Character Timer -- i.e. timeout= x*.1 s

  //Set local mode and enable the receiver
  options.c_cflag |= (CLOCAL | CREAD);

  //Purge serial port buffers
  Purge(comPort);

  //Set the new options for the port...
  int status=tcsetattr(comPort, TCSANOW, &options);

  if (status != 0){ //For error message

    printf("Configuring comport failed\n");
    return status;

  }

  //Purge serial port buffers
  Purge(comPort);

  return comPort;

}

// CloseComPort
// Closes a port that was previously opened with OpenComPort
void CloseComPort(ComPortHandle comPort){

  close(comPort);

}

// readComPort
// read the specivied number of bytes from the com port
int readComPort(ComPortHandle comPort, Byte* bytes, int bytesToRead){

  int bytesRead = read(comPort, bytes, bytesToRead);
  return bytesRead;

}

// writeComPort
// send bytes to the com port
int writeComPort(ComPortHandle comPort, unsigned char* bytesToWrite, int size){

  return write(comPort, bytesToWrite, size);

}

// Simple Linux Console interface function

// CommandDialog
// Prompts user for device commands and returns reply from device
int CommandDialog(ComPortHandle comPort, unsigned int command){

//  unsigned int command;
  unsigned char ccommand;
  int i=0;
  int size;
  Byte response[4096] = {0};

//  printf("\nEnter command in hexadecimal format, valid commands range from c1 to fe (00 to EXIT)\n");
//  printf("(SEE: 3DM-GX3® Data Communications Protocol Manual for more information):\n");

//  scanf("%x", &command);//takes 1 byte command in hexadecimal format
//  printf("%x 1\n", command);

//  command = 0xdf;
//  printf("%x 2\n", command);
  ccommand=(char)command;
  if(command==0x00)//command to exit program
    return FALSE;
  else
    writeComPort(comPort, &ccommand, 1);//write command to port
  //getchar();//flush keyboard buffer
  Purge(comPort);//flush port

  size = readComPort(comPort, &response[0], 4096);

  if(size<=0){
    printf("No data read from previous command.\n");
    return TRUE;
  }
  else{
//    printf("Data returned from device:\n");
    while(size>0){//loop to read until no more bytes in read buffer
      if(size<0){
       printf("BAD READ\n");
       return TRUE;
      }
      else{

	/* --------------------------Print Command 0xC2 Numerical Values------------------------------- */
	CC_AAMM CC_Data;
        for(i=0;i<3;i++)
	{
		CC_Data.Accel[i] = Bytes2Float(&response[1 + i*4]);
		CC_Data.AngRate[i] = Bytes2Float(&response[13 + i*4]);
		CC_Data.Mag[i] = Bytes2Float(&response[25 + i*4]);
		CC_Data.M1[i] = Bytes2Float(&response[37 + i*4]);
		CC_Data.M2[i] = Bytes2Float(&response[49 + i*4]);
		CC_Data.M3[i] = Bytes2Float(&response[61 + i*4]);
        }

	//Format [accelX],[accelY],[accelZ],[AngRateX],[AngRateY],[AngRateZ],[magX],[magY],[magZ],[mtx11],[mtx12],[mtx13],[mtx21],[mtx22],[mtx23],[mtx31],[mtx32],[mtx33],[pitchDeg],[rollDeg],[yawDeg],[headingDeg],[timestamp]
	printf("%f,%f,%f,", CC_Data.Accel[0], CC_Data.Accel[1], CC_Data.Accel[2]);
	printf("%f,%f,%f,", (180.0/PI)*CC_Data.AngRate[0], (180.0/PI)*CC_Data.AngRate[1], (180.0/PI)*CC_Data.AngRate[2]);
	printf("%f,%f,%f,", CC_Data.Mag[0], CC_Data.Mag[1], CC_Data.Mag[2]);
	printf("%f,%f,%f,", CC_Data.M1[0], CC_Data.M1[1], CC_Data.M1[2]);
	printf("%f,%f,%f,", CC_Data.M2[0], CC_Data.M2[1], CC_Data.M2[2]);
	printf("%f,%f,%f,", CC_Data.M3[0], CC_Data.M3[1], CC_Data.M3[2]);

		float mag_heading;
	if(CC_Data.Mag[1] > 0) {
		mag_heading = 90.0-(180.0/PI)*atan(CC_Data.Mag[0]/CC_Data.Mag[1]);
	}

	if(CC_Data.Mag[1] < 0) {
		mag_heading = 270.0-(180.0/PI)*atan(CC_Data.Mag[0]/CC_Data.Mag[1]);
	}

	if(CC_Data.Mag[1]==0 && CC_Data.Mag[0]<0) {
		mag_heading = 180.0;
	}

	if(CC_Data.Mag[1]==0 && CC_Data.Mag[0]>0) {
		mag_heading = 0.0;
	}

	mag_heading = 360.0-mag_heading;

	float pitch = (180.0/PI)*asin(-CC_Data.M1[2]);
	float roll = (180.0/PI)*atan(CC_Data.M2[2]/CC_Data.M3[2]);
	float yaw = (180.0/PI)*atan(CC_Data.M1[1]/CC_Data.M1[0]);
	printf("%f,", pitch);
	printf("%f,", roll);
	printf("%f,", yaw);
	printf("%f,", mag_heading);

	CC_Data.timer = Bytes2Ulong(&response[73]);
	float sec;
	sec = (float) (CC_Data.timer/262144.0);
	printf("%f\n", sec);
	/* -------------------------------------------------------------------------------------------- */

        return TRUE;
      }
      size = readComPort(comPort, &response[0], 4096);
    }
  }
}
//scandev
//finds attached microstrain devices and prompts user for choice then returns selected portname
char* scandev(){
  FILE *instream;
  char devnames[255][255];//allows for up to 256 devices with path links up to 255 characters long each
  int devct=0; //counter for number of devices
  int i=0;
  int j=0;
  int userchoice=0;
  char* device;

  std::string command = "find /dev/serial -print | grep -i microstrain";//search /dev/serial for microstrain devices
  //printf("Searching for devices...\n");

  instream=popen(command.c_str(), "r");//execute piped command in read mode

  if(!instream){//SOMETHING WRONG WITH THE SYSTEM COMMAND PIPE...EXITING
    printf("ERROR BROKEN PIPELINE %s\n", command.c_str());
    return device;
  }

  for(i=0;i<255&&(fgets(devnames[i],sizeof(devnames[i]), instream));i++){//load char array of device addresses
    ++devct;
  }

  for(i=0;i<devct;i++){
    for(j=0;j<sizeof(devnames[i]);j++){
      if(devnames[i][j]=='\n'){
        devnames[i][j]='\0';//replaces newline inserted by pipe reader with char array terminator character
        break;//breaks loop after replacement
      }
    }
    //printf("Device Found:\n%d: %s\n",i,devnames[i]);
  }

  //CHOOSE DEVICE TO CONNECT TO AND CONNECT TO IT (IF THERE ARE CONNECTED DEVICES)

  if(devct>0){
    //printf("Number of devices = %d\n", devct);
    if(devct>1){
      printf("Please choose the number of the device to connect to (0 to %i):\n",devct-1);
        while(scanf("%i",&userchoice)==0||userchoice<0||userchoice>devct-1){//check that there's input and in the correct range
          printf("Invalid choice...Please choose again between 0 and %d:\n", devct-1);
          getchar();//clear carriage return from keyboard buffer after invalid choice
        }
    }
    device=devnames[userchoice];
    return device;

  }
  else{
    printf("No MicroStrain devices found.\n");
    return device;
  }

}

/* ----------------------------------Main----------------------------------- */
int main(int argc, char* argv[]){

  ComPortHandle comPort;
  int go = TRUE;
  char* dev;
  char a;

  a='\0';

  dev=&a;

  if(argc<2) {
    //No port specified at commandline so search for attached devices
    dev=scandev();
    if(strcmp(dev,"")!=0) {
      //printf("Attempting to open port...%s\n",dev);
      comPort = OpenComPort(dev);

    }
    else {
      printf("Failed to find attached device.\n");
      return FALSE;
    }
  }
  else {
    //Open port specified at commandline
    printf("Attempting to open port...%s\n",argv[1]);
    comPort = OpenComPort(argv[1]);
  }

  if(comPort > 0) {
    unsigned int command;
    command = 0xcc;
    CommandDialog(comPort,command);
    CloseComPort(comPort);
  }

  return 0;
}
/* ---------------------------------------------------------------------------*/

/* ----------------------------------Utility Functions---------------------------------- */
float Bytes2Float(const unsigned char* pBytes)
{
	float f = 0;
	if(TestByteOrder() != FALSE) {
	   ((Byte*)(&f))[0] = pBytes[3];
	   ((Byte*)(&f))[1] = pBytes[2];
	   ((Byte*)(&f))[2] = pBytes[1];
	   ((Byte*)(&f))[3] = pBytes[0];
	}else{
	   ((Byte*)(&f))[0] = pBytes[0];
	   ((Byte*)(&f))[1] = pBytes[1];
	   ((Byte*)(&f))[2] = pBytes[2];
	   ((Byte*)(&f))[3] = pBytes[3];
	}

	return f;
}

unsigned long Bytes2Ulong(unsigned char* plByte)
{
	unsigned long ul = 0;
	if(TestByteOrder() != FALSE) {
	   ul = (plByte[0] <<24) + (plByte[1] <<16) + (plByte[2] <<8) + (plByte[3] & 0xFF);
	}else{
		ul = (unsigned long)plByte;
	}
  	return ul;
}

int TestByteOrder()
{
   short int word = 0x0001;
   char *byte = (char *) &word;
   return(byte[0] ? TRUE : FALSE);
}
/* ------------------------------------------------------------------------------------- */
