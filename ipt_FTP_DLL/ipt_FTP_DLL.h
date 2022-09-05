#pragma once
#include "pch.h"
#include "org_i1_reg_CFTP.h"
#include "jni.h"
#include "jni_md.h"
#include <stdio.h>
#include <Windows.h>
#include <WinInet.h>
#include "ftp.h"
#pragma comment(lib, "WinInet.lib")
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
);
