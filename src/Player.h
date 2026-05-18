#pragma once
#include "Entity.h"
#include "Projectile.h"
#include "Item.h"
#include <vector>

class Player : public Entity
{
public:
    Player();

    void reset();
    void handleInput();
    void update(float dt);
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;

    bool tryShoot(sf::Vector2f worldTarget, std::vector<Projectile>& outProjectiles);
    void damage(int dmg);
    void applyItem(ItemType type);

    float getDamage() const;
    void setDamage(float d);
    int getCoins() const;
    void addCoins(int amount);
    bool hasShield() const;

    sf::Vector2f getDesiredDelta() const;
    void applyMovement(sf::Vector2f delta);

    const std::vector<ItemType>& getItems() const;

    bool isDashing() const;
    bool isInvincible() const;

private:
    sf::RectangleShape m_Shape;
    sf::Vector2f m_Desired;

    float m_Damage;
    float m_FireRate;
    float m_FireTimer;
    int m_Coins;

    float m_DashCooldown;
    float m_DashTimer;
    float m_DashDuration;
    float m_DashSpeedMult;
    bool m_IsDashing;
    sf::Vector2f m_DashDir;

    float m_IFrameTimer;
    static constexpr float I_FRAME_DURATION = 1.0f;

    bool m_HasShield;
    std::vector<ItemType> m_Items;
};
