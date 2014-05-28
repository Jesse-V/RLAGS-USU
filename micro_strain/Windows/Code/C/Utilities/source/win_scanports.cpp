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

// win_queryport.cpp : Defines the entry point for the console application.

#include <cstdio>     //<stdlib.h>    //<cstdio>
#include <stdio.h>
#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <windows.h>
#include <wbemidl.h>
# pragma comment(lib, "wbemuuid.lib")  

#include "win_querysink.h"   

/* Prototypes */
BOOL ReadCharNoReturn(int* );

/*----------------------------------------------------------------------
 * scanports
 *
 * Scans windows serial ports to find attached inertia-link or 3DM-GX2
 * Returns non-zero valid port number if device found
 * Returns 0 if no device found
 *
 * parameters   portNum  : a port number (1..n).
 *--------------------------------------------------------------------*/
int scanports()
{
	 HRESULT hres;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 
    if (FAILED(hres))
    {
        printf( "Failed to initialize COM library. Error code = 0x \n",hres);
           //<< hex << hres << endl;
        return 1;                  // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------

    hres =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

                      
    if (FAILED(hres))
    {
        printf( "Failed to initialize security. Error code = 0x \n",hres);
           // << hex << hres << endl;
        CoUninitialize();
        return 1;                    // Program has failed.
    }
    
    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if (FAILED(hres))
    {
        printf( "Failed to create IWbemLocator object.");
        printf( " Err code = 0x \n", hres);
            //<< hex << hres << endl;
        CoUninitialize();
        return 1;                 // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices *pSvc = NULL;
	
    // Connect to the local root\cimv2 namespace
    // and obtain pointer pSvc to make IWbemServices calls.
    hres = pLoc->ConnectServer(
	    _bstr_t(L"ROOT\\CIMV2"), 
        NULL,
        NULL,
        0,
		NULL,
		0,
		0, 
		&pSvc
        );
    
    if (FAILED(hres))
    {
        printf( "Could not connect. Error code = 0x\n", hres);
             //<< hex << hres << endl;
        pLoc->Release();     
        CoUninitialize();
        return 1;                // Program has failed.
    }

    //printf( "Connected to ROOT\\CIMV2 WMI namespace" );


    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name 
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        printf( "Could not set proxy blanket. Error code = 0x\n",hres);
           // << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
        return 1;               // Program has failed.
    }

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // For example, get the name of the operating system.
    // The IWbemService::ExecQueryAsync method will call
    // the QuerySink::Indicate method when it receives a result
    // and the QuerySink::Indicate method will display the OS name
    QuerySink* pResponseSink = new QuerySink();
	IEnumWbemClassObject* pEnumerator = NULL;
   // hres = pSvc->ExecQueryAsync(
	hres = pSvc->ExecQuery(
        bstr_t("WQL"), 
        // bstr_t("SELECT * FROM Win32_OperatingSystem"),
		bstr_t("SELECT * FROM Win32_SerialPort"),
       //WBEM_FLAG_BIDIRECTIONAL, 
	   WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
       // pResponseSink);
	   &pEnumerator);
    
    if (FAILED(hres))
    {
        printf( "Query for serial name failed. Error code = 0x \n",hres);
        pSvc->Release();
        pLoc->Release();
        pResponseSink->Release();
        CoUninitialize();
        return 1;               // Program has failed.
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -----------
 

   // pSvc->CancelAsyncCall(pResponseSink);
	IWbemClassObject *pclsObj;
    ULONG uReturn = 0;
   
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
            &pclsObj, &uReturn);

        if(0 == uReturn)
        {
            break;
        }
        char device_type[45];
		char device_id[6][6]= {};
		char device_num[3]= {};
		int thing = 0, i=0;
        VARIANT vtProp;
		VARIANT vtPNP, vtCcn, vtSccn, vtDevId, vtStatus;

        // Get the value of the Name property
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
       // printf( "  Name : %s\n", vtProp.bstrVal);
		hr = pclsObj->Get(L"PNPDeviceID", 0, &vtPNP, 0, 0);
       // printf( "  PNPDeviceID : %s \n", vtPNP.bstrVal);
		hr = pclsObj->Get(L"CreationClassName", 0, &vtCcn, 0, 0);
       // printf( "  CreationClassName :%s \n", vtCcn.bstrVal );
		hr = pclsObj->Get(L"SystemCreationClassName", 0, &vtSccn, 0, 0);
       // printf( "  SystemCreationClassName : %s\n", vtSccn.bstrVal);
		hr = pclsObj->Get(L"DeviceID", 0, &vtDevId, 0, 0);
       // printf( "  DeviceID : %s \n " , vtDevId.bstrVal);
		hr = pclsObj->Get(L"Status", 0, &vtStatus, 0, 0);
       // printf( "  Status : %s \n", vtStatus.bstrVal);

		WideCharToMultiByte( CP_ACP, 0, vtPNP.bstrVal, -1,
                             device_type, 45, NULL, NULL );
		if (strstr(device_type, "4220")!= NULL){
		   // printf(" %s",device_type);
            WideCharToMultiByte( CP_ACP, 0, vtDevId.bstrVal, -1,
               device_id[0], 6, NULL, NULL );
			  // printf(" %s\n", device_id[0]);
			 for (int x=3; x<6; x++){
				if (device_id[x] !=NULL)
					device_num[x-3] = device_id[0][x];
			}
			
			//printf("device num not id %s\n", device_num );
			VariantClear(&vtProp);
		    VariantClear(&vtPNP);
            VariantClear(&vtCcn);
            VariantClear(&vtSccn);
		    VariantClear(&vtDevId);
		    VariantClear(&vtStatus);
			pSvc->Release();
            pLoc->Release();
            CoUninitialize();
	        int chOption = 0;
			return atoi(device_num);
		}

        VariantClear(&vtProp);
		VariantClear(&vtPNP);
        VariantClear(&vtCcn);
        VariantClear(&vtSccn);
		VariantClear(&vtDevId);
		VariantClear(&vtStatus);
    }

    // Cleanup
    // ========
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
	int chOption = 0;
	
	return 0;
}