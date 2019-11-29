#ifndef __CONVERT_UNICODE_HPP
#define __CONVERT_UNICODE_HPP
#include <windows.h>
#include <string>

// Convert UTF-16 string to UTF-8 string
std::string utf16to8(const std::wstring& wstr);

// Convert UTF-8 string to UTF-16 string
std::wstring utf8to16(const std::string& str);

// Retrieve UTF-16 text from WIN32 Unicode control
std::wstring GetWindowTextWString16(HWND hwnd);

// Set WIN32 Unicode control text
void SetWindowTextWString16(HWND hwnd, const std::wstring& str);

// Retrieve UTF-16 text from WIN32 Unicode control and convert it to UTF-8 string
std::string GetWindowTextWString(HWND hwnd);

// Set WIN32 Unicode control text by using converted UTF-8 string given
void SetWindowTextWString(HWND hwnd, const std::string& str);

#endif