/*
    OS-independent main file.
    Contains top-level logic.
    The init_data struct looks different depending on the build platform.
*/
#ifndef __MAIN_APP_HPP
#define __MAIN_APP_HPP
#include "config.hpp"

int main_app(struct init_data inid);

#endif