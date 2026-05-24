#include "Projectile.h"

Projectile::Projectile(sf::Vector2f pos, sf::Vector2f velocity,
    float damage, bool fromPlayer, sf::Color color)
    : m_Position(pos)
    , m_Velocity(velocity)
    , m_Damage(damage)
    , m_FromPlayer(fromPlayer)
    , m_Dead(false)
    , m_Lifetime(0.f)
    , m_CanPierce(false)
{
    float radius = fromPlayer ? 5.f : 4.f;
    m_Shape.setRadius(radius);
    m_Shape.setOrigin(radius, radius);
    m_Shape.setFillColor(color);
    m_Shape.setPosition(m_Position);
}

void Projectile::update(float dt)
{
    m_Position += m_Velocity * dt;
    m_Shape.setPosition(m_Position);
    m_Lifetime += dt;
}

void Projectile::draw(sf::RenderWindow& window)
{
    window.draw(m_Shape);
}

sf::FloatRect Projectile::getBounds() const
{
    return m_Shape.getGlobalBounds();
}

bool Projectile::isExpired() const
{
    return m_Lifetime >= MAX_LIFETIME;
}

void Projectile::kill()
{
    m_Dead = true;
}

bool Projectile::isDead() const
{
    return m_Dead;
}

bool Projectile::isFromPlayer() const
{
    return m_FromPlayer;
}

float Projectile::getDamage() const
{
    return m_Damage;
}

sf::Vector2f Projectile::getPosition() const
{
    return m_Position;
}