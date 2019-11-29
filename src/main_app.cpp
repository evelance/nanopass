#include <string>
#include <fstream>
#include "config.hpp"
#include "gui.hpp"
#include "main_app.hpp"
using namespace std;

static bool file_exists(const char* filename)
{
    ifstream ifs(filename);
    return ifs.good();
}

int main_app(struct init_data inid)
{
    // Create new database if it does not exist
    if (! file_exists(db_filename)) {
        init_gui_create_db(inid);
        show_gui_create_db(inid);
    }
    // If a database exists or has been created, show the decrypt window and on success the editor
    if (file_exists(db_filename)) {
        init_gui_decrypt_db(inid);
        init_gui_editor(inid);
        bool auto_closed;
        do {
            db database;
            auto_closed = false;
            // Load database into memory
            if (database.load(db_filename) != db::status::OK) {
                show_message(msg_err_load_database, database.get_error() + std::string("\r\n") + std::string(db_filename));
                return 1;
            }
            // Show password prompt and try to decrypt it
            if (show_gui_decrypt_db(inid, &database)) {
                // Show editor that can modify and save the database. If it was closed
                // due to inactivity, go back to the decrypt window.
                auto_closed = show_gui_editor(inid, &database);
            }
        } while (auto_closed);
    }
    return 0;
}

