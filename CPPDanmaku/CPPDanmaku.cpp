#include <iostream>//Contains header files
#include <Windows.h>
#include <wininet.h>//
#include "CJsonObject.hpp"

#pragma comment(lib,"wininet.lib")//wininet lib

#define URL_STRING L"http://api.live.bilibili.com/ajax/msg"//Bilive API
#define _HTTP_ARAC L"Content-Type: application/x-www-form-urlencoded\r\n"
char _HTTP_POST[] = "roomid=5322&csrf_token=&csrf=&visit_id=";//roomid parameters.

typedef struct {
	int uid;
	std::string name;
	std::string time;
	std::string text;
} DM_DATA;

DM_DATA old_list[10] = { 0 };
DM_DATA new_list[10] = { 0 };

//The server sends back data in utf-8 encoding, while most Windows are GBK encoding, which needs to be converted.
std::string Utf8ToGbk(const char* src_str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	std::string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
}

int main()
{
	TCHAR szHostName[128];
	TCHAR szUrlPath[256];
	URL_COMPONENTS crackedURL = { 0 };
	crackedURL.dwStructSize = sizeof(URL_COMPONENTS);
	crackedURL.lpszHostName = szHostName;
	crackedURL.dwHostNameLength = ARRAYSIZE(szHostName);
	crackedURL.lpszUrlPath = szUrlPath;
	crackedURL.dwUrlPathLength = ARRAYSIZE(szUrlPath);
	InternetCrackUrl(URL_STRING, (DWORD)URL_STRING, 0, &crackedURL);

	HINTERNET hInternet = InternetOpen(L"Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/15.0.849.0 Safari/535.1", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hInternet == NULL)
		return -1;

	HINTERNET hHttpSession = InternetConnect(hInternet, crackedURL.lpszHostName, crackedURL.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (hHttpSession == NULL) {
		InternetCloseHandle(hInternet);
		std::cout << GetLastError();
		return -2;
	}

	HINTERNET hHttpRequest = HttpOpenRequest(hHttpSession, L"POST", crackedURL.lpszUrlPath, NULL, L"", NULL, 0, 0);
	if (hHttpRequest == NULL) {
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hInternet);
		return -3;
	}

label:

	// 查询http状态码（这一步不是必须的）,但是HttpSendRequest()必须要调用
	DWORD dwRetCode = 0;
	DWORD dwSizeOfRq = sizeof(DWORD);
	if (!HttpSendRequest(hHttpRequest, _HTTP_ARAC, 0, _HTTP_POST, sizeof(_HTTP_POST)) ||
		!HttpQueryInfo(hHttpRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwRetCode, &dwSizeOfRq, NULL)
		|| dwRetCode >= 400) {
		InternetCloseHandle(hHttpRequest);
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hInternet);
		return -4;
	}

#define READ_BUFFER_SIZE 1024
	std::string strRet = "";
	BOOL bRet = FALSE;
	char szBuffer[READ_BUFFER_SIZE + 1] = { 0 };
	DWORD dwReadSize = READ_BUFFER_SIZE;
	while (true)
	{
		bRet = InternetReadFile(hHttpRequest, szBuffer, READ_BUFFER_SIZE, &dwReadSize);
		if (!bRet || (0 == dwReadSize))
		{
			break;
		}
		szBuffer[dwReadSize] = '\0';
		strRet.append(szBuffer);
	}

	std::string strRetGBK = Utf8ToGbk(strRet.c_str());

	
	neb::CJsonObject oJson(strRetGBK);
	
	for (int i = 0; i < 10; i++) {
		oJson["data"]["room"][i].Get("uid", new_list[i].uid);
		if (new_list[i].uid == 0)break;
		oJson["data"]["room"][i].Get("nickname", new_list[i].name);
		oJson["data"]["room"][i].Get("timeline", new_list[i].time);
		oJson["data"]["room"][i].Get("text", new_list[i].text);
	}
	
	for (int j = 0; j < 10; j++) {
		int k = 1;
		for (int i = 0; i < 10; i++) {
			if (old_list[i].uid == new_list[j].uid &&
				old_list[i].name == new_list[j].name &&
				old_list[i].time == new_list[j].time &&
				old_list[i].text == new_list[j].text)k = 0;
		}
		if (k)std::cout << "[" << new_list[j].name << "]" << new_list[j].text << std::endl;
	}
	for (int i = 0; i < 10; i++) {
		old_list[i].uid = new_list[i].uid;
		old_list[i].name = new_list[i].name;
		old_list[i].time = new_list[i].time;
		old_list[i].text = new_list[i].text;
	}
	
	Sleep(3000);
	goto label;


	return 0;
}