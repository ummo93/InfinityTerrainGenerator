#pragma once
#include "raylib.h"
#include "raymath.h"
#include "simplex.hpp"
#include <format>
#include <vector>
#include <optional>
#include <functional>


#define LANDSCAPE_NOISE_SMALL				0.001f
#define LANDSCAPE_NOISE_MEDIUM				0.01f
#define LANDSCAPE_NOISE_BIG					0.00009f
#define CHUNK_SIZE					        100
#define CHUNK_TO_MESH_SCALE					5 // if CHUNK_TO_MESH_SCALE = 10, heightmap 100 x 100, => mesh in world coordinates 1000x1000
#define FAR_AREA_LIMIT_DISTANCE				5 // local coordinates distance (if 5 = CHUNK_SIZE*CHUNK_TO_MESH_SCALE*5 meters)

struct ChunkNode {
    Vector2 location;
    Image heightMap;
    float heights[CHUNK_SIZE][CHUNK_SIZE];
    bool isLoaded;

    inline ChunkNode(Vector2 location) : location(location) {}

    inline ~ChunkNode()
    {
        Unload();
    }

    float CalcHeight(const double X, const double Y, int min, int max)
    {
        return Clamp(
            (SimplexNoise::noise(X * LANDSCAPE_NOISE_SMALL, Y * LANDSCAPE_NOISE_SMALL)) +
            (SimplexNoise::noise(X * LANDSCAPE_NOISE_MEDIUM, Y * LANDSCAPE_NOISE_MEDIUM)) +
            (SimplexNoise::noise(X * LANDSCAPE_NOISE_BIG, Y * LANDSCAPE_NOISE_BIG)),
            min, max
        );
    }

    inline void Load()
    {
        heightMap = GenImageColor(CHUNK_SIZE, CHUNK_SIZE, BLACK);
        int locationOffsetX = location.x * (CHUNK_SIZE - 1);
        int locationOffsetY = location.y * (CHUNK_SIZE - 1);
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                auto val = CalcHeight(locationOffsetX + x, locationOffsetY + y, 0, 2);
                heights[x][y] = val;
                ImageDrawPixel(&heightMap, x, y, { (unsigned char)((val / 2) * 255), (unsigned char)((val / 2) * 255), (unsigned char)((val / 2) * 255), 255 });
            }
        }
        isLoaded = true;
    }

    inline void Unload()
    {
        if (!isLoaded) return;
        UnloadImage(heightMap);
        isLoaded = false;
    }
};

struct Area {
    constexpr static int scaleOffset = CHUNK_SIZE * CHUNK_TO_MESH_SCALE;
    Vector2 location;
    std::shared_ptr<ChunkNode> chunkNode;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Model> model;
};

struct InfinityWorld {
    std::vector<std::shared_ptr<Area>> areas;

    InfinityWorld(int seed)
    {
        SimplexNoise::setSeed(seed);
    }

    ~InfinityWorld()
    {
        UnloadAll();
    }

    inline void EachArea(std::function<void(std::shared_ptr<Area>)> callback) const
    {
        for (auto a : areas)
        {
            callback(a);
        }
    }

    inline std::optional<std::shared_ptr<Area>> GetAreaByLocalPos(Vector2 pos) const
    {
        for (auto a : areas)
        {
            if (((int)a->location.x) == ((int)pos.x) && ((int)a->location.y) == ((int)pos.y))
            {
                return std::make_optional(a);
            }
        }
        return {};
    }

    inline std::shared_ptr<Area> GetArea(Vector3& pos)
    {
        auto location = GetChunkLocationByPosition(pos);
        return LoadOrGetAreaByLocation(location);
    }

    inline std::shared_ptr<Area> LoadOrGetAreaByLocation(Vector2& location)
    {
        auto foundArea = GetAreaByLocalPos(location);
        if (foundArea)
        {
            return foundArea.value();
        }
        auto chunk = std::make_shared<ChunkNode>(location);
        chunk->Load();
        auto texture = LoadTextureFromImage(chunk->heightMap);
        auto mesh = std::make_shared<Mesh>(GenMeshHeightmap(chunk->heightMap, { CHUNK_TO_MESH_SCALE, 1, CHUNK_TO_MESH_SCALE }));
        auto model = std::make_shared<Model>(LoadModelFromMesh(*mesh));
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
        auto area = std::make_shared<Area>(location, chunk, mesh, model);
        areas.push_back(area);
        return area;
    }

    inline void LoadNeighbours(Area& area)
    {
        Vector2 neighbourNorth = { area.location.x, area.location.y - 1 };
        Vector2 neighbourNorthEast = { area.location.x + 1, area.location.y - 1 };
        Vector2 neighbourEast = { area.location.x + 1, area.location.y };
        Vector2 neighbourSouthEast = { area.location.x + 1, area.location.y + 1 };
        Vector2 neighbourSouth = { area.location.x, area.location.y + 1 };
        Vector2 neighbourSouthWest = { area.location.x - 1, area.location.y + 1 };
        Vector2 neighbourWest = { area.location.x - 1, area.location.y };
        Vector2 neighbourNorthWest = { area.location.x - 1, area.location.y - 1 };
        LoadOrGetAreaByLocation(neighbourNorth);
        LoadOrGetAreaByLocation(neighbourNorthEast);
        LoadOrGetAreaByLocation(neighbourEast);
        LoadOrGetAreaByLocation(neighbourSouthEast);
        LoadOrGetAreaByLocation(neighbourSouth);
        LoadOrGetAreaByLocation(neighbourSouthWest);
        LoadOrGetAreaByLocation(neighbourWest);
        LoadOrGetAreaByLocation(neighbourNorthWest);
    }


    inline Vector2 GetChunkLocationByPosition(Vector3& pos) const
    {
        return {
            std::ceil(pos.x / Area::scaleOffset) - 1,
            std::ceil(pos.z / Area::scaleOffset) - 1
        };
    }

    inline void UnloadFarAreas(Vector3& pos)
    {
        auto currentLocation = GetChunkLocationByPosition(pos);
        for (auto it = areas.begin(); it != areas.end();)
        {
            std::shared_ptr<Area> area = *it;
            if (Vector2Distance(currentLocation, area->location) > 2)
            {
                it = areas.erase(it);
                Unload(area);
            }
            else {
                ++it;
            }
        }

    }

    inline void Unload(std::shared_ptr<Area> area)
    {
        area->chunkNode->Unload();
        UnloadTexture(area->model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
        UnloadModel(*area->model);
    }

    inline void UnloadAll()
    {
        for (auto a : areas)
        {
            Unload(a);
        }
        areas.clear();
    }
};