// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include<time.h>
#include<stdlib.h>
#include<Windows.h>
#pragma warning(disable : 4996)

char s[20];

/*
程序启动时会调用
*/
void __stdcall Start() {
    srand((unsigned)time(NULL));
}

/*
设置点名频率，单位:ms
该函数忽略mode
*/
int __stdcall Setfrequency() {
    return 10;
}

/*
设置点名模式
1：使用call.txt
2：使用call.dll
*/
int __stdcall SetMode() {
    return 2 ;
}

/*
点名的回调函数，决定了点名内容
*/
void __stdcall Callback(char *s) {
    itoa(rand() % 20 + 1, s, sizeof(s));
    //MessageBoxA(NULL, s, "", 0);
    return;
}

BOOL APIENTRY DllMain( HMODULE hModule,
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
    return TRUE;
}

