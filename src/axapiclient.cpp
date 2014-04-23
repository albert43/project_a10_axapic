#include <axapiclient.h>

size_t cbCurl( char *ptr, size_t size, size_t nmemb, void *userdata)
{
	AXAPIC_HANDLE		*hHandle = (AXAPIC_HANDLE *)userdata;

	hHandle->stResp.ulDataNum = size * nmemb;
	hHandle->stResp.pszData = ptr;
	
	ReleaseMutex(hHandle->hMutex);

	return (size * nmemb);
}

AXAPIC_RET Open(AXAPIC_HANDLE *hHandle, AXAPIC_ATTR *pstAttr)
{
	AXAPIC_RET		Ret;
	char			szMethod[256];
	AXAPIC_RESPONSE	stResp;
	json_error_t	stError;
	json_t			*pjsnRoot, *pjsnData;
	const char		*pszSessionId = NULL;

	//	Check input parameters
	if ((hHandle == NULL) || (pstAttr == NULL))
		return AXAPIC_RET_ERROR_PARAMETER;

	if (strlen(pstAttr->szHost) == 0)
		return AXAPIC_RET_ERROR_PARAMETER;

	//	Initialize curl
	curl_global_init(CURL_GLOBAL_ALL );

	//	Initialize mutex
	hHandle->hMutex = CreateMutex (NULL, FALSE, NULL);
	if (hHandle->hMutex == NULL)
		return AXAPIC_RET_ERROR_MUTEX;

	//	Copy attribute settings to handle structure
	memcpy (&hHandle->stAttr, pstAttr, sizeof(AXAPIC_ATTR));

	//	Get the session id
	hHandle->szSessionId[0] = 0;
	Ret = Send(hHandle, AXAPI_VERS_2_1, szMethod, NULL, &stResp);
	if (Ret != AXAPIC_RET_SUCCESS)
		return Ret;

	pjsnRoot = json_loads(stResp.pszData, 0, &stError );  
	if(pjsnRoot == NULL) 
	{
		Close(hHandle);
		return AXAPIC_RET_ERROR_JSON;
	}

    if (pjsnRoot->type != JSON_OBJECT)
	{
		json_decref(pjsnRoot);
		Close(hHandle);
		return AXAPIC_RET_ERROR_JSON;
	}

	pjsnData = json_object_get(pjsnRoot, "session_id");
	if (pjsnData == NULL)
	{
		json_decref(pjsnRoot);
		Close(hHandle);
		return AXAPIC_RET_ERROR_AUTH;
	}

	pszSessionId = json_string_value(pjsnData);
	strcpy_s(hHandle->szSessionId, pszSessionId);

    json_decref(pjsnRoot);

	return AXAPIC_RET_SUCCESS;
}

AXAPIC_RET Close(AXAPIC_HANDLE *hHandle)
{
	if (hHandle == NULL)
		return AXAPIC_RET_ERROR_PARAMETER;

	curl_global_cleanup();
	CloseHandle(hHandle->hMutex);

	return AXAPIC_RET_SUCCESS;
}

AXAPIC_RET Send(AXAPIC_HANDLE *hHandle, AXAPI_VERS Vers, char *pszMethod, char *pszPost, AXAPIC_RESPONSE *pstResp)
{
	CURLcode	CurlRet;
	char		szUrl[256];

	//	Check input parameters
	if ((hHandle == NULL) || (pszMethod == NULL) || (pstResp == NULL))
		return AXAPIC_RET_ERROR_PARAMETER;

	if ((Vers < AXAPI_VERS_1) || (Vers > AXAPI_VERS_END))
		return AXAPIC_RET_ERROR_PARAMETER;

	//	Initialize curl
	hHandle->hCurl = curl_easy_init();
	if (hHandle == NULL)
	{
		curl_easy_cleanup(hHandle->hCurl);
		return AXAPIC_RET_ERROR_CURL;
	}

	curl_easy_setopt(hHandle->hCurl, CURLOPT_WRITEFUNCTION, cbCurl);
	curl_easy_setopt(hHandle->hCurl, CURLOPT_WRITEDATA, hHandle);
//	curl_easy_setopt(hHandle->hCurl, CURLOPT_VERBOSE, 1);
//	curl_easy_setopt(hHandle->hCurl, CURLOPT_HEADER, 1);

	//	Set url
	if (strlen(hHandle->szSessionId) == 0)
//		sprintf_s(szUrl, "http://%s/services/rest/V1/?method=authenticate&username=%s&password=%s&format=json",
//			hHandle->stAttr.szHost, hHandle->stAttr.szUsername, hHandle->stAttr.szPassword);
	strcpy_s(szUrl, strlen("http://192.168.99.51/services/rest/V2.1/?format=json&session_id=af4f9a1724a52006e56cd73909aa3f&method=slb.class_list.search&name=ac") + 1, "http://192.168.99.51/services/rest/V2.1/?format=json&session_id=af4f9a1724a52006e56cd73909aa3f&method=slb.class_list.search&name=ac");
	else
		sprintf_s(szUrl, "http://%s/services/rest/%s/?format=json&session_id=%s&method=%s",
			hHandle->stAttr.szHost, gc_szAxapiVers[Vers], hHandle->szSessionId,pszMethod);
	curl_easy_setopt(hHandle->hCurl, CURLOPT_URL, szUrl);

	//	Set post data.
	if (pszPost)
;//		curl_easy_setopt(hHandle->hCurl, CURLOPT_POST, pszPost);

	printf ("%s\n", szUrl);
	printf ("Post:\n%s\n", pszPost);

	CurlRet = curl_easy_perform(hHandle->hCurl);
	if (CurlRet != CURLE_OK)
	{
		curl_easy_cleanup(hHandle->hCurl);
		return AXAPIC_RET_ERROR_CURL;
	}

	DWORD	dwRet;
	if ((dwRet = WaitForSingleObject(hHandle->hMutex, INFINITE)) != WAIT_OBJECT_0)
	{
		dwRet = GetLastError();
		curl_easy_cleanup(hHandle->hCurl);
		return AXAPIC_RET_ERROR_MUTEX;
	}

	pstResp->pszData = (char *)malloc(hHandle->stResp.ulDataNum + 1);
	if (pstResp->pszData == NULL)
	{
		curl_easy_cleanup(hHandle->hCurl);
		return AXAPIC_RET_ERROR_MEMORY;
	}
	strcpy_s(pstResp->pszData, hHandle->stResp.ulDataNum + 1, hHandle->stResp.pszData);

	//	Close curl
	curl_easy_cleanup(hHandle->hCurl);

	return AXAPIC_RET_SUCCESS;
}