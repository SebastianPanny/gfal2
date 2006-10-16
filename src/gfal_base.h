#ifndef GFAL_BASE_H_
#define GFAL_BASE_H_

#if _DUMA
#include "duma.h"
#endif

#ifdef _GFAL_TESTS
#include "CuTest.h"
#define GCuAssertPtrNotNull(...)	CuAssertPtrNotNull((CuTest*)ctx->chkPtr,##__VA_ARGS__);
#endif

#if defined(SYMBIAN)
# define LONG64 TInt64
# define ULONG64 TInt64
#elif !defined(WIN32)
# define LONG64 long long
# define ULONG64 unsigned LONG64
#elif defined(UNDER_CE)
# define LONG64 __int64
# define ULONG64 unsigned LONG64
#elif defined(__BORLANDC__)
# define LONG64 __int64
# define ULONG64 unsigned LONG64
#endif

/* srm v2 native mapping operations */
typedef enum _gfal_errclass {GFALERR_SUCCESS = 0, GFALERR_SOAP = 1, GFALERR_SERVER = 2, GFALERR_INTERNAL = 3, GFALERR_PROTOCOL = 4}
	gfal_errclass; 

#define GFAL_ERRSIZE	512

typedef struct _gfal_ctx
{

	/*
	 * hash list for environmental variables:
	 * bdii timeout
	 */
	//testing/reserved handle ptr
	unsigned int soap_timeout;
	unsigned int bdii_timeout;
	char user[256]; 
	void* chkPtr;
	//error section	
	char errbuf[GFAL_ERRSIZE];	
	int errnom;
	gfal_errclass errclass;
} gfal_ctx;



#endif /*GFAL_BASE_H_*/
