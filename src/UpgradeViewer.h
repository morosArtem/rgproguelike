#pragma once#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Upgrade.h"

class UpgradeViewer
{
public:
    UpgradeViewer();

    void show(const std::vector<UpgradeType>& acquiredUpgrades);
    void hide();
    bool isActive() const { return m_Active; }

    void update(float dt);
    void draw(sf::RenderWindow& window);

    void setOnCloseCallback(std::function<void()> callback) { m_OnClose = callback; }

private:
    struct UpgradeEntry
    {
        UpgradeType type;
        sf::RectangleShape background;
        sf::Text name;
        sf::Text description;
        sf::Text levelText;
        sf::CircleShape iconBg;
        sf::Text iconSymbol;
        float animTime;
    };

    void createEntries();
    void updateAnimation(float dt);
    void drawEntry(sf::RenderWindow& window, UpgradeEntry& entry, int index);

    std::vector<UpgradeEntry> m_Entries;
    std::vector<UpgradeType> m_AcquiredUpgrades;

    sf::RectangleShape m_Overlay;
    sf::RectangleShape m_Panel;
    sf::Text m_Title;
    sf::Text m_Subtitle;
    sf::Text m_EmptyText;
    sf::Text m_Instruction;
    sf::RectangleShape m_ScrollBar;
    sf::RectangleShape m_ScrollThumb;

    bool m_Active;
    float m_AnimTime;
    float m_ScrollOffset;
    float m_TargetScrollOffset;
    float m_MaxScrollOffset;

    bool m_MousePressed;
    bool m_DraggingScroll;
    sf::Vector2f m_LastMousePos;

    std::function<void()> m_OnClose;

    static constexpr int WIN_WIDTH = 1280;
    static constexpr int WIN_HEIGHT = 720;
    static constexpr int ENTRY_HEIGHT = 90;
    static constexpr int ENTRIES_PER_PAGE = 6;
};