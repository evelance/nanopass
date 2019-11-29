#define _UNICODE
#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <fstream>
#include "../resource/resource.h"
#include "../tabtree/tabtreeparser.hpp"
#include "../tabtree/tabtree.hpp"
#include "../config.hpp"
#include "../gui.hpp"
#include "../db.hpp"
#include "utfconvert.hpp"
#include "message.hpp"

static LPWSTR class_name = L"editor";
static HWND dialog = NULL;
static HFONT font_text = NULL;
static HBRUSH grey_brush = NULL;
static HWND inp_filter = NULL;
static HWND inp_editor = NULL;
static HMENU g_menu = NULL;
static HMENU g_menu_file = NULL;
static HMENU g_menu_help = NULL;

static db* database = NULL;
static tabtree  tabroot;
static tabtree* tabnode = NULL;
static bool data_changed = false;
static unsigned insert_tabs = 0;
static bool inactivity_close = false;

static WNDPROC inp_filter_proc_old;
static WNDPROC inp_editor_proc_old;

#define ON(cond) case cond: {
#define OFF } break;

// Get data from editor and put it into the currently selected tabnode
static void update_tabtree()
{
    std::string editor = GetWindowTextWString(inp_editor);
    if (tabnode->has_val) {
        tabnode->set_val(editor);
    } else {
        tabnode->set_content(editor);
    }
}

// Visually show in the title bar that there is unsaved data. Also do not close the window!
static void need_save()
{
    if (! data_changed) {
        SetWindowTextWString(dialog, std::string("* ") + program_title_c);
        data_changed = true;
    }
}

// Get all data from root node and save to database
static void save()
{
    if (! data_changed)
        return;
    // Make backup of old file
    std::ifstream ifs(db_filename, std::ios::binary);
    if (ifs.good()) { // File exists
        char bakname[1024];
        snprintf(bakname, sizeof bakname, "%s.%i.bak", db_filename, (int)time(NULL));
        std::ofstream bak(bakname, std::ios::binary);
        bak << ifs.rdbuf();
    }
    // Get current selection
    bool editor_has_focus = (GetFocus() == inp_editor);
    WORD start, end;
    SendMessage(dialog, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    // Fetch current data, encrypt it and write it to the new file
    update_tabtree();
    std::string content = tabroot.get_content();
    database->set_plaintext(content.c_str(), content.length());
    if (database->write(db_filename) != db::OK) {
        MessageBeep(MB_ICONEXCLAMATION);
        show_modal_message(dialog, msg_savefailed_title, database->get_error());
    } else {
        MessageBeep(MB_ICONASTERISK);
        show_modal_message(dialog, msg_saved_title, msg_saved_text);
    }
    // Restore selection and 
    if (editor_has_focus) {
        SetFocus(inp_editor);
        SendMessage(dialog, EM_SETSEL, (WPARAM)start, (LPARAM)end);
    }
    data_changed = false;
    SetWindowTextWString(dialog, program_title_c);
    SetTimer(dialog, ID_TIMEOUT_EDITOR, inactivity_timeout_ms, (TIMERPROC)NULL);
}

// Callback function for filter input element
static LRESULT CALLBACK inp_filter_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        ON(WM_GETDLGCODE) return DLGC_WANTALLKEYS; OFF // Process TAB and ENTER keys
        ON(WM_KEYDOWN)
            switch (wParam) {
                ON(VK_TAB)
                    // Lost focus because of tab
                    SetFocus(hwnd);
                    // Autocomplete filter with suggestion if there is any
                    std::string sugg = tabroot.search_suggest(GetWindowTextWString(hwnd));
                    if (! sugg.empty()) {
                        SetWindowTextWString(inp_filter, sugg);
                        // Put cursor to the end
                        SendMessage(hwnd, EM_SETSEL, (WPARAM) 0, (LPARAM)-1);
                        SendMessage(hwnd, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
                    }
                OFF
                ON(VK_RETURN)
                    // Select editor and copy editor content to clipboard
                    if (OpenClipboard(hwnd)) {
                        EmptyClipboard();
                        std::wstring editor16 = GetWindowTextWString16(inp_editor);
                        size_t len = (editor16.length() + 1) * sizeof(wchar_t); // c_str guarantees zero-terminated string
                        HGLOBAL data = GlobalAlloc(GMEM_MOVEABLE, len);
                        memcpy(GlobalLock(data), editor16.c_str(), len);
                        GlobalUnlock(data);
                        SetClipboardData(CF_UNICODETEXT, data);
                        CloseClipboard();
                        SendMessage(inp_editor, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
                        MessageBeep(MB_ICONASTERISK);
                    }
                    return 0;
                OFF
            }
        OFF
        ON(WM_CHAR)
            switch (wParam) {
                ON('\t') return 0; OFF // Prevent sound on tab
                ON(0x0D) return 0; OFF // Prevent sound on enter
                ON(0x7F) // DEL (control + backspace): Delete last word
                    std::wstring s = GetWindowTextWString16(hwnd);
                    size_t len = s.length();
                    if (len > 0) {
                        // Delete starting from cursor
                        WORD start, end;
                        SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
                        if (start > 0 && start <= len) {
                            len = start;
                        }
                        // Remove whitespace from the right side
                        while (len > 0 && s[len - 1] == L' ') --len;
                        // Remove non-whitespace characters from the right
                        while (len > 0 && s[len - 1] != L' ') --len;
                        s.resize(len);
                        SetWindowTextWString16(hwnd, s);
                        // Put cursor to the end
                        SendMessage(hwnd, EM_SETSEL, (WPARAM) 0, (LPARAM)-1);
                        SendMessage(hwnd, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
                    }
                    // Prevent insertion of DEL "character"
                    return 0;
                OFF
            }
        OFF
    }
    return CallWindowProc(inp_filter_proc_old, hwnd, msg, wParam, lParam);
}

// Callback function for editor multiline text field
static LRESULT CALLBACK inp_editor_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        ON(WM_GETDLGCODE) return DLGC_WANTALLKEYS; OFF // Process TAB and ENTER keys
        ON(WM_KEYDOWN)
            switch (wParam) {
                ON('A')
                    if (GetKeyState(VK_CONTROL) < 0) { // CTRL+A
                        SendMessage(hwnd, EM_SETSEL, (WPARAM)0, (LPARAM)-1); // Select all
                        return 0;
                    }
                OFF
                ON(VK_RETURN)
                    if (tabnode->has_val) {
                        // No enter allowed in value field
                        MessageBeep(MB_ICONEXCLAMATION);
                        show_modal_message(dialog, msg_noenter_title, msg_noenter_text);
                        SetFocus(inp_editor);
                        return 0;
                    }
                OFF
            }
        OFF
        ON(WM_CHAR)
            switch (wParam) {
                ON(0x01) return 0; OFF // ASCI SOH: Generated on CTRL+A. Prevents beeping
                ON(0x0D)
                    if (tabnode->has_val) {
                        return 0; // Prevent insertion of enter
                    } else {
                        // Auto-indent in multiline field (this message is too early for insertion, only count tabs needed)
                        WORD start, end;
                        SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
                        std::wstring s = GetWindowTextWString16(hwnd);
                        // Count tabs at the start of the current line
                        insert_tabs = 0;
                        if (start > 0) { // Are there characters left of the cursor?
                            size_t i = start; // Start points after the last index, i.e. for "|AB" it is 0 and for "AB|" 2
                            while (i > 0 && s[i - 1] != L'\n') --i;
                            while (i < start && s[i] == L'\t') { ++i; ++insert_tabs; }
                        }
                    }
                OFF
                ON(0x7F) // DEL (control + backspace): Delete last word
                    std::wstring s = GetWindowTextWString16(hwnd);
                    size_t len = s.length();
                    if (len > 0) {
                        // Delete starting from cursor
                        WORD start, end;
                        SendMessageW(hwnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
                        size_t i = (start >= 0 && start < len) ? start : len;
                        size_t oldi = (end > i) ? end : i;
                        // Remove whitespace from the right side
                        while (i > 0 && (s[i - 1] == L' ' || s[i - 1] == L'\t') && s[i - 1] != L'\r' && s[i - 1] != L'\n') --i;
                        // Remove non-whitespace characters from the right
                        while (i > 0 && s[i - 1] != L' ' && s[i - 1] != L'\t' && s[i - 1] != L'\r' && s[i - 1] != L'\n') --i;
                        // Remove newline if nothing has been removed
                        if (i >= 2 && (oldi - i) == 0 && s[i - 2] == L'\r' && s[i - 1] == L'\n') i -= 2;
                        SendMessageW(hwnd, EM_SETSEL, (WPARAM)i, (LPARAM)oldi);
                        SendMessageW(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)NULL);
                        if (oldi - i > 0) {
                            need_save();
                        }
                    }
                    // Prevent insertion of DEL "character"
                    return 0;
                OFF
            }
        OFF
    }
    return CallWindowProc(inp_editor_proc_old, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    #define MARGIN      (5)
    #define EDITHEIGHT (23)
    switch (wm) {
        ON(WM_CREATE)
            // Setup font and brush
            grey_brush = CreateSolidBrush(RGB(240, 240, 240));
            font_text = CreateFontW(15, 8, 0, 0, FW_NORMAL, false, false, false,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, L"Monospace");
            // Create window menu
            g_menu = CreateMenu();
            g_menu_file = CreatePopupMenu();
            AppendMenuW(g_menu_file, MF_STRING, ID_SAVE, editor_menu_save);
            AppendMenuW(g_menu_file, MF_STRING, ID_EXIT, editor_menu_exit);
            AppendMenuW(g_menu, MF_STRING | MF_POPUP, (INT_PTR)g_menu_file, editor_menu_file);
            // AppendMenuW(g_menu, MF_STRING, ID_SETTINGS, editor_menu_settings);
            g_menu_help = CreatePopupMenu();
            AppendMenuW(g_menu_help, MF_STRING, ID_INFO, editor_menu_info);
            AppendMenuW(g_menu_help, MF_STRING, ID_WEBSITE, editor_menu_web);
            AppendMenuW(g_menu, MF_STRING | MF_POPUP, (INT_PTR)g_menu_help, editor_menu_help);
            SetMenu(hwnd, g_menu);
            // Create input and buttons
            inp_filter = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD|WS_VISIBLE,
                0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);
            inp_editor = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL,
                WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_NOHIDESEL,
                0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);
            // Misc settings
            SendMessage(inp_filter, EM_SETCUEBANNER, (WPARAM)true, (LPARAM)editor_filter);
            SendMessage(inp_filter, WM_SETFONT, (WPARAM)font_text, (LPARAM)true);
            SendMessage(inp_editor, WM_SETFONT, (WPARAM)font_text, (LPARAM)true);
            inp_filter_proc_old = (WNDPROC)SetWindowLongPtr(inp_filter, GWLP_WNDPROC, (LONG_PTR)inp_filter_proc);
            inp_editor_proc_old = (WNDPROC)SetWindowLongPtr(inp_editor, GWLP_WNDPROC, (LONG_PTR)inp_editor_proc);
            // Start timer to close the window on inactivity
            SetTimer(hwnd, ID_TIMEOUT_EDITOR, inactivity_timeout_ms, (TIMERPROC)NULL);
            // Load text into editor
            SetWindowTextWString(inp_editor, tabroot.get_content());
            SetFocus(inp_filter);
        OFF
        ON(WM_COMMAND)
            switch(LOWORD(wParam)) {
                ON(ID_SAVE)
                    if (data_changed) {
                        save();
                    }
                    return TRUE;
                OFF
                ON(ID_INFO)
                    show_modal_message(dialog, msg_help_title, msg_help_text);
                OFF
                ON(ID_WEBSITE)
                    ShellExecuteW(NULL, L"open", program_home_url, NULL, NULL, SW_SHOWNORMAL);
                OFF
                ON(ID_FOCUS_SWITCH)
                    if (GetFocus() == inp_editor) {
                        SetFocus(inp_filter);
                    } else {
                        SetFocus(inp_editor);
                    }
                OFF
                ON(ID_FOCUS_FILTER)
                    SetFocus(inp_filter);
                OFF
                ON(ID_FOCUS_EDITOR)
                    SetFocus(inp_editor);
                OFF
                ON(ID_EXIT)
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                OFF
            }
            switch(HIWORD(wParam)) {
                ON(EN_UPDATE)
                    if ((HWND)lParam == inp_filter) { // Filter data has changed
                        update_tabtree();
                        tabtree* node = tabroot.search(GetWindowTextWString(inp_filter));
                        if (node) {
                            tabnode = node;
                            if (tabnode->has_val) {
                                SetWindowTextWString(inp_editor, tabnode->val);
                            } else {
                                SetWindowTextWString(inp_editor, tabnode->get_content());
                            }
                        }
                        // Reset inactivity timer
                        KillTimer(dialog, ID_TIMEOUT_EDITOR);
                        SetTimer(dialog, ID_TIMEOUT_EDITOR, inactivity_timeout_ms, (TIMERPROC)NULL);
                    } else if ((HWND)lParam == inp_editor) { // Editor data has changed
                        if (insert_tabs) {
                            WORD start, end;
                            SendMessage(inp_editor, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
                            if (start >= 2) {
                                std::wstring s = GetWindowTextWString16(inp_editor);
                                // Pressing enter does not neccessarily insert a new line
                                if (s[start - 2] == L'\r' && s[start - 1] == L'\n') {
                                    std::wstring tabs(insert_tabs, L'\t');
                                    SendMessageW(inp_editor, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)tabs.c_str());
                                }
                            }
                            insert_tabs = false; // Modifying the editor text causes another EN_UPDATE!
                        }
                        need_save();
                        // Reset inactivity timer
                        KillTimer(dialog, ID_TIMEOUT_EDITOR);
                        SetTimer(dialog, ID_TIMEOUT_EDITOR, inactivity_timeout_ms, (TIMERPROC)NULL);
                    }
                OFF
            }
        OFF
        ON(WM_SIZE)
			RECT r;
            GetClientRect(hwnd, &r);
            // Set input and button positions
            MoveWindow(inp_filter, MARGIN, MARGIN, r.right - 2 * MARGIN, EDITHEIGHT, true);
            MoveWindow(inp_editor, 0, 2 * MARGIN + EDITHEIGHT, r.right, r.bottom - (2 * MARGIN + EDITHEIGHT), true);
        OFF
        ON(WM_ERASEBKGND) return TRUE; OFF
        ON(WM_PAINT)
			RECT r, bg, u;
            GetClientRect(hwnd, &r);
            // Draw background
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
            bg.left = 0;
            bg.top  = 0;
            bg.right = r.right;
            bg.bottom = 2 * MARGIN + EDITHEIGHT;
            FillRect(hdc, &bg, grey_brush);
            // Restore GDI objects, cleanup
			EndPaint(hwnd, &ps);
            ReleaseDC(hwnd, hdc);
        OFF
        ON(WM_TIMER)
            switch (wParam) { 
                ON(ID_TIMEOUT_EDITOR)
                    KillTimer(hwnd, ID_TIMEOUT_EDITOR);
                    if (! data_changed) {
                        // Close editor after inactivity
                        inactivity_close = true;
                        // MessageBeep(MB_ICONEXCLAMATION);
                        PostMessage(hwnd, WM_CLOSE, 0, 0);
                    }
                    return 0;
                OFF
            }
        OFF
        ON(WM_SETFOCUS)
            SetFocus(inp_filter);
        OFF
        ON(WM_CLOSE)
            if (data_changed) {
                MessageBeep(MB_ICONEXCLAMATION);
                if (! show_yesno_box(dialog, msg_wantsave_title, msg_wantsave_text)) {
                    return TRUE;
                }
            }
            KillTimer(hwnd, ID_TIMEOUT_EDITOR);
            DestroyWindow(inp_editor);
            DestroyWindow(hwnd);
            DestroyMenu(g_menu); // Recursive
        OFF
        ON(WM_DESTROY)
            DeleteObject(grey_brush);
            DeleteObject(font_text);
            PostQuitMessage(0);
        OFF
        default: return DefWindowProc(hwnd, wm, wParam, lParam);
    }
    return FALSE;
}

void init_gui_editor(init_data inid)
{
    // Setup window class settings
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;//CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = inid.hInstance;
    wc.hIcon = LoadIcon(inid.hInstance, MAKEINTRESOURCE(ID_APPICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = class_name;
    wc.hIconSm = LoadIcon(inid.hInstance, MAKEINTRESOURCE(ID_APPICON));
    if(!RegisterClassEx(&wc)) {
        MessageBoxW(NULL, L"RegisterClassEx failed", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return;
    }
}

bool show_gui_editor(init_data inid, db* db)
{
    // Initialize global variables
    database = db;
    tabtreeparser p;
    tabroot = p.parse(db->get_plaintext());
    tabnode = &tabroot;
    data_changed = false;
    inactivity_close = false;
    
    // Create window
    HWND hwnd = CreateWindowEx(
        WS_EX_LEFT,
        class_name,
        program_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        editor_window_width, editor_window_height,
        NULL, NULL, inid.hInstance, NULL);
    if(hwnd == NULL) {
        MessageBoxW(NULL, L"CreateWindowEx failed", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }
    dialog = hwnd;
    ShowWindow(hwnd, inid.nCmdShow);
    UpdateWindow(hwnd);
    
    // Accelerator table (STRG+S)
    HACCEL accel = LoadAccelerators(inid.hInstance, MAKEINTRESOURCE(ID_ACCEL));
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        
        // (! IsDialogMessage(hwnd, &msg))
        if (! TranslateAccelerator(hwnd, accel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    DestroyAcceleratorTable(accel);
    return inactivity_close;
}

