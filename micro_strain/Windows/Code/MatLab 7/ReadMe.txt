%*-------------------------------------------------------------------------
%* (c) 2009 Microstrain, Inc.
%*-------------------------------------------------------------------------
%* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING 
%* CUSTOMERS WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER 
%* FOR THEM TO SAVE TIME. AS A RESULT, MICROSTRAIN SHALL NOT BE HELD LIABLE 
%* FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY 
%* CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY 
%* CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH 
%* THEIR PRODUCTS.
%*-------------------------------------------------------------------------
%*
%*-------------------------------------------------------------------------
%* MatLab Examples
%* This folder contains examples of functions used to communicate with the
%* MicroStrain 3DM-GX3 sensors. These include a funciton to read 
%* acceleration and angular rate, a function to set the data rate, a 
%* function to change the sensor's output mode, and a combined function
%* that can perform all the previously mentioned actions, as well as create
%* sample plots in the MatLab plotting environment. These functions operate
%* in the MatLab command window, but can also be compiled into stand-alone
%* .exe's with the MatLab compiler, which is sold separately from MatLab.
%* Also included is a library of basic functions used to communicate with
%* the device, which can easily be implemented into new functions. All of
%* the basic funcitons called by the larger functions are included as
%* subfunctions in the larger function's code. This is done so that MatLab
%* can run the large functions easily and quickly, and not have to search
%* for the subfunctions, which may not be located in the MatLab search
%* path. If a function is run while it is not in the MatLab search path, a
%* dialog box will be prompted, asking whether to add the funciton to the
%* search path. The only user input that is required to run these functions
%* is the COM port number, which can be found using the Windows Device 
%* Manager (located at C:\WINDOWS\system32\devmgmt.msc on most machines).
%* The device should appear under "Ports (COM % LPT)"as either "CP210x USB
%* to UART Bridge Controller" or "MicroStrain Virtual COM Port." After the
%* device's name will be "(COM#)." The # is the number that the function
%* i3dmgx2_OpenSerial uses to open the link (the variable "ComNum" in the
%* code). Once the i3dmgx_OpenSerial function is successfully used, MatLab
%* creates a handle for the serial link, and the number is no longer used.
%*-------------------------------------------------------------------------