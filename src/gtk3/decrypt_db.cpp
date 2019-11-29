#include <string>
#include <gtk/gtk.h>
#include "../db.hpp"
#include "../gui.hpp"
#include "../config.hpp"
#include "../resource/key.hpp"
#include "../resource/lock.cpp"
#include "message.hpp"

const size_t MARGIN = 8;
static GtkWidget* window = NULL;
static GtkWidget* entry = NULL;
static db* database = NULL;
static bool decrypt_success = false;

static void btn_decrypt_db(GtkWidget* widget, gpointer data)
{
    if (! decrypt_success) {
        std::string pw(gtk_entry_get_text(GTK_ENTRY(entry)));
        database->set_password(pw.c_str(), pw.length());
        db::status s = database->decrypt();
        if (s == db::OK) {
            decrypt_success = true;
            gtk_widget_destroy(window);
        } else if (s == db::ERR_HMAC_MISMATCH) {
            // Wrong password or corrupted ciphertext
            show_modal_message(GTK_WINDOW(window), msg_wrongpassword_title, msg_wrongpassword_text);
            gtk_entry_set_text(GTK_ENTRY(entry), "");
        } else {
            // Internal error
            show_modal_message(GTK_WINDOW(window), msg_decryptfailed_title, database->get_error());
        }
    }
}

static gboolean delete_event(GtkWidget *widget, GdkEvent* event, gpointer data)
{
    return FALSE; // FALSE = destroy
}

static void destroy(GtkWidget* widget, gpointer data)
{
    gtk_main_quit();
}

void init_gui_decrypt_db(init_data inid) { }

bool show_gui_decrypt_db(init_data inid, db* db)
{
    database = db;
    decrypt_success = false;
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), decrypt_db_window_width, decrypt_db_window_height);
    gtk_window_set_title(GTK_WINDOW(window), program_title_c);
    GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
    gdk_pixbuf_loader_write(loader, key_png, key_png_len, NULL);
    gdk_pixbuf_loader_close(loader, NULL);
    gtk_window_set_icon(GTK_WINDOW(window), gdk_pixbuf_loader_get_pixbuf(loader));
    g_object_unref(loader);
    gtk_container_set_border_width(GTK_CONTAINER(window), MARGIN);
    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
    
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, MARGIN);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    // Title
    GtkWidget* title = gtk_label_new(db_decrypt_title);
    // gtk_label_set_xalign(GTK_LABEL(title), 0.0); // Looks better but needs GTK+3.16
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_label_set_justify(GTK_LABEL(title), GTK_JUSTIFY_LEFT);
    gtk_label_set_ellipsize(GTK_LABEL(title), PANGO_ELLIPSIZE_END);
    PangoAttrList* pal = pango_attr_list_new();
    pango_attr_list_insert(pal, pango_attr_size_new(15 * PANGO_SCALE));
    gtk_label_set_attributes(GTK_LABEL(title), pal);
    pango_attr_list_unref(pal);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);
    
    // Image + text
    GtkWidget* hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), hbox1, TRUE, TRUE, 0);
    
    loader = gdk_pixbuf_loader_new();
    gdk_pixbuf_loader_write(loader, lock_png, lock_png_len, NULL);
    gdk_pixbuf_loader_close(loader, NULL);
    GtkWidget* image = gtk_image_new_from_pixbuf(gdk_pixbuf_loader_get_pixbuf(loader));
    g_object_unref(loader);
    gtk_widget_set_halign(image, GTK_ALIGN_START);
    gtk_widget_set_valign(image, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(hbox1), image, FALSE, FALSE, 0);
    
    GtkWidget* text = gtk_label_new(db_decrypt_descn);
    // gtk_label_set_xalign(GTK_LABEL(text), 0.0); // Looks better but needs GTK+3.16
    // gtk_label_set_yalign(GTK_LABEL(text), 0.0);
    gtk_widget_set_halign(text, GTK_ALIGN_START);
    gtk_widget_set_valign(text, GTK_ALIGN_START);
    gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
    gtk_label_set_line_wrap(GTK_LABEL(text), TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(text), PANGO_ELLIPSIZE_END);
    gtk_label_set_lines(GTK_LABEL(text), 2);
    gtk_box_pack_start(GTK_BOX(hbox1), text, TRUE, TRUE, 0);
    
    // Separator
    GtkWidget* line = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);
    
    // Password input + button
    GtkWidget* hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);
    
    entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    g_signal_connect(entry, "activate", G_CALLBACK(btn_decrypt_db), NULL);
    gtk_box_pack_start(GTK_BOX(hbox2), entry, TRUE, TRUE, 0);
    
    GtkWidget* button = gtk_button_new_with_label(db_decrypt_btn_open);
    g_signal_connect(button, "clicked", G_CALLBACK(btn_decrypt_db), NULL);
    gtk_box_pack_start(GTK_BOX(hbox2), button, FALSE, FALSE, 0);
    
    gtk_widget_show_all(window);
    gtk_main();
    return decrypt_success;
}

