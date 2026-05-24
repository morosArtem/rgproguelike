#include "Player.h"
#include "Input.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <random>
#include <chrono>

static constexpr float PLAYER_SIZE = 36.f;
static constexpr float BASE_SPEED = 210.f;
static constexpr int   BASE_MAX_HP = 150;

// Глобальный RNG для crit (вынесен из canCrit для производительности)
static std::random_device g_Rd;
static std::mt19937 g_Rng(g_Rd());
static std::uniform_real_distribution<float> g_CritDist(0.f, 1.f);

Player::Player()
    : Entity(640.f, 360.f, BASE_SPEED, BASE_MAX_HP)
    , m_Damage(25.f)
    , m_FireRate(0.3f)
    , m_FireTimer(0.f)
    , m_Coins(0)
    , m_DashCooldown(0.f)
    , m_DashTimer(0.f)
    , m_DashDuration(0.15f)
    , m_DashSpeedMult(3.0f)
    , m_IsDashing(false)
    , m_DashDir(0.f, 0.f)
    , m_IFrameTimer(0.f)
    , m_HasShield(false)
    , m_RegenAccumulator(0.f)
{
    m_Shape.setSize({ PLAYER_SIZE, PLAYER_SIZE });
    m_Shape.setOrigin(PLAYER_SIZE / 2.f, PLAYER_SIZE / 2.f);
    m_Shape.setFillColor(sf::Color(60, 200, 90));
    m_Shape.setOutlineColor(sf::Color(20, 100, 40));
    m_Shape.setOutlineThickness(2.f);
    m_Shape.setPosition(m_Position);
    resetStats();
}

void Player::reset()
{
    m_Position = { 640.f, 360.f };
    m_Shape.setPosition(m_Position);
    m_HP = BASE_MAX_HP;
    m_MaxHP = BASE_MAX_HP;
    m_Alive = true;
    m_Speed = BASE_SPEED;
    m_Damage = 25.f;
    m_FireRate = 0.3f;
    m_FireTimer = 0.f;
    m_Coins = 0;
    m_DashCooldown = 0.f;
    m_DashTimer = 0.f;
    m_IsDashing = false;
    m_IFrameTimer = 0.f;
    m_HasShield = false;
    m_Items.clear();
    m_ExpSystem.reset();
    resetStats();
}

void Player::resetStats()
{
    m_Stats = PlayerStats();
    m_HP = m_MaxHP;
    m_Speed = BASE_SPEED;
    m_Damage = 25.f;
    m_FireRate = 0.3f;
}

void Player::handleInput()
{
    bool dashKey = Input::isKeyPressed(sf::Keyboard::Space) || Input::isKeyPressed(sf::Keyboard::LShift);
    if (dashKey && m_DashCooldown <= 0.f && !m_IsDashing)
    {
        sf::Vector2f dir = Input::getMovementDirection();
        if (dir.x != 0.f || dir.y != 0.f)
        {
            m_DashDir = dir;
            m_IsDashing = true;
            m_DashTimer = m_DashDuration;
            m_DashCooldown = 1.0f;
        }
    }
}

void Player::update(float dt)
{
    if (m_FireTimer > 0.f)      m_FireTimer -= dt;
    if (m_DashCooldown > 0.f)   m_DashCooldown -= dt;
    if (m_IFrameTimer > 0.f)    m_IFrameTimer -= dt;

    sf::Vector2f dir(0.f, 0.f);
    if (m_IsDashing)
    {
        dir = m_DashDir;
        m_DashTimer -= dt;
        if (m_DashTimer <= 0.f) m_IsDashing = false;
    }
    else
    {
        dir = Input::getMovementDirection();
    }

    float speed = m_Speed * (m_IsDashing ? m_DashSpeedMult : 1.f);
    m_Desired = dir * speed * dt;

    // Оптимизирована смена цвета (меньше операций в кадр)
    if (m_IsDashing)
        m_Shape.setFillColor(sf::Color(180, 255, 200));
    else if (m_IFrameTimer > 0.f)
        m_Shape.setFillColor((static_cast<int>(m_IFrameTimer * 10.f) & 1) ?
            sf::Color(255, 160, 160) : sf::Color(60, 200, 90));
    else
        m_Shape.setFillColor(sf::Color(60, 200, 90));

    m_Shape.setOutlineColor(m_HasShield ? sf::Color(80, 220, 220) : sf::Color(20, 100, 40));

    updateRegeneration(dt);
}

void Player::updateRegeneration(float dt)
{
    if (m_Stats.hasRegeneration && m_HP < m_MaxHP && isAlive())
    {
        m_RegenAccumulator += m_Stats.regenRate * dt;
        if (m_RegenAccumulator >= 1.0f)
        {
            int healAmount = static_cast<int>(m_RegenAccumulator);
            heal(healAmount);
            m_RegenAccumulator -= static_cast<float>(healAmount);
        }
    }
}

void Player::draw(sf::RenderWindow& window)
{
    m_Shape.setPosition(m_Position);
    window.draw(m_Shape);
}

sf::FloatRect Player::getBounds() const
{
    return sf::FloatRect(m_Position.x - PLAYER_SIZE / 2.f, m_Position.y - PLAYER_SIZE / 2.f,
        PLAYER_SIZE, PLAYER_SIZE);
}

bool Player::tryShoot(sf::Vector2f worldTarget, std::vector<Projectile>& out)
{
    if (m_FireTimer > 0.f) return false;

    sf::Vector2f dir = worldTarget - m_Position;
    float lenSq = dir.x * dir.x + dir.y * dir.y;
    if (lenSq < 1e-4f) return false;

    float len = std::sqrt(lenSq);
    dir /= len;

    const float PROJ_SPEED = 650.f;
    float damage = m_Damage;

    bool crit = canCrit();
    if (crit)
        damage *= m_Stats.criticalMultiplier;

    out.reserve(out.size() + 3);  // Предвыделение памяти
    out.emplace_back(m_Position, dir * PROJ_SPEED, damage, true,
        crit ? sf::Color(255, 100, 100) : sf::Color(255, 230, 80));

    if (m_Stats.hasDoubleShot)
    {
        float angle = 0.15f;
        float c = std::cos(angle), s = std::sin(angle);
        sf::Vector2f dir2 = { dir.x * c - dir.y * s, dir.x * s + dir.y * c };
        sf::Vector2f dir3 = { dir.x * c + dir.y * s, dir.x * -s + dir.y * c };
        out.emplace_back(m_Position, dir2 * PROJ_SPEED, damage * 0.8f, true,
            sf::Color(255, 200, 100));
        out.emplace_back(m_Position, dir3 * PROJ_SPEED, damage * 0.8f, true,
            sf::Color(255, 200, 100));
    }

    out.back().setCanPierce(m_Stats.hasPiercingShots);

    m_FireTimer = m_FireRate;
    return true;
}

void Player::damage(int dmg)
{
    if (m_IFrameTimer > 0.f) return;

    if (m_HasShield)
    {
        m_HasShield = false;
        m_IFrameTimer = I_FRAME_DURATION;
        auto it = std::find(m_Items.begin(), m_Items.end(), ItemType::SHIELD);
        if (it != m_Items.end()) m_Items.erase(it);
        return;
    }

    takeDamage(dmg);
    m_IFrameTimer = I_FRAME_DURATION;
}

void Player::applyItem(ItemType type)
{
    switch (type)
    {
    case ItemType::HEALTH:    heal(30); break;
    case ItemType::DAMAGE_UP: m_Damage += 10.f; m_Items.push_back(type); break;
    case ItemType::SPEED_UP:  m_Speed += 50.f;  m_Items.push_back(type); break;
    case ItemType::SHIELD:    m_HasShield = true; m_Items.push_back(type); break;
    case ItemType::COIN:      m_Coins += 1; break;
    }
}

void Player::applyUpgrade(UpgradeType type)
{
    m_ExpSystem.applyUpgrade(type, m_Stats);

    m_MaxHP = getModifiedMaxHP();
    if (m_HP > m_MaxHP) m_HP = m_MaxHP;
    m_Speed = getModifiedSpeed();
    m_Damage = getModifiedDamage();
    m_FireRate = getModifiedFireRate();
}

void Player::onEnemyKilled(int scoreValue)
{
    m_ExpSystem.addExperienceFromKill(scoreValue);

    int coinsGain = 1;
    if (m_Stats.hasGreed)
        coinsGain = static_cast<int>(static_cast<float>(coinsGain) * m_Stats.greedMultiplier);
    m_Coins += coinsGain;

    if (m_Stats.hasShieldCharge && !m_HasShield)
    {
        m_HasShield = true;
        m_Items.push_back(ItemType::SHIELD);
    }

    if (m_Stats.hasDashReset)
    {
        m_DashCooldown = 0.f;
    }
}

bool Player::canCrit() const
{
    if (!m_Stats.hasCriticalHit) return false;
    return g_CritDist(g_Rng) < m_Stats.criticalChance;
}

float Player::getGreedMultiplier() const
{
    return m_Stats.hasGreed ? m_Stats.greedMultiplier : 1.0f;
}

float Player::getModifiedDamage() const
{
    return 25.f * (m_Stats.damagePercent / 100.0f);
}

float Player::getModifiedSpeed() const
{
    return BASE_SPEED * (m_Stats.speedPercent / 100.0f);
}

float Player::getModifiedFireRate() const
{
    return 0.3f * (100.0f / m_Stats.fireRatePercent);
}

int Player::getModifiedMaxHP() const
{
    return static_cast<int>(static_cast<float>(BASE_MAX_HP) * (m_Stats.maxHealthPercent / 100.0f));
}

float Player::getDamage() const { return m_Damage; }
void Player::setDamage(float d) { m_Damage = d; }
int Player::getCoins() const { return m_Coins; }
void Player::addCoins(int amount) { m_Coins += amount; }
bool Player::hasShield() const { return m_HasShield; }
sf::Vector2f Player::getDesiredDelta() const { return m_Desired; }
void Player::applyMovement(sf::Vector2f delta) { m_Position += delta; m_Shape.setPosition(m_Position); }
const std::vector<ItemType>& Player::getItems() const { return m_Items; }
bool Player::isDashing() const { return m_IsDashing; }
bool Player::isInvincible() const { return m_IFrameTimer > 0.f; }