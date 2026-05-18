#include "Player.h"
#include "Input.h"
#include <cmath>

static constexpr float PLAYER_SIZE = 36.f;
static constexpr float BASE_SPEED = 210.f;
static constexpr int   BASE_MAX_HP = 150;   // увеличено для лучшей выживаемости

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
{
    m_Shape.setSize({ PLAYER_SIZE, PLAYER_SIZE });
    m_Shape.setOrigin(PLAYER_SIZE / 2.f, PLAYER_SIZE / 2.f);
    m_Shape.setFillColor(sf::Color(60, 200, 90));
    m_Shape.setOutlineColor(sf::Color(20, 100, 40));
    m_Shape.setOutlineThickness(2.f);
    m_Shape.setPosition(m_Position);
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
}

void Player::handleInput()
{
    // Рывок (Dash): Space или LShift
    bool dashKey = Input::isKeyPressed(sf::Keyboard::Space) || Input::isKeyPressed(sf::Keyboard::LShift);
    if (dashKey && m_DashCooldown <= 0.f && !m_IsDashing)
    {
        sf::Vector2f dir = Input::getMovementDirection();
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.f)
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

    if (m_IsDashing)
        m_Shape.setFillColor(sf::Color(180, 255, 200));
    else if (m_IFrameTimer > 0.f)
        m_Shape.setFillColor((static_cast<int>(m_IFrameTimer * 10.f) % 2 == 0) ?
            sf::Color(255, 160, 160) : sf::Color(60, 200, 90));
    else
        m_Shape.setFillColor(sf::Color(60, 200, 90));

    m_Shape.setOutlineColor(m_HasShield ? sf::Color(80, 220, 220) : sf::Color(20, 100, 40));
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
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 1e-4f) return false;
    dir /= len;

    const float PROJ_SPEED = 650.f;
    out.emplace_back(m_Position, dir * PROJ_SPEED, m_Damage, true, sf::Color(255, 230, 80));
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
        for (auto it = m_Items.begin(); it != m_Items.end(); ++it)
        {
            if (*it == ItemType::SHIELD) { m_Items.erase(it); break; }
        }
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
