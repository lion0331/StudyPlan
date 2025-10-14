#include "Tab2Dialog.h"


INT_PTR CALLBACK Tab2DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hEdit;
    static TCHAR szDefaultPath[MAX_PATH] = { 0 };
    switch (message)
    {
    case WM_INITDIALOG:
    {
        TCHAR szModulePath[MAX_PATH] = { 0 };

        if (!GetModuleFileName(NULL, szModulePath, MAX_PATH))
        {
            MessageBox(hWnd, L"无法获取程序路径", L"错误", MB_OK | MB_ICONERROR);
            break;
        }

        PathRemoveFileSpec(szModulePath);
        PathCombine(szDefaultPath, szModulePath, _T("TeachingMaterial.txt"));
        // 加载文件（关键：只在初始化时加载一次）
        LoadFile(hWnd, IDC_TEXT_EDIT1, szDefaultPath);
        return TRUE;
    }

    case WM_SIZE:
    {
        // 如果标签页内部有控件需要调整大小，可以在这里处理
        // 例如调整编辑框的大小
        hEdit = GetDlgItem(hWnd, IDC_TEXT_EDIT1);
        if (hEdit && IsWindow(hEdit))
        {
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            // 调整编辑框大小，留出一些边距
            SetWindowPos(hEdit, NULL,
                10, 10,
                rcClient.right - 20, rcClient.bottom - 50, // 底部留出空间给按钮
                SWP_NOZORDER | SWP_NOACTIVATE);

            // 调整按钮位置
            HWND hButton = GetDlgItem(hWnd, IDC_SAVE_BUTTON1);
            if (hButton && IsWindow(hButton))
            {
                SetWindowPos(hButton, NULL,
                    rcClient.right - 100, rcClient.bottom - 35,
                    60, 25,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
    }
    return TRUE;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (wmId == IDC_SAVE_BUTTON1)
        {
            // 获取文件加载状态
            BOOL bFileLoaded = (BOOL)(INT_PTR)GetProp(hWnd, PROP_FILE_LOADED);
            hEdit = GetDlgItem(hWnd, IDC_TEXT_EDIT1);
            wchar_t currentText[1024] = { 0 };
            GetWindowText(hEdit, currentText, _countof(currentText));

            // 关键修复：如果是默认提示文本，直接提示输入内容，不弹出保存对话框
            if (IsDefaultPromptText(currentText))
            {
                MessageBox(hWnd, L"请先输入内容再保存", L"提示", MB_OK | MB_ICONINFORMATION);
                break;
            }

            // 正常保存逻辑
            if (bFileLoaded)
            {
                if (SaveFile(hWnd, IDC_TEXT_EDIT1, szDefaultPath))
                {
                    MessageBox(hWnd, L"文本已保存到 TeachingMaterial.txt", L"保存成功", MB_OK | MB_ICONINFORMATION);
                }
                else
                {
                    MessageBox(hWnd, L"保存失败", L"错误", MB_OK | MB_ICONERROR);
                }
            }
            else
            {
                OPENFILENAME ofn = { 0 };
                wchar_t szFileName[MAX_PATH] = { 0 };
                wcscpy_s(szFileName, MAX_PATH, szDefaultPath);

                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hWnd;
                ofn.lpstrFile = szFileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFilter = L"文本文件 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrDefExt = L"txt";
                ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

                if (GetSaveFileName(&ofn))
                {
                    if (SaveFile(hWnd, IDC_TEXT_EDIT1, ofn.lpstrFile))
                    {
                        SetProp(hWnd, PROP_FILE_LOADED, (HANDLE)(INT_PTR)TRUE);
                        MessageBox(hWnd, L"文本保存成功", L"成功", MB_OK | MB_ICONINFORMATION);
                    }
                    else
                    {
                        MessageBox(hWnd, L"无法保存文件", L"错误", MB_OK | MB_ICONERROR);
                    }
                }
            }
        }
        break;
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
    {
        RemoveProp(hWnd, PROP_FILE_LOADED);
        RemoveProp(hWnd, PROP_EDIT_FONT);
        break;
    }

    default:
        return FALSE;
    }
    return TRUE;
}