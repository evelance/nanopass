#define _UNICODE
#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <string>
#include "../resource/resource.h"
#include "../config.hpp"
#include "../gui.hpp"
#include "utfconvert.hpp"
using namespace std;

static HFONT font_text = NULL;
static HBRUSH grey_brush = NULL;
static HPEN line_pen = NULL;

static string windowtitle;
static LPWSTR textbuf = NULL;
static RECT tsz; // Text size
static bool is_question = false;
static bool answer = false;

static HWND okbtn = NULL;
static HWND yesbtn = NULL;
static HWND nobtn = NULL;

static const int margin = 8;
static const int btn_width = 80;
static const int btn_height = 23;
static const int bg_size = margin + btn_height + margin;
static const int box_width = 350;
static const int extra_height = 50;

#define ON(cond) case cond: {
#define OFF } break;

static INT_PTR CALLBACK proc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    switch (wm) {
        ON(WM_INITDIALOG)
            // Get font, brush and pen
            NONCLIENTMETRICS metrics;
            metrics.cbSize = sizeof(NONCLIENTMETRICS);
            SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
            metrics.lfMessageFont.lfHeight = 16;
            font_text = CreateFontIndirectW(&metrics.lfMessageFont);
            grey_brush = CreateSolidBrush(RGB(240, 240, 240));
            line_pen = CreatePen(PS_SOLID, 1, RGB(223, 223, 223));
            // Get parent window coordinates
            HWND parent = GetParent(hwnd);
            if (parent == NULL)
                parent = GetDesktopWindow();
            RECT par;
            GetWindowRect(parent, &par);
            // Adjust window to fit the text
            PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
            HGDIOBJ old_font = SelectObject(hdc, font_text);
            tsz.top = 0; tsz.left = 0; tsz.bottom = 0; tsz.right = box_width - (margin / 2);
            DrawTextExW(hdc, textbuf, -1, &tsz, DT_CALCRECT|DT_WORDBREAK|DT_TOP|DT_LEFT, NULL);
			EndPaint(hwnd, &ps);
            SelectObject(hdc, old_font);
            ReleaseDC(hwnd, hdc);
            int height = margin + tsz.bottom + margin + bg_size + extra_height;
            MoveWindow(hwnd, (par.left + par.right) / 2 - box_width / 2, (par.top + par.bottom) / 2 - height / 2, box_width, height, TRUE);
            SetWindowTextWString(hwnd, windowtitle);
            if (is_question) {
                nobtn = CreateWindow(L"BUTTON", message_btn_no, WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
                    0, 0, 0, 0, hwnd, (HMENU)IDNO, GetModuleHandle(NULL), NULL);
                yesbtn = CreateWindow(L"BUTTON", message_btn_yes, WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
                    0, 0, 0, 0, hwnd, (HMENU)IDYES, GetModuleHandle(NULL), NULL);
                SendMessage(yesbtn, WM_SETFONT, (LPARAM)font_text, true);
                SendMessage(nobtn, WM_SETFONT, (LPARAM)font_text, true);
            } else {
                okbtn = CreateWindow(L"BUTTON", message_btn_ok, WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,
                    0, 0, 0, 0, hwnd, (HMENU)IDOK, GetModuleHandle(NULL), NULL);
                SendMessage(okbtn, WM_SETFONT, (LPARAM)font_text, true);
            }
            return TRUE;
        OFF
        ON(WM_ERASEBKGND) return TRUE; OFF
        ON(WM_PAINT)
            // Setup GDI objects
            RECT r;
			GetClientRect(hwnd, &r);
            PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
            HGDIOBJ old_pen = SelectObject(hdc, line_pen);
            HGDIOBJ old_font = SelectObject(hdc, font_text);
            // Calculate positions
            RECT tpos; // Text position
            tpos.left = margin; tpos.top = margin;
            tpos.right = r.right - (margin / 2);
            tpos.bottom = r.bottom;
            RECT bg; // Darker background for button
            bg.left = 0;
            bg.top  = r.bottom - bg_size;
            bg.right = r.right; 
            bg.bottom = r.bottom;
            DrawTextExW(hdc, textbuf, -1, &tpos, DT_WORDBREAK|DT_TOP|DT_LEFT, NULL);
            // Draw separator and background
            FillRect(hdc, &bg, grey_brush);
            MoveToEx(hdc, bg.left, bg.top, NULL);
            LineTo(hdc, bg.right, bg.top);
            // Set button positions
            if (is_question) {
                MoveWindow(yesbtn, r.right - 2 * btn_width - 2 * margin, bg.top + margin + 1, btn_width, btn_height, true);
                MoveWindow(nobtn, r.right - btn_width - margin, bg.top + margin + 1, btn_width, btn_height, true);
            } else {
                MoveWindow(okbtn, r.right - btn_width - margin, bg.top + margin + 1, btn_width, btn_height, true);
            }
            // Restore GDI objects, cleanup
			EndPaint(hwnd, &ps);
            SelectObject(hdc, old_pen);
            SelectObject(hdc, old_font);
            ReleaseDC(hwnd, hdc);
            return TRUE;
        OFF
        ON(WM_COMMAND)
            switch(LOWORD(wParam)) {
                ON(IDOK)
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return TRUE;
                OFF
                ON(IDYES)
                    answer = true;
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return TRUE;
                OFF
                ON(IDNO)
                    answer = false;
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return TRUE;
                OFF
                ON(IDCANCEL)
                    answer = false;
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return TRUE;
                OFF
            }
        OFF
        ON(WM_CLOSE)
            EndDialog(hwnd, 0);
            DeleteObject(font_text);
            DeleteObject(grey_brush);
            DeleteObject(line_pen);
            return TRUE;
        OFF
    }
    return FALSE;
}

void show_message(string title, string text)
{
    wstring wtext = utf8to16(text);
    wstring wtitle = utf8to16(title);
    LPWSTR buf_text = new wchar_t[wtext.length() + 1];
    memcpy(buf_text,  &wtext[0],  (wtext.length() + 1) * sizeof(wchar_t));
    LPWSTR buf_title = new wchar_t[wtitle.length() + 1];
    memcpy(buf_title,  &wtitle[0],  (wtitle.length() + 1) * sizeof(wchar_t));
    MessageBoxW(NULL, buf_text, buf_title, MB_ICONEXCLAMATION | MB_OK);
    delete buf_text;
    delete buf_title;
}

void show_modal_message(HWND parent, string title, string text)
{
    is_question = false;
    windowtitle = title;
    wstring wtext = utf8to16(text);
    textbuf = new wchar_t[wtext.length() + 1];
    memcpy(textbuf,  &wtext[0],  (wtext.length() + 1) * sizeof(wchar_t));
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_MODALBOX), parent, proc);
    delete textbuf;
}

bool show_yesno_box(HWND parent, string title, string text)
{
    is_question = true;
    answer = false;
    windowtitle = title;
    wstring wtext = utf8to16(text);
    textbuf = new wchar_t[wtext.length() + 1];
    memcpy(textbuf,  &wtext[0],  (wtext.length() + 1) * sizeof(wchar_t));
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_MODALBOX), parent, proc);
    delete textbuf;
    return answer;
}

