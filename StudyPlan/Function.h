#pragma once
#include "resource.h"
#include <windows.h>
#include <commdlg.h>
#include <shlwapi.h>  // ����·��������
#include <tchar.h>
#include <new.h>

constexpr auto PROP_FILE_LOADED = L"FileLoadedFlag";
constexpr auto PROP_EDIT_FONT = L"EditControlFont";
constexpr auto DEFAULT_PROMPT_TEXT = L"�������ճ���ı�...";

BOOL IsDefaultPromptText(LPCWSTR text);
void LoadFile(HWND hDlg, int EDIT, LPCWSTR filePath);
BOOL SaveFile(HWND hDlg, int EDIT ,LPCWSTR filePath);