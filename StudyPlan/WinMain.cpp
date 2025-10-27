#include "WinMain.h"

#pragma comment(lib, "Shcore.lib")

// ������
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{

	HINSTANCE hInst = hInstance;

	if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
	{
		// ���� Per-Monitor ��֪ (Windows 8.1+)
		HRESULT hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		MessageBox(NULL, TEXT("����Ϊ Per-Monitor DPI ��֪"), TEXT("DPI����"), MB_OK | MB_ICONINFORMATION);
		if (FAILED(hr))
		{
			// ���ս���ΪϵͳDPI��֪ (Windows Vista+)
			SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
			MessageBox(NULL, TEXT("����Ϊϵͳ DPI ��֪"), TEXT("DPI����"), MB_OK | MB_ICONINFORMATION);
		}
	}

	// ���Դ���һ������������
	HANDLE mutex = CreateMutex(NULL, FALSE, TEXT("ѧϰ�ճ�"));

	// ������
	if (mutex == NULL)
	{
		MessageBox(NULL, TEXT("CreateMutex����!"), TEXT("����"), MB_OK | MB_ICONERROR);
		return -1;
	}
	// ��黥�����Ƿ��Ѵ��ڣ��������Ƿ������У�
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL, TEXT("�����Ѿ�������!"), TEXT("����"), MB_OK | MB_ICONERROR);
		CloseHandle(mutex);
		return -1;  // �����Ѿ����У��˳�����
	}

	BOOL bIsAdmin = IsRunAsAdmin();
	BOOL bIsGuest = IsUserInGroup(DOMAIN_ALIAS_RID_GUESTS);

	if (bIsAdmin)
	{
		// �Թ���ԱȨ������ʱ��ֱ��ִ�г������������
	}
	else if (bIsGuest) // ����û��������˻�����ʾ��Ϣ������Ȩ���������ԣ�ֱ��ִ��ʣ��ĳ������
	{
		MessageBox(NULL, TEXT("��ǰ�˻����ǹ���Ա���޷���ȡ����ԱȨ�ޣ����򼴽��رա�"), TEXT("����"), MB_OK | MB_ICONERROR);
		return -1;
	}
	else if (!bIsAdmin && !bIsGuest)
	{
		// �����ǰ�����Թ���Ա������У����������Թ���Ա�����������
		TCHAR szPath[MAX_PATH];
		if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		{
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.lpVerb = TEXT("runas");
			sei.lpFile = szPath;
			sei.hwnd = NULL;
			sei.nShow = SW_NORMAL;

			if (!ShellExecuteEx(&sei))
			{
				// �û����ܾܾ�����Ȩ�޻�����������
				MessageBox(NULL, TEXT("�޷��Թ���ԱȨ���������г���"), TEXT("����"), MB_OK | MB_ICONERROR);
				return -1; // �˳�����
			}
			// �ɹ���������Ȩ�޵Ľ��̣������˳���ǰ����
			return 0;
		}
		else
		{
			MessageBox(NULL, TEXT("��ȡ����·��ʧ�ܣ�"), TEXT("����"), MB_OK | MB_ICONERROR);
			return -1;
		}
	}

	// ��ȡϵͳĬ������
	LANGID currentLanguage = GetUserDefaultLangID();
	// �����̵߳��û���������
	SetThreadUILanguage(currentLanguage);

	// �����Ի���
	HWND hwndDlg = CreateDialogParam(
		hInstance,             // Ӧ�ó���ʵ���ľ��
		MAKEINTRESOURCE(IDD_Page_Main),  // �Ի���ģ�����Դ��ʶ��
		NULL,                  // �����ھ�������û�и����ڣ���ΪNULL
		MainDialogProc,            // �Ի�����̻ص�����
		(LPARAM)NULL           // ���ݸ��Ի�����̵��Զ��������������贫����ΪNULL
	);
	if (hwndDlg == NULL)
	{
		MessageBox(NULL, TEXT("�޷������Ի���"), TEXT("����"), MB_OK | MB_ICONERROR);
		return 0;
	}

	MSG msg;
	// ��ʾ�Ի���
	ShowWindow(hwndDlg, nShowCmd);
	UpdateWindow(hwndDlg);
	// ��Ϣѭ��
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(hwndDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	// ���˳�ǰ�ͷŻ�����
	CloseHandle(mutex);

	return (unsigned int)msg.wParam;
}

// ����: ��鵱ǰ�û��Ƿ�����ָ�����û���
BOOL IsUserInGroup(DWORD dwGroupRID)
{
	// ��ʼ������
	BOOL bIsUserInGroup = FALSE;
	// ����SID��ʶ��Ȩ��
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	// ָ���û����SID�İ�ȫ��ʶ��ָ��
	PSID pGroup;
	// ���䲢��ʼ��SID
	// �����ֱ�Ϊ��SID��ʶ��Ȩ�����ӱ�ʶ����������RID���û���RID��
	BOOL bResult = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		dwGroupRID,
		0, 0, 0, 0, 0, 0,
		&pGroup);
	// ������䲢��ʼ��SID�ɹ�
	if (bResult)
	{
		// ��⵱ǰ�û��������Ƿ����ָ�����û���SID
		// ��һ������NULL��ʾʹ�õ�ǰ���̵�����
		if (!CheckTokenMembership(NULL, pGroup, &bIsUserInGroup))
		{
			// ���CheckTokenMembershipʧ�ܣ�����ΪFALSE
			bIsUserInGroup = FALSE;
		}
		// �ͷ�SID�ڴ�
		FreeSid(pGroup);
	}
	// ���ؼ����
	return bIsUserInGroup;
}

// ����: �жϵ�ǰ�����Ƿ��Թ���Ա�������
BOOL IsRunAsAdmin()
{
	// ���ȳ���ʹ��IsUserInGroup���ж��û��Ƿ����ڹ���Ա��
	BOOL bIsAdmin = IsUserInGroup(DOMAIN_ALIAS_RID_ADMINS);

	// ��ȡ��ǰ���̵����ƾ��
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		// ����TOKEN_ELEVATION�ṹ�����ڴ洢���Ƶ�������Ϣ
		TOKEN_ELEVATION Elevation{};
		DWORD cbSize = sizeof(TOKEN_ELEVATION);

		// ��ȡ������Ϣ�е�������־
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize))
		{
			// �ر����ƾ��
			CloseHandle(hToken);
			// ���ص�ǰ�����Ƿ����������Ƿ��Թ���Ա������У�
			return Elevation.TokenIsElevated;
		}
	}

	// ���ǰ���ȡ���ƾ���ɹ�������Ҫ�رվ��
	if (hToken)
	{
		CloseHandle(hToken);
	}

	// ���û���Թ���Ա������У��򷵻�FALSE
	return FALSE;
}

// ������ǩҳ����
void InstallPage(HWND hWndParent, HWND hTabCtrl, std::vector<HWND>& hTabPages, int IDD_WND, const TCHAR* pageName, DLGPROC dlgProc)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // �����Ӵ��ڣ�����������ΪTab�ؼ����ڵ�������
    HWND hTabPage = CreateDialogParam(
        hInstance,
        MAKEINTRESOURCE(IDD_WND),
        hWndParent,
        dlgProc,
        NULL);

    if (!hTabPage)
    {
        DWORD err = GetLastError();
        TCHAR msg[256] = { 0 };
        _sntprintf_s(msg, _countof(msg), _TRUNCATE, _T("������ǩҳʧ��! ������: %d"), err);
        MessageBox(hWndParent, msg, _T("����"), MB_OK | MB_ICONERROR);
        return;
    }

    // ��ӱ�ǩҳ��Tab�ؼ�
    TCITEM tie = { 0 };
    tie.mask = TCIF_TEXT;
    tie.pszText = const_cast<TCHAR*>(pageName); // Tab�ؼ��Ḵ���ַ���

    int tabIndex = TabCtrl_GetItemCount(hTabCtrl);
    if (TabCtrl_InsertItem(hTabCtrl, tabIndex, &tie) == -1)
    {
        DWORD err = GetLastError();
        TCHAR msg[256] = { 0 };
        _sntprintf_s(msg, _countof(msg), _TRUNCATE, _T("�����ǩҳʧ��! ������: %d"), err);
        MessageBox(hWndParent, msg, _T("����"), MB_OK | MB_ICONERROR);

        DestroyWindow(hTabPage);
        return;
    }

    hTabPages.push_back(hTabPage);
    ShowWindow(hTabPage, SW_HIDE);
}

// ����Tab�ؼ����Ӵ��ڴ�С�ĺ���
void ResizeTabPages(HWND hTabCtrl, std::vector<HWND>& hTabPages)
{
    // ��ȡDPI��������
    const UINT dpi = GetDpiForWindow(hTabCtrl);
    const float scale = dpi / 96.0f;

    // ������ȷ����������
    RECT rcTab;
    GetClientRect(hTabCtrl, &rcTab);

    // ������ƫ�Ʋ��֣���Ҫ�����������Σ�
    const int tabHeaderHeight = static_cast<int>(22 * scale);
    rcTab.top += tabHeaderHeight;

    // �����Ӵ���λ�úʹ�С
    for (auto& page : hTabPages)
    {
        if (IsWindow(page))
        {
            SetWindowPos(
                page,
                NULL,
                rcTab.left,
                rcTab.top,
                rcTab.right - rcTab.left,
                rcTab.bottom - rcTab.top,
                SWP_NOZORDER | SWP_NOACTIVATE
            );
        }
    }
}

INT_PTR CALLBACK MainDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hTabCtrl;
    static std::vector<HWND> hTabPages;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        HICON ShakaIcon = LoadIcon(GetModuleHandle(NULL), TEXT("SHAKAFHD"));
        if (!ShakaIcon)
        {
            MessageBox(hWnd, TEXT("ͼ�����ʧ��"), TEXT("����"), MB_ICONERROR);
        }
        else
        {
            SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)ShakaIcon);
            SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)ShakaIcon);
        }

        // ��ȡ��Դ��Ԥ�����Tab�ؼ�
        hTabCtrl = GetDlgItem(hWnd, IDC_TAB1);
        if (!hTabCtrl)
        {
            MessageBox(hWnd, TEXT("Tab�ؼ���ȡʧ��"), TEXT("����"), MB_ICONERROR);
            EndDialog(hWnd, -1);
            return TRUE;
        }

        // ��ӱ�ǩҳ
        InstallPage(hWnd, hTabCtrl, hTabPages, IDD_Page_0, TEXT("ѧϰĿ��"), Tab1DialogProc);
        InstallPage(hWnd, hTabCtrl, hTabPages, IDD_Page_1, TEXT("�̲���Դ"), Tab2DialogProc);
        InstallPage(hWnd, hTabCtrl, hTabPages, IDD_Page_2, TEXT("ʱ�����"), Tab3DialogProc);
        InstallPage(hWnd, hTabCtrl, hTabPages, IDD_Page_3, TEXT("��ɽ���"), Tab4DialogProc);
        InstallPage(hWnd, hTabCtrl, hTabPages, IDD_Page_4, TEXT("ѧϰ����"), Tab5DialogProc);

        // ����Tab�ؼ����Ӵ��ڴ�С
        ResizeTabPages(hTabCtrl, hTabPages);

        // ��ʾ��һ����ǩҳ
        if (!hTabPages.empty())
        {
            TabCtrl_SetCurSel(hTabCtrl, 0);
            ShowWindow(hTabPages[0], SW_SHOW);
        }
    }
    return TRUE;

    case WM_SIZE:
    {
        // �������ڴ�С�ı�ʱ������Tab�ؼ���С
        if (hTabCtrl && IsWindow(hTabCtrl))
        {
            // ��ȡ�����ڿͻ�����С
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            // ����Tab�ؼ���С����������ͻ���
            SetWindowPos(hTabCtrl, NULL,
                0, 0,
                rcClient.right, rcClient.bottom,
                SWP_NOZORDER | SWP_NOACTIVATE);

            // �������б�ǩҳ�Ӵ��ڵĴ�С
            ResizeTabPages(hTabCtrl, hTabPages);
        }
    }
    return TRUE;

    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->idFrom == IDC_TAB1 && pnmh->code == TCN_SELCHANGE)
        {
            int nSel = TabCtrl_GetCurSel(hTabCtrl);
            for (size_t i = 0; i < hTabPages.size(); ++i)
            {
                ShowWindow(hTabPages[i], (i == nSel) ? SW_SHOW : SW_HIDE);
            }
        }
    }
    return FALSE;

    case WM_COMMAND:
    {
        // ���������������
        return 0;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return TRUE;

    case WM_DESTROY:
    {
        // ���������Ӵ���
        for (auto& page : hTabPages)
        {
            if (IsWindow(page))
                DestroyWindow(page);
        }
        hTabPages.clear();
        PostQuitMessage(0); // �˳���Ϣѭ��
    }
    return TRUE;

    default:
        return FALSE;
    }
}
