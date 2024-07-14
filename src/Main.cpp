#include <iostream>
#include <sstream>

#include "Window.h"

int main(int argc, char* argv[]) { 

    try {
        std::istringstream iss;
        int Width_of_Field, Height_of_Field;
        bool isCyclic;

        if (argc > 4) {
            throw std::runtime_error{"Wrong input: too much arguments!"};
        } else if (argc < 4) {
            throw std::runtime_error{"Wrong input: too few arguments!"};
        }

        iss = std::istringstream{argv[1]};
        iss >> std::noskipws >> Width_of_Field;
        if (!iss.eof() || iss.fail()) {
            throw std::runtime_error{"Wrong crop width input!"};
        } else if (Width_of_Field < 2) {
            throw std::runtime_error{"Wrong field width input: the value is too small!\n Acceptable values are integers from 2 to 10.000.000"};
        } else if (Width_of_Field > 10'000'000) {
            throw std::runtime_error{"Wrong field width input: the value is too large!\n Acceptable values are integers from 2 to 10.000.000"};
        }

        iss = std::istringstream{argv[2]};
        iss >> std::noskipws >> Height_of_Field;
        if (!iss.eof() || iss.fail()) {
            throw std::runtime_error{"Wrong crop height input!"};
        } else if (Height_of_Field < 2) {
            throw std::runtime_error{"Wrong field height input: the value is too small!\n Acceptable values are integers from 2 to 10.000.000"};
        } else if (Height_of_Field > 10'000'000) {
            throw std::runtime_error{"Wrong field height input: the value is too large!\n Acceptable values are integers from 2 to 10.000.000"};
        }

        std::string str;
        iss = std::istringstream{argv[3]};
        iss >> std::noskipws >> str;
        if (!iss.eof() || iss.fail()) {
            throw std::runtime_error{"Wrong boolean input!"};
        } else if (str == "true") {
            isCyclic = true;
        } else if (str == "false") {
            isCyclic = false;
        } else {
            throw std::runtime_error{"Wrong boolean input!"};
        }

        Window Window("Window", 1280, 720);
        Engine Engine(1280, 720, Width_of_Field, Height_of_Field);
        Engine.universe = Universe::Instance(Width_of_Field, Height_of_Field, isCyclic);
        
        Window.MainLoop(Engine);
    } catch (std::runtime_error& error) {
        std::cerr << "Fatal Error\n" << error.what() << std::endl;
        return -1;
    }

    return 0;
}