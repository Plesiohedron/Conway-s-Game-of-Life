#include <Universe.h>

const __m256i LifeChunk::v_lookup = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(LifeChunk::lookup));

LifeChunk::LifeChunk(): currentField(2 * Width + Width * Height, 0), nextField(2 * Width + Width * Height, 0) { }

bool LifeChunk::Get(int x, int y) const {
    return currentField[LifeChunk::Width + y * LifeChunk::Width + x] == 1;
}

void LifeChunk::Set(int x, int y, bool value) {
    currentField[LifeChunk::Width + y * LifeChunk::Width + x] = static_cast<uint8_t>(value ? 1 : 0);
}

bool LifeChunk::CalculateNextField() {
    bool isLivingChunk = false;

    for (size_t i = 2 * LifeChunk::Width; i < std::size(currentField) - 2 * LifeChunk::Width; i += 32) {
        __m256i topLeft = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&currentField[i - LifeChunk::Width - 1]));
        __m256i top = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&currentField[i - LifeChunk::Width]));
        __m256i topRight = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&currentField[i - LifeChunk::Width + 1]));
        __m256i left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&currentField[i - 1]));
        __m256i right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&currentField[i + 1]));
        __m256i bottomLeft = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&currentField[i + LifeChunk::Width - 1]));
        __m256i bottom = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&currentField[i + LifeChunk::Width]));
        __m256i bottomRight = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&currentField[i + LifeChunk::Width + 1]));

        __m256i sum1 = _mm256_add_epi8(topLeft, top);
        __m256i sum2 = _mm256_add_epi8(topRight, left);
        __m256i sum3 = _mm256_add_epi8(right, bottomLeft);
        __m256i sum4 = _mm256_add_epi8(bottom, bottomRight);
        __m256i sum5 = _mm256_add_epi8(sum1, sum2);
        __m256i sum6 = _mm256_add_epi8(sum3, sum4);

        __m256i neighbours = _mm256_add_epi8(sum5, sum6);
        __m256i alive = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&currentField[i]));

        alive = _mm256_slli_epi64(alive, 3);
        __m256i mask = _mm256_or_si256(alive, neighbours);
        __m256i shouldBeAlive = _mm256_shuffle_epi8(v_lookup, mask);


        _mm256_storeu_si256(reinterpret_cast<__m256i*>(&nextField[i]), shouldBeAlive);

        if (!_mm256_testz_si256(shouldBeAlive, shouldBeAlive)) {
            isLivingChunk = true;
        }
    }

    return isLivingChunk;
}

void LifeChunk::SwapFields() {
    std::swap(currentField, nextField);

    for (size_t y = 1; y < LifeChunk::Height - 1; ++y) {
        currentField[LifeChunk::Width + y * LifeChunk::Width] = 0;
        currentField[LifeChunk::Width + y * LifeChunk::Width + (LifeChunk::Width - 1)] = 0;
    }

    for (size_t x = 1; x < LifeChunk::Width - 1; ++x) {
        currentField[LifeChunk::Width + x] = 0;
        currentField[LifeChunk::Width + (LifeChunk::Height - 1) * LifeChunk::Width + x] = 0;
    }
}

Universe::Universe(const int cellsCountInWidth, const int cellsCountInHeight, const bool cyclicity): 
    CellsCountInWidth(cellsCountInWidth), CellsCountInHeight(cellsCountInHeight), isCyclic(cyclicity), argumentsList(ThreadsCount), countList(ThreadsCount) {
    ChunksCountInWidth = (CellsCountInWidth + LifeChunk::Width - 2 - 1) / (LifeChunk::Width - 2);
    ChunksCountInHeight = (CellsCountInHeight + LifeChunk::Height - 2 - 1) / (LifeChunk::Height - 2);

    WidthFromEdge = CellsCountInWidth - (ChunksCountInWidth - 1) * (LifeChunk::Width - 2);
    HeightFromEdge = CellsCountInHeight - (ChunksCountInHeight - 1) * (LifeChunk::Height - 2);
}


void Universe::InitializeChunk(int x, int y) {
    chunks.insert({{x, y}, LifeChunk()});
    chunksToDelete[{x, y}] = false;
}

void Universe::DeleteDeadChunks() {
    for (auto it = chunksToDelete.begin(); it != chunksToDelete.end(); ) {
        if (!it->second) {
            chunks.erase(it->first);
            it = chunksToDelete.erase(it);
        } else {
            ++it;
        }
    }
}

void Universe::InitializeNeighboringChunks() {
    std::unordered_map<std::pair<int, int>, bool, PairHash> chunksToInitialize;

    for (const auto& element : chunks) {
        std::pair<int, int> chunkCoordinates = element.first;
        const LifeChunk& chunk = element.second;

        if (chunkCoordinates.second + 1 < ChunksCountInHeight) {
            if (chunks.find(std::pair<int, int>(chunkCoordinates.first, chunkCoordinates.second + 1)) == chunks.end()) {
                for (int x = 1; x < LifeChunk::Width - 1; ++x) {
                    if (chunk.Get(x, LifeChunk::Height - 2)) {
                        chunksToInitialize[{chunkCoordinates.first, chunkCoordinates.second + 1}] = true;
                        break;
                    }
                }
            }
        } else if (isCyclic) {
            if (chunks.find(std::pair<int, int>(chunkCoordinates.first, 0)) == chunks.end()) {
                for (int x = 1; x < LifeChunk::Width - 1; ++x) {
                    if (chunk.Get(x, HeightFromEdge)) {
                        chunksToInitialize[{chunkCoordinates.first, 0}] = true;
                        break;
                    }
                }
            }
        }

        if (chunkCoordinates.second - 1 >= 0) {
            if (chunks.find(std::pair<int, int>(chunkCoordinates.first, chunkCoordinates.second - 1)) == chunks.end()) {
                for (int x = 1; x < LifeChunk::Width - 1; ++x) {
                    if (chunk.Get(x, 1)) {
                        chunksToInitialize[{chunkCoordinates.first, chunkCoordinates.second - 1}] = true;
                        break;
                    }
                }
            }
        } else if (isCyclic) {
            if (chunks.find(std::pair<int, int>(chunkCoordinates.first, ChunksCountInHeight - 1)) == chunks.end()) {
                for (int x = 1; x < LifeChunk::Width - 1; ++x) {
                    if (chunk.Get(x, 1)) {
                        chunksToInitialize[{chunkCoordinates.first, ChunksCountInHeight - 1}] = true;
                        break;
                    }
                }
            }
        }

        if (chunkCoordinates.first + 1 < ChunksCountInWidth) {
            if (chunks.find(std::pair<int, int>(chunkCoordinates.first + 1, chunkCoordinates.second)) == chunks.end()) {
                for (int y = 1; y < LifeChunk::Height - 1; ++y) {
                    if (chunk.Get(LifeChunk::Width - 2, y)) {
                        chunksToInitialize[{chunkCoordinates.first + 1, chunkCoordinates.second}] = true;
                        break;
                    }
                }
            }
        } else if (isCyclic) {
            if (chunks.find(std::pair<int, int>(0, chunkCoordinates.second)) == chunks.end()) {
                for (int y = 1; y < LifeChunk::Height - 1; ++y) {
                    if (chunk.Get(WidthFromEdge, y)) {
                        chunksToInitialize[{0, chunkCoordinates.second}] = true;
                        break;
                    }
                }
            }
        }

        if (chunkCoordinates.first - 1 >= 0) {
            if (chunks.find(std::pair<int, int>(chunkCoordinates.first - 1, chunkCoordinates.second)) == chunks.end()) {
                for (int y = 1; y < LifeChunk::Height - 1; ++y) {
                    if (chunk.Get(1, y)) {
                        chunksToInitialize[{chunkCoordinates.first - 1, chunkCoordinates.second}] = true;
                        break;
                    }
                }
            }
        } else if (isCyclic) {
            if (chunks.find(std::pair<int, int>(ChunksCountInWidth - 1, chunkCoordinates.second)) == chunks.end()) {
                for (int y = 1; y < LifeChunk::Height - 1; ++y) {
                    if (chunk.Get(1, y)) {
                        chunksToInitialize[{ChunksCountInWidth - 1, chunkCoordinates.second}] = true;
                        break;
                    }
                }
            }
        }


        if (chunkCoordinates.first + 1 < ChunksCountInWidth && chunkCoordinates.second + 1 < ChunksCountInHeight) {
            if (chunks.find(std::pair<int, int>(chunkCoordinates.first + 1, chunkCoordinates.second + 1)) == chunks.end()) {
                if (chunk.Get(LifeChunk::Width - 2, LifeChunk::Height - 2)) {
                    chunksToInitialize[{chunkCoordinates.first + 1, chunkCoordinates.second + 1}] = true;
                }
            }
        } else if (isCyclic) {
            std::pair<int, int> coords = {chunkCoordinates.first + 1 < ChunksCountInWidth ? chunkCoordinates.first + 1 : 0, chunkCoordinates.second + 1 < ChunksCountInHeight ? chunkCoordinates.second + 1 : 0};
            std::pair<int, int> get = {chunkCoordinates.first + 1 < ChunksCountInWidth ? LifeChunk::Width - 2 : WidthFromEdge, chunkCoordinates.second + 1 < ChunksCountInHeight ? LifeChunk::Height - 2 : HeightFromEdge};

            if (chunks.find(coords) == chunks.end()) {
                if (chunk.Get(get.first, get.second)) {
                    chunksToInitialize[coords] = true;
                }
            }
        }

        if (chunkCoordinates.first - 1 >= 0 && chunkCoordinates.second - 1 >= 0) {
            if (chunks.find(std::pair<int, int>(chunkCoordinates.first - 1, chunkCoordinates.second - 1)) == chunks.end()) {
                if (chunk.Get(1, 1)) {
                    chunksToInitialize[{chunkCoordinates.first - 1, chunkCoordinates.second - 1}] = true;
                }
            }
        } else if (isCyclic) {
            std::pair<int, int> coords = {chunkCoordinates.first - 1 >= 0 ? chunkCoordinates.first - 1 : ChunksCountInWidth - 1, chunkCoordinates.second - 1 >= 0 ? chunkCoordinates.second - 1 : ChunksCountInHeight - 1};

            if (chunks.find(coords) == chunks.end()) {
                if (chunk.Get(1, 1)) {
                    chunksToInitialize[coords] = true;
                }
            }
        }

        if (chunkCoordinates.first + 1 < ChunksCountInWidth && chunkCoordinates.second - 1 >= 0) {
            if (chunks.find(std::pair<int, int>(chunkCoordinates.first + 1, chunkCoordinates.second - 1)) == chunks.end()) {
                if (chunk.Get(LifeChunk::Width - 2, 1)) {
                    chunksToInitialize[{chunkCoordinates.first + 1, chunkCoordinates.second - 1}] = true;
                }
            }
        } else if (isCyclic) {
            std::pair<int, int> coords = {chunkCoordinates.first + 1 < ChunksCountInWidth ? chunkCoordinates.first + 1 : 0, chunkCoordinates.second - 1 >= 0 ? chunkCoordinates.second - 1 : ChunksCountInHeight - 1};
            std::pair<int, int> get = {chunkCoordinates.first + 1 < ChunksCountInWidth ? LifeChunk::Width - 2 : WidthFromEdge, 1};

            if (chunks.find(coords) == chunks.end()) {
                if (chunk.Get(get.first, get.second)) {
                    chunksToInitialize[coords] = true;
                }
            }
        }

        if (chunkCoordinates.first - 1 >= 0 && chunkCoordinates.second + 1 < ChunksCountInHeight) {
            if (chunks.find(std::pair<int, int>(chunkCoordinates.first - 1, chunkCoordinates.second + 1)) == chunks.end()) {
                if (chunk.Get(1, LifeChunk::Height - 2)) {
                    chunksToInitialize[{chunkCoordinates.first - 1, chunkCoordinates.second + 1}] = true;
                }
            }
        } else if (isCyclic) {
            std::pair<int, int> coords = {chunkCoordinates.first - 1 >= 0 ? chunkCoordinates.first - 1 : ChunksCountInWidth - 1, chunkCoordinates.second + 1 < ChunksCountInHeight ? chunkCoordinates.second + 1 : 0};
            std::pair<int, int> get = {1, chunkCoordinates.second + 1 < ChunksCountInHeight ? LifeChunk::Height - 2 : HeightFromEdge};

            if (chunks.find(coords) == chunks.end()) {
                if (chunk.Get(get.first, get.second)) {
                    chunksToInitialize[coords] = true;
                }
            }
        }
    }

    for (const auto& element : chunksToInitialize) {
        std::pair<int, int> coordinates = element.first;
        InitializeChunk(coordinates.first, coordinates.second);
    }
}

void Universe::ConsiderNeighboringChunks(const std::pair<int, int> coordinates) {

    if (coordinates.second + 1 < ChunksCountInHeight) {
        if (chunks.find(std::pair<int, int>(coordinates.first, coordinates.second + 1)) != chunks.end()) {
            for (int x = 1; x < LifeChunk::Width - 1; ++x) {
                chunks[coordinates].Set(x, LifeChunk::Height - 1, chunks[{coordinates.first, coordinates.second + 1}].Get(x, 1));
            }
        }
    } else if (isCyclic) {
        if (chunks.find(std::pair<int, int>(coordinates.first, 0)) != chunks.end()) {
            for (int x = 1; x < LifeChunk::Width - 1; ++x) {
                chunks[coordinates].Set(x, HeightFromEdge + 1, chunks[{coordinates.first, 0}].Get(x, 1));
            }
        }
    }

    if (coordinates.second - 1 >= 0) {
        if (chunks.find(std::pair<int, int>(coordinates.first, coordinates.second - 1)) != chunks.end()) {
            for (int x = 1; x < LifeChunk::Width - 1; ++x) {
                chunks[coordinates].Set(x, 0, chunks[{coordinates.first, coordinates.second - 1}].Get(x, LifeChunk::Height - 2));
            }
        }
    } else if (isCyclic) {
        if (chunks.find(std::pair<int, int>(coordinates.first, ChunksCountInHeight - 1)) != chunks.end()) {
            for (int x = 1; x < LifeChunk::Width - 1; ++x) {
                chunks[coordinates].Set(x, 0, chunks[{coordinates.first, ChunksCountInHeight - 1}].Get(x, HeightFromEdge));
            }
        }
    }

    if (coordinates.first + 1 < ChunksCountInWidth) {
        if (chunks.find(std::pair<int, int>(coordinates.first + 1, coordinates.second)) != chunks.end()) {
            for (int y = 1; y < LifeChunk::Height - 1; ++y) {
                chunks[coordinates].Set(LifeChunk::Width - 1, y, chunks[{coordinates.first + 1, coordinates.second}].Get(1, y));
            }
        }
    } else if (isCyclic) {
        if (chunks.find(std::pair<int, int>(0, coordinates.second)) != chunks.end()) {
            for (int y = 1; y < LifeChunk::Height - 1; ++y) {
                chunks[coordinates].Set(WidthFromEdge + 1, y, chunks[{0, coordinates.second}].Get(1, y));
            }
        }
    }

    if (coordinates.first - 1 >= 0) {
        if (chunks.find(std::pair<int, int>(coordinates.first - 1, coordinates.second)) != chunks.end()) {
            for (int y = 1; y < LifeChunk::Height - 1; ++y) {
                chunks[coordinates].Set(0, y, chunks[{coordinates.first - 1, coordinates.second}].Get(LifeChunk::Width - 2, y));
            }
        }
    } else if (isCyclic) {
        if (chunks.find(std::pair<int, int>(ChunksCountInWidth - 1, coordinates.second)) != chunks.end()) {
            for (int y = 1; y < LifeChunk::Height - 1; ++y) {
                chunks[coordinates].Set(0, y, chunks[{ChunksCountInWidth - 1, coordinates.second}].Get(WidthFromEdge, y));
            }
        }
    }

    if (coordinates.first + 1 < ChunksCountInWidth && coordinates.second + 1 < ChunksCountInHeight) {
        if (chunks.find(std::pair<int, int>(coordinates.first + 1, coordinates.second + 1)) != chunks.end()) {
            chunks[coordinates].Set(LifeChunk::Width - 1, LifeChunk::Height - 1, chunks[{coordinates.first + 1, coordinates.second + 1}].Get(1, 1));
        }
    } else if (isCyclic) {
        std::pair<int, int> coords = {coordinates.first + 1 < ChunksCountInWidth ? coordinates.first + 1 : 0, coordinates.second + 1 < ChunksCountInHeight ? coordinates.second + 1 : 0};
        std::pair<int, int> set = {coordinates.first + 1 < ChunksCountInWidth ? LifeChunk::Width - 1 : WidthFromEdge + 1, coordinates.second + 1 < ChunksCountInHeight ? LifeChunk::Height - 1 : HeightFromEdge + 1};

        if (chunks.find(coords) != chunks.end()) {
            chunks[coordinates].Set(set.first, set.second, chunks[coords].Get(1, 1));
        }
    }

    if (coordinates.first - 1 >= 0 && coordinates.second - 1 >= 0) {
        if (chunks.find(std::pair<int, int>(coordinates.first - 1, coordinates.second - 1)) != chunks.end()) {
            chunks[coordinates].Set(0, 0, chunks[{coordinates.first - 1, coordinates.second - 1}].Get(LifeChunk::Width - 2, LifeChunk::Height - 2));
        }
    } else if (isCyclic) {
        std::pair<int, int> coords = {coordinates.first - 1 >= 0 ? coordinates.first - 1 : ChunksCountInWidth - 1, coordinates.second - 1 >= 0 ? coordinates.second - 1 : ChunksCountInHeight - 1};
        std::pair<int, int> get = {coordinates.first - 1 >= 0 ? LifeChunk::Width - 2 : WidthFromEdge, coordinates.second - 1 >= 0 ? LifeChunk::Height - 2 : HeightFromEdge};

        if (chunks.find(coords) != chunks.end()) {
            chunks[coordinates].Set(0, 0, chunks[coords].Get(get.first, get.second));
        }
    }

    if (coordinates.first + 1 < ChunksCountInWidth && coordinates.second - 1 >= 0) {
        if (chunks.find(std::pair<int, int>(coordinates.first + 1, coordinates.second - 1)) != chunks.end()) {
            chunks[coordinates].Set(LifeChunk::Width - 1, 0, chunks[{coordinates.first + 1, coordinates.second - 1}].Get(1, LifeChunk::Height - 2));
        }
    } else if (isCyclic) {
        std::pair<int, int> coords = {coordinates.first + 1 < ChunksCountInWidth ? coordinates.first + 1 : 0, coordinates.second - 1 >= 0 ? coordinates.second - 1 : ChunksCountInHeight - 1};
        std::pair<int, int> set_get = {coordinates.first + 1 < ChunksCountInWidth ? LifeChunk::Width - 1 : WidthFromEdge + 1, coordinates.second - 1 >= 0 ? LifeChunk::Height - 2 : HeightFromEdge};

        if (chunks.find(coords) != chunks.end()) {
            chunks[coordinates].Set(set_get.first, 0, chunks[coords].Get(1, set_get.second));
        }
    }

    if (coordinates.first - 1 >= 0 && coordinates.second + 1 < ChunksCountInHeight) {
        if (chunks.find(std::pair<int, int>(coordinates.first - 1, coordinates.second + 1)) != chunks.end()) {
            chunks[coordinates].Set(0, LifeChunk::Height - 1, chunks[{coordinates.first - 1, coordinates.second + 1}].Get(LifeChunk::Width - 2, 1));
        }
    } else if (isCyclic) {
        std::pair<int, int> coords = {coordinates.first - 1 >= 0 ? coordinates.first - 1 : ChunksCountInWidth - 1, coordinates.second + 1 < ChunksCountInHeight ? coordinates.second + 1 : 0};
        std::pair<int, int> get_set = {coordinates.first - 1 >= 0 ? LifeChunk::Width - 2 : WidthFromEdge, coordinates.second + 1 < ChunksCountInHeight ? LifeChunk::Height - 1 : HeightFromEdge + 1};

        if (chunks.find(coords) != chunks.end()) {
            chunks[coordinates].Set(0, get_set.second, chunks[coords].Get(get_set.first, 1));
        }
    }

}

void Universe::ThreadFunction_Preparation(const std::vector<std::pair<int, int>>& arguments) {
    for (const auto argument : arguments) {
        ConsiderNeighboringChunks(argument);
        chunksToDelete[argument] = chunks[argument].CalculateNextField();
    }
}

void Universe::ThreadFunction_CalculateChunk(const std::vector<std::pair<int, int>>& arguments) {
    for (const auto argument : arguments) {
        chunks[argument].SwapFields();

        if (argument.first + 1 == ChunksCountInWidth && argument.second + 1 == ChunksCountInHeight) {
            if (WidthFromEdge != LifeChunk::Width) {
                for (size_t y = 0; y <= HeightFromEdge + 1; ++y) {
                    chunks[argument].currentField[LifeChunk::Width + y * LifeChunk::Width + (WidthFromEdge + 1)] = 0;
                    chunks[argument].currentField[LifeChunk::Width + y * LifeChunk::Width + (WidthFromEdge + 2)] = 0;
                }
            }

            if (HeightFromEdge != LifeChunk::Height) {
                for (size_t x = 0; x <= WidthFromEdge + 1; ++x) {
                    chunks[argument].currentField[LifeChunk::Width + (HeightFromEdge + 1) * LifeChunk::Width + x] = 0;
                    chunks[argument].currentField[LifeChunk::Width + (HeightFromEdge + 2) * LifeChunk::Width + x] = 0;
                }
            }
        } else if (argument.first + 1 == ChunksCountInWidth) {
            if (WidthFromEdge != LifeChunk::Width) {
                for (size_t y = 0; y <= HeightFromEdge + 1; ++y) {
                    chunks[argument].currentField[LifeChunk::Width + y * LifeChunk::Width + (WidthFromEdge + 1)] = 0;
                    chunks[argument].currentField[LifeChunk::Width + y * LifeChunk::Width + (WidthFromEdge + 2)] = 0;
                }
            }
        } else if (argument.second + 1 == ChunksCountInHeight) {
            if (HeightFromEdge != LifeChunk::Height) {
                for (size_t x = 0; x <= WidthFromEdge + 1; ++x) {
                    chunks[argument].currentField[LifeChunk::Width + (HeightFromEdge + 1) * LifeChunk::Width + x] = 0;
                    chunks[argument].currentField[LifeChunk::Width + (HeightFromEdge + 2) * LifeChunk::Width + x] = 0;
                }
            }
        }
    }
}

void Universe::CalculateAllChunks() {

    int restChunksCount = chunks.size();
    int chunksCountInThread = 0;

    for (int i = ThreadsCount; i >= 1; --i) {
        chunksCountInThread = restChunksCount / i;
        restChunksCount -= chunksCountInThread;
        countList[ThreadsCount - i] = chunksCountInThread;
        argumentsList[ThreadsCount - i].reserve(chunksCountInThread);
    }


    int index = 0;
    for (const auto& element : chunks) {
        while (countList[index] == 0) {
            ++index;
        }

        argumentsList[index].push_back(element.first);
        --countList[index];
    }

    for (int i = 0; i < ThreadsCount; ++i) {
        if (!argumentsList[i].empty()) {
            threads.emplace_back(&Universe::ThreadFunction_Preparation, this, std::ref(argumentsList[i]));
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }

    DeleteDeadChunks();
    
    for (int i = 0; i < ThreadsCount; ++i) {
        argumentsList[i].clear();
    }
    threads.clear();

    restChunksCount = chunks.size();
    chunksCountInThread = 0;

    for (int i = ThreadsCount; i >= 1; --i) {
        chunksCountInThread = restChunksCount / i;
        restChunksCount -= chunksCountInThread;
        countList[ThreadsCount - i] = chunksCountInThread;
        argumentsList[ThreadsCount - i].reserve(chunksCountInThread);
    }


    index = 0;
    for (const auto& element : chunks) {
        while (countList[index] == 0) {
            ++index;
        }

        argumentsList[index].push_back(element.first);
        --countList[index];
    }

    for (int i = 0; i < ThreadsCount; ++i) {
        if (!argumentsList[i].empty()) {
            threads.emplace_back(&Universe::ThreadFunction_CalculateChunk, this, std::ref(argumentsList[i]));
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (int i = 0; i < ThreadsCount; ++i) {
        argumentsList[i].clear();
    }
    threads.clear();

}

void Universe::CalculateNextGeneration() {
    InitializeNeighboringChunks();
    CalculateAllChunks();
}