/*
    Code by oxfemale
    email: projects@blackbox.team
    https://twitter.com/bytecodevm
    XMPP+OTR admin@blackbox.team
*/

#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <psapi.h>
#include <strsafe.h>
#include <wtypes.h>
#include <fstream>
#include <mshtml.h>
#include <exdisp.h>
#include <Exdisp.h>
#include <memory>
#include <string>  
#include <Shlwapi.h>


#define BUFSIZE 512
#define BUFFERSIZE 21
DWORD VirusWaitTime = 10000; //10 sec.
BOOL StopWork = FALSE;

DWORD PrintNotifyInfo(PFILE_NOTIFY_INFORMATION pNotify, wchar_t * lpszRootFolder);
DWORD WINAPI StartMonitoringDisk(void* lpszRootFolder);
DWORD StartMonitoring(HANDLE DiskThread[], wchar_t DiskName[], int &DiskList);




DWORD WINAPI StartMonitoringDisk(void* lpszRootFolder ) 
{
	wchar_t hdd[] = L"A:\\";
	wchar_t* temp = NULL;
	
	temp = (wchar_t*)lpszRootFolder;
	hdd[0] = temp[0];

	HANDLE hDir = CreateFileW(hdd,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		NULL
	);
	if ( hDir == INVALID_HANDLE_VALUE) return 0;

	OVERLAPPED o = {};
	o.hEvent = CreateEvent(0, FALSE, FALSE, 0);

	DWORD nBufferLength = 60 * 1024;
	BYTE* lpBuffer = new BYTE[nBufferLength];
	bool bStop = false;
	while (!bStop)
	{
		DWORD returnedBytes = 0;
		ReadDirectoryChangesW(hDir, lpBuffer, nBufferLength, TRUE, 
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | 
            FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | 
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS |
            FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY,
            &returnedBytes, &o, 0);
		DWORD dwWaitStatus = WaitForSingleObject(o.hEvent, INFINITE);
		switch (dwWaitStatus)
		{
			case WAIT_OBJECT_0:
			{
				DWORD seek = 0;
				while (seek < nBufferLength)
				{
					PFILE_NOTIFY_INFORMATION pNotify = PFILE_NOTIFY_INFORMATION(lpBuffer + seek);
					seek += pNotify->NextEntryOffset;

					PrintNotifyInfo(pNotify, hdd);

					if (pNotify->NextEntryOffset == 0)
						break;
				} // while
			}
			break;
			default:
				bStop = true;
				break;
		}
	}

	CloseHandle(o.hEvent);
	delete[] lpBuffer;
	return 0;
}



DWORD StartMonitoring(HANDLE DiskThread[], wchar_t DiskName[], int &DiskList)
{
	int n = 0;
BOOL Flag = FALSE;
BOOL ready = FALSE;
DWORD dr = GetLogicalDrives();
WORD OldErrorMode = 0;
wchar_t path[MAX_PATH] = { 0 };
wchar_t hdd[4] = L"A:\\";
UINT drive_type = 0;

for (int x = 0; x < 26; x++)
{
	n = ((dr >> x) & 1); // узнаём значение текущего бита
	if (n)
	{
		hdd[0] = (char)(65 + x); // получаем литеру диска
		//printf("Disk: %s\n", hdd );
		OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
		ready = PathFileExistsW(hdd);
		SetErrorMode(OldErrorMode); // восстанавливаем старый режим показа ошибок

		if (ready)
		{
			drive_type = GetDriveTypeW(hdd); // узнаём тип диска
            if (drive_type == DRIVE_REMOVABLE)
            {
                //wprintf(L"%s \t REMOVABLE\n", hdd);
            }
            else if (drive_type == DRIVE_FIXED)
			{
				wprintf(L"%s \t FIXED\n", hdd);
				DiskThread[DiskList] = CreateThread(NULL, 0, StartMonitoringDisk, (LPVOID)hdd, 0, NULL);
				if (NULL == DiskThread[DiskList])
				{
					continue;
				}
                wprintf(L"Start StartMonitoring Disk: %s\r\n", hdd);
				DiskName[DiskList] = hdd[0];
				DiskList++;
			}
			else if (drive_type == DRIVE_REMOTE)
			{
				//wprintf(L"%s \t REMOTE\n", hdd);
			}
			else if (drive_type == DRIVE_CDROM)
			{
				//wprintf(L"%s \t DRIVE_CDROM\n", hdd);
			}
			else if (drive_type == DRIVE_RAMDISK)
			{
				//wprintf(L"%s \t DRIVE_RAMDISK\n", hdd);
			}
			else
			{
				//wprintf(L"%s \t UNKNOW\n", hdd);
			}
		}
		else
		{
			wprintf(L"%s \t Disk not ready to read\n", hdd);
		}
	}
}

return 0;
}

void temp(HANDLE DiskThread[], wchar_t DiskName[], int &DiskList) {
	DiskThread[2] = (HANDLE)1;
	DiskName[2] = L'z';
	DiskList = 4;
	return;
}
DWORD PrintNotifyInfo(PFILE_NOTIFY_INFORMATION pNotify, wchar_t * lpszRootFolder)
{
	WCHAR szwFileName[MAX_PATH];
	WCHAR szwFullFileName[MAX_PATH];
	WCHAR szwFullFileNameLow[MAX_PATH];
	ULONG ulCount = min(pNotify->FileNameLength / 2, MAX_PATH - 1);
	wcsncpy_s(&szwFileName[0], MAX_PATH - 1, pNotify->FileName, ulCount + 3);
	szwFileName[ulCount] = L'\0';

	swprintf(szwFullFileName, MAX_PATH - 1, L"%s%s", lpszRootFolder, szwFileName);

	switch (pNotify->Action)
	{
	case FILE_ACTION_ADDED:
		wprintf(L"FILE_ACTION_ADDED: %s\n", szwFullFileName);
		break;
	case FILE_ACTION_REMOVED:
		wprintf(L"FILE_ACTION_REMOVED: %s\n", szwFullFileName);
		break;
	case FILE_ACTION_MODIFIED:
		wprintf(L"FILE_ACTION_MODIFIED: %s\n", szwFullFileName);
		break;
	case FILE_ACTION_RENAMED_OLD_NAME:
		wprintf(L"FILE_ACTION_RENAMED_OLD_NAME: %s\n", szwFullFileName);
		break;
	case FILE_ACTION_RENAMED_NEW_NAME:
		wprintf(L"FILE_ACTION_RENAMED_NEW_NAME: %s\n", szwFullFileName);
		break;
	}

	return 0;
}

void _tmain(int argc, TCHAR *argv[])
{
	HANDLE  DiskThread[64] = { 0 };
	wchar_t DiskName[64] = { 0 };
	int DiskList = 0;
	//DiskName[64] = { 0 };
	DWORD dwWaitStatus = 0;
	DWORD dwWaitTimer = 500;
	wchar_t hdd[] = L"A:\\";

    if (argc == 1)
    {
        printf("Usage:\r\n0 - Monitor all fixet disks.\r\nC - Monitor disk C:\\ or change disk char name to any A,B,C,D,E,...\r\n");
        exit(0);
    }
    memcpy(&hdd[0],argv[1],1);
    if (hdd[0] == '0')
    {
        StartMonitoring(DiskThread, DiskName, DiskList);
        printf("Disk number: [ %d ]\n", DiskList);
    }

    DiskThread[DiskList] = CreateThread(NULL, 0, StartMonitoringDisk, (LPVOID)hdd, 0, NULL);
    if (NULL == DiskThread[DiskList])
    {
        printf("ERROR: exit\r\n");
        exit(0);
    }
    wprintf(L"Start StartMonitoring Disk: %s\r\n", hdd);
    DiskName[DiskList] = hdd[0];
    DiskList++;

	if (DiskList > 0)
	{
		for (;;)
		{
			if (StopWork)
			{
				for (int i = 0; i < DiskList; i++)
				{
					TerminateThread(DiskThread[i], 0);
					CloseHandle(DiskThread[i]);
					Sleep(1);
				}
			}
			else
			{
				for (int i = 0; i < DiskList; i++)
				{
					dwWaitStatus = WaitForSingleObject(DiskThread[i], dwWaitTimer);
					switch (dwWaitStatus)
					{
						case WAIT_OBJECT_0:
							TerminateThread(DiskThread[i], 0);
							CloseHandle(DiskThread[i]);
							DiskThread[i] = NULL;
							hdd[0] = DiskName[i];
							DiskThread[i] = CreateThread(NULL, 0, StartMonitoringDisk, (LPVOID)hdd, 0, NULL);
							continue;
						case WAIT_ABANDONED:
							printf("disk [%c:\\] thread [%d] [ WAIT_ABANDONED ] \n", DiskName[i], DiskThread[i]);
							continue;
						case WAIT_TIMEOUT:
							//printf("disk [%c:\\] thread [%d] [ WAIT_TIMEOUT ] \n", DiskName[i], DiskThread[i]);
							continue;
						case WAIT_FAILED:
							printf("disk [%c:\\] thread [%d] [ WAIT_FAILED ] \n", DiskName[i], DiskThread[i]);
							continue;
					}
					Sleep(3000);
				}
			}
			Sleep(100);
		}
	}
}

