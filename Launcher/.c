#include <Windows.h>
#include <stdio.h>

#define WM_HIDEWNDADDTOTRAY		0x0401
#define WM_SYSTRAY				0x0402

HANDLE hMutex;
STARTUPINFO si;
PROCESS_INFORMATION pi;
DWORD nMilliseconds;

/*
* Win10LegacySettingsLauncher Command Line Switches Examples
* 
* When launched the 1st time:
* Win10LegacySettingsLauncher.exe										Launch the Win10LegacySettings HTML Application
* Win10LegacySettingsLauncher.exe --minimize-after-milliseconds 4000	Launch the Win10LegacySettings HTML Application and minimize it to tray after 4 seconds
*																		Note: this option only works when no previous instances exist
* 
* When there is already an instance:
* Win10LegacySettingsLauncher.exe										Bring the existing Win10LegacySettings HTML Application to the front
* Win10LegacySettingsLauncher.exe --minimize							Send message to the TrayManagerDummyWindow and minimize the existing Win10LegacySettings HTML Application to tray
* 
*/


DWORD WINAPI ThreadGUIProc_SleepAndMinimizeToTray(PVOID pM)
{
	//WCHAR szCMDLauncher[MAX_PATH] = L"Win10LegacySettingsLauncher.exe --minimize";
	if (pM)
	{
		Sleep(*(PDWORD)pM);
	}

	//memset(&si, 0, sizeof(si));
	//memset(&pi, 0, sizeof(pi));
	//si.cb = sizeof(STARTUPINFO);
	//if (CreateProcess(NULL, szCMDLauncher, NULL, NULL, FALSE, 0, NULL, pCurrentDirectoryOfLauncher, &si, &pi))
	//{
	//	CloseHandle(pi.hProcess);
	//	CloseHandle(pi.hThread);
	//}
	SendMessage(FindWindow(L"Win10LegacySettingsTray", L"Win10LegacySettingsTray"), WM_HIDEWNDADDTOTRAY, 0, (LPARAM)FindWindow(L"HTML Application Host Window Class", L"Win10 Legacy Settings (Administrator)"));

	return 0;
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
		hMutex = CreateMutexEx(NULL, L"Win10LegacySettings", CREATE_MUTEX_INITIAL_OWNER, 0);
		if (hMutex)
		{
			// WCHAR szCMDWScript[MAX_PATH] = L"\"WScript.exe\" Win10LegacySettings.vbs";
			// WCHAR szCMDHTA[MAX_PATH] = L"A:\\Windows\\System32\\mshta.exe A:\Users\ws\OneDrive\Files\BIN\Win10LegacySettings.hta {1E460BD7-F1C3-4B2E-88BF-4E770A288AF5}{1E460BD7-F1C3-4B2E-88BF-4E770A288AF5}";
			WCHAR szCMDHTA[MAX_PATH];
			WCHAR pCurrentDirectoryOfLauncher[MAX_PATH];
			memset(&si, 0, sizeof(si));
			memset(&pi, 0, sizeof(pi));
			si.cb = sizeof(STARTUPINFO);

			GetModuleFileName(NULL, pCurrentDirectoryOfLauncher, MAX_PATH);
			*wcsrchr(pCurrentDirectoryOfLauncher, '\\') = '\0';

			// GetWindowsDirectory(szCMDHTA, MAX_PATH);
			GetSystemDirectory(szCMDHTA, MAX_PATH);
			wcscat_s(szCMDHTA, MAX_PATH, L"\\mshta.exe ");
			wcscat_s(szCMDHTA, MAX_PATH, pCurrentDirectoryOfLauncher);
			wcscat_s(szCMDHTA, MAX_PATH, L"\\Win10LegacySettings.hta {1E460BD7-F1C3-4B2E-88BF-4E770A288AF5}{1E460BD7-F1C3-4B2E-88BF-4E770A288AF5}");

			// szCMDHTA MUST be writable
			// CreateProcess(L"Win10LegacySettings", L"WScript.exe Win10LegacySettings.vbs", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi); // Does NOT work !!
			if (CreateProcess(NULL, szCMDHTA, NULL, NULL, FALSE, 0, NULL, pCurrentDirectoryOfLauncher, &si, &pi))
			{
				if (nArgs >= 3 && !_wcsicmp(sArgv[1], L"--minimize-after-milliseconds"))
				{
					nMilliseconds = wcstoul(sArgv[2], NULL, 10);
					CreateThread(NULL, 0, ThreadGUIProc_SleepAndMinimizeToTray, &nMilliseconds, 0, NULL);
				}

				WaitForSingleObject(pi.hProcess, INFINITE);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
		}
		else // bring the existing HTML Application window to the front
		{
			HWND hWndHTA, hTrayManagerDummyWnd;

			hWndHTA = FindWindow(L"HTML Application Host Window Class", L"Win10 Legacy Settings (Administrator)");
			hTrayManagerDummyWnd = FindWindow(L"Win10LegacySettingsTray", L"Win10LegacySettingsTray");

			if (nArgs >= 2 && !_wcsicmp(sArgv[1], L"--minimize")) // Win10LegacySettings already launched, now act as hidetotray message sender
			{
				//if (hWndHTA)
				//	SendMessage(hWndHTA, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				if (hTrayManagerDummyWnd && hWndHTA)
					SendMessage(hTrayManagerDummyWnd, WM_HIDEWNDADDTOTRAY, 0, (LPARAM)hWndHTA);
			}
			else
			{
#ifndef TRAYHOOK_ENABLED
				if (hWndHTA && (GetWindowLong(hWndHTA, GWL_STYLE) & WS_VISIBLE)) // Win10LegacySettings already launched , now bring it to the front
				{
					ShowWindow(hWndHTA, SW_SHOWNORMAL);
					SetForegroundWindow(hWndHTA);
				}
				else
#endif
				{
					if (hTrayManagerDummyWnd) // Win10LegacySettings already launched and minimized to tray, now restore from tray icon
					{
						SendMessage(hTrayManagerDummyWnd, WM_SYSTRAY, 0, NIN_SELECT);
					}
				}
				
			}
		}
		LocalFree(sArgv);
	}

#ifdef _DEBUG
	return 0;
#else
	ExitProcess(0);
#endif
}
