#pragma once
#include "Room.h"
#include "Enemy.h"
#include "Item.h"
#include <vector>
#include <array>
#include <memory>

struct RoomNode
{
    int x = 0, y = 0;
    RoomType type = RoomType::NORMAL;
    bool visited = false;
    bool cleared = false;
    std::array<int, 4> neighbors = { -1, -1, -1, -1 };
    std::vector<Item> items;
    Room savedRoom;
    bool roomGenerated = false;

    RoomNode() : x(0), y(0), type(RoomType::NORMAL), visited(false), cleared(false), roomGenerated(false) {}
};

class Level
{
public:
    static constexpr int MAP_SIZE = 11;

    Level();

    void generate(int floorNum, unsigned int seed);

    int getRoomCount() const;
    int getCurrentRoomIndex() const;
    const RoomNode& getCurrentNode() const;
    RoomNode& getCurrentNode();

    bool moveInDirection(Direction dir, Direction& fromDir);
    void markCurrentCleared();
    std::array<bool, 4> getCurrentDoors() const;
    int getFloorNumber() const;
    const std::vector<RoomNode>& getNodes() const;

private:
    std::vector<RoomNode> m_Nodes;
    int m_Current;
    int m_FloorNumber;
    int m_Map[MAP_SIZE][MAP_SIZE];

    void clearMap();
    int indexAt(int x, int y) const;
    void addRoom(int x, int y, RoomType type);
    void linkNeighbors();
    int farthestFromStart(int startIdx) const;
};