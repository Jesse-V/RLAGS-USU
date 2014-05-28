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
 * Main for the 3DM-GX3 Adapter continuous/poll data console application.
 *--------------------------------------------------------------------*/		
#include <stdio.h>
#include <windows.h>
#include <sys/timeb.h>
#include <time.h>
#include "i3dmgx3_Errors.h"
#include "i3dmgx3_Cont.h"
#include "i3dmgx3_Utils.h"
#include "i3dmgx3_Utils_CM.h"
#include "i3dmgx3_readWriteDR.h"
#include "i3dmgx3_Serial.h"


int GetComPort(); //prompt user for comport and opens it
void SetDataRate(int portNum); 
void ReadContinuousData(int portNum); //puts the node in continuous mode and retrives and prits data until user interups
void ReadContinuousLogData(int portNum);
int OnGetSerialPorts();
int SetSampleSetting(int portNum, BYTE byteOne, BYTE byteTwo, unsigned char *Bresponse, short FW_Check);

int main(int argc, char* argv[])
{
	BOOL bQuit = FALSE;
	BOOL bPrintHeader = TRUE;
    int portNum = 0;
	int tryPortNum;
	int Ccount = 0;
	
	printf("\n   3DM-GX3 Read and Write Data Rate\n");

	/*-------- If user specifies a port, then use it */
	if (argc > 1) {
		tryPortNum = atoi(argv[1]);
		if (tryPortNum < 2 || tryPortNum > 256) {
			printf("   usage:  DataRate <portNumber>\n");
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
	
	printf("\n  3DM-GX3-25 Using COM Port #%d \n", portNum);
   
	while(!bQuit){
		int chOption = 0;

		if(bPrintHeader)
		{
			printf("\n");
			printf("Enter an Option: (D)ata Rate (C)ontinuous (L)og_Continuous (Q)uit\n");
			printf("D Data Rate      - Modify or review the current Data Rate \n");
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
			case 'D':
			case 'd': SetDataRate(portNum); bPrintHeader = TRUE; break;

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
		if(iComPort <2 || iComPort > 256){
			printf("   please enter a valid port number \n");
			printf("        valid ports are betweeen 2 and 256\n");
			if (errCount++ >= MaxFail)
				  exit(-1);
		}
		else{
		  /* open a port */
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
// SetDataRate
//---------------------------------------------------------------------------
// Description: Allows user to set the DataRate between 1Hz and 300Hz
//              Default is 100Hz
//
// Return: A display of the DataRate value which has been set.
//===========================================================================
void SetDataRate(int portNum)
{
	int status = 0, Value = 0, Ival =0;
	int chOption = 0;
	int rDataRate = 0;
	int mDataRate = 0;
	int sDataRate = 0;
	int valCheck = 0;
	BYTE Record[20];
	char AddA = 0xFC;
	char AddB = 0xA2;
    unsigned short nvalS  = 0;
	char Jjunk[20];

	purge_port(portNum);
	
	status = ReadDataRate(portNum, &Record[0] );
	if(status == 1){ //0
		printf("\n\nCurrent value of the DataRate is set at ");
	    nvalS = convert2ushort(&Record[1]);
	    rDataRate = (1000 / nvalS);
	    printf("%d Hz\n\n", rDataRate);
	} else {
		printf("Error Reading Data-Rate %d\n", status); // %s explainError(status));
		return;
	}
	while(Value == 0)
	{
		printf("Would you like to Modify the DataRate? (Y)es or (N)o \n");
		while(!ReadCharNoReturn(&chOption))
		{
			Sleep(50);
		}
     
		switch (chOption){
			case 'n':
			case 'N': Value = 1; break;

			case 'y':
			case 'Y': while (Ival == 0){
				            mDataRate = 0;
				            printf("Enter a value between 1 and 1000 to set:");
				            scanf("%d", &mDataRate);
							
							if ((mDataRate < 1 )|| (mDataRate > 1000) ){
								printf(" Invalid entry: out of range  \n\n");
				                Ival = 0;
								scanf("%s", Jjunk);
								mDataRate = 0;
								break;
							} else Ival = 1;   	
							
					  }
			          if(Ival == 1){
				          valCheck = WriteDataRate(portNum, mDataRate, &Record[0]);
				          if(valCheck != 1 ){
						       printf("DataRate set Error %d\n", valCheck);
						       break;
					   }
				       if( valCheck == 1) {
					        status = ReadDataRate(portNum, &Record[0] );
	                              if(status == 1){  //0
		                              printf("Current value of DataRate is set at ");
	                              nvalS = convert2ushort(&Record[1]);
	                              rDataRate = (1000 / nvalS);
	                              printf("%dHz\n\n", rDataRate);
						      Ival = 0;
						      break;
	                    } else {
		                      printf("Error Reading Data-Rate %d\n", status); //explainError(status));
		                      return;
	                    }
				    }
					break;
					
			default: printf("Invalid Option\n\n");  Value = 0;
           }
		 }			
		
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
		         setConXY(Curs_posX, Curs_posY, &consoleBuff[0]);
			     valid_rec++;
			}else if (cmd_return != 0xC4){
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
    int m_timerconst = 62500; 
	unsigned long AA_Time_Stamp=0;
	unsigned long AA_s_prev =0;
	float AA_convert = 0.0;
	float AA_prev = 0.0;

	struct __timeb64 timebuffer;
    char *timeline;
    
        _ftime64( &timebuffer );
        timeline = _ctime64( & ( timebuffer.time ) );

	while (LogFlag != 1){
	    printf("Enter Name of LogFile to use:");
		scanf("%s", &ComLogFile); // 255);
		printf("logFile %s\n", ComLogFile);
        if ( ( m_logFile= fopen( ComLogFile, "w")) == NULL){
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
	i = VerLanguageName (langId, szLanguage, 256); //dwSize = MAX_PATH; 
        fprintf(m_logFile, "Language: %s\n", szLanguage);

    while(valid_check == 0)
	{
       errorCode = i3dmgx3_getFirmwareVersion(portNum, &fw[0]);
	   if (errorCode < 0){
		   purge_port(portNum);
               printf("Firmware Error %d\n",errorCode);
		   if (valid_count++ > 6) {
               printf("Please Halt Current Data Display and Retry, count %d\n", valid_count);
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
    for ( i=0; i<4; i++){   //cycles through the valid device options
	
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
					   fprintf(m_logFile, "Device Name: %s\n",sn);
					   break;
				   case 1:
					   fprintf(m_logFile, "Device Model: %s\n",sn);
					   break;
				   case 2:
					   fprintf(m_logFile, "Device FirmWare Version: %s\n", fw);
					   fprintf(m_logFile, "Device Serial Number: %s\n", sn);
					   break;
				   case 3:
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
	/*  acceleration values go here */
	printf("\n\n\n"); 
	printf("C2____________________________Angular_Rate______________________________\n");
	printf("            X                       Y                      Z            \n");
	/* angle rate values go here  */
	getConXY(&Curs_posX, &Curs_posY); 
	printf("\n\n\n\n");
	
	//continue until the user hits the <s>top key
	while(!bStopContinuous)
	{

		if(ReadNextRecord(portNum, &Record, &cmd_return) != SUCCESS)
			error_record++;
	        if (cmd_return == 0xC2){
                if (AA_s_prev == 0){
	               fprintf(m_logFile, "\t  0.00");
				    AA_s_prev = Record.timer; //AA_Time_Stamp;
		         }
                  else
                {
				  AA_convert = ((float)(Record.timer - AA_s_prev)/m_timerconst); //19660800);
			      AA_s_prev = Record.timer; //AA_Time_Stamp;
				  AA_prev = AA_prev + AA_convert;
			      fprintf(m_logFile, "\t%6.2f", AA_prev);
			    }
			//move to the acceleration position and print the data
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
	fclose(m_logFile);
	if (error_record > 0)
	    printf("Number of records received in error were %d and received successfully %d\n", error_record, valid_rec);
	else
		printf("All %d records read successfully.\n", valid_rec);
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
				
					if (strstr(Verity,c_check) != NULL){
			           return i_portNum;					
	                     } //End string compare
					
                       else i3dmgx3_closeDevice(i_count);          
				
                 }// end open port
				
	}// end for loop
	   printf("No 3DM-GX3 devices found, please check connections\n");
       return -1;                
 }


//===========================================================================
// SetSampleSetting
//---------------------------------------------------------------------------
// Description: Allows user to set one of the optional Sample Settings
//
// Return: Confirmation that selection was set.
//===========================================================================
int SetSampleSetting(int portNum, BYTE byteOne, BYTE byteTwo, unsigned char *Bresponse, short FW_Check)
{
    int status = 0;
    unsigned char DR3_write_buff[21];
    int responseLength = 19;
    unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
    unsigned short Decimation = (1000/100);         //m_DRvalue);
    unsigned short DigFilter = 20;
    int ytrewq=0;  
	unsigned char cmd = ( unsigned char) CMD_FIRWARE_VERSION;  //0xE9
	BYTE FWbuff[7] = {0}; 
	short firmwareNum=0;
	int northComp=0, upComp=0;
	int NCval =0, Value=0;
	//char Jjunk[20];
	int ncOption = 0;

    purge_port(portNum);

	if(FW_Check >= 1){  //include case 8 and 9
       //this feature requires firmware version of 1.1.27 or greater		  	
       status = sendBuffData(portNum, &cmd, 1);
	   if (status == I3DMGX3_COMM_OK) {
           status = receiveData(portNum, &FWbuff[0], 7);
	       firmwareNum = convert2short(&FWbuff[3]);
	       if (firmwareNum < 1127){
		       printf("Firmware version must be 1.1.27 or greater to perfom this selection\n");
		       return -1;
	       }        
	   } 
	}

	if(FW_Check == 8){ //North Compensation
		while(Value == 0){
	         printf("Would you like to Modify the North Compensation? (Y)es or (N)o \n");
		     while(!ReadCharNoReturn(&ncOption))
		     {
			     Sleep(50);
		     }
     
		switch (ncOption){
			case 'n':
			case 'N': northComp = 10; Value = 1; break;

			case 'y':
			case 'Y':  while (NCval == 0){
			              northComp = 0;
			              printf("Enter a value between 1 and 1000 to set:");
			              scanf("%d", &northComp);
							
			              if ((northComp < 1 )|| (northComp > 1000) ){
				              printf(" Invalid entry: out of range  \n\n");
				              NCval = 0;
				              //scanf("%s", Jjunk);
				              northComp = 0;
		                  } 
						  else NCval = 1;
					  }
					  Value =1;
					  break;

			default:printf("Invalid selection\n");
				           Value =0;
						   break;
			}
		}
	}

	if(FW_Check == 9){ //Up Compensation
		Value = 0;
		NCval = 0;
		while(Value == 0){
	         printf("Would you like to Modify the Up Compensation value? (Y)es or (N)o \n");
		     while(!ReadCharNoReturn(&ncOption))
		     {
			     Sleep(50);
		     }
     
		switch (ncOption){
			case 'n':
			case 'N': upComp = 10; Value = 1; break;

			case 'y':
			case 'Y':  while (NCval == 0){
			              upComp = 0;
			              printf("Enter a value between 1 and 1000 to set:");
			              scanf("%d", &upComp);
							
			              if ((upComp < 1 )|| (upComp > 1000) ){
				              printf(" Invalid entry: out of range  \n\n");
				              NCval = 0;
				              upComp = 0;
		                  } 
						  else NCval = 1; 
					   }
					   Value =1;
					   //return;   //remove after validation
					   break;

			default:printf("Invalid selection\n");
				           Value =0;
						   break;
		    }	
		}
	}

    DR3_write_buff[0] = 0xDB;    //Command identifier for Write to Data-Rate
    DR3_write_buff[1] = 0xA8;	 //Required identifier for Data-Rate Write  was C1
    DR3_write_buff[2] = 0xB9;    //Required identifier for Data-Rate Write  was 29
    DR3_write_buff[3] = 0x01;    //Identifies an update to data rate, 0 == no update
                                     // 1 == change the parameters to new values
                                     // 2 == change and write to non-volatile memory
                                     // 3 == same as 2 but does not send a reply

    DR3_write_buff[4]  = (Decimation & MSB_MASK) >> 8;  //1st byte of Decimation value to write
    DR3_write_buff[5]  = Decimation & LSB_MASK;         //2ond byte of Decimation value to write
    DR3_write_buff[6]  = byteOne;                       //First Byte of Data Conditioning Function Selector      //0xC0;
    DR3_write_buff[7]  = byteTwo;                       //Second Byte of Data Conditioning Function Selector //0x00;
    DR3_write_buff[8]  = 0x0F;                          //Gyro and Accel digital filter window size default is 15
    DR3_write_buff[9]  = 0x1E;                          //Mag digital filter window size default is 17
    if(FW_Check == 9){
		DR3_write_buff[10]  = (upComp & MSB_MASK) >> 8;
        DR3_write_buff[11]  = upComp & LSB_MASK; 
	}else{
	    DR3_write_buff[10] = 0x00;                       //First Byte of Up Compensation in Sec
        DR3_write_buff[11] = 0x01;                       //Second Byte of Up Compensation default is 10
	}
	if(FW_Check == 8){
		DR3_write_buff[12]  = (northComp & MSB_MASK) >> 8;
        DR3_write_buff[13]  = northComp & LSB_MASK; 
	}else{
	    DR3_write_buff[12] = 0x00;                      //First Byte of North Compensation in Sec
        DR3_write_buff[13] = 0x01;                      //Second Byte of North Compensation default is 10
	}
    for(ytrewq=14; ytrewq<20; ytrewq++)
	  DR3_write_buff[ytrewq] = 0x00;        // Bytes 15-20 are reserved set to zero(s) 0
		
    status = sendBuffData(portNum, &DR3_write_buff[0], 20);	          
      
    if (status != I3DMGX3_COMM_OK) 
		return  status; 

    status = receiveData(portNum, &Bresponse[0], responseLength);

    if (status == I3DMGX3_COMM_OK) {
        wChecksum = convert2ushort(&Bresponse[responseLength-2]);
        wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); 
        if(wChecksum == wCalculatedCheckSum){
             return status;
         }
         else if(wChecksum != wCalculatedCheckSum)
             return status = I3DMGX3_CHECKSUM_ERROR;
	}

    return status;	                                               				   
}



