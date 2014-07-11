/*
 * Arduino-serial
 * --------------
 *
 * A simple command-line example program showing how a computer can
 * communicate with an Arduino board. Works on any POSIX system (Mac/Unix/PC)
 *
 *
 * Compile with something like:
 * gcc -o arduino-serial arduino-serial.c
 *
 * Created 5 December 2006
 * Copyleft (c) 2006, Tod E. Kurt, tod@todbot.com
 * http://todbot.com/blog/
 *
 *
 * Updated 8 December 2006:
 *  Justin McBride discoevered B14400 & B28800 aren't in Linux's termios.h.
 *  I've included his patch, but commented out for now.  One really needs a
 *  real make system when doing cross-platform C and I wanted to avoid that
 *  for this little program. Those baudrates aren't used much anyway. :)
 *
 * Updated 26 December 2007:
 *  Added ability to specify a delay (so you can wait for Arduino Diecimila)
 *  Added ability to send a binary byte number
 *
 */

#include <stdio.h>    /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>   /* Standard types */
#include <string.h>   /* String function definitions */
#include <unistd.h>   /* UNIX standard function definitions */
#include <fcntl.h>    /* File control definitions */
#include <errno.h>    /* Error number definitions */
#include <termios.h>  /* POSIX terminal control definitions */
#include <sys/ioctl.h>
#include <getopt.h>
#include <fstream>
#include <ctime>
#include <sstream>
#include <thread>
#include <cmath>
#include <iostream>

// void usage(void);
void getAngle(void);
bool is_number(const std::string& s);
int serialport_init(const char* serialport, int baud);
int serialport_writebyte(int fd, uint8_t b);
int serialport_write(int fd, const char* str);
int serialport_read_until(int fd, char* buf, char until);

bool shouldTerminate = false;
float servoAngle = 90.0;
bool angleUpdated = true;

// void usage(void) {
//     printf("Usage: arduino-serial -p <serialport> [OPTIONS]\n"
//     "\n"
//     "Options:\n"
//     "  -h, --help                   Print this help message\n"
//     "  -p, --port=serialport        Serial port Arduino is on\n"
//     "  -b, --baud=baudrate          Baudrate (bps) of Arduino\n"
//     "  -s, --send=data              Send data to Arduino\n"
//     "  -r, --receive                Receive data from Arduino & print it out\n"
//     "  -n  --num=num                Send a number as a single byte\n"
//     "  -d  --delay=millis           Delay for specified milliseconds\n"
//     "\n"
//     "Note: Order is important. Set '-b' before doing '-p'. \n"
//     "      Used to make series of actions:  '-d 2000 -s hello -d 100 -r' \n"
//     "      means 'wait 2secs, send 'hello', wait 100msec, get reply'\n"
//     "\n");
// }

int main(int argc, char *argv[])
{
    printf("starting program..\n");
    int fd = 0;
    char serialport[256];
    int baudrate = 19200;  // default
    char buf[256], dat[256], use[1];
    int rc,n;

    //baudrate = 19200;
    printf("initializing serial communication...");
    fd = serialport_init("/dev/ttyACM0", baudrate);
                if(fd==-1) return -1;
    printf("Done\n");
    usleep(1000 * 1000);

    printf("Spawning angle update thread..\n");
    std::thread t(getAngle);

    printf("Starting data collect..\n");
    n = serialport_read_until(fd, buf, ':');
    std::ofstream outFile;
    std::stringstream fileName;
    time_t timer;
    struct tm* y2k;
    time (&timer);
    y2k = localtime(&timer);
	fileName << "RLAGS_Data/RLAGS_" << y2k->tm_mon << "_" << y2k->tm_mday << "_" << y2k->tm_hour << "_" << y2k->tm_min << "_" << y2k->tm_sec << ".txt";
	while(!shouldTerminate) {
		strcpy(dat, "00000000:\0");
		//gets(use);
//		rc = serialport_write(fd, dat);
  //          		if(rc==-1) return -1;
	 	//printf("Waiting until UART buffer clears: %d\n", tcdrain(fd));
	 	n = serialport_read_until(fd, buf, ':');
     	printf("%s", buf);
		outFile.open(fileName.str().c_str(), std::ofstream::out | std::ofstream::app);
		outFile << buf;
		outFile.close();
        if(angleUpdated)
        {
            // sleep(2);
            // tcflush( fd, TCIFLUSH );
            serialport_writebyte(fd, servoAngle);
            angleUpdated = false;
            // if(blah >= 180)
            //     blah = 0;
            // else
            //     blah += 45;
            // update = false;
        }


		usleep(1000);
//         	printf("wrote %d bytes, read %d bytes: %s\n", rc, n, buf);
	}
    t.join();
	close(fd);

    exit(EXIT_SUCCESS);
} // end main

bool is_number(std::string s)
{
    return !s.empty() && s.find_first_not_of("0123456789") == std::string::npos;
}

void getAngle()
{
    while(!shouldTerminate)
    {
        float angle = 0;
        std::cin.clear();
        std::cin >> angle;
        // if(!std::cin)
        // {
        int newAngle = round(angle);
        if(newAngle == 181)
        {
            newAngle--;
            servoAngle = newAngle;
            angleUpdated = true;
        }
        if(newAngle > 180 || newAngle < 0)
        {
            if(newAngle == 200 || newAngle == 201)
            {
                printf("Changing the calibration lamp states\n");
                servoAngle = newAngle;
                angleUpdated = true;
            }
            else
            {
                printf("\nAngles must be decimal values between 0-180 degrees\n");
                printf("\nCalibration Lamp ON: decimal value 200\n");
                printf("\nCalibration Lamp OFF: decimal value 201\n");
            }
        }
        else
        {
            servoAngle = newAngle;
            angleUpdated = true;
        }
        // }
        // else
        // {
        //     printf("\nMust be angle between 0-180 degrees\n");
        // }
    }
}

int serialport_writebyte(int fd, uint8_t b)
{
    int n = write(fd,&b,1);
    if( n!=1)
        return -1;
    return 0;
}

int serialport_write(int fd, const char* str)
{
    int len = strlen(str);
    int n = write(fd, str, len);
    if( n!=len )
        return -1;
    return n;
}

int serialport_read_until(int fd, char* buf, char until)
{
    char b[1];
    int i=0;
    do {
        int n = read(fd, b, 1);  // read a char at a time
//	printf("%c",n);
        if( n==-1) return -1;    // couldn't read
        if( n==0 ) {
            usleep( 10 * 1000 ); // wait 10 msec try again
            continue;
        }
        buf[i] = b[0]; i++;
    } while( b[0] != until );

    buf[i-1] = 0;  // null terminate the string
    return i;
}

// takes the string name of the serial port (e.g. "/dev/tty.usbserial","COM1")
// and a baud rate (bps) and connects to that port at that speed and 8N1.
// opens the port in fully raw mode so you can send binary data.
// returns valid fd, or -1 on error
int serialport_init(const char* serialport, int baud)
{
    struct termios toptions;
    int fd;

    //fprintf(stderr,"init_serialport: opening port %s @ %d bps\n",
    //        serialport,baud);

    //fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open(serialport, O_RDWR | O_NOCTTY);
    if (fd == -1)  {
        perror("init_serialport: Unable to open port ");
        return -1;
    }

    if (tcgetattr(fd, &toptions) < 0) {
        perror("init_serialport: Couldn't get term attributes");
        return -1;
    }
    speed_t brate = baud; // let you override switch below if needed
    switch(baud) {
    case 4800:   brate=B4800;   break;
    case 9600:   brate=B9600;   break;
// if you want these speeds, uncomment these and set #defines if Linux
//#ifndef OSNAME_LINUX
//    case 14400:  brate=B14400;  break;
//#endif
    case 19200:  brate=B19200;  break;
//#ifndef OSNAME_LINUX
//    case 28800:  brate=B28800;  break;
//#endif
    //case 28800:  brate=B28800;  break;
    case 38400:  brate=B38400;  break;
    case 57600:  brate=B57600;  break;
    case 115200: brate=B115200; break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    // toptions.c_cflag &= ~CRTSCTS;

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 1;
    toptions.c_cc[VTIME] = 20;

    sleep(2);
    tcflush( fd, TCIFLUSH );
    if( tcsetattr(fd, TCSANOW, &toptions) < 0) {
        perror("init_serialport: Couldn't set term attributes");
        return -1;
    }

    return fd;
}

