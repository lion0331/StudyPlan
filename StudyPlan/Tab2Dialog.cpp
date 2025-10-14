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
            MessageBox(hWnd, L"�޷���ȡ����·��", L"����", MB_OK | MB_ICONERROR);
            break;
        }

        PathRemoveFileSpec(szModulePath);
        PathCombine(szDefaultPath, szModulePath, _T("TeachingMaterial.txt"));
        // �����ļ����ؼ���ֻ�ڳ�ʼ��ʱ����һ�Σ�
        LoadFile(hWnd, IDC_TEXT_EDIT1, szDefaultPath);
        return TRUE;
    }

    case WM_SIZE:
    {
        // �����ǩҳ�ڲ��пؼ���Ҫ������С�����������ﴦ��
        // ��������༭��Ĵ�С
        hEdit = GetDlgItem(hWnd, IDC_TEXT_EDIT1);
        if (hEdit && IsWindow(hEdit))
        {
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            // �����༭���С������һЩ�߾�
            SetWindowPos(hEdit, NULL,
                10, 10,
                rcClient.right - 20, rcClient.bottom - 50, // �ײ������ռ����ť
                SWP_NOZORDER | SWP_NOACTIVATE);

            // ������ťλ��
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
            // ��ȡ�ļ�����״̬
            BOOL bFileLoaded = (BOOL)(INT_PTR)GetProp(hWnd, PROP_FILE_LOADED);
            hEdit = GetDlgItem(hWnd, IDC_TEXT_EDIT1);
            wchar_t currentText[1024] = { 0 };
            GetWindowText(hEdit, currentText, _countof(currentText));

            // �ؼ��޸��������Ĭ����ʾ�ı���ֱ����ʾ�������ݣ�����������Ի���
            if (IsDefaultPromptText(currentText))
            {
                MessageBox(hWnd, L"�������������ٱ���", L"��ʾ", MB_OK | MB_ICONINFORMATION);
                break;
            }

            // ���������߼�
            if (bFileLoaded)
            {
                if (SaveFile(hWnd, IDC_TEXT_EDIT1, szDefaultPath))
                {
                    MessageBox(hWnd, L"�ı��ѱ��浽 TeachingMaterial.txt", L"����ɹ�", MB_OK | MB_ICONINFORMATION);
                }
                else
                {
                    MessageBox(hWnd, L"����ʧ��", L"����", MB_OK | MB_ICONERROR);
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
                ofn.lpstrFilter = L"�ı��ļ� (*.txt)\0*.txt\0�����ļ� (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrDefExt = L"txt";
                ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

                if (GetSaveFileName(&ofn))
                {
                    if (SaveFile(hWnd, IDC_TEXT_EDIT1, ofn.lpstrFile))
                    {
                        SetProp(hWnd, PROP_FILE_LOADED, (HANDLE)(INT_PTR)TRUE);
                        MessageBox(hWnd, L"�ı�����ɹ�", L"�ɹ�", MB_OK | MB_ICONINFORMATION);
                    }
                    else
                    {
                        MessageBox(hWnd, L"�޷������ļ�", L"����", MB_OK | MB_ICONERROR);
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