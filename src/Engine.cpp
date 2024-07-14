#include <iostream>
#include <math.h>

#include "Engine.h"

Engine::Engine(int ScreenWidth, int ScreenHeight, int CellsWidth, int CellsHeight)
    : Screen_Width(ScreenWidth), Screen_Height(ScreenHeight), Number_of_Cells_in_Width(CellsWidth / 2), Number_of_Cells_in_Height(CellsHeight / 2) {
    Grid_Center_X = Screen_Width / 2 + Offset_from_Center_X;
    Grid_Center_Y = Screen_Height / 2 + Offset_from_Center_Y;

    Scaled_Grid_Width = Number_of_Cells_in_Width * (CELL_WIDTH + 1) * round(10 * Scale_Factor) / 10;
    Scaled_Grid_Height = Number_of_Cells_in_Height * (CELL_HEIGHT + 1) * round(10 * Scale_Factor) / 10;

    Scaled_Cell_Width = (CELL_WIDTH + 1) * round(10 * Scale_Factor) / 10;
    Scaled_Cell_Height = (CELL_HEIGHT + 1) * round(10 * Scale_Factor) / 10;

    Odd_Factor_X = round((CELL_WIDTH + 1) * (CellsWidth % 2) * Scale_Factor);
    Odd_Factor_Y = round((CELL_HEIGHT + 1) * (CellsHeight % 2) * Scale_Factor);
}

bool Engine::Quit_Event(SDL_Event event) {
    if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
        isRunning = false;
        return true;
    } else
        return false;
}

void Engine::Keys_Event(SDL_Event event) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_z)
        Grid_isVisible = !Grid_isVisible;

    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_x)
        Scope_isVisible = !Scope_isVisible;

    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_c)
        Full_Cell = !Full_Cell;

    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
        Generation_Pause2 = !Generation_Pause2;

    // if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_i)
    //     std::cout << universe->chunks.size() << ' ' << frameTime << "   " << Offset_from_Center_X / (CELL_WIDTH + 1)  << ' ' << Offset_from_Center_Y / (CELL_HEIGHT + 1) << '\n';
}

bool Engine::Scale_Event(SDL_Event event) {
    if (event.type == SDL_MOUSEWHEEL) {
        if (event.wheel.y > 0 && (round(10 * Scale_Factor) + 2) <= round(10 * MAX_SCALE)) {
            Scale_Factor += 0.2;

            Offset_from_Center_X = round(Offset_from_Center_X / (Scale_Factor - 0.2) * Scale_Factor);
            Offset_from_Center_Y = round(Offset_from_Center_Y / (Scale_Factor - 0.2) * Scale_Factor);

            Odd_Factor_X = round(Odd_Factor_X / (Scale_Factor - 0.2) * Scale_Factor);
            Odd_Factor_Y = round(Odd_Factor_Y / (Scale_Factor - 0.2) * Scale_Factor);
        } else if (event.wheel.y < 0 && (round(10 * Scale_Factor) - 2) >= round(10 * MIN_SCALE)) {
            Scale_Factor -= 0.2;

            Offset_from_Center_X = round(Offset_from_Center_X / (Scale_Factor + 0.2) * Scale_Factor);
            Offset_from_Center_Y = round(Offset_from_Center_Y / (Scale_Factor + 0.2) * Scale_Factor);

            Odd_Factor_X = round(Odd_Factor_X / (Scale_Factor + 0.2) * Scale_Factor);
            Odd_Factor_Y = round(Odd_Factor_Y / (Scale_Factor + 0.2) * Scale_Factor);
        }

        return true;
    }

    return false;
}

bool Engine::Offset_Event(SDL_Event event) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        isDragging_LMB = true;
        Generation_Pause1 = true;
    } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        isDragging_LMB = false;
        Generation_Pause1 = false;
    } else if (event.type == SDL_MOUSEMOTION && isDragging_LMB) {
        if (Number_of_Cells_in_Width * (CELL_WIDTH + 1) * round(10 * Scale_Factor) + 10 * Odd_Factor_X >=
            10 * (Offset_from_Center_X + event.motion.xrel)) {

            if (-1 * Number_of_Cells_in_Width * (CELL_WIDTH + 1) * round(10 * Scale_Factor) <= 10 * (Offset_from_Center_X + event.motion.xrel))
                Offset_from_Center_X += event.motion.xrel;
            else
                Offset_from_Center_X = -1 * Number_of_Cells_in_Width * (CELL_WIDTH + 1) * round(10 * Scale_Factor) / 10;

        } else
            Offset_from_Center_X = Number_of_Cells_in_Width * (CELL_WIDTH + 1) * round(10 * Scale_Factor) / 10 + Odd_Factor_X;

        if (Number_of_Cells_in_Height * (CELL_HEIGHT + 1) * round(10 * Scale_Factor) + 10 * Odd_Factor_Y >=
            10 * (Offset_from_Center_Y + event.motion.yrel)) {

            if (-1 * Number_of_Cells_in_Height * (CELL_HEIGHT + 1) * round(10 * Scale_Factor) <= 10 * (Offset_from_Center_Y + event.motion.yrel))
                Offset_from_Center_Y += event.motion.yrel;
            else
                Offset_from_Center_Y = -1 * Number_of_Cells_in_Height * (CELL_HEIGHT + 1) * round(10 * Scale_Factor) / 10;

        } else
            Offset_from_Center_Y = Number_of_Cells_in_Height * (CELL_HEIGHT + 1) * round(10 * Scale_Factor) / 10 + Odd_Factor_Y;

        return true;
    }

    return false;
}

std::pair<int, int> Engine::Change_Cell_Event(SDL_Event event) {
    int x, y;
    int X = -1, Y = -1;
    SDL_GetMouseState(&x, &y);

    x -= Screen_Width / 2 + Offset_from_Center_X - Odd_Factor_X;
    y -= Screen_Height / 2 + Offset_from_Center_Y - Odd_Factor_Y;

    x = round(static_cast<double>(x) / Scale_Factor);
    y = round(static_cast<double>(y) / Scale_Factor);

    if (Odd_Factor_X != 0 && Odd_Factor_Y != 0) {
        if (x / (CELL_WIDTH + 1) > Number_of_Cells_in_Width || x / (CELL_WIDTH + 1) <= -1 * Number_of_Cells_in_Width ||
            y / (CELL_HEIGHT + 1) > Number_of_Cells_in_Height || y / (CELL_HEIGHT + 1) <= -1 * Number_of_Cells_in_Height)
            ;
        else {
            if (x < 0)
                X = floor(static_cast<double>(x) / (CELL_WIDTH + 1)) + Number_of_Cells_in_Width;
            else
                X = x / (CELL_WIDTH + 1) + Number_of_Cells_in_Width;

            if (y < 0)
                Y = floor(static_cast<double>(y) / (CELL_HEIGHT + 1)) + Number_of_Cells_in_Height;
            else
                Y = y / (CELL_HEIGHT + 1) + Number_of_Cells_in_Height;
        }
    } else if (Odd_Factor_X != 0) {
        if (x / (CELL_WIDTH + 1) > Number_of_Cells_in_Width || x / (CELL_WIDTH + 1) <= -1 * Number_of_Cells_in_Width ||
            y / (CELL_HEIGHT + 1) >= Number_of_Cells_in_Height || y / (CELL_HEIGHT + 1) <= -1 * Number_of_Cells_in_Height)
            ;
        else {
            if (x < 0)
                X = floor(static_cast<double>(x) / (CELL_WIDTH + 1)) + Number_of_Cells_in_Width;
            else
                X = x / (CELL_WIDTH + 1) + Number_of_Cells_in_Width;

            if (y < 0)
                Y = floor(static_cast<double>(y) / (CELL_HEIGHT + 1)) + Number_of_Cells_in_Height;
            else
                Y = y / (CELL_HEIGHT + 1) + Number_of_Cells_in_Height;
        }
    } else if (Odd_Factor_Y != 0) {
        if (x / (CELL_WIDTH + 1) >= Number_of_Cells_in_Width || x / (CELL_WIDTH + 1) <= -1 * Number_of_Cells_in_Width ||
            y / (CELL_HEIGHT + 1) > Number_of_Cells_in_Height || y / (CELL_HEIGHT + 1) <= -1 * Number_of_Cells_in_Height)
            ;
        else {
            if (x < 0)
                X = floor(static_cast<double>(x) / (CELL_WIDTH + 1)) + Number_of_Cells_in_Width;
            else
                X = x / (CELL_WIDTH + 1) + Number_of_Cells_in_Width;

            if (y < 0)
                Y = floor(static_cast<double>(y) / (CELL_HEIGHT + 1)) + Number_of_Cells_in_Height;
            else
                Y = y / (CELL_HEIGHT + 1) + Number_of_Cells_in_Height;
        }
    } else {
        if (x / (CELL_WIDTH + 1) >= Number_of_Cells_in_Width || x / (CELL_WIDTH + 1) <= -1 * Number_of_Cells_in_Width ||
            y / (CELL_HEIGHT + 1) >= Number_of_Cells_in_Height || y / (CELL_HEIGHT + 1) <= -1 * Number_of_Cells_in_Height)
            ;
        else {
            if (x < 0)
                X = floor(static_cast<double>(x) / (CELL_WIDTH + 1)) + Number_of_Cells_in_Width;
            else
                X = x / (CELL_WIDTH + 1) + Number_of_Cells_in_Width;

            if (y < 0)
                Y = floor(static_cast<double>(y) / (CELL_HEIGHT + 1)) + Number_of_Cells_in_Height;
            else
                Y = y / (CELL_HEIGHT + 1) + Number_of_Cells_in_Height;
        }
    }

    return std::make_pair(X, Y);
}

void Engine::Event_Handler() {
    frameDelay = 3000 / FPS;

    while (SDL_PollEvent(&event)) {
        if (Quit_Event(event))
            break;

        Keys_Event(event);

        if (Offset_Event(event) || Scale_Event(event)) {
            frameDelay = 1500 / FPS;

            Grid_Center_X = Screen_Width / 2 + Offset_from_Center_X;
            Grid_Center_Y = Screen_Height / 2 + Offset_from_Center_Y;

            Scaled_Grid_Width = Number_of_Cells_in_Width * (CELL_WIDTH + 1) * round(10 * Scale_Factor) / 10;
            Scaled_Grid_Height = Number_of_Cells_in_Height * (CELL_HEIGHT + 1) * round(10 * Scale_Factor) / 10;

            Scaled_Cell_Width = (CELL_WIDTH + 1) * round(10 * Scale_Factor) / 10;
            Scaled_Cell_Height = (CELL_HEIGHT + 1) * round(10 * Scale_Factor) / 10;
        }

        if (Generation_Pause2) {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
                isDragging_RMB = true;
                Current_Cell = std::make_pair(-1, -1);
            } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT)
                isDragging_RMB = false;

            if (isDragging_RMB) {
                std::pair<int, int> Cell = Change_Cell_Event(event);

                //std::cout << Cell.first << ' ' << Cell.second << '\n';

                if (Cell != std::make_pair(-1, -1) && Cell != Current_Cell) {
                    if (universe->chunks.find({Cell.first / (LifeChunk::Width - 2), Cell.second / (LifeChunk::Height - 2)}) == universe->chunks.end()) {
                        universe->InitializeChunk(Cell.first / (LifeChunk::Width - 2), Cell.second / (LifeChunk::Height - 2));
                    }

                    LifeChunk& chunk = universe->chunks[{Cell.first / (LifeChunk::Width - 2), Cell.second / (LifeChunk::Height - 2)}];

                    if (chunk.Get(Cell.first % (LifeChunk::Width - 2) + 1, Cell.second % (LifeChunk::Height - 2) + 1)) {
                        chunk.Set(Cell.first % (LifeChunk::Width - 2) + 1, Cell.second % (LifeChunk::Height - 2) + 1, false);
                    } else {
                        chunk.Set(Cell.first % (LifeChunk::Width - 2) + 1, Cell.second % (LifeChunk::Height - 2) + 1, true);
                    }

                    Current_Cell = Cell;
                }
            }
        }
    }
}

void Engine::Draw_Grid(SDL_Renderer* Renderer) {
    int Alpha = 255;

    int mn1 = std::min(Screen_Width, Grid_Center_X + Scaled_Grid_Width);
    int mx1 = std::max(0, Grid_Center_X - Scaled_Grid_Width - Odd_Factor_X);

    int Y1 = std::max(0, Grid_Center_Y - Scaled_Grid_Height - Odd_Factor_Y);
    int Y2 = std::min(Screen_Height, Grid_Center_Y + Scaled_Grid_Height);
    int x = std::floor((mn1 - Grid_Center_X) / Scaled_Cell_Width);
    for (int t = Grid_Center_X + x * Scaled_Cell_Width; t >= mx1; x -= 1, t -= Scaled_Cell_Width) {

        if (x % BRIGHT_LINE_EVERY_N_LINES)
            Alpha = 100;
        else
            Alpha = 255;

        SDL_SetRenderDrawColor(Renderer, Grid_RGB.r, Grid_RGB.g, Grid_RGB.b, Alpha);
        SDL_RenderDrawLine(Renderer, t, Y1, t, Y2);
    }

    int mn2 = std::min(Screen_Height, Grid_Center_Y + Scaled_Grid_Height);
    int mx2 = std::max(0, Grid_Center_Y - Scaled_Grid_Height - Odd_Factor_Y);

    int X1 = std::max(0, Grid_Center_X - Scaled_Grid_Width - Odd_Factor_X);
    int X2 = std::min(Screen_Width, Grid_Center_X + Scaled_Grid_Width);
    int y = std::floor((mn2 - Grid_Center_Y) / Scaled_Cell_Height);
    for (int t = Grid_Center_Y + y * Scaled_Cell_Height; t >= mx2; y -= 1, t -= Scaled_Cell_Height) {

        if (y % BRIGHT_LINE_EVERY_N_LINES)
            Alpha = 100;
        else
            Alpha = 255;

        SDL_SetRenderDrawColor(Renderer, Grid_RGB.r, Grid_RGB.g, Grid_RGB.b, Alpha);
        SDL_RenderDrawLine(Renderer, X1, t, X2, t);
    }
}

void Engine::Draw_Cells(SDL_Renderer* Renderer) {
    if (Generation_Pause2)
        SDL_SetRenderDrawColor(Renderer, Frozen_Cell_RGBA.r, Frozen_Cell_RGBA.g, Frozen_Cell_RGBA.b, Frozen_Cell_RGBA.a);
    else
        SDL_SetRenderDrawColor(Renderer, Cell_RGBA.r, Cell_RGBA.g, Cell_RGBA.b, Cell_RGBA.a);

    for (int x = (-Grid_Center_X + Scaled_Grid_Width + Odd_Factor_X) / Scaled_Cell_Width - 1; x <= (Screen_Width - Grid_Center_X + Scaled_Grid_Width + Odd_Factor_X) / Scaled_Cell_Width - 1 + 1; ++x) {
        for (int y = (-Grid_Center_Y + Scaled_Grid_Height + Odd_Factor_Y) / Scaled_Cell_Height - 1; y <= (Screen_Height - Grid_Center_Y + Scaled_Grid_Height + Odd_Factor_Y) / Scaled_Cell_Height - 1 + 1; ++y) {
            
            if (universe->chunks.find({x / (LifeChunk::Width - 2), y / (LifeChunk::Height - 2)}) != universe->chunks.end()) {
                const LifeChunk& chunk = universe->chunks[{x / (LifeChunk::Width - 2), y / (LifeChunk::Height - 2)}];

                if (chunk.Get(x % (LifeChunk::Width - 2) + 1, y % (LifeChunk::Height - 2) + 1)) {
                    SDL_Rect cell;
                    int X = Grid_Center_X - Scaled_Grid_Width - Odd_Factor_X + (x + 1) * Scaled_Cell_Width,
                        Y = Grid_Center_Y - Scaled_Grid_Height - Odd_Factor_Y + (y + 1) * Scaled_Cell_Height;

                    if (Full_Cell)
                        cell = {X - Scaled_Cell_Width, Y - Scaled_Cell_Height, Scaled_Cell_Width + 1, Scaled_Cell_Height + 1};
                    else
                        cell = {X - Scaled_Cell_Width + 1, Y - Scaled_Cell_Height + 1, Scaled_Cell_Width - 1, Scaled_Cell_Height - 1};

                    SDL_RenderFillRect(Renderer, &cell);
                }
            }

        }
    }
}

void Engine::Draw_Scope(SDL_Renderer* Renderer) {
    SDL_SetRenderDrawColor(Renderer, Scope_RGBA.r, Scope_RGBA.g, Scope_RGBA.b, Scope_RGBA.a);
    SDL_RenderDrawLine(Renderer, Screen_Width / 2 - SCOPE_WIDTH, Screen_Height / 2, Screen_Width / 2 + SCOPE_WIDTH, Screen_Height / 2);
    SDL_RenderDrawLine(Renderer, Screen_Width / 2, Screen_Height / 2 - SCOPE_HEIGHT, Screen_Width / 2, Screen_Height / 2 + SCOPE_HEIGHT);
}
