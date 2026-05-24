#pragma once
#include <SFML/Graphics.hpp>

class Projectile
{
public:
    Projectile(sf::Vector2f pos, sf::Vector2f velocity, float damage,
        bool fromPlayer, sf::Color color = sf::Color::Yellow);

    void update(float dt);
    void draw(sf::RenderWindow& window);

    sf::FloatRect getBounds() const;
    bool isExpired() const;
    bool isFromPlayer() const;
    float getDamage() const;
    sf::Vector2f getPosition() const;

    void kill();
    bool isDead() const;

    bool canPierce() const { return m_CanPierce; }
    void setCanPierce(bool pierce) { m_CanPierce = pierce; }

private:
    sf::CircleShape m_Shape;
    sf::Vector2f m_Position;
    sf::Vector2f m_Velocity;
    float m_Damage;
    bool m_FromPlayer;
    bool m_Dead;
    float m_Lifetime;
    bool m_CanPierce;
    static constexpr float MAX_LIFETIME = 3.0f;
};