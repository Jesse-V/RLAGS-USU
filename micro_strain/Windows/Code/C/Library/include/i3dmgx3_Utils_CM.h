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


/*----------------------------------------------------------------------
 * i3dmgx3UtilsCM.h
 *
 * Miscellaneous utility functions used by the 3DM-gx3 Adapter.
 * Windows specific utilities for console display.
 *--------------------------------------------------------------------*/

#include <stdio.h>	

int getConXY(int*, int*);
int setConXY(int, int, char *);
BOOL ReadCharNoReturn(int* );
void LogContinuousData();


/*-------------- end of i3dmgx3Utils_CM.h ----------------------*/
