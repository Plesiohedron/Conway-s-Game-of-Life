#pragma once

#include <iostream>
#include <unordered_map>

struct pair_hash {
    template <class T1, class T2> std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        return h1 ^ h2;
    }
};

class LifeEngine {
private:
    int Number_of_Cells_in_Width = 2, Number_of_Cells_in_Height = 2;

    bool isCyclic = false;

public:
    std::unordered_map<std::pair<int, int>, bool, pair_hash> Universe;

    LifeEngine(int FieldWidth, int FieldHeight, bool Cyclicity);

    std::unordered_map<std::pair<int, int>, bool, pair_hash> Calculate_Next_Generation();
    int Count_Neighbors(int x, int y);
};