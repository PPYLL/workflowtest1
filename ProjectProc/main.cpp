#include <windows.h>
#include<string.h>
#include<stdio.h>
#include <tlhelp32.h>


#ifdef _WIN64
typedef DWORD(WINAPI* Fn_ZwCreateThreadEx)(
	PHANDLE ThreadHandle,
	ACCESS_MASK DesiredAccess,
	LPVOID ObjectAttributes,
	HANDLE ProcessHandle,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	ULONG CreateThreadFlags,
	SIZE_T ZeroBits,
	SIZE_T StackSize,
	SIZE_T MaximumStackSize,
	LPVOID pUnkown);
#else
typedef DWORD(WINAPI* Fn_ZwCreateThreadEx)(
	PHANDLE ThreadHandle,
	ACCESS_MASK DesiredAccess,
	LPVOID ObjectAttributes,
	HANDLE ProcessHandle,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	BOOL CreateSuspended,
	DWORD dwStackSize,
	DWORD dw1,
	DWORD dw2,
	LPVOID pUnkown);
#endif


int ProcessCallBack(DWORD pid);//return:1,继续 0:终止

// 提权函数，启用调试特权
BOOL EnableDebugPrivilege()
{
	HANDLE hToken; // 用于保存进程访问令牌的句柄
	BOOL fOk = FALSE; // 用于保存函数是否执行成功的状态

	// 获取当前进程的访问令牌
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		TOKEN_PRIVILEGES tp; // 用于保存特权信息的结构体
		tp.PrivilegeCount = 1; // 设置特权数量为1

		// 获取“Debug Programs”特权的本地唯一标识符（LUID）
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);

		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; // 设置特权的属性为启用

		// 调整访问令牌，启用“Debug Programs”特权
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);

		fOk = (GetLastError() == ERROR_SUCCESS); // 检查是否成功启用特权
		CloseHandle(hToken); // 关闭访问令牌的句柄
	}
	return fOk; // 返回函数是否执行成功的状态
}
/*

void EnumProcessW(const WCHAR* s) {
	HANDLE hProcessSnap = NULL;
	DWORD pid = 0;
	SIZE_T len = wcslen(s);
	
	static PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		MessageBoxW(NULL, L"CreateToolhelp32Snapshot (of processes)", NULL, 4096);
		return;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		MessageBoxW(NULL, L"Process32First", NULL, 4096);
		CloseHandle(hProcessSnap);
		return;
	}

	do
	{
		
		if (len != wcslen(pe32.szExeFile)) continue;
		if (!memcmp(s, pe32.szExeFile, len)) {
            
            if(0==ProcessCallBack(pe32.th32ProcessID)) break;
		}
	} while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);
	hProcessSnap = NULL;
	return;
}
*/

void EnumProcessA(const char* s) {
	HANDLE hProcessSnap = NULL;
	DWORD pid = 0;
	int len = strlen(s);
	
	static PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "CreateToolhelp32Snapshot (of processes)", NULL, 4096);
		return;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		MessageBoxW(NULL, L"Process32First", NULL, 4096);
		CloseHandle(hProcessSnap);
		return;
	}

	do
	{
		
		if (len != strlen(pe32.szExeFile)) continue;
		if (!memcmp(s, pe32.szExeFile, len)) {
            
            
            if(0==ProcessCallBack(pe32.th32ProcessID)) break;
            /*
			//printf("%ls\n%ls\n",s ,pe32.szExeFile);
			struct PID* p = AddPidList();
			p->pid = pe32.th32ProcessID;
			*/
		}
	} while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);
	hProcessSnap = NULL;
	return;
}


int ProcessCallBack(DWORD pid){
    HANDLE hprocess=OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
	if (!hprocess) {
		puts("OpenProcess error\n");
		return 0;
	}
    //将自己进程中的LoadLibary写入目标进程
    LPVOID pLoadLibaryAddress = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    
    DWORD size=1000;//暂时写死，需要更改(TODO)
    
    LPVOID MyLoadLibary = VirtualAllocEx(hprocess, NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!MyLoadLibary) {
			puts("VirtualAllocEx error\n");
			return 0;
		}
    printf("MyLoadLibary::%p\n",MyLoadLibary);
    if (!WriteProcessMemory(hprocess, MyLoadLibary, pLoadLibaryAddress, size, NULL)) {
			puts("WriteProcessMemory error\n");
			return 0;
		}
    
    //向目标进程中写入dll路径
    char dllpath[]="D:\\dll.dll";//暂时写死，需要更改(TODO)
    size=strlen(dllpath)+1;
    LPVOID dlladdress = VirtualAllocEx(hprocess, NULL, size, MEM_COMMIT, PAGE_READWRITE);
		if (!MyLoadLibary) {
			puts("VirtualAllocEx error\n");
			return 0;
		}
    printf("Dlladdress::%p\n",dlladdress);
    printf("size::%d\n",size);
    if (!WriteProcessMemory(hprocess, dlladdress, dllpath, size, NULL)) {
			puts("WriteProcessMemory error\n");
			return 0;
		}
        
    //开始创建远线程
	Fn_ZwCreateThreadEx ZwCreateThreadEx = (Fn_ZwCreateThreadEx)GetProcAddress(LoadLibraryA("ntdll.dll"), "ZwCreateThreadEx");
	if (NULL == ZwCreateThreadEx)
	{
		printf("GetProcAddress error\n");
		return 1;
	}
    EnableDebugPrivilege();
    HANDLE hRemoteThread=NULL;
    DWORD dwStatus = ZwCreateThreadEx(&hRemoteThread, PROCESS_ALL_ACCESS, NULL, hprocess,
		(LPTHREAD_START_ROUTINE)MyLoadLibary, dlladdress, 0, 0, 0, 0, NULL);
	if (NULL == ZwCreateThreadEx)
	{
		printf("ZwCreateThreadEx error\n");
		return 1;
	}
    return 1;
}

int main(){
    printf("oldLoadLibraryA::%p\n",(PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"));
    EnumProcessA("explorer.exe");
    getchar();
    return 0;
}