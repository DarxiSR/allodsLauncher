/* --- Coded by Darxis, 03.03.2024 [5:55] */

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>

#define __Kernel32 "kernel32.dll"
#define __DllMainPath "games.dll"
#define __LoadLibraryA "LoadLibraryA"
#define __AllodsLauncherName "Launcher.exe"

using namespace std;

void __hideWindow()
{
	HWND GET_CONSOLE_PROCESS = GetConsoleWindow();
	ShowWindow(GET_CONSOLE_PROCESS, SW_HIDE);
	return;
}

void __setPriority()
{
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	SetErrorMode(SEM_FAILCRITICALERRORS);
	SetProcessPriorityBoost(GetCurrentProcess(), 1);
}

int __getProcessId(const char* _getProcessName) {

	HANDLE getProcessHandle = NULL;
	PROCESSENTRY32 getProcessEntry;
	BOOL getResult = NULL;
	int getPid = 0;

	getProcessHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (INVALID_HANDLE_VALUE == getProcessHandle)
	{
		return 0;
	}

	getProcessEntry.dwSize = sizeof(PROCESSENTRY32);

	getResult = Process32First(getProcessHandle, &getProcessEntry);

	while (getResult)
	{
		if (strcmp(_getProcessName, getProcessEntry.szExeFile) == 0)
		{
			getPid = getProcessEntry.th32ProcessID;
			break;
		}
		getResult = Process32Next(getProcessHandle, &getProcessEntry);
	}

	CloseHandle(getProcessHandle);
	return getPid;
}

int __getProcessAccess(HANDLE* _getProcessHandle, DWORD _getProcessId)
{
	*_getProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, false, _getProcessId);

	if (*_getProcessHandle == NULL)
	{
		wcout << "[-] Error of opening process!\n" << endl;
		system("pause");
		ExitProcess(-1000);
	}
	else
	{
		wcout << "[+] Success! Process is opened..." << endl;
		return 1000;
	}
}


int __injectDll(DWORD _getProcessId, char* _getDllFile)
{
	HANDLE getProcessHandle = NULL;
	LPVOID LoadLibraryA = NULL;
	LPVOID getBaseAddress = NULL;
	HANDLE getProcessThread = NULL;

	int dllFileSize = strlen(_getDllFile) + 1;

	if (__getProcessAccess(&getProcessHandle, _getProcessId) < 0)
	{
		wcout << "[-] Error of get process access!\n" << endl;
		system("pause");
		ExitProcess(-50);
	}

	LoadLibraryA = (LPVOID)GetProcAddress(GetModuleHandle(__Kernel32), __LoadLibraryA);

	if (!LoadLibraryA)
	{
		wcout << "[-] Error of calling ring3 'LoadLibraryA' API function!\n" << endl;
		system("pause");
		ExitProcess(-100);
	}
	else
	{
		getBaseAddress = VirtualAllocEx(getProcessHandle, NULL, dllFileSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		if (!getBaseAddress)
		{
			wcout << "[-] Error of calling VirtualAllocEx function!\n" << endl;
			system("pause");
			ExitProcess(-200);
		}
		else
		{
			if (!WriteProcessMemory(getProcessHandle, getBaseAddress, _getDllFile, dllFileSize, NULL))
			{
				wcout << "[-] Error of writing bytes in memory!\n" << endl;
				system("pause");
				ExitProcess(-300);
			}
			else
			{
				getProcessThread = CreateRemoteThread(getProcessHandle, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryA, getBaseAddress, 0, NULL);

				if (!getProcessThread)
				{
					wcout << "[-] Error of creating thread in process!\n" << endl;
					system("pause");
					ExitProcess(-400);
				}
				else
				{
					WaitForSingleObject(getProcessThread, INFINITE);
					VirtualFreeEx(getProcessHandle, getBaseAddress, dllFileSize, MEM_RELEASE);

					if (CloseHandle(getProcessThread) == NULL)
					{
						wcout << "[-] Error of closing HANDLE {getProcessThread}!\n" << endl;
						system("pause");
						ExitProcess(-500);
					}
					else
					{
						return 0;
					}
					if (CloseHandle(getProcessHandle) == NULL)
					{
						wcout << "[-] Error of closing HANDLE {getProcessHandle}!\n" << endl;
						system("pause");
						ExitProcess(-600);
					}
					else
					{
						return 0;
					}
				}
			}
		}
	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	STARTUPINFO getStartupInfo;
	PROCESS_INFORMATION getProcessInfo;

	ZeroMemory(&getStartupInfo, sizeof(getStartupInfo));
	getStartupInfo.cb = sizeof(getStartupInfo);
	ZeroMemory(&getProcessInfo, sizeof(getProcessInfo));

	if (!CreateProcess(__AllodsLauncherName, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &getStartupInfo, &getProcessInfo))
	{
		wcout << "[-] Error of starting Allods Online Launcher!\n" << endl;
		system("pause");
		ExitProcess(-10);
	}
	else
	{
		try
		{
			__setPriority();
			__hideWindow();
		}
		catch (...)
		{
			wcout << "[-] Exception error of process action!\n" << endl;
			system("pause");
			ExitProcess(-20);
		}

		try
		{
			int getProcessId = 0;
			char getDllPath[] = __DllMainPath;
			ifstream getFileHandle;

			getFileHandle.open(getDllPath);

			if (!getFileHandle)
			{
				getFileHandle.close();
				wcout << "[-] Error of opening game.dll file!\n" << endl;
				system("pause");
				ExitProcess(-30);
			}
			else
			{
				getFileHandle.close();
				getProcessId = __getProcessId(__AllodsLauncherName);

				__injectDll(getProcessId, getDllPath);
				Sleep(1000);
				const auto getLauncherProcess = OpenProcess(PROCESS_TERMINATE, false, getProcessId);
				TerminateProcess(getLauncherProcess, 0);
				CloseHandle(getLauncherProcess);

				return 0;
			}
		}
		catch (...)
		{
			wcout << "[-] Global exception error!\n" << endl;
			system("pause");
			ExitProcess(-40);
		}
	}
}