// LaccoreConverter.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "Resource.h"

#include <Magick++.h>
#include <ShellAPI.h>
#include <strsafe.h>

#include <string>

#include "LaccoreConverter.h"
#include "convert.h"

using namespace Magick;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
BOOL CALLBACK	DlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT HandleDropFiles(HWND hWnd, HDROP hDrop);
void ConvertFiles(HWND hWnd);
BOOL InitDialog(HWND hWnd);
void RemoveSelectedFromList(HWND hWnd);
void SaveDefaults(const int defaultDpi, const int defaultScale);
int getDpi(HWND hWnd);
int getScale(HWND hWnd);

HWND gCurrentHwnd = NULL;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	InitializeMagick(NULL);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LACCORECONVERTER, szWindowClass, MAX_LOADSTRING);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LACCORECONVERTER));

	// Main message loop:
	int status;
	while ((status = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (status == -1)
			return -1;
		if (gCurrentHwnd == NULL || !IsDialogMessage(gCurrentHwnd, &msg))
		{

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DlgProc);

   if (!hWnd)
   {
      return FALSE;
   }

   DragAcceptFiles(hWnd, TRUE);
   ShowWindow(hWnd, nCmdShow);
   RECT r;
   GetWindowRect(hWnd, &r);
   MoveWindow(hWnd, r.left + 150, r.top + 150, r.right - r.left, r.bottom - r.top, TRUE);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
BOOL CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_INITDIALOG:
		return InitDialog(hWnd);
	case WM_ACTIVATE:
	{
		if (wParam == 0) // becoming inactive
			gCurrentHwnd = NULL;
		else // becoming active
			gCurrentHwnd = hWnd;
		return TRUE;
	}
	case WM_VKEYTOITEM:
	{
		if (LOWORD(wParam) == VK_BACK || LOWORD(wParam) == VK_DELETE)
		{
			RemoveSelectedFromList(hWnd);
			return TRUE;
		}
		else
			return FALSE;
	}
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId)
		{
		case IDOK:
		{
			EnableWindow(GetDlgItem(hWnd, IDOK), FALSE);
			std::string origTitle("Convert");
			std::string convertTitle("Converting...");
			SendDlgItemMessage(hWnd, IDOK, WM_SETTEXT, 0, (LPARAM)convertTitle.c_str());
			ConvertFiles(hWnd);
			SendDlgItemMessage(hWnd, IDOK, WM_SETTEXT, 0, (LPARAM)origTitle.c_str());
			EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
			return TRUE;
		}
		case IDC_SAVEDEFAULT:
		{
			const int defaultDpi = getDpi(hWnd);
			const int defaultScale = getScale(hWnd);
			SaveDefaults(defaultDpi, defaultScale);
			return TRUE;
		}
		case IDCANCEL:
			closeLog();
			DestroyWindow(hWnd);
			return TRUE;
		}
		break;
	case WM_DROPFILES:
		return HandleDropFiles(hWnd, (HDROP)wParam);
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		closeLog();
		DestroyWindow(hWnd);
		return TRUE;
	}
	return FALSE;
}

void LoadDefaults(int &defaultDpi, int &defaultScale)
{
	FILE *f = fopen("defaults.txt", "rt");
	if (f)
	{
		fscanf(f, "%d", &defaultDpi);
		fscanf(f, "%d", &defaultScale);
		fclose(f);
	}
	else
	{
		defaultDpi = 508;
		defaultScale = 30;
		SaveDefaults(defaultDpi, defaultScale); // might as well save 'em off
	}
}

void SaveDefaults(const int defaultDpi, const int defaultScale)
{
	FILE *f = fopen("defaults.txt", "wt");
	if (f)
	{
		fprintf(f, "%d %d\n", defaultDpi, defaultScale);
		fclose(f);
	}
}

BOOL InitDialog(HWND hWnd)
{
	int defaultDpi = 0, defaultScale = 0;
	LoadDefaults(defaultDpi, defaultScale);

	// default values
	SetDlgItemInt(hWnd, IDC_DPI_EDIT, defaultDpi, TRUE);
	SetDlgItemInt(hWnd, IDC_SCALE_EDIT, defaultScale, TRUE);
	SetActiveWindow(hWnd);

	// populate ruler list
	WIN32_FIND_DATA ffd;
	std::string rulerDir("rulers\\*");
	HANDLE hFind = FindFirstFile(rulerDir.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		MessageBox(hWnd, "No ruler files found - place at least one file in the rulers directory", 
		"Wuh-oh", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	int rulerCount = 0;
	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			LRESULT result = SendDlgItemMessage(hWnd, IDC_RULER_COMBO, CB_ADDSTRING, rulerCount, (LPARAM)ffd.cFileName);
			rulerCount++;
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);

	SendDlgItemMessage(hWnd, IDC_RULER_COMBO, CB_SETCURSEL, 0, 0);

	return TRUE;
}

LRESULT HandleDropFiles(HWND hWnd, HDROP hDrop)
{
	HWND hFileList = GetDlgItem(hWnd, IDC_FILE_LIST);
	const int listFileCount = SendMessage(hFileList, LB_GETCOUNT, 0, 0);

	UINT droppedFileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	for (UINT i = 0; i < droppedFileCount; i++) {
        char filename[MAX_PATH];
        DragQueryFile(hDrop, i, filename, MAX_PATH);
        std::string pathStr(filename);
		SendMessage(hFileList, LB_INSERTSTRING, listFileCount + i, (LPARAM)pathStr.c_str());
    }
	
	return 0;
}

int getDpi(HWND hWnd)
{
	return GetDlgItemInt(hWnd, IDC_DPI_EDIT, NULL, FALSE);
}

int getScale(HWND hWnd)
{
	return GetDlgItemInt(hWnd, IDC_SCALE_EDIT, NULL, FALSE);
}

void ConvertFiles(HWND hWnd)
{
	createOutputDirs(); // ensure directories are in place

	bool totalSuccess = true;
	
	const long int dpi = getDpi(hWnd);
	const long int vertScale = getScale(hWnd);

	if (dpi == 0L || vertScale == 0L)
	{
		MessageBox(hWnd, "Invalid DPI or scale value - must be an integer", "Shucks", MB_OK | MB_ICONERROR);
		return;
	}

	const int curSelIdx = SendDlgItemMessage(hWnd, IDC_RULER_COMBO, CB_GETCURSEL, 0, 0);
	const int len = SendDlgItemMessage(hWnd, IDC_RULER_COMBO, CB_GETLBTEXTLEN, curSelIdx, 0);
	char *rulerBuf = new char[len + 1];
	SendDlgItemMessage(hWnd, IDC_RULER_COMBO, CB_GETLBTEXT, curSelIdx, (LPARAM)rulerBuf);
	const std::string rulername(rulerBuf);

	HWND hFileList = GetDlgItem(hWnd, IDC_FILE_LIST);
	int numFiles = SendMessage(hFileList, LB_GETCOUNT, 0, 0);
	int i = 0;
	for (; i < numFiles; i++)
	{
		const int len = SendMessage(hFileList, LB_GETTEXTLEN, i, 0);
		char *pathBuf = new char[len + 1];
		SendMessage(hFileList, LB_GETTEXT, i, (LPARAM)pathBuf);
		std::string filename(pathBuf);

		if (processImage(filename, rulername, dpi, vertScale))
		{
			SendMessage(hFileList, LB_DELETESTRING, i, 0);
			numFiles--; i--;
		}
		else
		{
			totalSuccess = false;
		}

		delete[] pathBuf;
	}

	if (!totalSuccess)
		MessageBox(hWnd, "One or more images could not be processed completely. Open _error.log for possibly useful details.",
			"Shucks", MB_OK | MB_ICONWARNING);
}

void RemoveSelectedFromList(HWND hWnd)
{
	const int curSelIdx = SendDlgItemMessage(hWnd, IDC_FILE_LIST, LB_GETCURSEL, 0, 0);
	if (curSelIdx != LB_ERR)
	{
		SendDlgItemMessage(hWnd, IDC_FILE_LIST, LB_DELETESTRING, curSelIdx, 0);
	}
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
