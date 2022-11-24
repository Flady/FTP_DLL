// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "org_i1_reg_CFTP.h"
#include "jni.h"
#include "jni_md.h"
#include <stdio.h>
#include <Windows.h>
#include <WinInet.h>
#include <string>
#include <iostream>
#include <TCHAR.h>
#include <winbase.h>
#include <shellapi.h>
#define MessageBox MessageBoxA
#define _T(x) __T(x)
#pragma comment(lib, "WinInet.lib")
using namespace std;

template< typename... Args >
std::string string_format(const char* format, Args... args)
{
	size_t length = std::snprintf(nullptr, 0, format, args...);
	if (length <= 0)
	{
		return "";
	}

	char* buf = new char[length + 1];
	std::snprintf(buf, length + 1, format, args...);

	std::string str(buf);
	delete[] buf;
	return std::move(str);
}
//ADD GBKTOUTF8 
string GBKToUTF8(const string& strGBK)
{
	string strOutUTF8 = "";
	WCHAR * str1;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char * str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	strOutUTF8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return strOutUTF8;
}

string Utf8ToGbk(const char* src_str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
}

BOOL FTP_Upload(char *szHostName, char *szUserName, char *szPassword, char *szUrlPath, char *FilePath)
{
	HINTERNET hInternet, hConnect, hFTPFile = NULL;
	DWORD dwBytesReturn = 0;
	DWORD UploadDataSize = 0;
	BYTE *pUploadData = NULL;
	DWORD dwRet, bRet = 0;
	hInternet = ::InternetOpen("WinInet Ftp", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	hConnect = ::InternetConnect(hInternet, szHostName, INTERNET_INVALID_PORT_NUMBER, szUserName, szPassword,
		INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
	if (!hConnect)
	{
		MessageBox(NULL, "Ftp Server Connect Failed!", "Notice", MB_OK);
		return FALSE;
	}
	if (Utf8ToGbk(FilePath).empty())
	{
		return FALSE;
	}
	else
	{	
	hFTPFile = ::FtpOpenFile(hConnect, szUrlPath, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, NULL);
	HANDLE hFile = ::CreateFile(Utf8ToGbk(FilePath).c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ |
		FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return FALSE;
	UploadDataSize = ::GetFileSize(hFile, NULL);
	pUploadData = new BYTE[UploadDataSize];
	::ReadFile(hFile, pUploadData, UploadDataSize, &dwRet, NULL);
	UploadDataSize = dwRet;
	bRet = ::InternetWriteFile(hFTPFile, pUploadData, UploadDataSize, &dwBytesReturn);
	if (FALSE == bRet)
	{
		delete[]pUploadData;
		return FALSE;
	}
	}
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
	delete[]pUploadData;
	return TRUE;
}

BOOL Ftp_SaveToFile(char *pszFileName, BYTE *pData, DWORD dwDataSize)
{
	HANDLE hFile = CreateFile(Utf8ToGbk(pszFileName).c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return FALSE;

	DWORD dwRet = 0;
	WriteFile(hFile, pData, dwDataSize, &dwRet, NULL);
	CloseHandle(hFile);
	return TRUE;
}

BOOL FTP_Download(char *szHostName, char *szUserName, char *szPassword, char *szUrlPath, char *SavePath)
{
	try {
		HINTERNET hInternet, hConnect, hFTPFile = NULL;
		BYTE *pDownloadData = NULL;
		DWORD dwDownloadDataSize = 0;
		DWORD dwBufferSize = 4096;
		BYTE *pBuf = NULL;
		DWORD dwBytesReturn = 0;
		DWORD dwOffset = 0;
		BOOL bRet = FALSE;
		LPCSTR filext = "This file does not exist";
		// 建立会话
		hInternet = InternetOpen("WinInet Ftp", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		// 建立连接
		hConnect = InternetConnect(hInternet, szHostName, INTERNET_INVALID_PORT_NUMBER,
			szUserName, szPassword, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
		if (!hConnect)
		{
			MessageBox(NULL, "Ftp Server Connect Failed!", "Notice", MB_OK);
			return FALSE;
		}
		// 打开FTP文件, 文件操作和本地操作相似
		if (Utf8ToGbk(szUrlPath).empty())
		{
			return FALSE;
		}
		hFTPFile = FtpOpenFile(hConnect, Utf8ToGbk(szUrlPath).c_str(), GENERIC_READ, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, NULL);
		if (!hFTPFile)
		{
			string szurlPath = Utf8ToGbk(szUrlPath).c_str();
			MessageBox(NULL, Utf8ToGbk(szUrlPath).c_str(), "Notice", MB_OK);
			MessageBox(NULL, filext, "Notice" , MB_OK);
			return FALSE;
		}
		// 获取文件大小
		dwDownloadDataSize = FtpGetFileSize(hFTPFile, NULL);

		// 获取文件大小
		dwDownloadDataSize = FtpGetFileSize(hFTPFile, NULL);
		// 申请动态内存
		pDownloadData = new BYTE[dwDownloadDataSize];

		RtlZeroMemory(pDownloadData, dwDownloadDataSize);
		pBuf = new BYTE[dwBufferSize];
		RtlZeroMemory(pBuf, dwBufferSize);
		// 接收数据
		do
		{
			bRet = InternetReadFile(hFTPFile, pBuf, dwBufferSize, &dwBytesReturn);
			if (FALSE == bRet)
				break;
			RtlCopyMemory((pDownloadData + dwOffset), pBuf, dwBytesReturn);
			dwOffset = dwOffset + dwBytesReturn;
		} while (dwDownloadDataSize > dwOffset);

		// 将数据保存为文件
		Ftp_SaveToFile(SavePath, pDownloadData, dwDownloadDataSize);
		// 释放内存
		delete[]pDownloadData;
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return TRUE;
	}
	catch (exception ex) {
		throw ex;
		return FALSE;
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}

char* jstringTostring(JNIEnv* env, jstring jstr)
{
	char* rtn = NULL;
	jclass clsstring = env->FindClass("java/lang/String");
	jstring strencode = env->NewStringUTF("utf-8");
	jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
	jsize alen = env->GetArrayLength(barr);
	jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
	if (alen > 0)
	{
		rtn = (char*)malloc(alen + 1);
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	env->ReleaseByteArrayElements(barr, ba, 0);
	return rtn;
}

JNIEXPORT jlong JNICALL Java_org_i1_reg_CFTP_FTPDownload
(JNIEnv * env, jobject, jstring sUrl, jstring sFilename, jstring sUser, jstring sPassword, jlong lShow_Download, jstring sFile_Type)
{
	string replace = "ftp://";
	string url = jstringTostring(env, sUrl);
	string curl = url.replace(url.find(replace), replace.length(), "");
	int pos = curl.rfind("/") + 1;
	string rurl = curl.substr(0, pos - 1);
	string local_filename = jstringTostring(env, sFile_Type);
	//Get ftp's filename
	string ftp_filename = curl.substr(pos);
	TCHAR szTempPathBuffer[MAX_PATH];
	GetTempPath(MAX_PATH,            // length of the buffer
		szTempPathBuffer);
	string local_Filename = szTempPathBuffer + local_filename;
	//MessageBox(NULL, const_cast<char *>(rurl.data()), "Notice", MB_OK);
	//BOOL bRet = FTP_Download(const_cast<char*>(rurl.data()), const_cast<char*>(jstringTostring(env, sUser)), const_cast<char*>(jstringTostring(env, sPassword)), const_cast<char*>(source_filename.data()), const_cast<char*>(filename.data()));
	//BOOL bRet = FTP_Download(const_cast<char*>(rurl.data()), const_cast<char*>(jstringTostring(env, sUser)), const_cast<char*>(jstringTostring(env, sPassword)), "/dh.txt", "E://dh2.txt");
	bool bret = FTP_Download(const_cast<char*>(rurl.data()), const_cast<char*>(jstringTostring(env, sUser)), const_cast<char*>(jstringTostring(env, sPassword)), const_cast<char*>(ftp_filename.data()), const_cast<char*>(local_Filename.data()));

	if (TRUE == bret) {
		ShellExecute(NULL, _T("open"), const_cast<LPCSTR>(local_Filename.data()), NULL, NULL, SW_NORMAL);
		return 1;
	}
	else
	{
		return -1L;
	}
}
JNIEXPORT jlong JNICALL Java_org_i1_reg_CFTP_FTPUpload
(JNIEnv * env, jobject, jstring sUrl, jstring sLocal_Filename, jstring sUser, jstring sPassword) {

	string rep = "ftp://";
	string buf = jstringTostring(env, sUrl);
	string Ftpurl = (buf.replace(buf.find(rep), rep.length(), ""));
	int pos = Ftpurl.rfind("/") + 1;
	string rurl = Ftpurl.substr(0, pos - 1);
	string ftp_filename = Ftpurl.substr(pos);
	string source_filename = jstringTostring(env, sLocal_Filename);
	string repstr = "\\";
	string source_filename1 = source_filename.replace(source_filename.find(repstr), repstr.length(), "\\");
	string dest_filename = ftp_filename;
	BOOL bRET = FALSE;
	bRET = FTP_Upload(const_cast<char*>(rurl.data()), const_cast<char*>(jstringTostring(env, sUser)), const_cast<char*>(jstringTostring(env, sPassword)), const_cast<char*>(dest_filename.data()), const_cast<char*>(source_filename1.data()));
	if (TRUE == bRET)
	{
		return 1;
	}
	else
	{
		return -1L;
	}
}