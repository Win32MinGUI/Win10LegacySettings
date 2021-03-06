#include <Windows.h>
#include <stdio.h>

#define WM_HIDEWNDADDTOTRAY		0x0401
#define WM_SYSTRAY				0x0402
#define NOTIFYICONDATA_ID		__LINE__

HINSTANCE hInst;
#ifdef TRAYHOOK_ENABLED
HMODULE hHookDLL;
BOOL(*fRegisterHook)(HMODULE);
VOID(*fUnRegisterHook)();
#endif
HANDLE hMutex;
HWND hTrayManagerDummyWnd;
HWND hHookedWnd;

NOTIFYICONDATA nid;
UINT WM_TASKBARCREATED;

HICON GetWindowIcon(HWND hWnd)
{
	HICON hIco;
	if (hIco = (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL, 0))
		return hIco;
	if (hIco = (HICON)SendMessage(hWnd, WM_GETICON, ICON_BIG, 0))
		return hIco;
	if (hIco = (HICON)GetClassLongPtr(hWnd, GCLP_HICONSM))
		return hIco;
	if (hIco = (HICON)GetClassLongPtr(hWnd, GCLP_HICON))
		return hIco;
	// as for those windows without an icon, use the win logo instead
	return LoadIcon(NULL, IDI_WINLOGO);
}

VOID RestoreWindowFromTray(HWND hWnd)
{
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	//SwitchToThisWindow(hWnd, TRUE);
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

VOID HideWindowAndAddToTray(HWND hWnd)
{
	// Don't minimize MDI child windows
	if ((UINT)GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_MDICHILD)
		return;

	// If hWnd is a child window, find parent window (e.g. minimize button in
	// Office 2007 (ribbon interface) is in a child window)
	if ((UINT)GetWindowLongPtr(hWnd, GWL_STYLE) & WS_CHILD)
	{
		hWnd = GetAncestor(hWnd, GA_ROOT);
	}

	hHookedWnd = hWnd;
	nid.hIcon = GetWindowIcon(hHookedWnd);
	Shell_NotifyIcon(NIM_ADD, &nid);
	Shell_NotifyIcon(NIM_SETVERSION, &nid);
	// Hide window
	ShowWindow(hWnd, SW_HIDE);
}
LRESULT CALLBACK TrayManager_DummyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		WM_TASKBARCREATED = RegisterWindowMessage(L"TaskbarCreated");

#ifdef TRAYHOOK_ENABLED
#if defined(_M_X64) || defined(__amd64__)
		hHookDLL = LoadLibrary(L"Win10LegacySettingsTrayHook64.dll");
#elif defined(_M_IX86) || defined(__i386__)
		hHookDLL = LoadLibrary(L"Win10LegacySettingsTrayHook32.dll");
#endif
		fRegisterHook = (BOOL(*)(HMODULE))GetProcAddress(hHookDLL, "RegisterHook");
		fUnRegisterHook = (VOID(*)())GetProcAddress(hHookDLL, "UnRegisterHook");
		if (fRegisterHook && fUnRegisterHook)
			fRegisterHook(hHookDLL);
#endif
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = WM_SYSTRAY;
		nid.uID = NOTIFYICONDATA_ID;
		nid.hWnd = hWnd;
		nid.uVersion = NOTIFYICON_VERSION;
		wcscpy_s(nid.szTip, 128, L"Win10LegacySettings");
		break;

	case WM_HIDEWNDADDTOTRAY:
		HideWindowAndAddToTray((HWND)lParam);
		break;

	case WM_SYSTRAY:
		switch (lParam)
		{
		case NIN_SELECT:
			RestoreWindowFromTray(hHookedWnd);
			break;

		default:
			break;
		}
		break;

	case WM_DESTROY:
#ifdef TRAYHOOK_ENABLED
		fUnRegisterHook();
		FreeLibrary(hHookDLL);
#endif
		Shell_NotifyIcon(NIM_DELETE, &nid);
		PostQuitMessage(0);
		break;

	default:
		// Explorer 被關閉後重新啓動，讓程序任務欄通知圖標重新顯示出來
		if (uMsg == WM_TASKBARCREATED)
			Shell_NotifyIcon(NIM_ADD, &nid);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);;
}

ATOM TrayManager_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.lpfnWndProc = TrayManager_DummyWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"Win10LegacySettingsTray"; // TrayManagerDummyWindow name

	return RegisterClass(&wc);
}

#ifdef _DEBUG
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
#else
VOID Win32MinGUIEntryPoint()
#endif
{
	PWSTR* sArgv;
	INT nArgs;

	sArgv = CommandLineToArgvW(GetCommandLine(), &nArgs);
	if (sArgv)
	{
		// "--exit" switch
		if (nArgs >= 2 && !_wcsicmp(sArgv[1], L"--exit"))
		{
			// just send the WM_CLOSE message and then exit
			SendMessage(FindWindow(L"Win10LegacySettingsTray", L"Win10LegacySettingsTray"), WM_CLOSE, 0, 0);
			goto Exit;
		}

		hMutex = CreateMutexEx(NULL, L"Win10LegacySettingsTray", CREATE_MUTEX_INITIAL_OWNER, 0);
		if (hMutex)
		{
#ifdef _DEBUG
			hInst = hInstance;
#else
			hInst = GetModuleHandle(NULL);
#endif
			// Sleep(10000);
			TrayManager_RegisterClass(hInst);
			// 0-sized dummy window
			hTrayManagerDummyWnd = CreateWindow(L"Win10LegacySettingsTray", L"Win10LegacySettingsTray", WS_OVERLAPPED, 0, 0, 0, 0, (HWND)NULL, (HMENU)NULL, hInst, (LPVOID)NULL);
			if (hTrayManagerDummyWnd)
			{
				MSG msg;
				while (IsWindow(hTrayManagerDummyWnd) && GetMessage(&msg, NULL, 0, 0))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			CloseHandle(hMutex);
		}

	Exit:
		LocalFree(sArgv);
	}


#ifdef _DEBUG
	return 0;
#else
	ExitProcess(0);
#endif
}
