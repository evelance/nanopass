#ifndef __MESSAGE_HPP
#define __MESSAGE_HPP
#include <string>
#include <gtk/gtk.h>

void show_modal_message(GtkWindow* parent, std::string title, std::string text);
bool show_yesno_box(GtkWindow* parent, std::string title, std::string text);

#endif