#include <windows.h>
#include <shellapi.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_APP_ICON 101
#define ID_TRAY_EXIT 2

NOTIFYICONDATA nid = {};
HWND hwnd;
bool isActive;

DWORD executeCommand(const char* command) {
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    
    HANDLE hRead, hWrite;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        return 1;
    }

    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);
    
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;

    PROCESS_INFORMATION pi;
    if (!CreateProcess(NULL, const_cast<char*>(command), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return  1;
    }
    CloseHandle(hWrite);
    CloseHandle(hRead);

    // Wait for the process to finish
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
        exitCode = GetLastError();
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return exitCode;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_TRAYICON) {
        if (lParam == WM_LBUTTONUP) {
            if (isActive){
                const char* cmd = "cmd /c taskkill /IM komorebi.exe /F";
                executeCommand(cmd);
            } else {
                const char* cmd = "cmd /c start \"\" /min komorebi.exe";
                executeCommand(cmd);
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
