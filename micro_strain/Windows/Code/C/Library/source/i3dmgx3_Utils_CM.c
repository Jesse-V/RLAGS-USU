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
 * i3dmgx3Utils_CM.c
 *
 * Miscellaneous utility functions used by the 3DM-GX3 Adapter
 * Specific to Windows console display.
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <windows.h>
#include "i3dmgx3_Utils_CM.h"

/*----------------------------------------------------------------------
 * getConXY
 * Obtains the current cursor position co-ordinates X Y.
 *
 * parameters:  Xpos : X cursor position
 *              Ypos : Y cursor position
 *
 * returns:     an error on failure.
 *--------------------------------------------------------------------*/
int getConXY(int* Xpos, int* Ypos){


HANDLE hStdOut;
BOOL constat;
CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

constat = GetConsoleScreenBufferInfo(hStdOut, &consoleInfo);
if (!constat) {
    printf("getconsoleScreenbuffer failed \n");
    return -1;
    }

*Xpos= consoleInfo.dwCursorPosition.X;
*Ypos= consoleInfo.dwCursorPosition.Y;
return 1;

}
/*----------------------------------------------------------------------
 * setConXY
 * Set the cursor position for continuous data display.
 *
 * parameters:  Xset : X cursor position
 *              Yset : Y cursor position
 *              ConBuff: prformated output buffer string.
 *
 * returns:     an error on failure.
 *--------------------------------------------------------------------*/
int setConXY(int Xset, int Yset, char *ConBuff){
//HANDLE hConsoleOutput;
HANDLE hStdOut;
//struct _COORD coord;
CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
BOOL constat;
int conBuffWritten = 0;

hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
GetConsoleScreenBufferInfo(hStdOut, &consoleInfo);

consoleInfo.dwCursorPosition.X = Xset;  /* 0 would start at first position */
consoleInfo.dwCursorPosition.Y = Yset;

constat = SetConsoleCursorPosition(hStdOut, consoleInfo.dwCursorPosition);
if (!constat) {
    printf("setconsole failed \n");
    return -1;
    }

WriteConsole(hStdOut, ConBuff, lstrlen(ConBuff), &conBuffWritten, NULL);
return 1;

}
/* **************************************************************
*   ReadCharNoReturn:
*   checks for a termination code and exits continuous mode
****************************************************************/
BOOL ReadCharNoReturn(int* ch)
{
	//check for a key
	if(_kbhit())
	{
		//key was found get it
		*ch = _getch(); 
		return TRUE;
	}

	return FALSE;
}
/*-------------- end of i3dmgx3Utils.c ----------------------*/
