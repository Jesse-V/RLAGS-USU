/*----------------------------------------------------------------------
 *
 * I3DM-gx3 Interface Software
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

/*--------------------------------------------------------------------------
 * i3dmgx3Errors.h
 *
 * Definition of error codes.
 *--------------------------------------------------------------------------*/
#define SUCCESS          1
#define SYSTEM_ERROR          -1  //use getlasterror to retrieve status

#define I3DMGX3_COMM_OK 0 
#define I3DMGX3_OK 1

#define I3DMGX3_COMM_FAILED -1
#define I3DMGX3_COMM_INVALID_PORTNUM -2
#define I3DMGX3_COMM_WRITE_ERROR -3
#define I3DMGX3_COMM_READ_ERROR -4
#define I3DMGX3_COMM_RDLEN_ERROR -5
#define I3DMGX3_COMM_RDTIMEOUT_ERROR -6
#define I3DMGX3_CHECKSUM_ERROR -7
#define I3DMGX3_INVALID_DEVICENUM -8
#define I3DMGX3_EERPOM_DATA_ERROR -9
#define I3DMGX3_EERPOM_ADDR_ERROR -10
#define I3DMGX3_GYROSCALE_ERROR -11
#define I3DMGX3_INVALID_CMD_ERROR -12


#define LAST_ERROR -12    /* last error number. */

/*-------------- end of i3dmgx3Errors.h ----------------------*/
