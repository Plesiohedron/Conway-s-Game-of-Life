#pragma once

#include <../include/sdl/SDL.h>
#include <string>

#include "Engine.h"

class Window {
private:
    SDL_Window* mWindow = NULL;
    SDL_Renderer* mRenderer = NULL;

public:
    Window(const std::string& Title, int ScreenWidth, int ScreenHeight);
    ~Window();

    void MainLoop(Engine Engine, LifeEngine Life);
};
