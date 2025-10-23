#include <iostream>
#include "datactl.h"

#undef main

extern int ffplay_main(int argc, char **argv);

int main(int argc, char **argv)
{
    return ffplay_main(argc, argv);
}
