#include <curl/curl.h>
#include <jansson.h>

typedef enum _AXAPIC_RET
{
	AXAPIC_RET_ERROR_AUTH = -100,
	AXAPIC_RET_ERROR_JSON,
	AXAPIC_RET_ERROR_CURL,
	AXAPIC_RET_ERROR_MUTEX,
	AXAPIC_RET_ERROR_PARAMETER,
	AXAPIC_RET_ERROR_MEMORY,
	AXAPIC_RET_SUCCESS = 0
}AXAPIC_RET;

typedef struct _AXAPIC_ATTR
{
	char		szUsername[32];
	char		szPassword[32];
	char		szHost[256];
}AXAPIC_ATTR;

typedef struct _AXAPIC_RESPONSE
{
	unsigned long	ulDataNum;
	char			*pszData;
}AXAPIC_RESPONSE;

typedef struct _AXAPIC_HANDLE
{
	CURL			*hCurl;
	AXAPIC_ATTR		stAttr;
	char			szSessionId[32];
	HANDLE			hMutex;
	AXAPIC_RESPONSE stResp;
}AXAPIC_HANDLE;

typedef enum _AXAPI_VERS
{
	AXAPI_VERS_1 = 0,
	AXAPI_VERS_1_1,
	AXAPI_VERS_1_2,
	AXAPI_VERS_1_3,
	AXAPI_VERS_2,
	AXAPI_VERS_2_1,
	AXAPI_VERS_3,
	AXAPI_VERS_END
}AXAPI_VERS;

const char	gc_szAxapiVers[][5] = 
{
	"V1",
	"V1.1",
	"V1.2",
	"V1.3",
	"V2",
	"V2.1",
	"V3"
};

AXAPIC_RET Open(AXAPIC_HANDLE *hHandle, AXAPIC_ATTR *pstAttr);
AXAPIC_RET Close(AXAPIC_HANDLE *hHandle);
AXAPIC_RET Send(AXAPIC_HANDLE *hHandle, AXAPI_VERS Vers, char *pszMethod, char *pszPost, AXAPIC_RESPONSE *pstResp);
int testculr();
int getJson(char *pszData);