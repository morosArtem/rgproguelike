#pragma once
#include "Entity.h"
#include "Projectile.h"
#include "Room.h"
#include <vector>

enum class EnemyType
{
    SLIME,
    SHOOTER,
    BERSERKER,
    BOSS
};

class Enemy : public Entity
{
public:
    Enemy(EnemyType type, float x, float y, int difficulty = 1);

    void update(float dt, sf::Vector2f playerPos, std::vector<Projectile>& out, const Room& room);
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;

    EnemyType getType() const;
    float getDamage() const;
    int getScoreValue() const;

    bool canContactDamage() const;
    void flashHit();
    float getRadius() const { return m_Radius; }

private:
    EnemyType m_Type;
    sf::CircleShape m_Shape;
    float m_Damage;
    float m_DetectionRadius;
    float m_AttackCooldown;
    float m_AttackTimer;
    float m_HitFlashTimer;

    bool  m_Charging;
    float m_ChargeCooldown;
    sf::Vector2f m_ChargeDir;

    int m_BossPhase;
    float m_PreferredDistance;
    float m_Radius;
    int m_ScoreValue;

    void setupByType(int difficulty);
};
