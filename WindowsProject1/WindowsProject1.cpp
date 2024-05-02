#include <Windows.h>
#include<stdio.h>
#include<time.h>
#pragma comment(lib, "urlmon.lib")
#pragma warning(disable : 4996)
#define ID "2.1\n"

typedef void(__stdcall* StartQ)();
typedef int(__stdcall* SetfrequencyQ)();
typedef int(__stdcall* SetModeQ)();
typedef void(__stdcall* CallbackQ)(char* s);
StartQ Start;
SetfrequencyQ Setfrequency;
SetModeQ SetMode;
CallbackQ Callback;

void update(int flag, LPWSTR updatePath);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CALLBACK Timer1(HWND hwnd, UINT message, UINT_PTR iTimerID, DWORD dwTime);
char* randomFromFile();

char text[100];
int mode = 1;
int f = 30;
//int x, y;


//设置子控件自己的消息循环: SetWindowLong(hSubWnd,GWL_WNDPROC,(LONG)ButtonProc);
int WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
) {
	//MessageBoxA(NULL, "0", "start", 0);
	

	LPWSTR* argv; int argc = 0;
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	for (int i = 0; i < argc; ++i) {
		//MessageBoxW(NULL, argv[i], L"", 0);
		if (!(lstrcmpW(argv[i], L"/update"))) update(1, argv[2]);
	}
	update(0, argv[0]);


	HINSTANCE hModule = LoadLibraryA("call.dll");
	if (hModule) {
		mode = 2;
		Start = (StartQ)GetProcAddress(hModule, "Start");
		Setfrequency = (SetfrequencyQ)GetProcAddress(hModule, "Setfrequency");
		SetMode = (SetModeQ)GetProcAddress(hModule, "SetMode");
		Callback = (CallbackQ)GetProcAddress(hModule, "Callback");
		if ((!Start) || (!Setfrequency) || (!SetMode) || (!Callback)) {
			MessageBoxA(NULL, "Function缺失","error",0);
			ExitProcess(0);
		}
		Start();
		f = Setfrequency();
		mode = SetMode();
	}

	//MessageBoxA(NULL, "1", "start", 0);
	static TCHAR szClassName[] = TEXT("DMWin");   //窗口类名
	HWND hwnd;         //窗口句柄
	MSG msg;           //消息
	WNDCLASS wndclass; // 窗口类

	/**********第①步：注册窗口类**********/
	//为窗口类的各个字段赋值
	wndclass.style = CS_HREDRAW | CS_VREDRAW;	//窗口风格
	wndclass.lpfnWndProc = WndProc;		//窗口过程
	wndclass.cbClsExtra = 0;			//暂时不需要理解
	wndclass.cbWndExtra = 0;			//暂时不需要理解
	wndclass.hInstance = NULL;			//当前窗口句柄
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//窗口图标
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);		//鼠标样式
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//窗口背景画刷
	wndclass.lpszMenuName = NULL;		//窗口菜单
	wndclass.lpszClassName = szClassName; // 窗口类名

	//注册窗口
	RegisterClass(&wndclass);


	/*****第②步：创建窗口(并让窗口显示出来)*****/
	hwnd = CreateWindow(
		szClassName,		//窗口类的名字
		TEXT("点名器V2.1"),		//窗口标题（出现在标题栏）
		WS_VISIBLE | (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX),	//窗口风格
		(GetSystemMetrics(SM_CXSCREEN)-300)/2,			//初始化时x轴的位置
		(GetSystemMetrics(SM_CYSCREEN)-300)/2,			//初始化时y轴的位置
		300,			//窗口宽度
		300,			//窗口高度
		NULL,			//父窗口句柄
		NULL,			//窗口菜单句柄
		NULL,		//当前窗口的句柄
		NULL			//不使用该值
	);

	//显示窗口
	ShowWindow(hwnd, nShowCmd);
	//更新（绘制）窗口
	UpdateWindow(hwnd);


	/**********第③步：消息循环**********/
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

/**********第④步：窗口过程**********/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;		  //设备环境句柄
	PAINTSTRUCT ps;   //存储绘图环境的相关信息
	static HWND hBtn;
	RECT rect;
	switch (message)
	{
	case WM_CREATE:
		srand((unsigned)time(NULL));

		//ShowCursor(0);
		GetClientRect(hWnd, &rect);
		
		//创建按钮控件
		hBtn = CreateWindow(
			TEXT("button"), //按钮控件的类名
			TEXT("开始"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT| BS_OWNERDRAW/*扁平样式*/,
			rect.right / 2 - 100 /*X坐标*/, rect.bottom / 3 * 2 /*Y坐标*/, 200 /*宽度*/, 70/*高度*/,
			hWnd, (HMENU)1 /*控件唯一标识符*/, NULL, NULL
		);
		SetWindowPos(hWnd, HWND_TOPMOST,0,0,0,0, SWP_NOMOVE| SWP_NOSIZE);
		break;

	case WM_PAINT: {

		char s[100];
		ZeroMemory(s, sizeof(s));
		GetClientRect(hWnd, &rect);
		hdc = BeginPaint(hWnd, &ps);    //开始绘图并返回环境句柄

		LOGFONT logfont; //改变输出字体
		ZeroMemory(&logfont, sizeof(LOGFONT));
		logfont.lfCharSet = DEFAULT_CHARSET;
		logfont.lfWeight = 1000;
		logfont.lfHeight = -MulDiv(50, GetDeviceCaps(hdc, LOGPIXELSY), 72);//改26->27字体增大
		SelectObject(hdc, CreateFontIndirect(&logfont));

		SetTextColor(hdc, RGB(0, 0, 255));
		SetBkMode(hdc, TRANSPARENT);
		//sprintf(s, "(%u，%u)", x, y);
		//MessageBoxA(NULL, text, s, 0);
		if(text) strcpy(s, text);
		//MoveWindow(hBtn, rect.right / 2 - 100 /*X坐标*/, rect.bottom / 5 * 3 /*Y坐标*/, 200 /*宽度*/, 70/*高度*/, 0);

		rect.top = rect.bottom / 5;
		
		rect.bottom = rect.bottom / 5 * 3;
		//DrawTextA(hdc, s, strlen(s), &rect, DT_CENTER | DT_SINGLELINE);

		if(text)DrawTextA(hdc, text, strlen(text), &rect, DT_CENTER | DT_SINGLELINE|DT_VCENTER);


		EndPaint(hWnd, &ps);       //结束绘图并释放环境句柄
		
		
		/*
		//draw bottom
		hdc = BeginPaint(hBtn, &ps);    //开始绘图并返回环境句柄
		
		
		RECT itemRect;
		GetClientRect(hBtn, &itemRect);
		GetWindowTextA(hBtn, s, sizeof(s));
		
		
		//LOGFONT logfont; //改变输出字体
		ZeroMemory(&logfont, sizeof(LOGFONT));
		logfont.lfCharSet = DEFAULT_CHARSET;
		logfont.lfWeight = 800;
		logfont.lfHeight = -MulDiv(35, GetDeviceCaps(hdc, LOGPIXELSY), 72);//改26->27字体增大
		

		SetTextColor(hdc, RGB(0, 0, 255));
		SetBkMode(hdc, TRANSPARENT);

		FillRect(hdc, &itemRect, CreateSolidBrush(RGB(229, 228, 226)));
	
		SelectObject(hdc, CreateFontIndirect(&logfont));
		DrawTextA(hdc, s, strlen(s), &itemRect, DT_CENTER | DT_SINGLELINE| DT_VCENTER);
		
		EndPaint(hBtn, &ps);   
		*/
		
		break;
	}

	case WM_DRAWITEM: { //对于按钮
		
		PAINTSTRUCT ps;
		LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
		//声明一个指向DRAWITEMSTRUCT结构体的指针并将其指向存储着按钮构造信息的lParam
		RECT itemRect = lpDIS->rcItem; //按钮的矩形区域
		
		if (lpDIS->CtlType!= ODT_BUTTON) return 0;
		
		HDC hdc = lpDIS->hDC; //用于按钮绘制的DC
		HWND hBtn = lpDIS->hwndItem;
		
		// 这里画被按下去的按钮
		char s[100];
		GetWindowTextA(hBtn, s, sizeof(s));
		
		
		LOGFONT logfont; //改变输出字体
		ZeroMemory(&logfont, sizeof(LOGFONT));
		logfont.lfCharSet = DEFAULT_CHARSET;
		logfont.lfWeight = 800;
		logfont.lfHeight = -MulDiv(35, GetDeviceCaps(hdc, LOGPIXELSY), 72);//改26->27字体增大
		

		SetTextColor(hdc, RGB(0, 0, 255));
		SetBkMode(hdc, TRANSPARENT);

		FillRect(hdc, &itemRect, CreateSolidBrush(RGB(229, 228, 226)));
	
		SelectObject(hdc, CreateFontIndirect(&logfont));
		DrawTextA(hdc, s, strlen(s), &itemRect, DT_CENTER | DT_SINGLELINE| DT_VCENTER);
		
		return 1;
	}

	case WM_DESTROY:		//窗口销毁消息
		PostQuitMessage(0);
		break;

	case WM_COMMAND:

		if (LOWORD(wParam) == 1) {

			char s[100];
			GetWindowTextA(hBtn, s, sizeof(s));

			if (!strcmp(s, "开始")) {
				Timer1(NULL, 0, 0, 0);
				SetTimer(hWnd, 1, f, Timer1);
				SetWindowTextA(hBtn, (LPCSTR)"停止");
			}

			if (!strcmp(s, "停止")) {
				KillTimer(hWnd, 1);
				SetWindowTextA(hBtn, (LPCSTR)"开始");
			}

		}

	case WM_TIMER:
		switch (wParam) {
		case 1:
			break;
		default:
			break;
		}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/*timer回调*/
void CALLBACK Timer1(HWND hwnd, UINT message, UINT_PTR iTimerID, DWORD dwTime)
{
	/**
	x = rand() % 8 + 1; 
	if (1 == x) y = rand() % 4 + 1;
	if (2 == x || 3 == x || 7 == x || 8 == x || 9 == x) y = rand() % 5 + 1;
	if (4 == x || 5 == x || 6 == x) y = rand() % 6 + 1;
	*/
	if(1==mode)strcpy(text, randomFromFile());
	if(2==mode)Callback(text);
	//MessageBoxA(NULL, text, "", 0);
	
	
	RECT rect;
	GetClientRect(hwnd, &rect);
	rect.top = rect.bottom / 5;
	rect.bottom = rect.bottom / 5 * 3;
	InvalidateRect(hwnd,&rect , 1);
	return;

}




void update(int flag, LPWSTR updatePath) {
	static int isRestart;
	if (isRestart)return;
	if (0 == flag) {
		URLDownloadToFileA(0, "https://ppyll.github.io/rollcaller/update.ini", "rollcaller.ini", 0, NULL);
		HANDLE hFile = CreateFileA("rollcaller.ini", GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
		char str[100];
		ZeroMemory(str, sizeof(str));
		ReadFile(hFile, str, sizeof(str), NULL, NULL);
		//MessageBoxA(NULL, str, str, 0);
		CloseHandle(hFile);
		DeleteFileA("rollcaller.ini");
		char path[100];
		GetEnvironmentVariableA("TEMP", path, sizeof(path));
		strcat(path, "\\rollcaller.exe");
		DeleteFileA(path);
		if (strcmp(str, ID)) {//存在更新
			//MessageBoxA(NULL, path, "Update", 0);
			if (!(CopyFileA("rollcaller.exe", path, 0))) {
				char serr[100];
				itoa(GetLastError(), serr, 10);
				MessageBoxA(NULL, serr, "copyfile:err", 0);
			}
			
			PROCESS_INFORMATION lpProcessInformation;
			STARTUPINFOW lpStartupInfo;
			ZeroMemory(&lpStartupInfo, sizeof(lpStartupInfo));
			lpStartupInfo.cb = sizeof(lpStartupInfo);
			WCHAR s[1000];
			//lstrcpyW(s, updatePath);
			GetEnvironmentVariableW(L"TEMP", s, sizeof(s));
			lstrcatW(s, L"\\rollcaller.exe /update ");
			lstrcatW(s, updatePath);
			//MessageBoxW(NULL, s, L"cs", 0);
			CreateProcessW(NULL, s, NULL, NULL, FALSE, NULL, NULL, NULL, &lpStartupInfo, &lpProcessInformation);
			ExitProcess(0);
		}
	}
	else {
		if (6 == (MessageBoxA(NULL, "发现新版本，是否更新？", "更新", MB_YESNO| MB_SYSTEMMODAL))) {
			DeleteFileW(updatePath);
			URLDownloadToFileW(0, L"https://ppyll.github.io/rollcaller/rollcaller.exe", updatePath, 0, NULL);
			//MessageBoxW(NULL, updatePath, L"update1", 0);
			PROCESS_INFORMATION lpProcessInformation;
			STARTUPINFOW lpStartupInfo;
			ZeroMemory(&lpStartupInfo, sizeof(lpStartupInfo));
			lpStartupInfo.cb = sizeof(lpStartupInfo);
			CreateProcessW(NULL, updatePath, NULL, NULL, FALSE, NULL, NULL, NULL, &lpStartupInfo, &lpProcessInformation);
			ExitProcess(0);
		} 
		else {
			isRestart = 1;
		}

	}
	return;
}


int NameCount() {
	FILE* fp = fopen("call.txt", "r");
	if (!fp) {
		MessageBoxA(NULL, "缺少call.txt", "error", 4096);
		ExitProcess(0);
	}
	int count = 0;
	char c;
	while ((fread(&c, 1, 1, fp))==1) {
		if ('\n' == c) ++count;
	}
	fclose(fp);
	return count;
}

char *randomFromFile() {
	FILE* fp = fopen("call.txt", "r");
	if (!fp) {
		MessageBoxA(NULL, "缺少call.txt", "error", 4096);
		ExitProcess(0);
	}
	int count = NameCount();
	char s[100]; 
	for (int i = 0; i < rand() % (count+1) + 1;++i) fgets(s, sizeof(s), fp);
	fclose(fp);
	return s;
}
