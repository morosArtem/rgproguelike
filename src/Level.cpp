#include "Level.h"
#include <random>
#include <queue>
#include <algorithm>

Level::Level()
    : m_Current(0), m_FloorNumber(1)
{
    clearMap();
}

void Level::clearMap()
{
    for (int y = 0; y < MAP_SIZE; ++y)
        for (int x = 0; x < MAP_SIZE; ++x)
            m_Map[y][x] = -1;
}

int Level::indexAt(int x, int y) const
{
    if (x < 0 || y < 0 || x >= MAP_SIZE || y >= MAP_SIZE) return -1;
    return m_Map[y][x];
}

void Level::addRoom(int x, int y, RoomType type)
{
    RoomNode node;
    node.x = x;
    node.y = y;
    node.type = type;
    m_Nodes.push_back(node);
    m_Map[y][x] = static_cast<int>(m_Nodes.size()) - 1;
}

void Level::linkNeighbors()
{
    for (size_t i = 0; i < m_Nodes.size(); ++i)
    {
        auto& n = m_Nodes[i];
        n.neighbors[0] = indexAt(n.x, n.y - 1); // NORTH
        n.neighbors[1] = indexAt(n.x, n.y + 1); // SOUTH
        n.neighbors[2] = indexAt(n.x - 1, n.y); // WEST
        n.neighbors[3] = indexAt(n.x + 1, n.y); // EAST
    }
}

int Level::farthestFromStart(int startIdx) const
{
    std::vector<int> dist(m_Nodes.size(), -1);
    std::queue<int> q;
    q.push(startIdx);
    dist[startIdx] = 0;
    int farthest = startIdx;
    int maxD = 0;

    while (!q.empty())
    {
        int cur = q.front(); q.pop();
        for (int n : m_Nodes[cur].neighbors)
        {
            if (n >= 0 && dist[n] < 0)
            {
                dist[n] = dist[cur] + 1;
                if (dist[n] > maxD) { maxD = dist[n]; farthest = n; }
                q.push(n);
            }
        }
    }
    return farthest;
}

void Level::generate(int floorNum, unsigned int seed)
{
    m_FloorNumber = floorNum;
    m_Nodes.clear();
    clearMap();

    std::mt19937 rng(seed);
    int targetCount = 6 + (floorNum - 1) + static_cast<int>(rng() % 3);
    targetCount = std::min(targetCount, 10);

    int startX = MAP_SIZE / 2;
    int startY = MAP_SIZE / 2;
    addRoom(startX, startY, RoomType::START);

    int attempts = 0;
    while (static_cast<int>(m_Nodes.size()) < targetCount && attempts < 1000)
    {
        int idx = static_cast<int>(rng() % m_Nodes.size());
        auto base = m_Nodes[idx];
        int dir = rng() % 4;
        int nx = base.x, ny = base.y;
        switch (dir)
        {
        case 0: ny -= 1; break;
        case 1: ny += 1; break;
        case 2: nx -= 1; break;
        case 3: nx += 1; break;
        }
        if (nx >= 1 && ny >= 1 && nx < MAP_SIZE - 1 && ny < MAP_SIZE - 1
            && indexAt(nx, ny) == -1)
        {
            addRoom(nx, ny, RoomType::NORMAL);
        }
        attempts++;
    }

    linkNeighbors();

    int startIdx = m_Map[startY][startX];
    int bossIdx = farthestFromStart(startIdx);
    if (bossIdx != startIdx)
        m_Nodes[bossIdx].type = RoomType::BOSS;

    std::vector<int> candidates;
    candidates.reserve(m_Nodes.size());
    for (size_t i = 0; i < m_Nodes.size(); ++i)
    {
        if (static_cast<int>(i) != startIdx && static_cast<int>(i) != bossIdx
            && m_Nodes[i].type == RoomType::NORMAL)
            candidates.push_back(static_cast<int>(i));
    }
    std::shuffle(candidates.begin(), candidates.end(), rng);
    if (!candidates.empty())
    {
        m_Nodes[candidates.back()].type = RoomType::ITEM;
        candidates.pop_back();
    }
    if (floorNum >= 2 && !candidates.empty())
    {
        m_Nodes[candidates.back()].type = RoomType::SHOP;
        candidates.pop_back();
    }

    m_Current = startIdx;
    m_Nodes[m_Current].visited = true;
    m_Nodes[m_Current].cleared = true;
}

int Level::getRoomCount() const { return static_cast<int>(m_Nodes.size()); }
int Level::getCurrentRoomIndex() const { return m_Current; }
const RoomNode& Level::getCurrentNode() const { return m_Nodes[m_Current]; }
RoomNode& Level::getCurrentNode() { return m_Nodes[m_Current]; }

bool Level::moveInDirection(Direction dir, Direction& fromDir)
{
    int neighbor = m_Nodes[m_Current].neighbors[static_cast<int>(dir)];
    if (neighbor < 0) return false;

    switch (dir)
    {
    case Direction::NORTH: fromDir = Direction::SOUTH; break;
    case Direction::SOUTH: fromDir = Direction::NORTH; break;
    case Direction::WEST:  fromDir = Direction::EAST;  break;
    case Direction::EAST:  fromDir = Direction::WEST;  break;
    }

    m_Current = neighbor;
    m_Nodes[m_Current].visited = true;
    return true;
}

void Level::markCurrentCleared()
{
    m_Nodes[m_Current].cleared = true;
}

std::array<bool, 4> Level::getCurrentDoors() const
{
    std::array<bool, 4> doors = { false, false, false, false };
    const auto& n = m_Nodes[m_Current];
    for (int i = 0; i < 4; ++i)
        doors[i] = (n.neighbors[i] >= 0);
    return doors;
}

int Level::getFloorNumber() const { return m_FloorNumber; }

const std::vector<RoomNode>& Level::getNodes() const { return m_Nodes; }