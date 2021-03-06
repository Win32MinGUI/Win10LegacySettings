#include <Windows.h>
#include <stdio.h>

#define HOOKEDNAME L"Win10LegacySettingsTray"
#define WM_HIDEWNDADDTOTRAY  0x0401

#define DLLIMPORT __declspec(dllexport)

BOOL DLLIMPORT RegisterHook(HMODULE);
VOID DLLIMPORT UnRegisterHook();

HHOOK hHookMouse = NULL;
//HHOOK hHookGetMsg = NULL;
HWND hWndLastHit = NULL;

WCHAR sWndClassNameMouseProc[MAX_PATH];
WCHAR sWndTitleNameMouseProc[MAX_PATH];
//WCHAR sWndClassNameGetMsgProc[MAX_PATH];
//WCHAR sWndTitleNameGetMsgProc[MAX_PATH];


// A modified version of RBTray (http://rbtray.sourceforge.net/, https://sourceforge.net/projects/rbtray/)

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		MOUSEHOOKSTRUCT* info = (MOUSEHOOKSTRUCT*)lParam;

		GetClassName(info->hwnd, sWndClassNameMouseProc, MAX_PATH);
		GetWindowText(info->hwnd, sWndTitleNameMouseProc, MAX_PATH);
		if (!wcscmp(sWndClassNameMouseProc, L"HTML Application Host Window Class") && !wcscmp(sWndTitleNameMouseProc, L"Win10 Legacy Settings (Administrator)"))
		{
			if (wParam == WM_NCLBUTTONDOWN || wParam == WM_NCLBUTTONUP)
			{
				BOOL isHit = (info->wHitTestCode == HTMINBUTTON);
				if (wParam == WM_NCLBUTTONDOWN && isHit)
				{
					hWndLastHit = info->hwnd;
					return 1;
				}
				else if (wParam == WM_NCLBUTTONUP && isHit)
				{
					if (info->hwnd == hWndLastHit)
					{
						PostMessage(FindWindow(HOOKEDNAME, HOOKEDNAME), WM_HIDEWNDADDTOTRAY, 0, (LPARAM)info->hwnd);
					}
					hWndLastHit = NULL;
					return 1;
				}
				else
				{
					hWndLastHit = NULL;
				}
			}
			// else if (wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP)
			else if (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP)
			{
				hWndLastHit = NULL;
			}
		}
	}
	return CallNextHookEx(hHookMouse, nCode, wParam, lParam);
}
/*
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		MSG* msg = (MSG*)lParam;

		GetClassName(msg->hwnd, sWndClassNameGetMsgProc, MAX_PATH);
		GetWindowText(msg->hwnd, sWndTitleNameGetMsgProc, MAX_PATH);
		if (!wcscmp(sWndClassNameGetMsgProc, L"HTML Application Host Window Class") && !wcscmp(sWndTitleNameGetMsgProc, L"Win10 Legacy Settings (Administrator)"))
		{
			// minimized to taskbar
			if (msg->message == WM_SIZE && msg->lParam == 0 && msg->wParam == SIZE_MINIMIZED)
			{
				PostMessage(FindWindow(HOOKEDNAME, HOOKEDNAME), WM_HIDEWNDADDTOTRAY, 0, (LPARAM)msg->hwnd);
				return 1;
			}
		}

	}

	return CallNextHookEx(hHookGetMsg, nCode, wParam, lParam);
}
*/
BOOL RegisterHook(HMODULE hLib)
{
	hHookMouse = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)MouseProc, hLib, 0);
	//hHookGetMsg = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, hLib, 0);
	//if (hHookMouse == NULL || hHookGetMsg == NULL)
	//	return FALSE;
	if (hHookMouse == NULL)
		return FALSE;
	return TRUE;
}

VOID UnRegisterHook()
{
	if (hHookMouse)
	{
		UnhookWindowsHookEx(hHookMouse);
		hHookMouse = NULL;
	}
	//if (hHookGetMsg)
	//{
	//	UnhookWindowsHookEx(hHookGetMsg);
	//	hHookGetMsg = NULL;
	//}
}
/*
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	// Perform actions based on the reason for calling.

	Sleep(12000);
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
*/