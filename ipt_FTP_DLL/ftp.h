#pragma once
#include "pch.h"
#include <stdio.h>
#include <Windows.h>
#include <WinInet.h>
#pragma comment(lib, "WinInet.lib")

BOOL FTP_Upload(char *szHostName, char *szUserName, char *szPassword, char *szUrlPath, char *FilePath);
BOOL Ftp_SaveToFile(char *pszFileName, BYTE *pData, DWORD dwDataSize);
BOOL FTP_Download(char *szHostName, char *szUserName, char *szPassword, char *szUrlPath, char *SavePath);
char* jstringTostring(JNIEnv* env, jstring jstr);
jstring stoJstring(JNIEnv* env, const char* pat);