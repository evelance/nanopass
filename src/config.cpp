#include "config.hpp"

// General settings
const char* program_title_c             = "Nanopass";
CONFIG_STR program_title                = CONFIG_PFX("Nanopass");
CONFIG_STR program_home_url             = CONFIG_PFX("https://evelance.de/nanopass_password_manager");
const char* db_filename                 = "passwords.npw";
const char* db_initial_text             = "";
#ifdef NANOPASS_LINUX
const int create_db_window_width        = 470;
const int create_db_window_height       = 190 - 30;
const int decrypt_db_window_width       = 413;
const int decrypt_db_window_height      = 162 - 30;
const int editor_window_width           = 620;
const int editor_window_height          = 620 - 30;
#else
const int create_db_window_width        = 470;
const int create_db_window_height       = 190;
const int decrypt_db_window_width       = 413;
const int decrypt_db_window_height      = 162;
const int editor_window_width           = 620;
const int editor_window_height          = 620;
#endif

// Main
const char* msg_err_load_database       = "Error loading database";

// Create database
const char* msg_masterpwempty_title     = "Error";
const char* msg_masterpwempty_text      = "The master password is empty.";
const char* msg_dbcreated_title         = "Database successfully created";
const char* msg_dbcreated_text          = "The database file has been created. You can now open and edit it.";
const char* msg_dbnotcreated_title      = "Error creating database";
CONFIG_STR db_create_title              = CONFIG_PFX("Setup new password database");
CONFIG_STR db_create_descn              = CONFIG_PFX("No database file found. To setup a new one, please provide the master password that will "
                                           "be used for the encryption of the database. It is very important to remember it as you will "
                                           "always have to enter it to access the stored passwords.");
CONFIG_STR db_create_inp_cue            = CONFIG_PFX("Enter master password");
CONFIG_STR db_create_btn_create         = CONFIG_PFX("Create database");

// Decrypt database
const char* msg_wrongpassword_title     = "Wrong password";
const char* msg_wrongpassword_text      = "The given password is wrong. Please try again.";
const char* msg_decryptfailed_title     = "Could not decrypt database";
CONFIG_STR db_decrypt_title             = CONFIG_PFX("Decrypt password database");
CONFIG_STR db_decrypt_descn             = CONFIG_PFX("Please enter the database password below to encrypt the password database.");
CONFIG_STR db_decrypt_inp_cue           = CONFIG_PFX("Enter database password");
CONFIG_STR db_decrypt_btn_open          = CONFIG_PFX("Open database");

// Editor
unsigned inactivity_timeout_ms          = 1000 * 60 * 10;
const char* msg_saved_title             = "Data successfully saved";
const char* msg_saved_text              = "All data has been encrypted and was saved.";
const char* msg_savefailed_title        = "Error saving data";
const char* msg_noenter_title           = "Multiple lines not allowed";
const char* msg_noenter_text            = "You are currently editing a value field that only has one line.";
const char* msg_wantsave_title          = "Discard changes?";
const char* msg_wantsave_text           = "There are currently unsaved changes. Do you really want to exit?";
const char* msg_help_title              = "Nanopass 1.0";
const char* msg_help_text               = "Encryption: AES256 with SHA3-HMAC\r\n"
                                          "Text structured by indentation can be accessed using the filter.\r\n"
                                          "\r\n"
                                          "Filter commands\r\n"
                                          "\tTab: Autocomplete\r\n"
                                          "\tSpace: Access deeper level\r\n"
                                          "\tEnter: Copy editor content to clipboard\r\n"
                                          "Structuring characters\r\n"
                                          "\tTab: Multiline content like this:\r\n"
                                          "\t\tA\r\n"
                                          "\t\t\tText for A\r\n"
                                          "\t\t\tMore in A\r\n"
                                          "\t':': Single-line content like this:\r\n"
                                          "\t\tA: Text line for A\r\n"
                                          "\r\n"
                                          "Icons by Yannick Lung - http://yannicklung.com/\r\n";
CONFIG_STR editor_menu_file             = CONFIG_PFX("" CONFIG_MNEMONIC "File");
CONFIG_STR editor_menu_save             = CONFIG_PFX("" CONFIG_MNEMONIC "Save");
CONFIG_STR editor_menu_exit             = CONFIG_PFX("E" CONFIG_MNEMONIC "xit");
CONFIG_STR editor_menu_settings         = CONFIG_PFX("" CONFIG_MNEMONIC "Settings");
CONFIG_STR editor_menu_help             = CONFIG_PFX("" CONFIG_MNEMONIC "Help");
CONFIG_STR editor_menu_info             = CONFIG_PFX("" CONFIG_MNEMONIC "Information and usage");
CONFIG_STR editor_menu_web              = CONFIG_PFX("Visit " CONFIG_MNEMONIC "website");
CONFIG_STR editor_filter                = CONFIG_PFX("Filter...");

// Message dialog                       
CONFIG_STR message_btn_ok               = CONFIG_PFX("Ok");
CONFIG_STR message_btn_yes              = CONFIG_PFX("Yes");
CONFIG_STR message_btn_no               = CONFIG_PFX("No");

