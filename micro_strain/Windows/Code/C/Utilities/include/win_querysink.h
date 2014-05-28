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


#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

typedef struct _Comm_Conn/* struc for query port identification*/
{
	char  device_type[20][10]; /* the microstrain device type */
	char  device_id[20][6];    /* the comm port number */
	int   count;               /* number of valid entries discovered */
} Comm_Conn;

class QuerySink : public IWbemObjectSink
{
    LONG m_lRef;
    bool bDone;
	CRITICAL_SECTION threadLock; // for thread safety

public:
    QuerySink() { m_lRef = 0; bDone = false; 
	    InitializeCriticalSection(&threadLock); }
    ~QuerySink() { bDone = true;
        DeleteCriticalSection(&threadLock); }

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();        
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
        void** ppv);

    virtual HRESULT STDMETHODCALLTYPE Indicate( 
            LONG lObjectCount,
            IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray
            );
        
    virtual HRESULT STDMETHODCALLTYPE SetStatus( 
            /* [in] */ LONG lFlags,
            /* [in] */ HRESULT hResult,
            /* [in] */ BSTR strParam,
            /* [in] */ IWbemClassObject __RPC_FAR *pObjParam
            );

	bool IsDone();
};