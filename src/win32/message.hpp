#ifndef __MESSAGE_HPP
#define __MESSAGE_HPP
#include <windows.h>
#include <string>

// Custom window looking like windows error message, but is centered on the given parent window
void show_modal_message(HWND parent, std::string title, std::string text);

// Custom window looking centered on the given parent window, has Yes/No buttons
bool show_yesno_box(HWND parent, std::string title, std::string text);

#endif