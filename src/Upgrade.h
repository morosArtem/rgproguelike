#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// ќбъ€вл€ем u8 функцию (она определена в Utf.h)
namespace sf {
    class String;
}
sf::String u8(const std::string& s);
sf::String u8(const char* s);

enum class UpgradeType
{
    HEALTH_BOOST,
    DAMAGE_BOOST,
    SPEED_BOOST,
    FIRE_RATE_BOOST,
    REGENERATION,
    PIERCING_SHOTS,
    EXPLOSIVE_SHOTS,
    LIFESTEAL,
    DOUBLE_SHOT,
    SHIELD_CHARGE,
    DASH_RESET,
    GREED,
    CRITICAL_HIT
};

struct UpgradeInfo
{
    UpgradeType type;
    std::string name;
    std::string description;
    sf::Color color;

    UpgradeInfo(UpgradeType t, const std::string& n, const std::string& d, sf::Color c)
        : type(t), name(n), description(d), color(c) {
    }
};

class Upgrade
{
public:
    Upgrade(UpgradeType type);

    UpgradeType getType() const { return m_Type; }
    const std::string& getName() const { return m_Name; }
    const std::string& getDescription() const { return m_Description; }
    sf::Color getColor() const { return m_Color; }

    static const UpgradeInfo& getInfo(UpgradeType type);
    static std::vector<UpgradeType> getRandomUpgrades(int count, const std::vector<UpgradeType>& exclude = {});

private:
    UpgradeType m_Type;
    std::string m_Name;
    std::string m_Description;
    sf::Color m_Color;

    void init();
};