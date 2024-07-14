#pragma once

#include <vector>
#include <unordered_map>
#include <thread>
#include <immintrin.h>

class LifeChunk {
public:
    static const size_t Width = 512 + 2;
    static const size_t Height = 512 + 2;

    std::vector<uint8_t> currentField;
    std::vector<uint8_t> nextField;

private:
    static constexpr uint8_t lookup[32] = {
        0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0
    };

    static const __m256i v_lookup;

public:
    LifeChunk();

    bool Get(const int x, const int y) const;
    void Set(const int x, const int y, const bool value);

    bool CalculateNextField();
    void SwapFields();
};

class Universe {
public:
    inline static Universe* Instance(const int cellsCountInWidth, const int CellsCountInHeight, const bool cyclicity) {
        static Universe* instance = new Universe{cellsCountInWidth, CellsCountInHeight, cyclicity};

        return instance;
    }



private:
    struct PairHash {
        template <class T1, class T2> std::size_t operator()(const std::pair<T1, T2>& p) const {
            auto h1 = std::hash<T1> {}(p.first);
            auto h2 = std::hash<T2> {}(p.second);

            return h1 ^ h2;
        }
    };

    std::unordered_map<std::pair<int, int>, bool, PairHash> chunksToDelete;

public:
    std::unordered_map<std::pair<int, int>, LifeChunk, PairHash> chunks;

private:
    static constexpr int ThreadsCount = 6;

    std::vector<std::thread> threads;
    std::vector<std::vector<std::pair<int, int>>> argumentsList;
    std::vector<int> countList;

private:
    size_t CellsCountInWidth;
    size_t CellsCountInHeight;

    size_t ChunksCountInWidth;
    size_t ChunksCountInHeight;

    size_t WidthFromEdge;
    size_t HeightFromEdge;

    bool isCyclic;

private:
    Universe(const int cellsCountInWidth, const int cellsCountInHeight, const bool cyclicity);

    Universe(const Universe&) = delete;
    Universe(Universe&&) = delete;
    Universe& operator=(const Universe&) = delete;
    Universe& operator=(Universe&&) = delete;

public:
    void InitializeChunk(const int x, const int y);
    void DeleteDeadChunks();

    void InitializeNeighboringChunks();
    void ConsiderNeighboringChunks(const std::pair<int, int> coordinates);

    void ThreadFunction_Preparation(const std::vector<std::pair<int, int>>& arguments);
    void ThreadFunction_CalculateChunk(const std::vector<std::pair<int, int>>& arguments);

    void CalculateAllChunks();
    void CalculateNextGeneration();
};