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
 * i3dmgx3Utils.c
 *
 * Miscellaneous utility functions used by the 3DM-GX3 Adapter.
 *--------------------------------------------------------------------*/

#include <stdio.h>

#include "i3dmgx3_Utils.h"
#include "i3dmgx3_Errors.h"

/* error explanations */
char *ERROR_TEXT[] = {
    /* 0 */ "Undefined error",
    /* 1 */ "General failure",
    /* 2 */ "Invalid port number",
    /* 3 */ "Port write error",
    /* 4 */ "Port read error",
    /* 5 */ "Port read length error",
    /* 6 */ "Port read timout error",
    /* 7 */ "Checksum error",
    /* 8 */ "Invalid device Number",
    /* 9 */ "EEPROM data error",
    /* 10 */ "EEPROM address error",
    /* 11 */ "Unable to read gyro scale from EEPROM"
};

/*----------------------------------------------------------------------
 * calcChecksum
 * Calculate checksum on a received data buffer.
 *
 * Note: The last two bytes, which contain the received checksum,
 *       are not included in the calculation.
 *
 * parameters:  buffer : pointer to the start of the received buffer.
 *              length - the length (in chars) of the buffer.
 *
 * returns:     the calculated checksum.
 *--------------------------------------------------------------------*/
int calcChecksum( unsigned char* buffer, int length) {
	int CHECKSUM_MASK = 0xFFFF;
	int checkSum, i;

	if (length<4)
		return -1;

	checkSum = buffer[0] & LSB_MASK;
	for (i=1; i<length-2; i = i+2) {
		checkSum += convert2short(&buffer[i]);
	}
	return(checkSum & CHECKSUM_MASK);
}

/*----------------------------------------------------------------------
 * i3dmgx3_Checksum
 * Calculate checksum on a received data buffer.
 *
 * Note: The last two bytes, which contain the received checksum,
 *       are not included in the calculation.
 *
 * parameters:  buffer : pointer to the start of the received buffer.
 *              length - the length (in chars) of the buffer.
 *
 * returns:     the calculated checksum.
 *--------------------------------------------------------------------*/
unsigned short i3dmgx3_Checksum(const unsigned char* pBytes, int count)
{
	unsigned short i3_checksum = 0;
	int i = 0;

	for(i = 0; i < count; ++i)
	{
		i3_checksum+=pBytes[i];
	}

	return i3_checksum;
}
/*----------------------------------------------------------------------
 * TestByteOrder()
 * Tests byte alignment to determine Endian Format of local host.
 *
 * returns:     The ENDIAN platform identifier.
 *--------------------------------------------------------------------*/
int TestByteOrder()
{
   short int word = 0x0001;
   char *byte = (char *) &word;
   return(byte[0] ? LITTLE_ENDIAN : BIG_ENDIAN);
}

/*----------------------------------------------------------------------
 * FloatFromBytes
 * Converts bytes to Float.
 *
 * parameters:  pBytes : received buffer containing pointer to 4 bytes
 *
 * returns:     a float value.
 *--------------------------------------------------------------------*/
float FloatFromBytes(const unsigned char* pBytes)
{
	float f = 0;
	if(TestByteOrder() != BIG_ENDIAN) {
	   ((BYTE*)(&f))[0] = pBytes[3];
	   ((BYTE*)(&f))[1] = pBytes[2];
	   ((BYTE*)(&f))[2] = pBytes[1];
	   ((BYTE*)(&f))[3] = pBytes[0];
	}else{
	   ((BYTE*)(&f))[0] = pBytes[0];
	   ((BYTE*)(&f))[1] = pBytes[1];
	   ((BYTE*)(&f))[2] = pBytes[2];
	   ((BYTE*)(&f))[3] = pBytes[3];
	}
	
	return f; 
}
/*----------------------------------------------------------------------
 * convert2short
 * Convert two adjacent bytes to an integer.
 *
 * parameters:  buffer : pointer to first of two buffer bytes.
 * returns:     the converted value aa a signed short -32 to +32k.
 *--------------------------------------------------------------------*/
short convert2short(unsigned char* buffer) {
	short x;
	if(TestByteOrder() != BIG_ENDIAN) {
	   x = (buffer[0] <<8) + (buffer[1] & 0xFF);
	}else{
		x = (short)buffer;
	}
	return x;
}
/*----------------------------------------------------------------------
 * convert2ushort
 * Convert two adjacent bytes to a short.
 *
 * parameters:  buffer : pointer to first of two buffer bytes.
 * returns:     the converted value as a unsigned short 0-64k.
 *--------------------------------------------------------------------*/
unsigned short convert2ushort(unsigned char* buffer) {
	unsigned short x;
	if(TestByteOrder() != BIG_ENDIAN) {
	   x = (buffer[0] <<8) + (buffer[1] & 0xFF);
	}else{
		x = (unsigned short)buffer;
	}
	return x;
}
/*----------------------------------------------------------------------
 * convert2long
 * Convert four adjacent bytes to a signed long.
 *
 * parameters:  buffer : pointer to a 4 byte buffer.
 * returns:     the converted value as a signed long.
 *--------------------------------------------------------------------*/
 long convert2long(unsigned char* plbyte) {
    long l = 0;
	if(TestByteOrder() != BIG_ENDIAN) {
	   l = (plbyte[0] <<24) + (plbyte[1] <<16) + (plbyte[2] <<8) + (plbyte[3] & 0xFF);    
	 }else{
		 l = (long)plbyte;
	}
  	return l;
}
/*----------------------------------------------------------------------
 * convert2ulong
 * Convert four adjacent bytes to a unsigned long.
 *
 * parameters:  buffer : pointer to a 4 byte buffer.
 * returns:     the converted value as a unsigned long.
 *--------------------------------------------------------------------*/
unsigned long convert2ulong(unsigned char* plbyte) {
	unsigned long ul = 0;
	if(TestByteOrder() != BIG_ENDIAN) {
	   ul = (plbyte[0] <<24) + (plbyte[1] <<16) + (plbyte[2] <<8) + (plbyte[3] & 0xFF);    
	}else{
		ul = (unsigned long)plbyte;
	}
  	return ul;
}
/*----------------------------------------------------------------------
 * explainError
 * Provide a text explanation for an error code.
 *
 * parameters:  errornum : error number
 *
 * returns:     a string (char *) corresponding to the error number.
 *--------------------------------------------------------------------*/
char * explainError(int errornum) {
    if (errornum < LAST_ERROR || errornum > 0)
        return ERROR_TEXT[0];
    else
        return ERROR_TEXT[-1*errornum];
}
/*-------------- end of i3dmgx3Utils.c ----------------------*/
