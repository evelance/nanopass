#include <gtk/gtk.h>
#include "../config.hpp"
#include "../main_app.hpp"
using namespace std;

int main(int argc, char** argv)
{
    gtk_init(&argc, &argv);
    init_data inid;
    inid.argc = argc;
    inid.argv = argv;
    return main_app(inid);
}

