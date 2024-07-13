#include <iostream>
#include <sstream>

#include "Window.h"

Window::Window(const std::string& Title, const int ScreenWidth, const int ScreenHeight) {

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::ostringstream Error;
        Error << "Can't initialize SDL: " << SDL_GetError();
        throw std::runtime_error(Error.str());
    }

    mWindow = SDL_CreateWindow(Title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ScreenWidth, ScreenHeight, 0);
    if (mWindow == NULL) {
        std::ostringstream Error;
        Error << "Can't create window: " << SDL_GetError();
        throw std::runtime_error(Error.str());
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
    if (mRenderer == NULL) {
        std::ostringstream Error;
        Error << "Can't create renderer: " << SDL_GetError();
        throw std::runtime_error(Error.str());
    }
}

Window::~Window() {
    SDL_DestroyRenderer(mRenderer);
    mRenderer = NULL;

    SDL_DestroyWindow(mWindow);
    mWindow = NULL;

    SDL_Quit();
}

void Window::MainLoop(Engine& Engine) {

    SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);

    while (Engine.isRunning) {
        Engine.frameStart = SDL_GetTicks();

        Engine.Event_Handler();

        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 0);
        SDL_RenderClear(mRenderer);

        if (Engine.Grid_isVisible)
            Engine.Draw_Grid(mRenderer);

        Engine.Draw_Cells(mRenderer);

        if (Engine.Scope_isVisible)
            Engine.Draw_Scope(mRenderer);

        SDL_RenderPresent(mRenderer);

        if (!Engine.Generation_Pause1 && !Engine.Generation_Pause2)
            Engine.universe->CalculateNextGeneration();

        Engine.frameTime = SDL_GetTicks() - Engine.frameStart;
        if (Engine.frameDelay > Engine.frameTime)
             SDL_Delay(Engine.frameDelay - Engine.frameTime);
    }
}
