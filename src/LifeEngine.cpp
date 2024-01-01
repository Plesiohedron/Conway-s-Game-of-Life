#include "LifeEngine.h"

LifeEngine::LifeEngine(int FieldWidth, int FieldHeight) : Number_of_Cells_in_Width(FieldWidth), Number_of_Cells_in_Height(FieldHeight) {
    // std::pair<int, int> coordinates[] = {
    //     {3, 20}, {3, 21}, {3, 22}, {2, 22}, {1, 21},
    //     // {14, 11}, {15, 11}, {14, 12}, {13, 13}, {14, 13},
    //     // {13, 14}, {13, 15}, {14, 15}, {15, 16}, {16, 15},
    //     // {21, 15}, {17, 14}, {19, 14}, {20, 14}, {21, 14},
    //     // {17, 13}, {18, 13}, {19, 13}, {17, 16}, {18, 17},
    //     // {19, 17}, {19, 18}, {18, 19}, {19, 19}, {18, 20},
    //     // {17, 21}, {18, 21}, {11, 17}, {16, 17}, {11, 18},
    //     // {12, 18}, {13, 18}, {15, 18}, {13, 19}, {14, 19},
    //     // {15, 19}
    // };

    // for (const auto& coordinate : coordinates) {
    //     Universe[coordinate] = true;
    // }
}

std::unordered_map<std::pair<int, int>, bool, pair_hash> LifeEngine::Calculate_Next_Generation() {
    std::unordered_map<std::pair<int, int>, bool, pair_hash> Auxiliary_Universe = Universe;

    for (const auto& cell : Universe) {
        int x = cell.first.first;
        int y = cell.first.second;
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                int new_x = (x + i + Number_of_Cells_in_Width) % Number_of_Cells_in_Width;
                int new_y = (y + j + Number_of_Cells_in_Height) % Number_of_Cells_in_Height;
                if (!Auxiliary_Universe.count({new_x, new_y})) {
                    int neighbors = Count_Neighbors(new_x, new_y);
                    if (neighbors == 3) {
                        Auxiliary_Universe[{new_x, new_y}] = true;
                    }
                }
            }
        }
    }

    for (auto it = Auxiliary_Universe.begin(); it != Auxiliary_Universe.end();) {
        int x = it->first.first;
        int y = it->first.second;
        int neighbors = Count_Neighbors(x, y);
        if (Universe.count({x, y}) && Universe.at({x, y}) && (neighbors < 2 || neighbors > 3)) {
            it = Auxiliary_Universe.erase(it);
        } else {
            ++it;
        }
    }

    return Auxiliary_Universe;
}

int LifeEngine::Count_Neighbors(int x, int y) {
    int neighbors = 0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0)
                continue;

            if (Universe.count(
                    {(x + i + Number_of_Cells_in_Width) % Number_of_Cells_in_Width, (y + j + Number_of_Cells_in_Height) % Number_of_Cells_in_Height}))
                neighbors += 1;
        }
    }
    return neighbors;
}
