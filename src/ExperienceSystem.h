#pragma once
#include <vector>
#include "Upgrade.h"

// Статистика игрока для модификаторов
struct PlayerStats
{
    float maxHealthPercent = 100.0f;    // процент от базового
    float damagePercent = 100.0f;       // процент от базового
    float speedPercent = 100.0f;        // процент от базового
    float fireRatePercent = 100.0f;     // процент от базового

    // Специальные флаги
    bool hasRegeneration = false;
    bool hasPiercingShots = false;
    bool hasExplosiveShots = false;
    bool hasLifesteal = false;
    bool hasDoubleShot = false;
    bool hasShieldCharge = false;
    bool hasDashReset = false;
    bool hasGreed = false;
    bool hasCriticalHit = false;

    float regenRate = 0.5f;      // HP в секунду
    float lifestealPercent = 0.1f;  // 10%
    float criticalChance = 0.15f;   // 15%
    float criticalMultiplier = 2.0f; // x2 урон
    float greedMultiplier = 1.5f;    // +50% монет
    int extraProjectiles = 0;        // доп. снаряды
};

class ExperienceSystem
{
public:
    ExperienceSystem();

    void addExperience(int amount);
    void addExperienceFromKill(int scoreValue); // опыт зависит от ценности врага

    int getCurrentLevel() const { return m_Level; }
    int getCurrentExp() const { return m_CurrentExp; }
    int getExpForNextLevel() const { return getExpRequired(m_Level); }
    float getLevelProgress() const;

    bool hasPendingUpgrade() const { return m_HasPendingUpgrade; }
    void clearPendingUpgrade() { m_HasPendingUpgrade = false; }

    const std::vector<UpgradeType>& getAcquiredUpgrades() const { return m_AcquiredUpgrades; }

    // Применить улучшение
    void applyUpgrade(UpgradeType type, PlayerStats& stats);

    // Получить случайные улучшения для выбора
    std::vector<UpgradeType> getUpgradeChoices(int count = 3) const;

    // Сброс (при новой игре)
    void reset();

    // Эффекты уровня (можно расширить)
    void onLevelUp();

private:
    int getExpRequired(int level) const;

    int m_Level;
    int m_CurrentExp;
    bool m_HasPendingUpgrade;
    std::vector<UpgradeType> m_AcquiredUpgrades;

    static constexpr int BASE_EXP = 100;
    static constexpr float EXP_SCALING = 1.2f;
};