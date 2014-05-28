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

#include "i3dmgx3_Utils.h"
#include "i3dmgx3_Errors.h"
#include "i3dmgx3_Serial.h"
#include "i3dmgx3_readWriteDR.h"
/*----------------------------------------------------------------------
 * ReadDataRate	0xE5
 *
 * parameters   portNum      : the port number of the sensor device
 *              address      : the address location for Data-Rate
 *              
 *              Bresponse    : buffer to return contains checksum 
 *
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns
 *                          an error code.
 *--------------------------------------------------------------------*/
int ReadDataRate(int portNum,  unsigned char *Bresponse) {
	int status = 0; 
	unsigned char DR_buff[20];
	int responseLength = 19;
	unsigned short wChecksum = 0;
      unsigned short wCalculatedCheckSum = 0;
      int qwerty=0;


	/* initialize send buffer to obtain Data-Rate value at address */
	DR_buff[0]    = 0xDB;        // Identifier of Read Data-Rate value
	DR_buff[1]    = 0xA8;        
	DR_buff[2]    = 0xB9;  
	DR_buff[3]    = 0x00;
      for (qwerty=4; qwerty<20; qwerty++)
	       DR_buff[qwerty] = 0x0; 

	
	 status = sendBuffData(portNum, &DR_buff[0], 20);
		//printf("Status is %i\n", status);
     if (status != I3DMGX3_COMM_OK)
	     return status = I3DMGX3_COMM_READ_ERROR; 
	 status = receiveData(portNum, &Bresponse[0], responseLength);
     if (status != I3DMGX3_OK)
		 return status = I3DMGX3_COMM_READ_ERROR;
	 else {      
		  wChecksum = convert2ushort(&Bresponse[responseLength-2]);
	        wCalculatedCheckSum = i3dmgx3_Checksum(&Bresponse[0], responseLength-2); 
		  if(wChecksum == wCalculatedCheckSum){
			   	return status;
		  }
		  else if(wChecksum != wCalculatedCheckSum){
			              return status = I3DMGX3_CHECKSUM_ERROR;
		  }
	  }
      return I3DMGX3_COMM_OK;

}
			
/*----------------------------------------------------------------------
 * WriteDataRate  0xE4
 *
 * parameters   portNum      : the number of the sensor device (1..n)
 *              value        : the value to write to the specified 
 *                             address
 *		    Bresponse    : return buffer
 *
 *
 * returns:     errorCode : I3DMGX3_OK if succeeded, otherwise returns an
 *                          error code.
 *-------------------------------------------------------------------------*/
int WriteDataRate(int portNum, int m_DRvalue, unsigned char *Bresponse) {
    int status = 0;
    unsigned char DR3_write_buff[21];
    int responseLength = 19;
    unsigned short wChecksum = 0;
    unsigned short wCalculatedCheckSum = 0;
    int oversampleRate = 1000;
    int response3Length = 19;
    unsigned short Decimation = (1000/m_DRvalue);
    unsigned short DigFilter = 20;
    int ytrewq=0;   

    DR3_write_buff[0] = 0xDB;                               //Command identifier for Write to Data-Rate
    DR3_write_buff[1] = 0xA8;		                        //Required identifier for Data-Rate Write  was C1
    DR3_write_buff[2] = 0xB9;                               //Required identifier for Data-Rate Write  was 29
    DR3_write_buff[3] = 0x01;                               //Identifies an update to data rate, 0 == no update

    DR3_write_buff[4] = (Decimation & MSB_MASK) >> 8;       //1st byte of Decimation value to write
    DR3_write_buff[5] =  Decimation & LSB_MASK;             //2ond byte of Decimation value to write
    DR3_write_buff[6] = 0xC0;
    DR3_write_buff[7] = 0x00;
    DR3_write_buff[8] = 0x02;
    DR3_write_buff[9] = 0x02;

    for(ytrewq=10; ytrewq<20; ytrewq++)
	  DR3_write_buff[ytrewq] = 0x00;        // Bytes 11-20 are reserved set to zero(s) 0
		
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
		 else if(wChecksum != wCalculatedCheckSum){
				 return status = I3DMGX3_CHECKSUM_ERROR;
		 }
	   }	   
    return status;
}