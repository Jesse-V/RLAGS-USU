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

// win_QuerySink.cpp
#include <stdio.h>
#include "win_querysink.h"

ULONG QuerySink::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG QuerySink::Release()
{
    LONG lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

HRESULT QuerySink::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
    {
        *ppv = (IWbemObjectSink *) this;
        AddRef();
        return WBEM_S_NO_ERROR;
    }
    else return E_NOINTERFACE;
}


HRESULT QuerySink::Indicate(long lObjectCount,
    IWbemClassObject **apObjArray)
{
	HRESULT hres = S_OK;

    for (int i = 0; i < lObjectCount; i++)
    {
        VARIANT varName;
        hres = apObjArray[i]->Get(_bstr_t(L"Name"), 0, &varName, 0, 0);

		VARIANT vtName, vtCap, vtDesc, vtStat, vtDev, vtProv, vtSysnm;
		hres = apObjArray[i]->Get(_bstr_t(L"Caption"),
            0, &vtCap, 0, 0);
		hres = apObjArray[i]->Get(_bstr_t(L"Description"),
            0, &vtDesc, 0, 0);
		hres = apObjArray[i]->Get(_bstr_t(L"Status"),
            0, &vtStat, 0, 0);
		hres = apObjArray[i]->Get(_bstr_t(L"DeviceID"),
            0, &vtDev, 0, 0);
		hres = apObjArray[i]->Get(_bstr_t(L"ProviderType"),
            0, &vtProv, 0, 0);
		hres = apObjArray[i]->Get(_bstr_t(L"SystemName"),
            0, &vtSysnm, 0, 0);

        if (FAILED(hres))
        {
            printf("Failed to get the data from the query Error code = 0x \n", hres);
            return WBEM_E_FAILED;       // Program has failed.
        }

        printf("Name: %ls\n", V_BSTR(&varName));
		printf("Caption: %ls\n", V_BSTR(&vtCap));
		printf("Description: %ls\n", V_BSTR(&vtDesc));
		printf("Status: %ls\n", V_BSTR(&vtStat));
		printf("DeviceID: %ls\n", V_BSTR(&vtDev));
		printf("ProviderType: %ls\n", V_BSTR(&vtProv));
		printf("SystemName: %ls\n", V_BSTR(&vtSysnm));
    }

    return WBEM_S_NO_ERROR;
}

HRESULT QuerySink::SetStatus(
            /* [in] */ LONG lFlags,
            /* [in] */ HRESULT hResult,
            /* [in] */ BSTR strParam,
            /* [in] */ IWbemClassObject __RPC_FAR *pObjParam
        )
{
	if(lFlags == WBEM_STATUS_COMPLETE)
	{
		printf("Call complete.\n");

		EnterCriticalSection(&threadLock);
		bDone = true;
		LeaveCriticalSection(&threadLock);
	}
	else if(lFlags == WBEM_STATUS_PROGRESS)
	{
		printf("Call in progress.\n");
	}
    
    return WBEM_S_NO_ERROR;
}


bool QuerySink::IsDone()
{
    bool done = true;

	EnterCriticalSection(&threadLock);
	done = bDone;
	LeaveCriticalSection(&threadLock);

    return done;
}    // end of QuerySink.cpp