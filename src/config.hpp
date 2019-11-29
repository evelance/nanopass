#ifndef __CONFIG_HPP
#define __CONFIG_HPP

// Windows/Linux differences
#ifdef NANOPASS_LINUX
    #define CONFIG_STR      const char*
    #define CONFIG_PFX(s)   s
    #define CONFIG_MNEMONIC "_"
    struct init_data {
        int argc;
        char** argv;
    };
#else
    #include <windows.h>
    #define CONFIG_STR      LPWSTR
    #define CONFIG_PFX(s)   L ## s
    #define CONFIG_MNEMONIC "&"
    struct init_data {
        HINSTANCE hInstance;
        int nCmdShow;
    };
#endif

// General settings
extern const char* program_title_c;
extern CONFIG_STR program_title;
extern CONFIG_STR program_home_url;
extern const char* db_filename;
extern const char* db_initial_text;
extern const int create_db_window_width;
extern const int create_db_window_height;
extern const int decrypt_db_window_width;
extern const int decrypt_db_window_height;
extern const int editor_window_width;
extern const int editor_window_height;

// Main
extern const char* msg_err_load_database;

// Create database
extern const char* msg_masterpwempty_title;
extern const char* msg_masterpwempty_text;
extern const char* msg_dbcreated_title;
extern const char* msg_dbcreated_text;
extern const char* msg_dbnotcreated_title;
extern CONFIG_STR db_create_title;
extern CONFIG_STR db_create_descn;
extern CONFIG_STR db_create_inp_cue;
extern CONFIG_STR db_create_btn_create;

// Decrypt database
extern const char* msg_wrongpassword_title;
extern const char* msg_wrongpassword_text;
extern const char* msg_decryptfailed_title;
extern CONFIG_STR db_decrypt_title;
extern CONFIG_STR db_decrypt_descn;
extern CONFIG_STR db_decrypt_inp_cue;
extern CONFIG_STR db_decrypt_btn_open;

// Editor
extern unsigned inactivity_timeout_ms;
extern const char* msg_saved_title;
extern const char* msg_saved_text;
extern const char* msg_savefailed_title;
extern const char* msg_noenter_title;
extern const char* msg_noenter_text;
extern const char* msg_wantsave_title;
extern const char* msg_wantsave_text;
extern const char* msg_help_title;
extern const char* msg_help_text;
extern CONFIG_STR editor_menu_file;
extern CONFIG_STR editor_menu_save;
extern CONFIG_STR editor_menu_exit;
extern CONFIG_STR editor_menu_settings;
extern CONFIG_STR editor_menu_help;
extern CONFIG_STR editor_menu_info;
extern CONFIG_STR editor_menu_web;
extern CONFIG_STR editor_filter;

// Message dialog
extern CONFIG_STR message_btn_ok;
extern CONFIG_STR message_btn_yes;
extern CONFIG_STR message_btn_no;

#endif