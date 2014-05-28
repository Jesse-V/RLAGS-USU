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
 * i3dmgx3_Main_CM.c
 *
 * Main for the 3DM-GX3 Adapter continious/poll data console application.
 *--------------------------------------------------------------------*/		
#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include "i3dmgx3_Errors.h"
#include "i3dmgx3_Cont.h"
#include "i3dmgx3_Utils.h"
#include "i3dmgx3_Utils_CM.h"

int GetComPort(); //prompt user for comport and opens it
void PollData(int portNum); 
void ReadContinuousData(int portNum); //puts the node in continuous mode and retrives and prits data until user interups
void ReadContinuousLogData(int portNum);
void ReadMultC2CBD3(int portNum);
void ReadOscC2CB(int portNum);
int OnGetSerialPorts();

int main(int argc, char* argv[])
{
	BOOL bQuit = FALSE;
	BOOL bPrintHeader = TRUE;
      int portNum = 0;
	int Ccount = 0;
	int  tryPortNum = 1;
	//I3dmgx3Set pRecord;
	printf("\n   3DM-GX3 Continuous Mode \n");

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
		    exit(1);
	        }

        }else{
          portNum=OnGetSerialPorts();
          if(portNum<0)
             exit(1);

        }
	printf("\n   Using COM Port #%d \n", portNum);

   
	while(!bQuit){
		int chOption = 0;

		if(bPrintHeader)
		{
			printf("\n");
			printf("Enter an Option: (P)oll (C)ontinuous (L)og_Continuous (Q)uit\n");
			printf("P Poll           - Poll the node for a single record of data and print it\n");
			printf("C Continuous     - Put the node in continuous mode and print each record\n");
			printf("L Log Continuous - Put the node in continuous mode log and print each record\n");
			printf("Q Quit           - Quit the application\n");

			bPrintHeader = FALSE;
		}

		//read option from the keyboard
		while(!ReadCharNoReturn(&chOption))
		{
			Sleep(50);
		}

		//
		switch(chOption)
		{
			case 'p':
			case 'P': PollData(portNum); bPrintHeader = TRUE; break;

			case 'C':
			case 'c': ReadContinuousData(portNum); bPrintHeader = TRUE; break;

			case 'L':
			case 'l': 
				      ReadContinuousLogData(portNum); 
				      bPrintHeader = TRUE; 
				      break;

			case 'q':
			case 'Q': bQuit = TRUE; break;

			case 'h':
			case 'H': bPrintHeader = TRUE; break;

			default: printf("Invalid Option\n\n"); bPrintHeader = TRUE;
		}
	}
				
	return 0;
}

//===========================================================================
// GetComPort
//---------------------------------------------------------------------------
// Description: Prompt user for the comport and then opens it.  The user is 
//              prompted until a valid comport is selected and successfully 
//              opened.
//
// Return: HANDLE - handle to the opened comport
//===========================================================================
int GetComPort()
{
	int portNum   = 0;
	int iComPort  = 0;
	int errCount = 0;
	int MaxFail = 5;
	//Get comport number ask user for the comport number and open it
	while(portNum == 0)
	{
		printf("Enter Comport to use:");
		scanf("%d", &iComPort);
		if (iComPort < 2 || iComPort > 256) {
			printf("   Please enter a valid port number\n");
			printf("        valid ports are 2..256\n");
		}
		else {
		     /* open a port, map a device */
             portNum = i3dmgx3_openPort(iComPort, 115200, 8, 0, 1, 1024, 1024);
             if (portNum<0) {
                 printf("port open failed. ");
                 printf("Comm error %d, %s:\n", portNum, explainError(portNum));
			     if (errCount++ >= MaxFail)
				     exit(-1);
			     iComPort = 0;
		     }
		     else {
                 printf("\n   Using Comm Port #%d \n", portNum);
		     }
		}
	} 

	return portNum;
}

//===========================================================================
// PollData
//---------------------------------------------------------------------------
// Description: Polls the node for a single record of sensor data and prints
//              it to the screen.
//
// Return: HANDLE - handle to the opened comport
//===========================================================================
void PollData(int portNum)
{
	int status = 0, i = 0;
	BYTE Record[31];
	C2Accel_AngRecord   Accel_AngRecord;

	status = i3dmgx3_AccelAndAngRate(portNum, &Record[0]);
	if(status == 0)
	{
		for (i=0; i<3; i++) {
			Accel_AngRecord.Accel[i] = FloatFromBytes(&Record[1 + i*4]); 
			Accel_AngRecord.AngRt[i] = FloatFromBytes(&Record[13 + i*4]);
		}
		printf("\n");
   		printf("Acceleration X:%f  \tY:%f\tZ:%f\n", Accel_AngRecord.Accel[0], Accel_AngRecord.Accel[1], Accel_AngRecord.Accel[2]);
		printf("Angular Rate X:%f  \tY:%f\tZ:%f\n", Accel_AngRecord.AngRt[0], Accel_AngRecord.AngRt[1], Accel_AngRecord.AngRt[2]);
		Accel_AngRecord.timer = convert2ulong(&Record[25]);
	    //printf("\n Timer Stamp: %u\n", Accel_AngRecord.timer);
	}
	else
	{
		printf("PollData() Failed\n");
		printf("Error Code: %d\n", status); 
	}
}
/*===========================================================================
* ReadContinuousData
*---------------------------------------------------------------------------
* Description: Puts the node in continuous mode, reads the sensor data and
*              prints to the screen until the user interrupts.
*
* Return: HANDLE - handle to the opened comport
*===========================================================================*/
void ReadContinuousData(int portNum)
{
	int iCount = 0;
	BOOL bStopContinuous = FALSE;
	DWORD dwcharsRead = 0;
	I3dmgx3Set Record;
	BYTE cmd_return;
	int Curs_posY = 0;
	int Curs_posX = 0;
	int status    = 0;
	int error_record = 0;
	char consoleBuff[60] = {0};
	long valid_rec = 0;
	unsigned char error_cmd;
	
	//put the node in continuous mode
	status =SetContinuousMode(portNum, 0xC2);
	printf("setcontinuous is %d", status);
	//set up the output for the data
	printf("\n\n");
	printf("Reading streaming data (hit s to Stop streaming).\n");
	printf("C2___________________________Acceleration_______________________________\n");
	printf("            X                       Y                      Z            \n");
	/*  acceleration values go here, save the position */
	printf("\n\n\n"); 
	printf("C2____________________________Angular_Rate______________________________\n");
	printf("            X                       Y                      Z            \n");
	/* angle rate values go here, save the position */
	getConXY(&Curs_posX, &Curs_posY); 
	printf("\n\n\n\n");
	
	//continue until the user hits the s key
	while(!bStopContinuous)
	{
		if(ReadNextRecord(portNum, &Record, &cmd_return) != SUCCESS)
			error_record++;
			if (cmd_return == 0xC2){
			//move to the acceleration position and print the data
		    sprintf(consoleBuff, "\t%2.6f\t\t%2.6f\t\t%2.6f", Record.setA[0], Record.setA[1], Record.setA[2]);
		    setConXY(Curs_posX, Curs_posY -5, &consoleBuff[0]);
			sprintf(consoleBuff, "\t%2.6f\t\t%2.6f\t\t%2.6f", Record.setB[0], Record.setB[1], Record.setB[2]);
		    setConXY(Curs_posX, Curs_posY -0, &consoleBuff[0]);
			valid_rec++;
			}else if (cmd_return != 0xc4){
				if((cmd_return == 0xCB || cmd_return == 0xD3) && error_record == 0)
				    error_cmd = cmd_return;
				else
					error_record++;
			}
			
		
		//check for a key every 50 iterations
		if(iCount++ > 50)	{
			int ch = 0;
			if(ReadCharNoReturn(&ch)){
				bStopContinuous = (ch == 's' || ch == 'S');
			}
			//reset the counter
			iCount = 0;
		}
	}
	printf("\n\n\nStopping Continuous Mode...");
	StopContinuousMode(portNum);
	printf("stopped.\n");
	if (error_record > 0)
	    printf("Number of records received in error were %d and received successfully %d\n", error_record, valid_rec);
	else
		printf("All %d records read successfully.\n", valid_rec);
}
/*===========================================================================
* ReadContinuousLogData
*---------------------------------------------------------------------------
* Description: Puts the node in continuous mode, reads the sensor data and
*              prints to the screen and logs data to user file until the user 
*              interrupts.
*
* Return: HANDLE - handle to the opened comport
*===========================================================================*/
void ReadContinuousLogData(int portNum)
{
	int iCount = 0;
	BOOL bStopContinuous = FALSE;
	DWORD dwcharsRead = 0;
	I3dmgx3Set Record;
	BYTE cmd_return;
	int Curs_posY = 0;
	int Curs_posX = 0;
	int status    = 0;
	int error_record = 0;
	int valid_check = 0;
	int valid_count = 0;
	char consoleBuff[60] = {0};
	long valid_rec = 0;
	unsigned char error_cmd;
	char ComLogFile[256]; 
	FILE *m_logFile;
	int LogFlag = 0;
	int error_count = 0;
	int errorCode =0, i=0;
	char fw[20] = {0};
    char sn[20] = {0};
	char mListSep[4];
    char mDecSep[4];
	LANGID langId;
	char szLanguage[256]; /*MAX_PATH]; */
	char idchar[] = {'\x02', '\x00', '\x01', '\x03', '\x04'};
	//SYSTEMTIME st;
	struct __timeb64 timebuffer;
    char *timeline;
	int m_timerconst = 62500; 
	unsigned long AA_Time_Stamp=0;
	unsigned long AA_s_prev =0;
	float AA_convert = 0.0;
	float AA_prev = 0.0;
    
    _ftime64( &timebuffer );
    timeline = _ctime64( & ( timebuffer.time ) );

	while (LogFlag != 1){
	    printf("Enter Name of LogFile to use:");
		scanf("%s", &ComLogFile);
		printf("logFile %s\n", ComLogFile);
		if( (m_logFile  = fopen(ComLogFile, "w" )) == NULL ){
			printf("File: %s not opened\n", ComLogFile);
			if(++error_count > 2)
				return;
		}else LogFlag = 1;	
	}
	
	fprintf(m_logFile, "[SESSION START TAG]\n");
	fprintf(m_logFile, "Session Start Time:%.19s.%hu \n", timeline, timebuffer.millitm );
	fprintf(m_logFile, "Time Source: HOST\n");
	GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,LOCALE_SLIST,mListSep,4);
	fprintf(m_logFile, "List Separator: %s\n", mListSep);
	GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,LOCALE_SDECIMAL,mDecSep,4);
        fprintf(m_logFile, "Decimal Separator: %s\n", mDecSep);
	langId = GetSystemDefaultLangID (); 
	 i = VerLanguageName (langId, szLanguage, 256);
         fprintf(m_logFile, "Language: %s\n", szLanguage);
	/* 0xE9  get firmware number (as a string) 
    *-----------------------------------------------------------------*/
	 //StopContinuousMode(portNum);
	// purge_port(portNum);
	//printf("\n 0xE9 Read Firmware Number \n");
    while(valid_check == 0)
	{
       errorCode = i3dmgx3_getFirmwareVersion(portNum, &fw[0]);
	   if (errorCode < 0){
		   purge_port(portNum); //purge_port
		   if (valid_count++ > 6) {
               printf("Please Halt Current Data Display and Retry, count %d portNumber %d errorCode %d\n", valid_count,portNum, errorCode);
		       return;
		   }
	   }else if (errorCode >= 0){
		   valid_check = 1;
	   }
	} 
	
   /*------------------------------------------------------------------
    * 0xEA  get serial information number, model and options (as string) 
    *-----------------------------------------------------------------*/
	valid_count =0;
    for ( i=0; i<4; i++){   //was 5 but no need to display opt 4
	
	   errorCode = i3dmgx3_getDeviceIdentiy(portNum, idchar[i], &sn[0]);
	   if (errorCode < 0){
		    purge_port(portNum);
			i--;
			if (valid_count++ >6){
			    printf("Error Read Device Identity: %s\n", explainError(errorCode));
			}
	   } else{
			 switch( i ) {
			       case 0:
					   //dlg.m_ModelNum = sn;
					   fprintf(m_logFile, "Device Name: %s\n",sn);
					   break;
				   case 1:
					   //dlg.m_ModelSerial = sn;
					   fprintf(m_logFile, "Device Model: %s\n",sn);
					   break;
				   case 2:
					  //dlg.m_ModelName = sn;
					   fprintf(m_logFile, "Device FirmWare Version: %s\n", fw);
					   fprintf(m_logFile, "Device Serial Number: %s\n", sn);
					   break;
				   case 3:
					 // dlg.m_ModelOpt = (CString)sn;
					   fprintf(m_logFile, "Device Options: %s\n", sn);
					   break;
				   case 4:
				   default:
					   break;
            }
		 }
	
	} 

	fprintf(m_logFile, "Command C2\n[DATA START TAG]\n\t Time%s Accel X%s Accel Y%s Accel Z%s AngRate X%s AngRate Y%s AngRate Z%s Ticks\n", mListSep, mListSep, mListSep, mListSep, mListSep, mListSep, mListSep);

	//put the node in continuous mode
	status =SetContinuousMode(portNum, 0xC2);
	printf("setcontinuous is %d", status);
	//set up the output for the data
	printf("\n\n");
	printf("Reading streaming data (hit s to Stop streaming).\n");
	printf("C2___________________________Acceleration_______________________________\n");
	printf("            X                       Y                      Z            \n");
	/*  acceleration values go here, save the position */
	printf("\n\n\n"); 
	printf("C2____________________________Angular_Rate______________________________\n");
	printf("            X                       Y                      Z            \n");
	/* angle rate values go here, save the position */
	getConXY(&Curs_posX, &Curs_posY); 
	printf("\n\n\n\n");
	
	//continue until the user hits the s key
	while(!bStopContinuous)
	{
		
		//GetLocalTime(&st);
		status = ReadNextRecord(portNum, &Record, &cmd_return);
		//if(ReadNextRecord(portNum, &Record, &cmd_return) != SUCCESS)
		if(status!= SUCCESS){
			fprintf(m_logFile, "error status %i  on cmd: %x\n",status, cmd_return);
			error_record++;
		}
			if (cmd_return == 0xC2){
				if (AA_s_prev == 0){
	               fprintf(m_logFile, "\t  0.00");
				    AA_s_prev = Record.timer; //AA_Time_Stamp;
		         }
                  else
                {
				 // AA_convert = ((float)(AA_Time_Stamp - AA_s_prev)/m_timerconst); //19660800);
				  AA_convert = ((float)(Record.timer - AA_s_prev)/m_timerconst); //19660800);
			      AA_s_prev = Record.timer; //AA_Time_Stamp;
				  AA_prev = AA_prev + AA_convert;
			      fprintf(m_logFile, "\t%6.2f", AA_prev);
			    }
			   //move to the acceleration position and print the data
			   //fprintf(m_logFile, "\t%02d.%03d", st.wSecond, st.wMilliseconds);
		         sprintf(consoleBuff, "\t%2.6f\t\t%2.6f\t\t%2.6f", Record.setA[0], Record.setA[1], Record.setA[2]);
			   fprintf(m_logFile, "%s %2.6f%s %2.6f%s %2.6f%s ", mListSep, Record.setA[0],mListSep, Record.setA[1], mListSep, Record.setA[2], mListSep);
		         setConXY(Curs_posX, Curs_posY -5, &consoleBuff[0]);
			   sprintf(consoleBuff, "\t%2.6f\t\t%2.6f\t\t%2.6f", Record.setB[0], Record.setB[1], Record.setB[2]);
		         fprintf(m_logFile, "%2.6f%s %2.6f%s %2.6f%s %u\n", Record.setB[0], mListSep, Record.setB[1], mListSep, Record.setB[2], mListSep, Record.timer);
			   setConXY(Curs_posX, Curs_posY, &consoleBuff[0]);
			   valid_rec++;
			}else if (cmd_return != 0xc4){
				if((cmd_return == 0xCB || cmd_return == 0xD3) && error_record == 0)
				    error_cmd = cmd_return;
				else{
					fprintf(m_logFile,"cmd_return:%x, error_record:%i\n",cmd_return, error_record);
					error_record++;
				}
			}
			
		
		//check for a key every 50 iterations
		if(iCount++ > 50)	{
			int ch = 0;
			if(ReadCharNoReturn(&ch)){
				bStopContinuous = (ch == 's' || ch == 'S');
			}
			//reset the counter
			iCount = 0;
		}
	}
	printf("\n\n\nStopping Continuous Mode...");
	StopContinuousMode(portNum);
	printf("stopped.\n");
	fflush(m_logFile);
	fclose(m_logFile);
	if (error_record > 0)
	    printf("Number of records received in error were %d and received successfully %d\n", error_record, valid_rec);
	else
		printf("All %d records read successfully.\n", valid_rec);
}

void LogContinuousData()
{
	char ComLogFile[256]; 
	printf("Enter Name of LogFile to use:");
		scanf("%s", &ComLogFile);
		printf("logFile %s\n", ComLogFile);
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