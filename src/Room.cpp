#include "Room.h"
#include <random>
#include <cmath>

Room::Room() : m_Type(RoomType::NORMAL), m_Locked(false) {
    std::array<bool, 4> d = { true, true, true, true };
    generate(RoomType::START, d, 0);
}

void Room::generate(RoomType type, const std::array<bool, 4>& doors, int seed) {
    m_Type = type;
    m_Doors = doors;
    m_Locked = false;

    for (int y = 0; y < GRID_HEIGHT; ++y)
        for (int x = 0; x < GRID_WIDTH; ++x)
            m_Grid[y][x] = 0;

    for (int x = 0; x < GRID_WIDTH; ++x) {
        m_Grid[0][x] = 1;
        m_Grid[GRID_HEIGHT - 1][x] = 1;
    }
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        m_Grid[y][0] = 1;
        m_Grid[y][GRID_WIDTH - 1] = 1;
    }

    placeDoors();
    if (type == RoomType::NORMAL || type == RoomType::BOSS) placeObstacles(seed);
    buildVisuals();
}

void Room::placeDoors() {
    if (m_Doors[(int)Direction::NORTH]) m_Grid[0][GRID_WIDTH / 2] = 2;
    if (m_Doors[(int)Direction::SOUTH]) m_Grid[GRID_HEIGHT - 1][GRID_WIDTH / 2] = 2;
    if (m_Doors[(int)Direction::WEST])  m_Grid[GRID_HEIGHT / 2][0] = 2;
    if (m_Doors[(int)Direction::EAST])  m_Grid[GRID_HEIGHT / 2][GRID_WIDTH - 1] = 2;
}

void Room::placeObstacles(int seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> countDist(2, 5);
    std::uniform_int_distribution<int> xDist(3, GRID_WIDTH - 4);
    std::uniform_int_distribution<int> yDist(2, GRID_HEIGHT - 3);
    int count = countDist(rng);
    for (int i = 0; i < count; ++i) {
        int ox = xDist(rng), oy = yDist(rng);
        if (ox == GRID_WIDTH / 2 && oy == GRID_HEIGHT / 2) continue;
        m_Grid[oy][ox] = 1;
    }
}

void Room::buildVisuals() {
    m_Tiles.clear();
    sf::Color floorA(122, 95, 70), floorB(138, 108, 80);
    switch (m_Type) {
    case RoomType::START: floorA = sf::Color(100, 130, 90); floorB = sf::Color(115, 145, 105); break;
    case RoomType::ITEM:  floorA = sf::Color(130, 125, 90); floorB = sf::Color(150, 145, 105); break;
    case RoomType::SHOP:  floorA = sf::Color(130, 105, 140); floorB = sf::Color(150, 120, 160); break;
    case RoomType::BOSS:  floorA = sf::Color(100, 60, 60); floorB = sf::Color(120, 75, 75); break;
    default: break;
    }
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            sf::RectangleShape tile({ (float)TILE_SIZE, (float)TILE_SIZE });
            tile.setPosition((float)(x * TILE_SIZE), (float)(y * TILE_SIZE));
            if (m_Grid[y][x] == 1) {
                tile.setFillColor(sf::Color(55, 55, 62));
                tile.setOutlineColor(sf::Color(30, 30, 36));
                tile.setOutlineThickness(1.f);
            }
            else if (m_Grid[y][x] == 2) {
                tile.setFillColor(m_Locked ? sf::Color(120, 40, 40) : sf::Color(205, 170, 110));
                tile.setOutlineColor(sf::Color(80, 60, 30));
                tile.setOutlineThickness(1.f);
            }
            else {
                tile.setFillColor(((x + y) % 2 == 0) ? floorA : floorB);
            }
            m_Tiles.push_back(tile);
        }
    }
}

void Room::setDoorsLocked(bool locked) {
    if (m_Locked != locked) { m_Locked = locked; buildVisuals(); }
}

void Room::draw(sf::RenderWindow& window) {
    for (auto& tile : m_Tiles) window.draw(tile);
}

bool Room::isSolid(float x, float y, bool ignoreDoors) const {
    if (x < 0 || y < 0) return true;
    int gx = static_cast<int>(x) / TILE_SIZE;
    int gy = static_cast<int>(y) / TILE_SIZE;
    if (gx < 0 || gy < 0 || gx >= GRID_WIDTH || gy >= GRID_HEIGHT) return true;
    int cell = m_Grid[gy][gx];
    if (cell == 1) return true;
    if (cell == 2) return ignoreDoors ? false : m_Locked;
    return false;
}

bool Room::isRectSolid(const sf::FloatRect& rect) const {
    int left = std::max(0, (int)(rect.left / TILE_SIZE));
    int top = std::max(0, (int)(rect.top / TILE_SIZE));
    int right = std::min(GRID_WIDTH - 1, (int)((rect.left + rect.width) / TILE_SIZE));
    int bottom = std::min(GRID_HEIGHT - 1, (int)((rect.top + rect.height) / TILE_SIZE));
    for (int y = top; y <= bottom; ++y)
        for (int x = left; x <= right; ++x) {
            int cell = m_Grid[y][x];
            if (cell == 1) return true;
            if (cell == 2 && m_Locked) return true;
        }
    return false;
}

bool Room::isAtDoor(const sf::FloatRect& bounds, Direction dir) const {
    if (!m_Doors[(int)dir]) return false;
    float cx = bounds.left + bounds.width / 2.f;
    float cy = bounds.top + bounds.height / 2.f;
    int doorX = GRID_WIDTH / 2, doorY = GRID_HEIGHT / 2;
    const float edge = (float)TILE_SIZE * 0.4f;
    switch (dir) {
    case Direction::NORTH: return cy < edge && std::abs(cx - (doorX + 0.5f) * TILE_SIZE) < TILE_SIZE * 0.55f;
    case Direction::SOUTH: return cy > GRID_HEIGHT * TILE_SIZE - edge && std::abs(cx - (doorX + 0.5f) * TILE_SIZE) < TILE_SIZE * 0.55f;
    case Direction::WEST:  return cx < edge && std::abs(cy - (doorY + 0.5f) * TILE_SIZE) < TILE_SIZE * 0.55f;
    case Direction::EAST:  return cx > GRID_WIDTH * TILE_SIZE - edge && std::abs(cy - (doorY + 0.5f) * TILE_SIZE) < TILE_SIZE * 0.55f;
    }
    return false;
}

sf::Vector2f Room::getEntryPosition(Direction fromDir) const {
    float cx = (GRID_WIDTH / 2 + 0.5f) * TILE_SIZE;
    float cy = (GRID_HEIGHT / 2 + 0.5f) * TILE_SIZE;
    switch (fromDir) {
    case Direction::NORTH: return { cx, (float)TILE_SIZE * 1.5f };
    case Direction::SOUTH: return { cx, (float)(GRID_HEIGHT - 2) * TILE_SIZE + TILE_SIZE / 2.f };
    case Direction::WEST:  return { (float)TILE_SIZE * 1.5f, cy };
    case Direction::EAST:  return { (float)(GRID_WIDTH - 2) * TILE_SIZE + TILE_SIZE / 2.f, cy };
    }
    return { cx, cy };
}

sf::Vector2f Room::getCenter() const {
    return { (GRID_WIDTH / 2 + 0.5f) * TILE_SIZE, (GRID_HEIGHT / 2 + 0.5f) * TILE_SIZE };
}

std::vector<sf::Vector2f> Room::getSpawnPoints(int count, int seed, float entityRadius) const
{
    std::vector<sf::Vector2f> result;
    std::mt19937 rng(seed ^ 0x5A5A);
    std::uniform_int_distribution<int> xDist(1, GRID_WIDTH - 2);  // Изменено: 1 и -2 вместо 2 и -3
    std::uniform_int_distribution<int> yDist(1, GRID_HEIGHT - 2); // Изменено: 1 и -2 вместо 2 и -3

    int attempts = 0;
    while ((int)result.size() < count && attempts < 400)
    {
        int gx = xDist(rng), gy = yDist(rng);

        // Проверяем, что клетка - пол (не стена и не дверь)
        if (m_Grid[gy][gx] != 0)
        {
            attempts++;
            continue;
        }

        sf::Vector2f p((gx + 0.5f) * TILE_SIZE, (gy + 0.5f) * TILE_SIZE);

        // УСИЛЕННАЯ ПРОВЕРКА: создаём прямоугольник врага и проверяем все его углы
        bool validSpot = true;

        if (entityRadius > 0.f) {
            // Проверяем не только центр, но и все четыре угла + края
            float left = p.x - entityRadius;
            float right = p.x + entityRadius;
            float top = p.y - entityRadius;
            float bottom = p.y + entityRadius;

            // Проверяем углы
            if (isSolid(left, top, false) ||
                isSolid(right, top, false) ||
                isSolid(left, bottom, false) ||
                isSolid(right, bottom, false))
            {
                validSpot = false;
            }

            // Дополнительная проверка: центр клеток вокруг
            if (validSpot) {
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int checkX = gx + dx;
                        int checkY = gy + dy;
                        if (checkX >= 0 && checkX < GRID_WIDTH &&
                            checkY >= 0 && checkY < GRID_HEIGHT) {
                            if (m_Grid[checkY][checkX] == 1) { // Стена рядом
                                // Проверяем, не слишком ли близко
                                float wallCenterX = (checkX + 0.5f) * TILE_SIZE;
                                float wallCenterY = (checkY + 0.5f) * TILE_SIZE;
                                float dxW = p.x - wallCenterX;
                                float dyW = p.y - wallCenterY;
                                float distToWall = std::sqrt(dxW * dxW + dyW * dyW);
                                if (distToWall < entityRadius + TILE_SIZE * 0.3f) {
                                    validSpot = false;
                                    break;
                                }
                            }
                        }
                    }
                    if (!validSpot) break;
                }
            }
        }

        if (!validSpot) {
            attempts++;
            continue;
        }

        // Проверка, чтобы враги не спавнились слишком близко друг к другу
        bool tooClose = false;
        for (auto& e : result) {
            float dx = e.x - p.x, dy = e.y - p.y;
            float minDist = entityRadius * 2.5f; // Увеличено с 2.0f до 2.5f
            if (dx * dx + dy * dy < minDist * minDist) {
                tooClose = true;
                break;
            }
        }

        if (tooClose) {
            attempts++;
            continue;
        }

        result.push_back(p);
        attempts++;
    }

    // Если не удалось найти достаточно мест, используем fallback - центр комнаты
    if ((int)result.size() < count && count > 0) {
        sf::Vector2f center = getCenter();
        bool centerValid = true;

        // Проверяем центр
        float left = center.x - entityRadius;
        float right = center.x + entityRadius;
        float top = center.y - entityRadius;
        float bottom = center.y + entityRadius;

        if (isSolid(left, top, false) || isSolid(right, top, false) ||
            isSolid(left, bottom, false) || isSolid(right, bottom, false)) {
            centerValid = false;
        }

        if (centerValid && result.empty()) {
            result.push_back(center);
        }
    }

    return result;
}

RoomType Room::getType() const { return m_Type; }
bool Room::hasDoor(Direction d) const { return m_Doors[(int)d]; }
bool Room::isDoorsLocked() const { return m_Locked; }
