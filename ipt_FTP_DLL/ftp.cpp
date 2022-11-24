
#include "pch.h"
#include <stdio.h>
#include <Windows.h>
#include <WinInet.h>
#include "jni.h"
#include "jni_md.h"
#include "stdlib.h"
#pragma comment(lib, "WinInet.lib")

BOOL FTP_Upload(char *szHostName, char *szUserName, char *szPassword, char *szUrlPath, char *FilePath)
{
	HINTERNET hInternet, hConnect, hFTPFile = NULL;
	DWORD dwBytesReturn = 0;
	DWORD UploadDataSize = 0;
	BYTE *pUploadData = NULL;
	DWORD dwRet, bRet = 0;

	hInternet = ::InternetOpen("WinInet Ftp Upload V1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	hConnect = ::InternetConnect(hInternet, szHostName, INTERNET_INVALID_PORT_NUMBER, szUserName, szPassword,
		INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
	hFTPFile = ::FtpOpenFile(hConnect, szUrlPath, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, NULL);
	HANDLE hFile = ::CreateFile(FilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ |
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
	delete[]pUploadData;
	return TRUE;
}

BOOL Ftp_SaveToFile(char *pszFileName, BYTE *pData, DWORD dwDataSize)
{
	HANDLE hFile = CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE,
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
	HINTERNET hInternet, hConnect, hFTPFile = NULL;
	BYTE *pDownloadData = NULL;
	DWORD dwDownloadDataSize = 0;
	DWORD dwBufferSize = 4096;
	BYTE *pBuf = NULL;
	DWORD dwBytesReturn = 0;
	DWORD dwOffset = 0;
	BOOL bRet = FALSE;

	// �����Ự
	hInternet = InternetOpen("WinInet Ftp", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	// ��������
	hConnect = InternetConnect(hInternet, szHostName, INTERNET_INVALID_PORT_NUMBER,
		szUserName, szPassword, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);

	// ��FTP�ļ�, �ļ������ͱ��ز�������
	hFTPFile = FtpOpenFile(hConnect, szUrlPath, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, NULL);
	// ��ȡ�ļ���С
	dwDownloadDataSize = FtpGetFileSize(hFTPFile, NULL);
	// ���붯̬�ڴ�
	pDownloadData = new BYTE[dwDownloadDataSize];

	RtlZeroMemory(pDownloadData, dwDownloadDataSize);
	pBuf = new BYTE[dwBufferSize];
	RtlZeroMemory(pBuf, dwBufferSize);
	// ��������
	do
	{
		bRet = InternetReadFile(hFTPFile, pBuf, dwBufferSize, &dwBytesReturn);
		if (FALSE == bRet)
			break;
		RtlCopyMemory((pDownloadData + dwOffset), pBuf, dwBytesReturn);
		dwOffset = dwOffset + dwBytesReturn;
	} while (dwDownloadDataSize > dwOffset);

	// �����ݱ���Ϊ�ļ�
	Ftp_SaveToFile(SavePath, pDownloadData, dwDownloadDataSize);
	// �ͷ��ڴ�
	delete[]pDownloadData;
	pDownloadData = NULL;
	return TRUE;
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

jstring stoJstring(JNIEnv* env, const char* pat)
{
	jclass strClass = env->FindClass("Ljava/lang/String;");
	jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
	jbyteArray bytes = env->NewByteArray(strlen(pat));
	env->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
	jstring encoding = env->NewStringUTF("utf-8");
	return (jstring)env->NewObject(strClass, ctorID, bytes, encoding);
}