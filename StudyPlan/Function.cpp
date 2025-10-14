#include "Function.h"

#pragma comment(lib, "shlwapi.lib")

// 检查文本是否为默认提示文本
BOOL IsDefaultPromptText(LPCWSTR text)
{
	return wcscmp(text, DEFAULT_PROMPT_TEXT) == 0;
}

// 读取文件内容到编辑框
void LoadFile(HWND hDlg, int EDIT, LPCWSTR filePath)
{
	BOOL bFileLoaded = FALSE;
	HWND hEdit = GetDlgItem(hDlg, EDIT);

	// 确保编辑框存在
	if (!hEdit) return;

	// 文件不存在时直接设置默认文本并返回
	if (!PathFileExists(filePath))
	{
		SetWindowText(hEdit, DEFAULT_PROMPT_TEXT);
		SetProp(hDlg, PROP_FILE_LOADED, (HANDLE)(INT_PTR)FALSE);
		return;
	}

	// 打开文件
	HANDLE hFile = CreateFile(
		filePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		TCHAR szMsg[256];
		_stprintf_s(szMsg, _countof(szMsg), L"无法打开文件: %d", GetLastError());
		MessageBox(hDlg, szMsg, L"错误", MB_OK | MB_ICONERROR);
		SetWindowText(hEdit, DEFAULT_PROMPT_TEXT);
		SetProp(hDlg, PROP_FILE_LOADED, (HANDLE)(INT_PTR)FALSE);
		return;
	}

	// 读取文件内容（省略部分重复代码，保持原有逻辑）
	DWORD fileSize = GetFileSize(hFile, NULL);
	if (fileSize == INVALID_FILE_SIZE || fileSize == 0)
	{
		CloseHandle(hFile);
		SetWindowText(hEdit, DEFAULT_PROMPT_TEXT);
		SetProp(hDlg, PROP_FILE_LOADED, (HANDLE)(INT_PTR)FALSE);
		return;
	}

	BYTE* pBuffer = new (std::nothrow) BYTE[fileSize + 2];
	if (!pBuffer)
	{
		MessageBox(hDlg, L"内存分配失败", L"错误", MB_OK | MB_ICONERROR);
		CloseHandle(hFile);
		SetWindowText(hEdit, DEFAULT_PROMPT_TEXT);
		SetProp(hDlg, PROP_FILE_LOADED, (HANDLE)(INT_PTR)FALSE);
		return;
	}

	DWORD bytesRead;
	if (ReadFile(hFile, pBuffer, fileSize, &bytesRead, NULL) && bytesRead > 0)
	{
		pBuffer[bytesRead] = 0;
		pBuffer[bytesRead + 1] = 0;
		LPCWSTR fileContent = (LPCWSTR)pBuffer;

		// 检查文件内容有效性
		BOOL hasValidContent = FALSE;
		for (DWORD i = 0; i < bytesRead / sizeof(WCHAR); i++)
		{
			if (fileContent[i] != L'\0' && !iswspace(fileContent[i]))
			{
				hasValidContent = TRUE;
				break;
			}
		}

		if (hasValidContent)
		{
			SetWindowText(hEdit, fileContent);
			bFileLoaded = TRUE;
		}
		else
		{
			SetWindowText(hEdit, DEFAULT_PROMPT_TEXT);
			bFileLoaded = FALSE;
		}
	}
	else
	{
		TCHAR szMsg[256];
		_stprintf_s(szMsg, _countof(szMsg), L"读取文件失败: %d", GetLastError());
		MessageBox(hDlg, szMsg, L"错误", MB_OK | MB_ICONERROR);
		SetWindowText(hEdit, DEFAULT_PROMPT_TEXT);
		bFileLoaded = FALSE;
	}

	// 清理资源
	delete[] pBuffer;
	CloseHandle(hFile);
	SetProp(hDlg, PROP_FILE_LOADED, (HANDLE)(INT_PTR)bFileLoaded);
}

// 保存编辑框内容到文件
BOOL SaveFile(HWND hDlg, int EDIT, LPCWSTR filePath)
{
	HWND hEdit = GetDlgItem(hDlg, EDIT);
	if (!hEdit) return FALSE;

	int textLength = GetWindowTextLength(hEdit);
	wchar_t* buffer = new (std::nothrow) wchar_t[textLength + 1];
	if (!buffer)
	{
		MessageBox(hDlg, L"内存分配失败", L"错误", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	GetWindowText(hEdit, buffer, textLength + 1);

	// 检查是否为默认提示文本或空文本
	if (textLength == 0 || IsDefaultPromptText(buffer))
	{
		MessageBox(hDlg, L"请输入有效文本后再保存", L"提示", MB_OK | MB_ICONINFORMATION);
		delete[] buffer;
		return FALSE;
	}

	// 文件保存逻辑（保持原有）
	HANDLE hFile = CreateFile(
		filePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	BOOL success = FALSE;
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		const WCHAR BOM = 0xFEFF;

		if (WriteFile(hFile, &BOM, sizeof(WCHAR), &bytesWritten, NULL) &&
			bytesWritten == sizeof(WCHAR))
		{
			if (WriteFile(hFile, buffer, textLength * sizeof(wchar_t), &bytesWritten, NULL) &&
				bytesWritten == textLength * sizeof(wchar_t))
			{
				success = TRUE;
			}
			else
			{
				TCHAR szMsg[256];
				_stprintf_s(szMsg, _countof(szMsg), L"写入内容失败: %d", GetLastError());
				MessageBox(hDlg, szMsg, L"错误", MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			TCHAR szMsg[256];
			_stprintf_s(szMsg, _countof(szMsg), L"写入BOM失败: %d", GetLastError());
			MessageBox(hDlg, szMsg, L"错误", MB_OK | MB_ICONERROR);
		}
		CloseHandle(hFile);
	}
	else
	{
		TCHAR szMsg[256];
		_stprintf_s(szMsg, _countof(szMsg), L"无法创建文件: %d", GetLastError());
		MessageBox(hDlg, szMsg, L"错误", MB_OK | MB_ICONERROR);
	}

	delete[] buffer;
	return success;
}