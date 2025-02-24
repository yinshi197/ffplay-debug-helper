#include <iostream>
#include "datactl.h"

#undef main
int main2(int, char**)
{
    av_log_set_level(AV_LOG_DEBUG);
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Log("SDL Log Test");

    av_log(nullptr, AV_LOG_DEBUG, "FFmpeg Log Test\n");

    std::cout << "Hello, from ffplay!\n";

    return 0;
}
