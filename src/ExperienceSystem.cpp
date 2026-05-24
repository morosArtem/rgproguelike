#include "ExperienceSystem.h"
#include <cmath>
#include <random>

static std::random_device g_Rd;
static std::mt19937 g_Rng(g_Rd());

ExperienceSystem::ExperienceSystem()
    : m_Level(1)
    , m_CurrentExp(0)
    , m_HasPendingUpgrade(false)
{
}

int ExperienceSystem::getExpRequired(int level) const
{
    // Ѕыстрее: предвычисленные значени€ дл€ уровней 1-20
    static const int s_ExpTable[20] = {
        100, 120, 144, 172, 207, 248, 298, 357, 429, 515,
        618, 741, 889, 1067, 1280, 1536, 1843, 2212, 2654, 3185
    };
    if (level >= 1 && level <= 20) return s_ExpTable[level - 1];
    return static_cast<int>(BASE_EXP * std::pow(EXP_SCALING, level - 1));
}

void ExperienceSystem::addExperience(int amount)
{
    if (amount <= 0 || m_Level >= 20) return;

    m_CurrentExp += amount;
    int required = getExpRequired(m_Level);

    while (m_CurrentExp >= required && m_Level < 20)
    {
        m_CurrentExp -= required;
        m_Level++;
        onLevelUp();
        required = getExpRequired(m_Level);
    }
}

void ExperienceSystem::addExperienceFromKill(int scoreValue)
{
    addExperience(scoreValue);
}

float ExperienceSystem::getLevelProgress() const
{
    return static_cast<float>(m_CurrentExp) / getExpRequired(m_Level);
}

std::vector<UpgradeType> ExperienceSystem::getUpgradeChoices(int count) const
{
    return Upgrade::getRandomUpgrades(count, m_AcquiredUpgrades);
}

void ExperienceSystem::applyUpgrade(UpgradeType type, PlayerStats& stats)
{
    m_AcquiredUpgrades.push_back(type);

    switch (type)
    {
    case UpgradeType::HEALTH_BOOST:   stats.maxHealthPercent += 20.0f; break;
    case UpgradeType::DAMAGE_BOOST:   stats.damagePercent += 15.0f; break;
    case UpgradeType::SPEED_BOOST:    stats.speedPercent += 10.0f; break;
    case UpgradeType::FIRE_RATE_BOOST:stats.fireRatePercent += 20.0f; break;
    case UpgradeType::REGENERATION:   stats.hasRegeneration = true; break;
    case UpgradeType::PIERCING_SHOTS: stats.hasPiercingShots = true; break;
    case UpgradeType::EXPLOSIVE_SHOTS:stats.hasExplosiveShots = true; break;
    case UpgradeType::LIFESTEAL:      stats.hasLifesteal = true; break;
    case UpgradeType::DOUBLE_SHOT:    stats.hasDoubleShot = true; stats.extraProjectiles++; break;
    case UpgradeType::SHIELD_CHARGE:  stats.hasShieldCharge = true; break;
    case UpgradeType::DASH_RESET:     stats.hasDashReset = true; break;
    case UpgradeType::GREED:          stats.hasGreed = true; break;
    case UpgradeType::CRITICAL_HIT:   stats.hasCriticalHit = true; break;
    default: break;
    }

    m_HasPendingUpgrade = true;
}

void ExperienceSystem::onLevelUp()
{
    m_HasPendingUpgrade = true;
}

void ExperienceSystem::reset()
{
    m_Level = 1;
    m_CurrentExp = 0;
    m_HasPendingUpgrade = false;
    m_AcquiredUpgrades.clear();
}