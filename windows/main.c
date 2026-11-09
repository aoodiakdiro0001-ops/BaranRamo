/*
 * BaranRamo — Windows launcher: extracts the embedded single-file game to
 * %LOCALAPPDATA%\BaranRamo\BaranRamo.html and opens it in the default browser.
 * Fully offline. Game saves (localStorage) persist across runs.
 * (c) 2026 Baran Avisat - All Rights Reserved.
 */
#include <windows.h>
#include <shlobj.h>
#include <strsafe.h>
#include "game_html.h"

static int embed_game_to(const wchar_t *path) {
    HANDLE h = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    DWORD written = 0;
    BOOL ok = WriteFile(h, game_html, game_html_len, &written, NULL);
    CloseHandle(h);
    return ok && written == game_html_len;
}

static unsigned long existing_size(const wchar_t *path) {
    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    LARGE_INTEGER sz;
    sz.QuadPart = 0;
    GetFileSizeEx(h, &sz);
    CloseHandle(h);
    return (unsigned long)sz.QuadPart;
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, PWSTR cmdLine, int showCmd) {
    (void)hInst; (void)hPrev; (void)cmdLine; (void)showCmd;

    wchar_t dir[MAX_PATH];
    wchar_t file[MAX_PATH];

    if (FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, dir))) {
        if (GetTempPathW(MAX_PATH, dir) == 0) {
            MessageBoxW(NULL, L"BaranRamo", L"BaranRamo", MB_ICONERROR);
            return 1;
        }
    }
    StringCchCatW(dir, MAX_PATH, L"\\BaranRamo");
    CreateDirectoryW(dir, NULL);
    StringCchCopyW(file, MAX_PATH, dir);
    StringCchCatW(file, MAX_PATH, L"\\BaranRamo.html");

    /* Extract (or refresh when the embedded build changed). */
    if (existing_size(file) != game_html_len) {
        if (!embed_game_to(file)) {
            MessageBoxW(NULL,
                L"Could not prepare the game file.\nPlease check disk permissions.",
                L"BaranRamo", MB_ICONERROR | MB_OK);
            return 1;
        }
    }

    SHELLEXECUTEINFOW sei;
    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC;
    sei.lpVerb = L"open";
    sei.lpFile = file;
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&sei)) {
        /* Fallback: explicit open via ShellExecute. */
        if ((INT_PTR)ShellExecuteW(NULL, L"open", file, NULL, NULL, SW_SHOWNORMAL) <= 32) {
            MessageBoxW(NULL,
                L"Could not open the game. Open BaranRamo.html with any browser.",
                L"BaranRamo", MB_ICONERROR | MB_OK);
            return 1;
        }
    }
    return 0;
}
