#include <fstream>
#include <string>
#include <time.h>
#include <gtk/gtk.h>
#include "../tabtree/tabtreeparser.hpp"
#include "../tabtree/tabtree.hpp"
#include "../db.hpp"
#include "../gui.hpp"
#include "../config.hpp"
#include "../resource/key.hpp"
#include "message.hpp"
using namespace std;

const size_t MARGIN = 5;
static GtkWidget* window = NULL;
static GtkWidget* filter = NULL;
static GtkWidget* editor = NULL;
static GtkTextBuffer* editorbuf = NULL;
static gulong editor_changed_signal = 0;
static db* database = NULL;
static tabtree  tabroot;
static tabtree* tabnode = NULL;
static bool data_changed = false;
static bool window_closed = false;
static bool inactivity_close = false;

// Get data from editor and put it into the currently selected tabnode
static void update_tabtree()
{
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(editorbuf, &start);
    gtk_text_buffer_get_end_iter(editorbuf, &end);
    std::string editor(gtk_text_buffer_get_text(editorbuf, &start, &end, TRUE));
    if (tabnode->has_val) {
        tabnode->set_val(editor);
    } else {
        tabnode->set_content(editor);
    }
}

// Intercept tab key press and use it for filter autocompletion, and enter to set the clipboard
gboolean filter_keypress(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
    if (event->keyval == GDK_KEY_Tab && !(event->state & GDK_CONTROL_MASK)) {
        std::string sugg = tabroot.search_suggest(gtk_entry_get_text(GTK_ENTRY(filter)));
        if (! sugg.empty()) {
            gtk_entry_set_text(GTK_ENTRY(filter), sugg.c_str());
            gtk_editable_set_position(GTK_EDITABLE(filter), -1); // Put cursor to the end
        }
        return TRUE; // No other handlers will be called
    }
    if (event->keyval == GDK_KEY_Return) {
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(editorbuf, &start);
        gtk_text_buffer_get_end_iter(editorbuf, &end);
        gtk_text_buffer_select_range(editorbuf, &start, &end);
        gtk_text_buffer_copy_clipboard(editorbuf, gtk_clipboard_get(gdk_atom_intern("CLIPBOARD", FALSE)));
    }
    return FALSE; // Propagates event
}

// Filter has changed: Search for tabtree node
void filter_changed(GtkEditable* editable, gpointer user_data)
{
    update_tabtree();
    string fs(gtk_entry_get_text(GTK_ENTRY(filter)));
    tabtree* node = tabroot.search(fs);
    if (node) {
        tabnode = node;
        string s = tabnode->has_val ? tabnode->val : tabnode->get_content("\n");
        // Update buffer without triggering "data changed"
        g_signal_handler_block(editorbuf, editor_changed_signal);
        gtk_text_buffer_set_text(editorbuf, s.c_str(), s.length());
        g_signal_handler_unblock(editorbuf, editor_changed_signal);
    }
}

// Visually show in the title bar that there is unsaved data. Also do not close the window!
static void need_save(GtkTextBuffer* textbuffer, gpointer user_data)
{
    if (! data_changed) {
        string s("* ");
        s += program_title_c;
        gtk_window_set_title(GTK_WINDOW(window), s.c_str());
        data_changed = true;
    }
}

// Show information about usage
static void on_info(GtkMenuItem* item, gpointer data)
{
    show_modal_message(GTK_WINDOW(window), msg_help_title, msg_help_text);
}

// Open website URL
static void on_web(GtkMenuItem* item, gpointer data)
{
    gtk_show_uri(NULL, program_home_url, GDK_CURRENT_TIME, NULL);
}

// Get all data from root node and save to database
static void on_save(GtkMenuItem* item, gpointer data)
{
    if (! data_changed)
        return;
    // Make backup of old file
    std::ifstream ifs(db_filename, std::ios::binary);
    if (ifs.good()) { // File exists
        char bakname[1024];
        snprintf(bakname, sizeof bakname, "%s.%i.bak", db_filename, (int)time(NULL));
        std::ofstream bak(bakname, std::ios::binary);
        bak << ifs.rdbuf();
    }
    // Fetch current data, encrypt it and write it to the new file
    update_tabtree();
    std::string content = tabroot.get_content("\n");
    database->set_plaintext(content.c_str(), content.length());
    if (database->write(db_filename) != db::OK) {
        show_modal_message(GTK_WINDOW(window), msg_savefailed_title, database->get_error());
    } else {
        show_modal_message(GTK_WINDOW(window), msg_saved_title, msg_saved_text);
    }
    data_changed = false;
    gtk_window_set_title(GTK_WINDOW(window), program_title_c);
}

static gboolean delete_event(GtkWidget* widget, GdkEvent* event, gpointer data)
{
    if (window_closed)
        return FALSE;
    if (data_changed) {
        if (! show_yesno_box(GTK_WINDOW(window), msg_wantsave_title, msg_wantsave_text)) {
            return TRUE;
        }
    }
    window_closed = true;
    return FALSE; // FALSE = destroy
}

static void destroy(GtkWidget* widget, gpointer data)
{
    gtk_main_quit();
}

static void on_want_exit(GtkMenuItem* item, gpointer data)
{
    if (! delete_event(NULL, NULL, NULL)) {
        destroy(NULL, NULL);
    }
}

void init_gui_editor(init_data inid) { }

bool show_gui_editor(init_data inid, db* db)
{
    database = db;
    tabtreeparser p;
    tabroot = p.parse(db->get_plaintext());
    tabnode = &tabroot;
    data_changed = false;
    window_closed = false;
    inactivity_close = false;
    
    // Main window with taskbar icon
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), editor_window_width, editor_window_height);
    gtk_window_set_title(GTK_WINDOW(window), program_title_c);
    GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
    gdk_pixbuf_loader_write(loader, key_png, key_png_len, NULL);
    gdk_pixbuf_loader_close(loader, NULL);
    gtk_window_set_icon(GTK_WINDOW(window), gdk_pixbuf_loader_get_pixbuf(loader));
    g_object_unref(loader);
    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
    
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    // Menu
    GtkWidget* menu = gtk_menu_bar_new();
    
    GtkAccelGroup* accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
    
    GtkWidget* file = gtk_menu_item_new_with_mnemonic(editor_menu_file);
    GtkWidget* file_menu = gtk_menu_new();
    GtkWidget* save = gtk_menu_item_new_with_mnemonic(editor_menu_save);
    GtkWidget* exit = gtk_menu_item_new_with_mnemonic(editor_menu_exit);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), exit);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), file);
    g_signal_connect(G_OBJECT(save), "activate", G_CALLBACK(on_save), NULL);
    g_signal_connect(G_OBJECT(exit), "activate", G_CALLBACK(on_want_exit), NULL);
    gtk_widget_add_accelerator(save, "activate", accel_group, GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(exit, "activate", accel_group, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    GtkWidget* help = gtk_menu_item_new_with_mnemonic(editor_menu_help);
    GtkWidget* help_sub = gtk_menu_new();
    GtkWidget* info = gtk_menu_item_new_with_mnemonic(editor_menu_info);
    GtkWidget* web = gtk_menu_item_new_with_mnemonic(editor_menu_web);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), help_sub);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_sub), info);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_sub), web);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), help);
    g_signal_connect(G_OBJECT(info), "activate", G_CALLBACK(on_info), NULL);
    g_signal_connect(G_OBJECT(web), "activate", G_CALLBACK(on_web), NULL);
    gtk_widget_add_accelerator(info, "activate", accel_group, GDK_KEY_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    gtk_box_pack_start(GTK_BOX(vbox), menu, FALSE, FALSE, 3);
    
    // Only for margin
    GtkWidget* vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox2), MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), vbox2, FALSE, TRUE, 0);
    
    // Single-line Filter
    filter = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox2), filter, FALSE, TRUE, 0);    
    g_signal_connect(G_OBJECT(filter), "changed", G_CALLBACK(filter_changed), NULL);
    g_signal_connect(G_OBJECT(filter), "key-press-event", G_CALLBACK(filter_keypress), NULL);
    
    // Multi-line editor
    GtkWidget* sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    editor = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(sw), editor);
    gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
    editorbuf = gtk_text_buffer_new(NULL);
    string s = tabroot.get_content("\n");
    gtk_text_buffer_set_text(editorbuf, s.c_str(), s.length());
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(editor), editorbuf);
    editor_changed_signal = g_signal_connect(G_OBJECT(editorbuf), "changed", G_CALLBACK(need_save), NULL);
    
    gtk_widget_show_all(window);
    gtk_main();
    return inactivity_close;
}

