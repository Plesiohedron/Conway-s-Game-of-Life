#pragma once

#include <../include/sdl/SDL.h>

#include "Universe.h"

class Engine {
private:
    const int FPS = 75;

    const int CELL_WIDTH = 4;
    const int CELL_HEIGHT = 4;

    const int SCOPE_WIDTH = 10;
    const int SCOPE_HEIGHT = 10;

    const int BRIGHT_LINE_EVERY_N_LINES = 10;

    const double MIN_SCALE = 1.0, MAIN_SCALE = 3.0, MAX_SCALE = 5.0;

    const SDL_Color Scope_RGBA = {255, 255, 255, 255};
    const SDL_Color Cell_RGBA = {255, 255, 255, 255};
    const SDL_Color Frozen_Cell_RGBA = {0, 84, 166, 255};
    const SDL_Color Grid_RGB = {150, 150, 150};

    int Screen_Width = 0;
    int Screen_Height = 0;

    int Grid_Center_X, Grid_Center_Y;
    int Scaled_Grid_Width, Scaled_Grid_Height;
    int Scaled_Cell_Width, Scaled_Cell_Height;

    double Offset_from_Center_X = 0, Offset_from_Center_Y = 0;
    int Odd_Factor_X = 0, Odd_Factor_Y = 0;

    double Scale_Factor = MAIN_SCALE;

    bool isDragging_LMB = false;
    bool isDragging_RMB = false;

    std::pair<int, int> Current_Cell;

private:
    SDL_Event event;

    bool Quit_Event(SDL_Event event);
    void Keys_Event(SDL_Event event);

    bool Scale_Event(SDL_Event event);
    bool Offset_Event(SDL_Event event);
    std::pair<int, int> Change_Cell_Event(SDL_Event event);

public:
    Uint32 frameStart;
    Uint32 frameTime;

    bool isRunning = true;
    bool Grid_isVisible = true;
    bool Scope_isVisible = true;

    bool Generation_Pause1 = false;
    bool Generation_Pause2 = true;

    bool Full_Cell = false;

    int frameDelay = 3000 / FPS;
    int Number_of_Cells_in_Width = 0, Number_of_Cells_in_Height = 0;

    Universe* universe;

    Engine(int ScreenWidth, int ScreenHeight, int CellsWidth, int CellsHeight);

    void Event_Handler();

    void Draw_Grid(SDL_Renderer* Renderer);
    void Draw_Cells(SDL_Renderer* Renderer);
    void Draw_Scope(SDL_Renderer* Renderer);
};