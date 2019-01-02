#include <stdlib.h>
#include <stdio.h>

#include "MgHttpClient.h"



bool writeJsonToFile(const char* szBuffer, int nlen, const char* szFileName)
{
	if ( NULL == szBuffer || nlen <= 0 || NULL == szFileName )
	{
		return true;
	}

	FILE* fpJson = fopen(szFileName, "wb");
	if (NULL == fpJson)
	{
		printf("fopen failed %s\n", szFileName);
		return false;
	}
	fwrite( szBuffer, 1, nlen, fpJson );
	fclose( fpJson );
	return true;
}

int main(int argc, char** argv)
{
	const char* szJsonUrl			= "http://www.hkex.com.hk/chi/csm/shortsell/data_tab_short_selling_20181218c.js";
	const char* szSaveResFileName = "20181218c.js";
	const char* szSaveResFileName2 = "20181219c.js";

	{
		//block model;
		MgHttpClient httpClient;
		httpClient.sendReq(szJsonUrl, writeJsonToFile, szSaveResFileName, NULL, NULL, false);
		int i = 0;
		++i;
	}
	

	//nonblock model(nonHttpClient 这个对象必须在callback函数执行完才能释放,或者一直存在);
	MgHttpClient nonHttpClient;
	nonHttpClient.sendReq(szJsonUrl, writeJsonToFile, szSaveResFileName2, NULL, NULL, true);
	int i = 0;
	++i;



	system("pause");
	return 0;
}