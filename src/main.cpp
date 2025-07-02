#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_APP_ICON 101
#define ID_TRAY_EXIT 2

NOTIFYICONDATA nid = {};
HWND hwnd;
bool isActive;

bool terminateProcessByName(const wchar_t* exeName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, exeName) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                    CloseHandle(hSnapshot);
                    return true;
                }
            }
        } while (Process32NextW(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return false;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_TRAYICON) {
        if (lParam == WM_LBUTTONUP) {
            if (isActive){
                terminateProcessByName(L"komorebi.exe");
            } else {
                STARTUPINFO si = { sizeof(si) };
                si.dwFlags = STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_HIDE;

                PROCESS_INFORMATION pi;
                CreateProcess(
                    NULL,
                    (LPSTR)"komorebi.exe",
                    NULL,
                    NULL,
                    FALSE,
                    CREATE_NO_WINDOW,
                    NULL,
                    NULL,
                    &si,
                    &pi
                );
            }
            isActive = !isActive;
            lstrcpy(nid.szTip, isActive ? TEXT("Komorebi: Running") : TEXT("Komorebi: Idle"));
            Shell_NotifyIcon(NIM_MODIFY, &nid);
        } else if (lParam == WM_RBUTTONUP) {
            // Right click: show menu
            HMENU menu = CreatePopupMenu();
            AppendMenu(menu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));

            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd); // Required to make menu work properly
            TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
            DestroyMenu(menu);
        }
    } else if (message == WM_COMMAND) {
        if (LOWORD(wParam) == ID_TRAY_EXIT) {
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
        }
    } else if (message == WM_DESTROY) {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // Register window class
    isActive = false;
    const char CLASS_NAME[] = "TrayAppWindow";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ID_TRAY_APP_ICON));
    RegisterClass(&wc);

    // Create invisible window
    hwnd = CreateWindowEx(0, CLASS_NAME, "HiddenWindow", 0,
                          0, 0, 0, 0, NULL, NULL, hInstance, NULL);

    // Load tray icon
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_APP_ICON;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ID_TRAY_APP_ICON));
    lstrcpy(nid.szTip, TEXT("Komorebi: Idle"));

    Shell_NotifyIcon(NIM_ADD, &nid);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
