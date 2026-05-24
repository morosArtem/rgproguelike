#include "Upgrade.h"
#include "Utf.h"
#include <random>
#include <algorithm>

static const std::vector<UpgradeInfo> UPGRADE_INFOS = {
    {UpgradeType::HEALTH_BOOST,    "Health Boost",     "+20% max health",           sf::Color(80, 200, 80)},
    {UpgradeType::DAMAGE_BOOST,    "Damage Boost",     "+15% damage",                sf::Color(220, 60, 60)},
    {UpgradeType::SPEED_BOOST,     "Speed Boost",      "+10% speed",                 sf::Color(80, 150, 220)},
    {UpgradeType::FIRE_RATE_BOOST, "Fire Rate",        "+20% fire rate",             sf::Color(200, 180, 60)},
    {UpgradeType::REGENERATION,    "Regeneration",     "0.5 HP/sec regeneration",    sf::Color(100, 200, 150)},
    {UpgradeType::PIERCING_SHOTS,  "Piercing Shots",   "Shots pierce enemies",       sf::Color(150, 100, 220)},
    {UpgradeType::EXPLOSIVE_SHOTS, "Explosive Shots",  "Shots explode on hit",       sf::Color(220, 120, 40)},
    {UpgradeType::LIFESTEAL,       "Lifesteal",        "10% damage -> health",       sf::Color(200, 40, 100)},
    {UpgradeType::DOUBLE_SHOT,     "Double Shot",      "Shoot 2 projectiles",        sf::Color(100, 200, 200)},
    {UpgradeType::SHIELD_CHARGE,   "Shield Charge",    "Kills restore shield",       sf::Color(80, 200, 200)},
    {UpgradeType::DASH_RESET,      "Dash Reset",       "Kills reset dash",           sf::Color(200, 200, 80)},
    {UpgradeType::GREED,           "Greed",            "+50% coins",                 sf::Color(240, 200, 40)},
    {UpgradeType::CRITICAL_HIT,    "Critical Hit",     "15% chance 2x damage",       sf::Color(255, 100, 100)}
};

Upgrade::Upgrade(UpgradeType type)
    : m_Type(type)
{
    init();
}

void Upgrade::init()
{
    const auto& info = getInfo(m_Type);
    m_Name = info.name;
    m_Description = info.description;
    m_Color = info.color;
}

const UpgradeInfo& Upgrade::getInfo(UpgradeType type)
{
    for (const auto& info : UPGRADE_INFOS)
    {
        if (info.type == type)
            return info;
    }
    return UPGRADE_INFOS[0];
}

std::vector<UpgradeType> Upgrade::getRandomUpgrades(int count, const std::vector<UpgradeType>& exclude)
{
    std::vector<UpgradeType> available;
    available.reserve(UPGRADE_INFOS.size());

    for (const auto& info : UPGRADE_INFOS)
    {
        bool excluded = false;
        for (auto ex : exclude)
        {
            if (ex == info.type)
            {
                excluded = true;
                break;
            }
        }
        if (!excluded)
            available.push_back(info.type);
    }

    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::shuffle(available.begin(), available.end(), rng);

    if (count > static_cast<int>(available.size()))
        count = static_cast<int>(available.size());

    std::vector<UpgradeType> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i)
    {
        result.push_back(available[i]);
    }
    return result;
}