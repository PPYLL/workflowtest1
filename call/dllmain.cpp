#include <Windows.h>
#include "MinHook.h"

#pragma comment(lib,"libMinHook-x64-v120-md.lib")
  
typedef HWND (WINAPI *FindWindow)(LPCTSTR lpClassName,LPCTSTR lpWindowName);

OldMessageBox fpMessageBoxA = NULL;

// 自定义弹窗
int WINAPI MyMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
  int ret = fpMessageBoxA(hWnd, "Hook Inject hello lyshark", lpCaption, uType);
  return ret;
}

// 安装钩子
void SetHook()
{
  if (MH_Initialize() == MB_OK)
  {
    MH_CreateHook(&MessageBoxA, &MyMessageBoxA, reinterpret_cast<void**>(&fpMessageBoxA));
    MH_EnableHook(&MessageBoxA);
  }
}

// 卸载钩子
void UnHook()
{
  if (MH_DisableHook(&MessageBoxA) == MB_OK)
  {
    MH_Uninitialize();
  }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    SetHook();
    break;
  case DLL_PROCESS_DETACH:
    UnHook();
    break;
  }
  return TRUE;
}
