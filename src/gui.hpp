/*
    All GUI windows (OS-independent general headers).
    The init_data struct looks different depending on the build platform.
    init_gui_* must be called once before calling show_gui_*.
    show_gui_* shows the dialog and blocks until it is closed.
*/
#ifndef __GUI_HPP
#define __GUI_HPP
#include <string>
#include "config.hpp"
#include "db.hpp"

// Database creation view with masterpassword input. Creates the database
// file with the given password. Check if the database file exists to
// check for success.
void init_gui_create_db(init_data inid);
void show_gui_create_db(init_data inid);

// Database decryption view with password input. Decrypts the database with the
// given password.
// show_gui_decrypt_db returns true if the database was succesfully decrypted.
void init_gui_decrypt_db(init_data inid);
bool show_gui_decrypt_db(init_data inid, db* db);

// Editor view with filter and text content. Enables the user to
// view, search and edit the tabtree and save the result to the database.
// show_gui_editor returns true if it was closed automatically due to inactivity.
void init_gui_editor(init_data inid);
bool show_gui_editor(init_data inid, db* db);

// Standard message to warn user
void show_message(std::string title, std::string text);

#endif