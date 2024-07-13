#include <iostream>

#include "Window.h"

int main(int argc, char* argv[]) {

    int Width_of_Field = 2, Height_of_Field = 2;
    bool isCyclic = false;
    std::string s = "huh?";

    std::cout << "Input the width of the field: ";
    std::cin >> Width_of_Field;

    std::cout << "Input the height of the field: ";
    std::cin >> Height_of_Field;

    std::cout << "The cyclicity of the field: ";
    std::cin >> s;

    if (s == "True")
        isCyclic = true;

    if (Width_of_Field < 2)
        Width_of_Field = 2;
    else if (Width_of_Field > 10'000'000)
        Width_of_Field = 10'000'000;
    
    if (Height_of_Field < 2)
        Height_of_Field = 2;
    else if (Height_of_Field > 10'000'000)
        Height_of_Field = 10'000'000;
    


    try {
        Window Window("Window", 1280, 720);
        Engine Engine(1280, 720, Width_of_Field, Height_of_Field);
        Engine.universe = Universe::Instance(Width_of_Field, Height_of_Field, isCyclic);
        
        Window.MainLoop(Engine);
    } catch (std::runtime_error& error) {
        std::cout << "Fatal Error" << std::endl;
        std::cout << error.what() << std::endl;
        return -1;
    }

    return 0;
}