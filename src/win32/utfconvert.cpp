#include <windows.h>
#include <string>
#include "utfconvert.hpp"
using namespace std;

string utf16to8(const wstring& wstr)
{
    if (wstr.empty()) return string();
    int strlen = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string str(strlen, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], strlen, NULL, NULL);
    return str;
}

wstring utf8to16(const string& str)
{
    if(str.empty()) return wstring();
    int wstrlen = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstr(wstrlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], wstrlen);
    return wstr;
}

wstring GetWindowTextWString16(HWND hwnd)
{
    // GetWindowTextW sometimes returns nothing for 1 or 8 characters
    // if there is not enough extra space. 32 more seems to work...
    size_t len16 = GetWindowTextLengthW(hwnd) + 32;
    wstring wstr(len16, 0);
    int actuallen = SendMessageW(hwnd, WM_GETTEXT, (WPARAM)len16, (LPARAM)&wstr[0]);
    wstr.resize((actuallen > 0) ? actuallen : 0);
    return wstr;
}

void SetWindowTextWString16(HWND hwnd, const wstring& str)
{
    SetWindowTextW(hwnd, str.c_str());
}

string GetWindowTextWString(HWND hwnd)
{
    return utf16to8(GetWindowTextWString16(hwnd));
}

void SetWindowTextWString(HWND hwnd, const string& str)
{
    SetWindowTextWString16(hwnd, utf8to16(str));
}

