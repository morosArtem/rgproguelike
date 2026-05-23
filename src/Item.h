#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// Типы предметов
enum class ItemType
{
    HEALTH,     // Аптечка — восстанавливает HP
    DAMAGE_UP,  // Апгрейд урона
    SPEED_UP,   // Апгрейд скорости
    SHIELD,     // Щит — поглощает один удар
    COIN        // Монета (выпадает из врагов)
};

// Предмет, лежащий на полу комнаты
class Item
{
public:
    Item(ItemType type, sf::Vector2f position, int cost = 0);
    Item(const Item& other);

    void draw(sf::RenderWindow& window);

    sf::FloatRect getBounds() const;
    ItemType getType() const;
    bool isCollected() const;
    void collect();
    int getCost() const;

    sf::Vector2f getPosition() const { return m_Position; }

    static std::string getDescription(ItemType type);
    static sf::Color getColor(ItemType type);
    static std::string getLabel(ItemType type);

private:
    ItemType m_Type;
    sf::Vector2f m_Position;
    sf::RectangleShape m_Shape;
    sf::Text m_Label;
    bool m_Collected;
    int m_Cost; // для магазина (0 = бесплатно)
};
