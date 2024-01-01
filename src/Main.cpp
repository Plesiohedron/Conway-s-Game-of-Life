#include <iostream>

#include "Window.h"

int main(int argc, char* argv[]) {

    int Width_of_Field, Height_of_Field;

    std::cout << "Input the width of the grid: ";
    std::cin >> Width_of_Field;

    std::cout << "Input the height of the grid: ";
    std::cin >> Height_of_Field;

    try {
        Window Window("Window", 1280, 720);
        Engine Engine(1280, 720, Width_of_Field, Height_of_Field);
        LifeEngine Life(Width_of_Field, Height_of_Field);
        Window.MainLoop(Engine, Life);
    } catch (std::runtime_error& error) {
        std::cout << "Fatal Error" << std::endl;
        std::cout << error.what() << std::endl;
        return -1;
    }

    return 0;
}