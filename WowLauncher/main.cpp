/*
 * WoW Multi-Instance Launcher
 * Компилятор: w64devkit (MinGW-w64, 32-bit)
 * Сборка:
 *   cd WowLauncher
 *   g++ -m32 -O2 -mwindows -o WowLauncher.exe main.cpp -lcomctl32 -lcomdlg32 -lshlwapi
 *
 * Требования: Windows 10, 32-bit компилятор w64devkit
 */

#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlwapi.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shlwapi.lib")

// ─── ID элементов ────────────────────────────────────────────────────────────
#define ID_BTN_BROWSE     101
#define ID_BTN_LAUNCH     102
#define ID_BTN_CLEAR_LOG  103
#define ID_EDIT_PATH      104
#define ID_EDIT_COUNT     105
#define ID_EDIT_LOG       106
#define ID_SPIN_COUNT     107
#define ID_STATIC_PATH    108
#define ID_STATIC_COUNT   109
#define ID_STATIC_STATUS  110
#define ID_STATIC_CONF    111

// ─── Цвета (тёмная тема) ─────────────────────────────────────────────────────
#define CLR_BG          RGB(8,  12,  20)
#define CLR_CARD        RGB(13, 20,  33)
#define CLR_CARD2       RGB(17, 25,  40)
#define CLR_BORDER      RGB(30, 45,  69)
#define CLR_GOLD        RGB(240,180, 41)
#define CLR_BLUE        RGB(74, 158, 255)
#define CLR_GREEN       RGB(34, 197, 94)
#define CLR_RED         RGB(239, 68, 68)
#define CLR_TEXT        RGB(226,232,240)
#define CLR_MUTED       RGB(100,116,139)
#define CLR_INPUT_BG    RGB(8,  12,  20)

// ─── Глобальные переменные ────────────────────────────────────────────────────
HWND g_hWnd        = NULL;
HWND g_hEditPath   = NULL;
HWND g_hEditCount  = NULL;
HWND g_hEditLog    = NULL;
HWND g_hBtnBrowse  = NULL;
HWND g_hBtnLaunch  = NULL;
HWND g_hBtnClear   = NULL;
HWND g_hStatus     = NULL;
HWND g_hConfLabel  = NULL;

HBRUSH g_hBrushBg    = NULL;
HBRUSH g_hBrushCard  = NULL;
HBRUSH g_hBrushInput = NULL;
HFONT  g_hFontTitle  = NULL;
HFONT  g_hFontMain   = NULL;
HFONT  g_hFontMono   = NULL;
HFONT  g_hFontBtn    = NULL;

bool g_isRunning = false;

// ─── Структура учётных данных из log.conf ────────────────────────────────────
struct WowAccount {
    std::wstring login;
    std::wstring password;
    std::wstring realm;
    std::wstring region;
};

// ─── Путь к log.conf (рядом с exe) ───────────────────────────────────────────
std::wstring GetConfPath() {
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    PathRemoveFileSpecW(buf);
    return std::wstring(buf) + L"\\log.conf";
}

// ─── Получить текущее время как строку ───────────────────────────────────────
std::wstring GetTimeStr() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buf[32];
    swprintf(buf, 32, L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    return std::wstring(buf);
}

// ─── Добавить строку в лог ───────────────────────────────────────────────────
void AppendLog(const wchar_t* prefix, const wchar_t* msg) {
    std::wstring line = L"[" + GetTimeStr() + L"] " + prefix + L"  " + msg + L"\r\n";

    int len = GetWindowTextLengthW(g_hEditLog);
    // Ограничение 32000 символов — очищаем старые
    if (len > 28000) {
        SendMessageW(g_hEditLog, EM_SETSEL, 0, len / 2);
        SendMessageW(g_hEditLog, EM_REPLACESEL, FALSE, (LPARAM)L"");
        len = GetWindowTextLengthW(g_hEditLog);
    }
    SendMessageW(g_hEditLog, EM_SETSEL, len, len);
    SendMessageW(g_hEditLog, EM_REPLACESEL, FALSE, (LPARAM)line.c_str());
    SendMessageW(g_hEditLog, EM_SCROLL, SB_BOTTOM, 0);
}

void LogInfo(const wchar_t* msg)    { AppendLog(L"[INF]", msg); }
void LogSuccess(const wchar_t* msg) { AppendLog(L"[OK] ", msg); }
void LogError(const wchar_t* msg)   { AppendLog(L"[ERR]", msg); }
void LogWarn(const wchar_t* msg)    { AppendLog(L"[WRN]", msg); }

// ─── Создать log.conf с шаблоном ─────────────────────────────────────────────
void CreateDefaultConf(const std::wstring& path) {
    std::wofstream f(path);
    if (f.is_open()) {
        f << L"# WoW Launcher — файл учётных данных\n";
        f << L"# Заполните поля ниже и сохраните файл\n\n";
        f << L"login=YOUR_LOGIN\n";
        f << L"password=YOUR_PASSWORD\n";
        f << L"realm=Warmane\n";
        f << L"region=EU\n";
        f.close();
    }
}

// ─── Прочитать log.conf ───────────────────────────────────────────────────────
WowAccount ReadConf(const std::wstring& path) {
    WowAccount acc;
    std::wifstream f(path);
    std::wstring line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == L'#') continue;
        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;
        std::wstring key = line.substr(0, eq);
        std::wstring val = line.substr(eq + 1);
        // Trim
        while (!val.empty() && (val.back() == L'\r' || val.back() == L'\n' || val.back() == L' '))
            val.pop_back();
        if      (key == L"login")    acc.login    = val;
        else if (key == L"password") acc.password = val;
        else if (key == L"realm")    acc.realm    = val;
        else if (key == L"region")   acc.region   = val;
    }
    return acc;
}

// ─── Обновить лейбл статуса log.conf ─────────────────────────────────────────
void UpdateConfStatus() {
    std::wstring path = GetConfPath();
    bool exists = (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES);
    SetWindowTextW(g_hConfLabel, exists
        ? L"log.conf: НАЙДЕН ✓"
        : L"log.conf: НЕ НАЙДЕН (будет создан при запуске)");
}

// ─── Запись в лог-файл на диске ──────────────────────────────────────────────
void WriteToLogFile(const std::wstring& msg) {
    wchar_t exeDir[MAX_PATH];
    GetModuleFileNameW(NULL, exeDir, MAX_PATH);
    PathRemoveFileSpecW(exeDir);
    std::wstring logPath = std::wstring(exeDir) + L"\\launcher.log";

    std::wofstream f(logPath, std::ios::app);
    if (f.is_open()) {
        f << L"[" << GetTimeStr() << L"] " << msg << L"\n";
    }
}

// ─── Запуск нескольких копий exe ─────────────────────────────────────────────
void LaunchInstances(const std::wstring& exePath, int count) {
    // Проверить/создать log.conf
    std::wstring confPath = GetConfPath();
    if (GetFileAttributesW(confPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        LogWarn(L"Файл log.conf не найден. Создаю шаблон...");
        CreateDefaultConf(confPath);
        LogInfo(L"Создан log.conf — заполните login/password и запустите снова.");
        WriteToLogFile(L"Создан шаблон log.conf");
        UpdateConfStatus();
        g_isRunning = false;
        EnableWindow(g_hBtnLaunch, TRUE);
        SetWindowTextW(g_hBtnLaunch, L"ЗАПУСТИТЬ");
        return;
    }

    WowAccount acc = ReadConf(confPath);

    if (acc.login.empty() || acc.login == L"YOUR_LOGIN") {
        LogError(L"Заполните поля login и password в файле log.conf!");
        WriteToLogFile(L"Ошибка: login не заполнен в log.conf");
        g_isRunning = false;
        EnableWindow(g_hBtnLaunch, TRUE);
        SetWindowTextW(g_hBtnLaunch, L"ЗАПУСТИТЬ");
        return;
    }

    LogInfo((L"Учётные данные загружены. Пользователь: " + acc.login).c_str());
    LogInfo((L"Realm: " + acc.realm + L" | Region: " + acc.region).c_str());

    int successCount = 0;
    int failCount    = 0;

    for (int i = 1; i <= count; i++) {
        // Формируем командную строку с аргументами автологина
        std::wstring cmdLine = L"\"" + exePath + L"\""
            + L" -login " + acc.login
            + L" -password " + acc.password;

        STARTUPINFOW si = {};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {};

        BOOL ok = CreateProcessW(
            exePath.c_str(),
            &cmdLine[0],
            NULL, NULL,
            FALSE,
            0,
            NULL, NULL,
            &si, &pi
        );

        if (ok) {
            wchar_t buf[256];
            swprintf(buf, 256,
                L"Экземпляр #%d запущен (PID: %lu) | %s",
                i, pi.dwProcessId, exePath.c_str());
            LogSuccess(buf);
            WriteToLogFile(std::wstring(buf));

            // Автологин через посылку символов (WoW использует аргументы командной строки,
            // для более старых версий можно добавить SendInput после задержки)
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            successCount++;
            Sleep(500); // пауза между запусками
        } else {
            wchar_t buf[256];
            swprintf(buf, 256,
                L"Ошибка запуска экземпляра #%d (код: %lu)", i, GetLastError());
            LogError(buf);
            WriteToLogFile(std::wstring(buf));
            failCount++;
        }
    }

    wchar_t summary[256];
    swprintf(summary, 256,
        L"Завершено: %d/%d успешно, %d ошибок.", successCount, count, failCount);
    LogInfo(summary);
    WriteToLogFile(std::wstring(summary));

    g_isRunning = false;
    EnableWindow(g_hBtnLaunch, TRUE);
    SetWindowTextW(g_hBtnLaunch, L"ЗАПУСТИТЬ");
}

// ─── Поток для запуска (чтобы не блокировать UI) ─────────────────────────────
struct LaunchParams {
    std::wstring path;
    int count;
};

DWORD WINAPI LaunchThread(LPVOID param) {
    LaunchParams* p = (LaunchParams*)param;
    LaunchInstances(p->path, p->count);
    delete p;
    return 0;
}

// ─── Диалог выбора файла ─────────────────────────────────────────────────────
void BrowseForExe() {
    wchar_t path[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = g_hWnd;
    ofn.lpstrFilter  = L"Исполняемые файлы (*.exe)\0*.exe\0Все файлы (*.*)\0*.*\0";
    ofn.lpstrFile    = path;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrTitle   = L"Выберите exe-файл";
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        SetWindowTextW(g_hEditPath, path);
        std::wstring msg = L"Выбран файл: ";
        msg += path;
        LogInfo(msg.c_str());
    }
}

// ─── Нарисовать рамку-карточку ───────────────────────────────────────────────
void DrawCard(HDC hdc, RECT r, COLORREF borderColor) {
    HBRUSH br = CreateSolidBrush(CLR_CARD);
    FillRect(hdc, &r, br);
    DeleteObject(br);

    HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, r.left, r.top, r.right, r.bottom, 12, 12);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBr);
    DeleteObject(pen);
}

// ─── Обработчик сообщений окна ───────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
        // Шрифты
        g_hFontTitle = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontMain = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontMono = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
        g_hFontBtn = CreateFontW(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        // Кисти
        g_hBrushBg    = CreateSolidBrush(CLR_BG);
        g_hBrushCard  = CreateSolidBrush(CLR_CARD);
        g_hBrushInput = CreateSolidBrush(CLR_INPUT_BG);

        // ── Заголовок ──
        HWND hTitle = CreateWindowW(L"STATIC", L"WoW Multi-Instance Launcher",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 14, 420, 28, hWnd, (HMENU)200, NULL, NULL);
        SendMessageW(hTitle, WM_SETFONT, (WPARAM)g_hFontTitle, TRUE);

        HWND hSub = CreateWindowW(L"STATIC", L"v1.0  |  w64devkit  |  Win32",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 44, 300, 18, hWnd, (HMENU)201, NULL, NULL);
        SendMessageW(hSub, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

        // ── Блок "Путь к EXE" ──
        HWND hLblPath = CreateWindowW(L"STATIC", L"ПУТЬ К EXE-ФАЙЛУ",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 80, 200, 18, hWnd, (HMENU)202, NULL, NULL);
        SendMessageW(hLblPath, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

        g_hEditPath = CreateWindowExW(WS_EX_CLIENTEDGE,
            L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            20, 102, 490, 30, hWnd, (HMENU)ID_EDIT_PATH, NULL, NULL);
        SendMessageW(g_hEditPath, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

        g_hBtnBrowse = CreateWindowW(L"BUTTON", L"Обзор...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            520, 102, 100, 30, hWnd, (HMENU)ID_BTN_BROWSE, NULL, NULL);
        SendMessageW(g_hBtnBrowse, WM_SETFONT, (WPARAM)g_hFontBtn, TRUE);

        // ── Блок "Количество" ──
        HWND hLblCnt = CreateWindowW(L"STATIC", L"КОЛИЧЕСТВО КОПИЙ (1–50)",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 150, 260, 18, hWnd, (HMENU)203, NULL, NULL);
        SendMessageW(hLblCnt, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

        g_hEditCount = CreateWindowExW(WS_EX_CLIENTEDGE,
            L"EDIT", L"1",
            WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_CENTER,
            20, 172, 80, 30, hWnd, (HMENU)ID_EDIT_COUNT, NULL, NULL);
        SendMessageW(g_hEditCount, WM_SETFONT, (WPARAM)g_hFontTitle, TRUE);

        // Спиннер
        HWND hSpin = CreateWindowExW(0, UPDOWN_CLASSW, NULL,
            WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_ARROWKEYS,
            0, 0, 0, 0, hWnd, (HMENU)ID_SPIN_COUNT, NULL, NULL);
        SendMessageW(hSpin, UDM_SETBUDDY, (WPARAM)g_hEditCount, 0);
        SendMessageW(hSpin, UDM_SETRANGE, 0, MAKELONG(50, 1));
        SendMessageW(hSpin, UDM_SETPOS, 0, 1);

        // ── Кнопка ЗАПУСТИТЬ ──
        g_hBtnLaunch = CreateWindowW(L"BUTTON", L"ЗАПУСТИТЬ",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            120, 172, 500, 30, hWnd, (HMENU)ID_BTN_LAUNCH, NULL, NULL);
        SendMessageW(g_hBtnLaunch, WM_SETFONT, (WPARAM)g_hFontBtn, TRUE);

        // ── Статус log.conf ──
        g_hConfLabel = CreateWindowW(L"STATIC", L"log.conf: проверка...",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 215, 580, 18, hWnd, (HMENU)ID_STATIC_CONF, NULL, NULL);
        SendMessageW(g_hConfLabel, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

        // ── Лог ──
        HWND hLblLog = CreateWindowW(L"STATIC", L"ЖУРНАЛ СОБЫТИЙ",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 244, 300, 18, hWnd, (HMENU)204, NULL, NULL);
        SendMessageW(hLblLog, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

        g_hBtnClear = CreateWindowW(L"BUTTON", L"Очистить",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            545, 240, 75, 22, hWnd, (HMENU)ID_BTN_CLEAR_LOG, NULL, NULL);
        SendMessageW(g_hBtnClear, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

        g_hEditLog = CreateWindowExW(WS_EX_CLIENTEDGE,
            L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            20, 268, 600, 240, hWnd, (HMENU)ID_EDIT_LOG, NULL, NULL);
        SendMessageW(g_hEditLog, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

        // ── Строка статуса ──
        g_hStatus = CreateWindowW(L"STATIC", L"Готов к работе",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 520, 600, 18, hWnd, (HMENU)ID_STATIC_STATUS, NULL, NULL);
        SendMessageW(g_hStatus, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

        // Инициализация
        UpdateConfStatus();
        LogInfo(L"WoW Launcher запущен. Выберите exe-файл и нажмите «ЗАПУСТИТЬ».");

        // Проверить наличие log.conf сразу
        std::wstring cp = GetConfPath();
        if (GetFileAttributesW(cp.c_str()) == INVALID_FILE_ATTRIBUTES) {
            LogWarn(L"Файл log.conf не найден. Будет создан автоматически при первом запуске.");
        } else {
            LogInfo((L"Найден log.conf: " + cp).c_str());
            WowAccount acc = ReadConf(cp);
            if (!acc.login.empty() && acc.login != L"YOUR_LOGIN")
                LogInfo((L"Загружен пользователь: " + acc.login).c_str());
        }
        break;
    }

    case WM_COMMAND: {
        WORD id = LOWORD(wParam);

        if (id == ID_BTN_BROWSE) {
            BrowseForExe();
        }
        else if (id == ID_BTN_CLEAR_LOG) {
            SetWindowTextW(g_hEditLog, L"");
            LogInfo(L"Лог очищен.");
        }
        else if (id == ID_BTN_LAUNCH && !g_isRunning) {
            // Получить путь
            wchar_t pathBuf[MAX_PATH] = {};
            GetWindowTextW(g_hEditPath, pathBuf, MAX_PATH);
            if (wcslen(pathBuf) == 0) {
                LogError(L"Путь к exe-файлу не указан!");
                MessageBoxW(hWnd,
                    L"Укажите путь к exe-файлу через кнопку «Обзор».",
                    L"Ошибка", MB_ICONWARNING | MB_OK);
                break;
            }
            if (GetFileAttributesW(pathBuf) == INVALID_FILE_ATTRIBUTES) {
                LogError(L"Файл не найден по указанному пути!");
                break;
            }

            // Получить количество
            wchar_t cntBuf[8] = {};
            GetWindowTextW(g_hEditCount, cntBuf, 8);
            int cnt = _wtoi(cntBuf);
            if (cnt < 1 || cnt > 50) {
                LogError(L"Количество копий должно быть от 1 до 50.");
                break;
            }

            g_isRunning = true;
            EnableWindow(g_hBtnLaunch, FALSE);
            SetWindowTextW(g_hBtnLaunch, L"Запуск...");
            SetWindowTextW(g_hStatus, L"Запуск экземпляров...");

            wchar_t info[256];
            swprintf(info, 256, L"Запуск %d копий: %s", cnt, pathBuf);
            LogInfo(info);

            // Запуск в отдельном потоке
            LaunchParams* p = new LaunchParams();
            p->path  = pathBuf;
            p->count = cnt;
            CloseHandle(CreateThread(NULL, 0, LaunchThread, p, 0, NULL));
        }
        break;
    }

    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, CLR_INPUT_BG);
        SetTextColor(hdc, CLR_TEXT);
        return (LRESULT)g_hBrushInput;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc  = (HDC)wParam;
        HWND hw  = (HWND)lParam;
        SetBkColor(hdc, CLR_BG);

        if (hw == g_hConfLabel) {
            wchar_t buf[64];
            GetWindowTextW(hw, buf, 64);
            if (wcsstr(buf, L"НАЙДЕН"))
                SetTextColor(hdc, CLR_GREEN);
            else
                SetTextColor(hdc, CLR_RED);
        } else if (hw == g_hStatus) {
            SetTextColor(hdc, CLR_BLUE);
        } else {
            SetTextColor(hdc, CLR_MUTED);
        }
        return (LRESULT)g_hBrushBg;
    }

    case WM_CTLCOLORBTN: {
        // Цвет фона для кнопок через owner-draw — здесь просто вернём стандарт
        return (LRESULT)g_hBrushBg;
    }

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT r;
        GetClientRect(hWnd, &r);
        FillRect(hdc, &r, g_hBrushBg);

        // Разделитель под заголовком
        HPEN pen = CreatePen(PS_SOLID, 1, CLR_BORDER);
        HPEN old = (HPEN)SelectObject(hdc, pen);
        MoveToEx(hdc, 0, 68, NULL);
        LineTo(hdc, r.right, 68);
        SelectObject(hdc, old);
        DeleteObject(pen);

        return 1;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Золотая полоска слева у "ПУТЬ К EXE"
        HPEN penGold = CreatePen(PS_SOLID, 3, CLR_GOLD);
        HPEN old = (HPEN)SelectObject(hdc, penGold);
        MoveToEx(hdc, 12, 80, NULL);
        LineTo(hdc, 12, 98);
        SelectObject(hdc, old);
        DeleteObject(penGold);

        // Синяя полоска у "КОЛИЧЕСТВО"
        HPEN penBlue = CreatePen(PS_SOLID, 3, CLR_BLUE);
        old = (HPEN)SelectObject(hdc, penBlue);
        MoveToEx(hdc, 12, 150, NULL);
        LineTo(hdc, 12, 168);
        SelectObject(hdc, old);
        DeleteObject(penBlue);

        // Зелёная полоска у "ЖУРНАЛ"
        HPEN penGreen = CreatePen(PS_SOLID, 3, CLR_GREEN);
        old = (HPEN)SelectObject(hdc, penGreen);
        MoveToEx(hdc, 12, 244, NULL);
        LineTo(hdc, 12, 262);
        SelectObject(hdc, old);
        DeleteObject(penGreen);

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        DeleteObject(g_hFontTitle);
        DeleteObject(g_hFontMain);
        DeleteObject(g_hFontMono);
        DeleteObject(g_hFontBtn);
        DeleteObject(g_hBrushBg);
        DeleteObject(g_hBrushCard);
        DeleteObject(g_hBrushInput);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ─── WinMain ─────────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // Инициализация Common Controls (для спиннера)
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_UPDOWN_CLASS | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    const wchar_t CLASS_NAME[] = L"WowLauncherWnd";

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon         = LoadIconW(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(CLR_BG);

    RegisterClassExW(&wc);

    g_hWnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        CLASS_NAME,
        L"WoW Multi-Instance Launcher",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        660, 580,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
