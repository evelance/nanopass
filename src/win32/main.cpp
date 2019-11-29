#define _UNICODE
#define UNICODE
#include <windows.h>
#include "../config.hpp"
#include "../main_app.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    init_data inid;
    inid.hInstance = hInstance;
    inid.nCmdShow = nCmdShow;
    return main_app(inid);
}

