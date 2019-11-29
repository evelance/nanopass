#include <string>
#include <string.h>
#include <gtk/gtk.h>
#include "message.hpp"
#include "../db.hpp"
#include "../gui.hpp"
#include "../config.hpp"
#include "../resource/key.hpp"
#include "../resource/database.cpp"

const size_t MARGIN = 8;
static GtkWidget* window = NULL;
static GtkWidget* entry = NULL;

static void btn_create_db(GtkWidget* widget, gpointer data)
{
    std::string masterpw(gtk_entry_get_text(GTK_ENTRY(entry)));
    if (masterpw.empty()) {
        show_modal_message(GTK_WINDOW(window), msg_masterpwempty_title, msg_masterpwempty_text);
        return;
    }
    db database;
    database.set_plaintext(db_initial_text, strlen(db_initial_text));
    database.set_password(masterpw.c_str(), masterpw.length());
    if (database.write(db_filename) == db::OK) {
        show_modal_message(GTK_WINDOW(window), msg_dbcreated_title, msg_dbcreated_text);
        gtk_widget_destroy(window);
    } else {
        show_modal_message(GTK_WINDOW(window), msg_dbnotcreated_title, database.get_error());
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

void init_gui_create_db(init_data inid) { }

void show_gui_create_db(init_data inid)
{
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), create_db_window_width, create_db_window_height);
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
    GtkWidget* title = gtk_label_new(db_create_title);
    // gtk_label_set_xalign(GTK_LABEL(title), 0.0); // Looks better but needs GTK+3.16
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_label_set_justify(GTK_LABEL(title), GTK_JUSTIFY_LEFT);
    gtk_label_set_ellipsize(GTK_LABEL(title), PANGO_ELLIPSIZE_END);
    PangoAttrList* pal = pango_attr_list_new();
    pango_attr_list_insert(pal, pango_attr_size_new(15 * PANGO_SCALE));
    gtk_label_set_attributes(GTK_LABEL(title), pal);
    pango_attr_list_unref(pal);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);
    
    // Image + Text
    GtkWidget* hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), hbox1, TRUE, TRUE, 0);
    
    loader = gdk_pixbuf_loader_new();
    gdk_pixbuf_loader_write(loader, database_png, database_png_len, NULL);
    gdk_pixbuf_loader_close(loader, NULL);
    GtkWidget* image = gtk_image_new_from_pixbuf(gdk_pixbuf_loader_get_pixbuf(loader));
    g_object_unref(loader);
    gtk_widget_set_halign(image, GTK_ALIGN_START);
    gtk_widget_set_valign(image, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(hbox1), image, FALSE, FALSE, 0);
    
    GtkWidget* text = gtk_label_new(db_create_descn);
    // gtk_label_set_xalign(GTK_LABEL(text), 0.0); // Looks better but needs GTK+3.16
    // gtk_label_set_yalign(GTK_LABEL(text), 0.0);
    gtk_widget_set_halign(text, GTK_ALIGN_START);
    gtk_widget_set_valign(text, GTK_ALIGN_START);
    gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
    gtk_label_set_line_wrap(GTK_LABEL(text), TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(text), PANGO_ELLIPSIZE_END);
    gtk_label_set_lines(GTK_LABEL(text), 4);
    gtk_box_pack_start(GTK_BOX(hbox1), text, TRUE, TRUE, 0);
    
    // Separater
    GtkWidget* line = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 0);
    
    // Password input + Button
    GtkWidget* hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);
    
    entry = gtk_entry_new();
    g_signal_connect(entry, "activate", G_CALLBACK(btn_create_db), NULL);
    gtk_box_pack_start(GTK_BOX(hbox2), entry, TRUE, TRUE, 0);
    
    GtkWidget* button = gtk_button_new_with_label(db_create_btn_create);
    g_signal_connect(button, "clicked", G_CALLBACK(btn_create_db), NULL);
    gtk_box_pack_start(GTK_BOX(hbox2), button, FALSE, FALSE, 0);
    
    gtk_widget_show_all(window);
    gtk_main();
}

