#pragma once
#include "resource.h"
#include <Windows.h>
#include <vector>
#include <shellscalingapi.h> // For GetDpiForWindow
#include <tchar.h>
#include <commctrl.h> // For Tab Control
#include "Tab1Dialog.h"
#include "Tab2Dialog.h"
#include "Tab3Dialog.h"
#include "Tab4Dialog.h"
#include "Tab5Dialog.h"

BOOL IsUserInGroup(DWORD dwGroupRID);// ����û��Ƿ�����ĳ����
BOOL IsRunAsAdmin();// ����Ƿ��Թ���Ա�������
void InstallPage(HWND hWndParent, HWND hTabCtrl, std::vector<HWND>& hTabPages, int IDD_WND, const TCHAR* pageName, DLGPROC dlgProc);
void ResizeTabPages(HWND hTabCtrl, std::vector<HWND>& hTabPages);
INT_PTR CALLBACK MainDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);