#pragma once
#include "Room.h"
#include "Enemy.h"
#include "Item.h"
#include <vector>
#include <array>
#include <memory>

// Узел карты этажа
struct RoomNode
{
    int x = 0, y = 0;
    RoomType type = RoomType::NORMAL;
    bool visited = false;
    bool cleared = false;
    std::array<int, 4> neighbors = { -1, -1, -1, -1 };
    std::vector<Item> items;
    Room savedRoom;           // ← ДОБАВЬТЕ ЭТУ СТРОЧКУ
    bool roomGenerated = false; // ← И ЭТУ СТРОЧКУ
};

// Этаж — граф комнат
class Level
{
public:
    static constexpr int MAP_SIZE = 11;

    Level();

    // Сгенерировать этаж номер floorNum (1..N)
    void generate(int floorNum, unsigned int seed);

    // Кол-во комнат
    int getRoomCount() const;

    int getCurrentRoomIndex() const;
    const RoomNode& getCurrentNode() const;
    RoomNode& getCurrentNode();

    // Перейти в соседнюю комнату. Возвращает направление, с которого вошёл игрок.
    // Если соседа нет — возвращает false.
    bool moveInDirection(Direction dir, Direction& fromDir);

    // Пометить текущую комнату как пройденную
    void markCurrentCleared();

    // Узнать, какие двери есть у текущей комнаты
    std::array<bool, 4> getCurrentDoors() const;

    int getFloorNumber() const;

    // Для миникарты
    const std::vector<RoomNode>& getNodes() const;

private:
    std::vector<RoomNode> m_Nodes;
    int m_Current;
    int m_FloorNumber;

    // Сетка индексов: map[y][x] = индекс в m_Nodes или -1
    int m_Map[MAP_SIZE][MAP_SIZE];

    void clearMap();
    int indexAt(int x, int y) const;
    void addRoom(int x, int y, RoomType type);
    void linkNeighbors();
    int farthestFromStart(int startIdx) const;
};
