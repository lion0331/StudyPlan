#include "WinMain.h"

#pragma comment(lib, "Shcore.lib")

// 主函数
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{

	HINSTANCE hInst = hInstance;

	if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
	{
		// 尝试 Per-Monitor 感知 (Windows 8.1+)
		HRESULT hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		MessageBox(NULL, TEXT("降级为 Per-Monitor DPI 感知"), TEXT("DPI设置"), MB_OK | MB_ICONINFORMATION);
		if (FAILED(hr))
		{
			// 最终降级为系统DPI感知 (Windows Vista+)
			SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
			MessageBox(NULL, TEXT("降级为系统 DPI 感知"), TEXT("DPI设置"), MB_OK | MB_ICONINFORMATION);
		}
	}

	// 尝试创建一个命名互斥体
	HANDLE mutex = CreateMutex(NULL, FALSE, TEXT("学习日常"));

	// 检查错误
	if (mutex == NULL)
	{
		MessageBox(NULL, TEXT("CreateMutex错误!"), TEXT("错误"), MB_OK | MB_ICONERROR);
		return -1;
	}
	// 检查互斥体是否已存在（即程序是否已运行）
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL, TEXT("程序已经在运行!"), TEXT("错误"), MB_OK | MB_ICONERROR);
		CloseHandle(mutex);
		return -1;  // 程序已经运行，退出程序
	}

	BOOL bIsAdmin = IsRunAsAdmin();
	BOOL bIsGuest = IsUserInGroup(DOMAIN_ALIAS_RID_GUESTS);

	if (bIsAdmin)
	{
		// 以管理员权限运行时，直接执行程序的正常代码
	}
	else if (bIsGuest) // 如果用户是来宾账户，显示信息并跳过权限提升尝试，直接执行剩余的程序代码
	{
		MessageBox(NULL, TEXT("当前账户不是管理员，无法获取管理员权限，程序即将关闭。"), TEXT("错误"), MB_OK | MB_ICONERROR);
		return -1;
	}
	else if (!bIsAdmin && !bIsGuest)
	{
		// 如果当前不是以管理员身份运行，则尝试重新以管理员身份启动程序
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
				// 用户可能拒绝提升权限或发生其他错误
				MessageBox(NULL, TEXT("无法以管理员权限重新运行程序！"), TEXT("错误"), MB_OK | MB_ICONERROR);
				return -1; // 退出程序
			}
			// 成功启动提升权限的进程，现在退出当前进程
			return 0;
		}
		else
		{
			MessageBox(NULL, TEXT("获取程序路径失败！"), TEXT("错误"), MB_OK | MB_ICONERROR);
			return -1;
		}
	}

	// 获取系统默认语言
	LANGID currentLanguage = GetUserDefaultLangID();
	// 设置线程的用户界面语言
	SetThreadUILanguage(currentLanguage);

	// 创建对话框
	HWND hwndDlg = CreateDialogParam(
		hInstance,             // 应用程序实例的句柄
		MAKEINTRESOURCE(IDD_Page_Main),  // 对话框模版的资源标识符
		NULL,                  // 父窗口句柄，如果没有父窗口，则为NULL
		MainDialogProc,            // 对话框过程回调函数
		(LPARAM)NULL           // 传递给对话框过程的自定义参数，如果无需传递则为NULL
	);
	if (hwndDlg == NULL)
	{
		MessageBox(NULL, TEXT("无法创建对话框！"), TEXT("错误"), MB_OK | MB_ICONERROR);
		return 0;
	}

	MSG msg;
	// 显示对话框
	ShowWindow(hwndDlg, nShowCmd);
	UpdateWindow(hwndDlg);
	// 消息循环
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(hwndDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	// 在退出前释放互斥体
	CloseHandle(mutex);

	return (unsigned int)msg.wParam;
}

// 函数: 检查当前用户是否属于指定的用户组
BOOL IsUserInGroup(DWORD dwGroupRID)
{
	// 初始化变量
	BOOL bIsUserInGroup = FALSE;
	// 定义SID标识符权威
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	// 指向用户组的SID的安全标识符指针
	PSID pGroup;
	// 分配并初始化SID
	// 参数分别为：SID标识符权威、子标识符数量、域RID、用户组RID等
	BOOL bResult = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		dwGroupRID,
		0, 0, 0, 0, 0, 0,
		&pGroup);
	// 如果分配并初始化SID成功
	if (bResult)
	{
		// 检测当前用户的令牌是否包含指定的用户组SID
		// 第一个参数NULL表示使用当前进程的令牌
		if (!CheckTokenMembership(NULL, pGroup, &bIsUserInGroup))
		{
			// 如果CheckTokenMembership失败，设置为FALSE
			bIsUserInGroup = FALSE;
		}
		// 释放SID内存
		FreeSid(pGroup);
	}
	// 返回检测结果
	return bIsUserInGroup;
}

// 函数: 判断当前进程是否以管理员身份运行
BOOL IsRunAsAdmin()
{
	// 首先尝试使用IsUserInGroup来判断用户是否属于管理员组
	BOOL bIsAdmin = IsUserInGroup(DOMAIN_ALIAS_RID_ADMINS);

	// 获取当前进程的令牌句柄
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		// 定义TOKEN_ELEVATION结构体用于存储令牌的提升信息
		TOKEN_ELEVATION Elevation{};
		DWORD cbSize = sizeof(TOKEN_ELEVATION);

		// 获取令牌信息中的提升标志
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize))
		{
			// 关闭令牌句柄
			CloseHandle(hToken);
			// 返回当前令牌是否被提升（即是否以管理员身份运行）
			return Elevation.TokenIsElevated;
		}
	}

	// 如果前面获取令牌句柄成功，则需要关闭句柄
	if (hToken)
	{
		CloseHandle(hToken);
	}

	// 如果没有以管理员身份运行，则返回FALSE
	return FALSE;
}

// 创建标签页函数
void InstallPage(HWND hWndParent, HWND hTabCtrl, std::vector<HWND>& hTabPages, int IDD_WND, const TCHAR* pageName, DLGPROC dlgProc)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // 创建子窗口，父窗口设置为Tab控件所在的主窗口
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
        _sntprintf_s(msg, _countof(msg), _TRUNCATE, _T("创建标签页失败! 错误码: %d"), err);
        MessageBox(hWndParent, msg, _T("错误"), MB_OK | MB_ICONERROR);
        return;
    }

    // 添加标签页到Tab控件
    TCITEM tie = { 0 };
    tie.mask = TCIF_TEXT;
    tie.pszText = const_cast<TCHAR*>(pageName); // Tab控件会复制字符串

    int tabIndex = TabCtrl_GetItemCount(hTabCtrl);
    if (TabCtrl_InsertItem(hTabCtrl, tabIndex, &tie) == -1)
    {
        DWORD err = GetLastError();
        TCHAR msg[256] = { 0 };
        _sntprintf_s(msg, _countof(msg), _TRUNCATE, _T("插入标签页失败! 错误码: %d"), err);
        MessageBox(hWndParent, msg, _T("错误"), MB_OK | MB_ICONERROR);

        DestroyWindow(hTabPage);
        return;
    }

    hTabPages.push_back(hTabPage);
    ShowWindow(hTabPage, SW_HIDE);
}

// 调整Tab控件和子窗口大小的函数
void ResizeTabPages(HWND hTabCtrl, std::vector<HWND>& hTabPages)
{
    // 获取DPI缩放因子
    const UINT dpi = GetDpiForWindow(hTabCtrl);
    const float scale = dpi / 96.0f;

    // 计算正确的内容区域
    RECT rcTab;
    GetClientRect(hTabCtrl, &rcTab);

    // 仅缩放偏移部分（不要缩放整个矩形）
    const int tabHeaderHeight = static_cast<int>(22 * scale);
    rcTab.top += tabHeaderHeight;

    // 设置子窗口位置和大小
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
            MessageBox(hWnd, TEXT("图标加载失败"), TEXT("错误"), MB_ICONERROR);
        }
        else
        {
            SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)ShakaIcon);
            SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)ShakaIcon);
        }

        // 获取资源中预定义的Tab控件
        hTabCtrl = GetDlgItem(hWnd, IDC_TAB1);
        if (!hTabCtrl)
        {
            MessageBox(hWnd, TEXT("Tab控件获取失败"), TEXT("错误"), MB_ICONERROR);
            EndDialog(hWnd, -1);
            return TRUE;
        }

        // 添加标签页
        InstallPage(hWnd, hTabCtrl, hTabPages, IDD_Page_0, TEXT("学习目标"), Tab1DialogProc);
        InstallPage(hWnd, hTabCtrl, hTabPages, IDD_Page_1, TEXT("教材资源"), Tab2DialogProc);
        InstallPage(hWnd, hTabCtrl, hTabPages, IDD_Page_2, TEXT("时间分配"), Tab3DialogProc);
        InstallPage(hWnd, hTabCtrl, hTabPages, IDD_Page_3, TEXT("完成进度"), Tab4DialogProc);
        InstallPage(hWnd, hTabCtrl, hTabPages, IDD_Page_4, TEXT("学习问题"), Tab5DialogProc);

        // 调整Tab控件和子窗口大小
        ResizeTabPages(hTabCtrl, hTabPages);

        // 显示第一个标签页
        if (!hTabPages.empty())
        {
            TabCtrl_SetCurSel(hTabCtrl, 0);
            ShowWindow(hTabPages[0], SW_SHOW);
        }
    }
    return TRUE;

    case WM_SIZE:
    {
        // 当主窗口大小改变时，调整Tab控件大小
        if (hTabCtrl && IsWindow(hTabCtrl))
        {
            // 获取主窗口客户区大小
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            // 调整Tab控件大小以填充整个客户区
            SetWindowPos(hTabCtrl, NULL,
                0, 0,
                rcClient.right, rcClient.bottom,
                SWP_NOZORDER | SWP_NOACTIVATE);

            // 调整所有标签页子窗口的大小
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
        // 可以添加命令处理代码
        return 0;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return TRUE;

    case WM_DESTROY:
    {
        // 销毁所有子窗口
        for (auto& page : hTabPages)
        {
            if (IsWindow(page))
                DestroyWindow(page);
        }
        hTabPages.clear();
        PostQuitMessage(0); // 退出消息循环
    }
    return TRUE;

    default:
        return FALSE;
    }
}
