#include "Function.h"

#pragma comment(lib, "shlwapi.lib")

// ����ı��Ƿ�ΪĬ����ʾ�ı�
BOOL IsDefaultPromptText(LPCWSTR text)
{
	return wcscmp(text, DEFAULT_PROMPT_TEXT) == 0;
}

// ��ȡ�ļ����ݵ��༭��
void LoadFile(HWND hDlg, int EDIT, LPCWSTR filePath)
{
	BOOL bFileLoaded = FALSE;
	HWND hEdit = GetDlgItem(hDlg, EDIT);

	// ȷ���༭�����
	if (!hEdit) return;

	// �ļ�������ʱֱ������Ĭ���ı�������
	if (!PathFileExists(filePath))
	{
		SetWindowText(hEdit, DEFAULT_PROMPT_TEXT);
		SetProp(hDlg, PROP_FILE_LOADED, (HANDLE)(INT_PTR)FALSE);
		return;
	}

	// ���ļ�
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
		_stprintf_s(szMsg, _countof(szMsg), L"�޷����ļ�: %d", GetLastError());
		MessageBox(hDlg, szMsg, L"����", MB_OK | MB_ICONERROR);
		SetWindowText(hEdit, DEFAULT_PROMPT_TEXT);
		SetProp(hDlg, PROP_FILE_LOADED, (HANDLE)(INT_PTR)FALSE);
		return;
	}

	// ��ȡ�ļ����ݣ�ʡ�Բ����ظ����룬����ԭ���߼���
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
		MessageBox(hDlg, L"�ڴ����ʧ��", L"����", MB_OK | MB_ICONERROR);
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

		// ����ļ�������Ч��
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
		_stprintf_s(szMsg, _countof(szMsg), L"��ȡ�ļ�ʧ��: %d", GetLastError());
		MessageBox(hDlg, szMsg, L"����", MB_OK | MB_ICONERROR);
		SetWindowText(hEdit, DEFAULT_PROMPT_TEXT);
		bFileLoaded = FALSE;
	}

	// ������Դ
	delete[] pBuffer;
	CloseHandle(hFile);
	SetProp(hDlg, PROP_FILE_LOADED, (HANDLE)(INT_PTR)bFileLoaded);
}

// ����༭�����ݵ��ļ�
BOOL SaveFile(HWND hDlg, int EDIT, LPCWSTR filePath)
{
	HWND hEdit = GetDlgItem(hDlg, EDIT);
	if (!hEdit) return FALSE;

	int textLength = GetWindowTextLength(hEdit);
	wchar_t* buffer = new (std::nothrow) wchar_t[textLength + 1];
	if (!buffer)
	{
		MessageBox(hDlg, L"�ڴ����ʧ��", L"����", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	GetWindowText(hEdit, buffer, textLength + 1);

	// ����Ƿ�ΪĬ����ʾ�ı�����ı�
	if (textLength == 0 || IsDefaultPromptText(buffer))
	{
		MessageBox(hDlg, L"��������Ч�ı����ٱ���", L"��ʾ", MB_OK | MB_ICONINFORMATION);
		delete[] buffer;
		return FALSE;
	}

	// �ļ������߼�������ԭ�У�
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
				_stprintf_s(szMsg, _countof(szMsg), L"д������ʧ��: %d", GetLastError());
				MessageBox(hDlg, szMsg, L"����", MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			TCHAR szMsg[256];
			_stprintf_s(szMsg, _countof(szMsg), L"д��BOMʧ��: %d", GetLastError());
			MessageBox(hDlg, szMsg, L"����", MB_OK | MB_ICONERROR);
		}
		CloseHandle(hFile);
	}
	else
	{
		TCHAR szMsg[256];
		_stprintf_s(szMsg, _countof(szMsg), L"�޷������ļ�: %d", GetLastError());
		MessageBox(hDlg, szMsg, L"����", MB_OK | MB_ICONERROR);
	}

	delete[] buffer;
	return success;
}