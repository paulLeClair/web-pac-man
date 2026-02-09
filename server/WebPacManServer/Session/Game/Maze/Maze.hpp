//
// Created by paull on 2026-02-08.
//

#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class TileType : unsigned char {
    UNKNOWN,
    OPEN,
    INTERSECTION
};

struct MazeCell
{
    int gridX,gridY;
    TileType type;
    float pixelX, pixelY;
};

class MazeFile
{
public:
    explicit MazeFile(const std::string &fileName)
    {
        if (fileName.empty()) return;

        const std::filesystem::path path(fileName);

        // TODO -> input sanitization and validation
        if (std::fstream file(path, std::fstream::in | std::fstream::binary); file.is_open())
        {
            int x,y;
            char type;
            while (file >> x >> y >> type)
            {
                auto tileType = type != 'I' ? TileType::OPEN : TileType::INTERSECTION;

                // subtract one from each coordinate for 0-indexing
                x--;
                y--;

                int packedCoordinates = x << 16 | y;
                validTileCoordinates.insert(packedCoordinates);
                tileTypes[packedCoordinates] = tileType;

                const float pixelPositionX = x * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS;
                const float pixelPositionY = y * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS;
                mazeCells.emplace_back(x, y, tileType, pixelPositionX, pixelPositionY);
                cellIndices[packedCoordinates] = mazeCells.size() - 1;
            }
        }

    }
    ~MazeFile() = default;

    [[nodiscard]] TileType getTileType(const int x, const int y) const
    {
        const int keyVal = x << 16 | y;
        if (!tileTypes.contains(keyVal)) return TileType::UNKNOWN;
        return tileTypes.at(keyVal);
    }

    [[nodiscard]] bool isWalkable(const int x, const int y) const
    {
        return validTileCoordinates.contains(x << 16 | y);
    }

    MazeCell *getCell(const int x, const int y)
    {
        return &mazeCells[cellIndices[x << 16 | y]];
    }

private:
    static constexpr uint32_t NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS = 8;

    std::vector<MazeCell> mazeCells = {};

    // contains index into root mazeCells array for convenience
    std::unordered_map<int, int> cellIndices = {}; // TODO -> this should use pairs for coords

    // convenience set for quickly querying if a tile is present in the sparse grid
    std::unordered_set<int> validTileCoordinates = {};

    // convenience set for quickly querying the type of a tile
    std::unordered_map<int, TileType> tileTypes = {};

    // stores any tunnels from one tile to another, for the "warp" tunnel effect on the classic map
    std::unordered_map<int, int> tunnels = {}; // THESE ARE TODO FOR NOW
};