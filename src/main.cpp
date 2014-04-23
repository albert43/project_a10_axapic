#include <stdio.h>
#include <axapiclient.h>

int main(void)
{
	AXAPIC_RET		Ret;
	AXAPIC_HANDLE	hAxapi;
	AXAPIC_ATTR		stAttr;
	AXAPIC_RESPONSE	stResp;

	strcpy_s(stAttr.szHost, "192.168.99.51");
	strcpy_s(stAttr.szUsername, "admin");
	strcpy_s(stAttr.szPassword, "a10");
	Ret = Open(&hAxapi, &stAttr);
	if (Ret != AXAPIC_RET_SUCCESS)
		printf ("Open Failure. Ret = %d\n", Ret);

	Ret = Send(&hAxapi, AXAPI_VERS_2_1, "slb.class_list.search", "{\"name\":\"ac\"}", &stResp);
	if (Ret != AXAPIC_RET_SUCCESS)
		printf ("Send Failure. Ret = %d\n", Ret);

	printf ("Response: %d bytes\n", stResp.ulDataNum);
	printf ("%s\n", stResp.pszData);

	Close(&hAxapi);
}