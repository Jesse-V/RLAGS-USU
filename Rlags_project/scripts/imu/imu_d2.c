//Example code for interfacing with the Microstrain 3DM-GX3-25 Sensor

/* compile using: gcc -o imu_d2 imu_d2.c -lm

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
#include <string.h>
#include <math.h>


#define TRUE 1
#define FALSE 0

typedef int ComPortHandle;
typedef unsigned char Byte;

/*----------------------------------------------------*/
int TestByteOrder();
float Bytes2Float(const unsigned char*);
unsigned long Bytes2Ulong(unsigned char*);
#define PI 3.14159265359

typedef struct _D2_Stab_AAM
{
	float StabAccel[3];	/* Accel      x y z */
	float AngRate[3]; 	/* Ang Rate   x y z */
	float StabMag[3]; 	/* Magnetomer x y z */
	unsigned long timer;	/* Timer in Seconds */
} D2_Stab_AAM;
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
	/* --------------------------Print Command 0xD2 Numerical Values------------------------------- */
	D2_Stab_AAM D2_Data;
        for(i=0;i<3;i++)
	{
		D2_Data.StabAccel[i] = Bytes2Float(&response[1 + i*4]);		
		D2_Data.AngRate[i] = Bytes2Float(&response[13 + i*4]);
		D2_Data.StabMag[i] = Bytes2Float(&response[25 + i*4]);
        }
	printf("\n\tGyro Stabilized Acceleration, Angular Rate & Magnetometer\n");
	printf("\t-------------------------------------------------------------------\n");
	printf("\n\tStab Accel X\t\tStab Accel Y\t\tStab Accel Z\n");
	printf("\t-------------------------------------------------------------------\n");
	printf("\t%f (G)\t\t%f (G)\t\t%f (G)\n\n", D2_Data.StabAccel[0], D2_Data.StabAccel[1], D2_Data.StabAccel[2]);
	printf("\n\tAng Rate X\t\tAng Rate Y\t\tAng Rate Z\n");
	printf("\t-------------------------------------------------------------------\n");
	printf("\t%f (deg/sec)\t%f (deg/sec)\t%f (deg/sec)\n\n", (180.0/PI)*D2_Data.AngRate[0], (180.0/PI)*D2_Data.AngRate[1], (180.0/PI)*D2_Data.AngRate[2]);
	printf("\n\tStab Magneto X\t\tStab Magneto Y\t\tStab Magneto Z\n");
	printf("\t-------------------------------------------------------------------\n");
	printf("\t%f (Gauss)\t%f (Gauss)\t%f (Gauss)\n\n", D2_Data.StabMag[0], D2_Data.StabMag[1], D2_Data.StabMag[2]);
	
	float north;
	if(D2_Data.StabMag[1] > 0) {
		north = 90.0-(180.0/PI)*atan(D2_Data.StabMag[0]/D2_Data.StabMag[1]);
	}

	if(D2_Data.StabMag[1] < 0) {
		north = 270.0-(180.0/PI)*atan(D2_Data.StabMag[0]/D2_Data.StabMag[1]);
	}
	
	if(D2_Data.StabMag[1]==0 && D2_Data.StabMag[0]<0) {
		north = 180.0;
	}

	if(D2_Data.StabMag[1]==0 && D2_Data.StabMag[0]>0) {
		north = 0.0;
	}

	north = 360.0-north;
	
	D2_Data.timer = Bytes2Ulong(&response[37]);
	float sec;
	sec = (float) (D2_Data.timer/262144.0);
	printf("\n\t-------------------------------------------------------------------");
	printf("\n\tHeading: %f (deg)\n", north);
	printf("\n\tTime Stamp: %f (sec)\n", sec);
	printf("\t-------------------------------------------------------------------\n\n");
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

  char *command = "find /dev/serial -print | grep -i microstrain";//search /dev/serial for microstrain devices
  //printf("Searching for devices...\n");

  instream=popen(command, "r");//execute piped command in read mode

  if(!instream){//SOMETHING WRONG WITH THE SYSTEM COMMAND PIPE...EXITING
    printf("ERROR BROKEN PIPELINE %s\n", command);
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
    command = 0xd2;
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
