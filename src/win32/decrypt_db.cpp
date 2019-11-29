#define _UNICODE
#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <string>
#include "../resource/resource.h"
#include "../config.hpp"
#include "../gui.hpp"
#include "../db.hpp"
#include "message.hpp"
#include "utfconvert.hpp"

static LPWSTR class_name = L"decrypt_db";
static HFONT font_title;
static HFONT font_text;
static HPEN line_pen;
static HBRUSH grey_brush;
static HBITMAP lock_image = NULL;

static HWND dialog = NULL;
static HWND inp_passw = NULL;
static HWND btn_decrypt = NULL;

static db* database = NULL;
static bool decrypt_success = false;

static WNDPROC inp_passw_proc_old;

#define ON(cond) case cond: {
#define OFF } break;

static LRESULT CALLBACK inp_passw_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        ON(WM_GETDLGCODE)
            return DLGC_WANTALLKEYS; // Process TAB and ENTER keys
        OFF
        ON(WM_KEYDOWN)
            switch (wParam) {
                ON(VK_TAB)
                    SetFocus(btn_decrypt);
                    return 0;
                OFF
                ON(VK_RETURN)
                    SendMessageW(dialog, WM_COMMAND, MAKEWPARAM(ID_BTN_DECRYPT, 0), (LPARAM)0);
                    return 0;
                OFF
            }
        OFF
        ON(WM_CHAR)
            switch (wParam) {
                ON('\t') return 0; OFF // Prevent sound on tab
                ON(0x0D) return 0; OFF // Prevent sound on enter
                ON(0x7F) // CTRL + BS: Clear input
                    SetWindowTextW(hwnd, L"");
                    return 0;
                OFF
            }
        OFF
    }
    return CallWindowProc(inp_passw_proc_old, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    switch (wm) {
        ON(WM_CREATE)
            // Load fonts, pen, brush and image
            NONCLIENTMETRICS metrics;
            metrics.cbSize = sizeof(NONCLIENTMETRICS);
            SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
            metrics.lfMessageFont.lfHeight = 25;
            font_title = CreateFontIndirectW(&metrics.lfMessageFont);
            metrics.lfMessageFont.lfHeight = 16;
            font_text = CreateFontIndirectW(&metrics.lfMessageFont);
            line_pen = CreatePen(PS_SOLID, 1, RGB(223, 223, 223));
            grey_brush = CreateSolidBrush(RGB(240, 240, 240));
            lock_image = LoadBitmapW(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_LOCK));
            // Create input and buttons
            inp_passw = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_TABSTOP|WS_VISIBLE|WS_CHILD|ES_WANTRETURN|ES_PASSWORD,
                0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);
            btn_decrypt = CreateWindow(L"BUTTON", db_decrypt_btn_open, WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
                0, 0, 0, 0, hwnd, (HMENU)ID_BTN_DECRYPT, GetModuleHandle(NULL), NULL);
            SendMessageW(inp_passw, WM_SETFONT, (LPARAM)font_text, true);
            SendMessageW(btn_decrypt, WM_SETFONT, (LPARAM)font_text, true);
            SendMessageW(inp_passw, EM_SETCUEBANNER, true, (LPARAM)db_decrypt_inp_cue);
            inp_passw_proc_old = (WNDPROC)SetWindowLongPtr(inp_passw, GWLP_WNDPROC, (LONG_PTR)inp_passw_proc);
            SetFocus(inp_passw);
        OFF
        ON(WM_PAINT)
            #define MARGIN 5
            // Setup GDI objects
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
            HGDIOBJ old_pen = SelectObject(hdc, line_pen);
            HGDIOBJ old_font = SelectObject(hdc, font_title);
            // Load image
            HDC hdc_mem = CreateCompatibleDC(hdc);
            HBITMAP bm_old = (HBITMAP)SelectObject(hdc_mem, lock_image);
            BITMAP bm;
            GetObject(lock_image, sizeof bm, &bm);
            // Calculate offsets
			RECT r, r1, r2, footer;
            GetClientRect(hwnd, &r);
            r1.top =  MARGIN; r1.left = 2 * MARGIN; r1.bottom = r.bottom; r1.right = r.right - MARGIN;
            r2.top =      35; r2.left = 4 * MARGIN + bm.bmWidth; r2.bottom = r.bottom; r2.right = r.right - MARGIN;
            // Draw title and description
            SetTextColor(hdc, RGB(0, 30, 80));
            DrawText(hdc, db_decrypt_title, -1, &r1, DT_END_ELLIPSIS|DT_TOP|DT_LEFT);
            SelectObject(hdc, font_text);
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawTextW(hdc, db_decrypt_descn, -1, &r2, DT_CALCRECT|DT_WORDBREAK|DT_TOP|DT_LEFT); // Calculates text height
            int below_descn = r2.bottom + 8, over_bottom = r.bottom - 23 - 2 * 8;
            footer.left = 0;
            footer.top  = below_descn < over_bottom ? over_bottom : below_descn;
            footer.bottom = r.bottom;
            footer.right = r.right;
            DrawTextW(hdc, db_decrypt_descn, -1, &r2, DT_WORDBREAK|DT_TOP|DT_LEFT);
            // Draw image
            BitBlt(hdc, 2 * MARGIN, r2.top + MARGIN, bm.bmWidth, bm.bmHeight, hdc_mem, 0, 0, SRCCOPY);
            SelectObject(hdc_mem, bm_old);
            DeleteDC(hdc_mem);
            // Draw separator and footer background
            FillRect(hdc, &footer, grey_brush);
            MoveToEx(hdc, footer.left, footer.top, NULL);
            LineTo(hdc, footer.right, footer.top);
            // Set input and button positions
            SetWindowPos(inp_passw, 0, r.left + 8, footer.top + 9, r.right - 105 - 3 * 8, 23, SWP_NOZORDER);
            SetWindowPos(btn_decrypt, 0, r.right - 105 - 8, footer.top + 9, 105, 23, SWP_NOZORDER);
            // Restore GDI objects, cleanup
			EndPaint(hwnd, &ps);
            SelectObject(hdc, old_pen);
            SelectObject(hdc, old_font);
            ReleaseDC(hwnd, hdc);
        OFF
        ON(WM_COMMAND)
            switch(LOWORD(wParam)) {
                ON(ID_BTN_DECRYPT)
                    if (! decrypt_success) {
                        std::string password = GetWindowTextWString(inp_passw);
                        database->set_password(password.c_str(), password.length());
                        db::status s = database->decrypt();
                        if (s == db::OK) {
                            decrypt_success = true;
                            PostMessage(hwnd, WM_CLOSE, 0, 0);
                        } else if (s == db::ERR_HMAC_MISMATCH) {
                            // Wrong password or corrupted ciphertext
                            MessageBeep(MB_ICONEXCLAMATION);
                            show_modal_message(hwnd, msg_wrongpassword_title, msg_wrongpassword_text);
                            SetWindowTextW(inp_passw, NULL);
                            SetFocus(inp_passw);
                        } else {
                            // Internal error
                            MessageBeep(MB_ICONEXCLAMATION);
                            show_modal_message(hwnd, msg_decryptfailed_title, database->get_error());
                            SetFocus(inp_passw);
                        }
                    }
                OFF
            }
        OFF
        ON(WM_CLOSE)
            DestroyWindow(inp_passw);
            DestroyWindow(btn_decrypt);
            DestroyWindow(hwnd);
        OFF
        ON(WM_DESTROY)
            DeleteObject(font_title);
            DeleteObject(font_text);
            DeleteObject(grey_brush);
            DeleteObject(line_pen);
            DeleteObject(lock_image);
            PostQuitMessage(0);
        OFF
        default: return DefWindowProc(hwnd, wm, wParam, lParam);
    }
    return 0;
}

void init_gui_decrypt_db(init_data inid)
{
    // Setup window class settings
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
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
    if (! RegisterClassEx(&wc)) {
        MessageBoxW(NULL, L"RegisterClassEx failed", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return;
    }
}

bool show_gui_decrypt_db(init_data inid, db* db)
{
    // Initialize global variables
    database = db;
    decrypt_success = false;
    
    // Create window
    HWND hwnd = CreateWindowEx(
        WS_EX_LEFT,
        class_name,
        program_title,
        WS_OVERLAPPEDWINDOW, // &~WS_SIZEBOX &~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        decrypt_db_window_width, decrypt_db_window_height,
        NULL, NULL, inid.hInstance, NULL);
    if(hwnd == NULL) {
        MessageBoxW(NULL, L"CreateWindowEx failed", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }
    dialog = hwnd;
    ShowWindow(hwnd, inid.nCmdShow);
    UpdateWindow(hwnd);
    
    // Start message Loop
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    // Window now closed
    return decrypt_success;
}

