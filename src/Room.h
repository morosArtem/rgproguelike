#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <array>

enum class RoomType { START, NORMAL, ITEM, SHOP, BOSS };
enum class Direction { NORTH = 0, SOUTH = 1, WEST = 2, EAST = 3 };

class Room
{
public:
    static constexpr int GRID_WIDTH = 16;
    static constexpr int GRID_HEIGHT = 9;
    static constexpr int TILE_SIZE = 80;

    Room();
    void generate(RoomType type, const std::array<bool, 4>& doors, int seed);
    void setDoorsLocked(bool locked);
    void draw(sf::RenderWindow& window);

    bool isSolid(float x, float y, bool ignoreDoors) const;
    bool isRectSolid(const sf::FloatRect& rect) const;
    bool isAtDoor(const sf::FloatRect& bounds, Direction dir) const;

    sf::Vector2f getEntryPosition(Direction fromDir) const;
    sf::Vector2f getCenter() const;
    std::vector<sf::Vector2f> getSpawnPoints(int count, int seed, float entityRadius = 0.f) const;

    RoomType getType() const;
    bool hasDoor(Direction d) const;
    bool isDoorsLocked() const;

private:
    int m_Grid[GRID_HEIGHT][GRID_WIDTH];
    std::array<bool, 4> m_Doors;
    RoomType m_Type;
    bool m_Locked;
    std::vector<sf::RectangleShape> m_Tiles;
    std::vector<sf::RectangleShape> m_Decorations;

    void buildVisuals();
    void placeDoors();
    void placeObstacles(int seed);
};
