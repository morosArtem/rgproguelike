#include "Item.h"
#include "AssetManager.h"

Item::Item(ItemType type, sf::Vector2f position, int cost)
    : m_Type(type)
    , m_Position(position)
    , m_Collected(false)
    , m_Cost(cost)
{
    float size = (type == ItemType::COIN) ? 16.f : 30.f;
    m_Shape.setSize({ size, size });
    m_Shape.setOrigin(size / 2.f, size / 2.f);
    m_Shape.setPosition(m_Position);
    m_Shape.setFillColor(getColor(type));
    m_Shape.setOutlineColor(sf::Color::White);
    m_Shape.setOutlineThickness(1.f);

    if (AssetManager::instance().isFontLoaded())
    {
        m_Label.setFont(AssetManager::instance().getFont());
        m_Label.setString(getLabel(type));
        m_Label.setCharacterSize(type == ItemType::COIN ? 10 : 16);
        m_Label.setFillColor(sf::Color::White);
        auto lb = m_Label.getLocalBounds();
        m_Label.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        m_Label.setPosition(m_Position);
    }
}

void Item::draw(sf::RenderWindow& window)
{
    if (m_Collected) return;
    window.draw(m_Shape);
    if (AssetManager::instance().isFontLoaded())
        window.draw(m_Label);
}

sf::FloatRect Item::getBounds() const
{
    return m_Shape.getGlobalBounds();
}

ItemType Item::getType() const { return m_Type; }
bool Item::isCollected() const { return m_Collected; }
void Item::collect() { m_Collected = true; }
int Item::getCost() const { return m_Cost; }

std::string Item::getDescription(ItemType type)
{
    switch (type)
    {
    case ItemType::HEALTH:    return "HP +30";
    case ItemType::DAMAGE_UP: return "DMG +10";
    case ItemType::SPEED_UP:  return "SPD +50";
    case ItemType::SHIELD:    return "Shield";
    case ItemType::COIN:      return "Coin";
    }
    return "";
}

sf::Color Item::getColor(ItemType type)
{
    switch (type)
    {
    case ItemType::HEALTH:    return sf::Color(50, 180, 50);
    case ItemType::DAMAGE_UP: return sf::Color(200, 50, 50);
    case ItemType::SPEED_UP:  return sf::Color(50, 100, 220);
    case ItemType::SHIELD:    return sf::Color(50, 200, 200);
    case ItemType::COIN:      return sf::Color(240, 200, 40);
    }
    return sf::Color::White;
}

std::string Item::getLabel(ItemType type)
{
    switch (type)
    {
    case ItemType::HEALTH:    return "+";
    case ItemType::DAMAGE_UP: return "D";
    case ItemType::SPEED_UP:  return "S";
    case ItemType::SHIELD:    return "O";
    case ItemType::COIN:      return "$";
    }
    return "?";
}
