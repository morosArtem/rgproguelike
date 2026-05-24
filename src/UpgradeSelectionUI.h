#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Upgrade.h"

class UpgradeSelectionUI
{
public:
    UpgradeSelectionUI();

    void show(const std::vector<UpgradeType>& upgrades);
    void hide();
    bool isActive() const { return m_Active; }

    void update(float dt);
    void draw(sf::RenderWindow& window);

    UpgradeType handleInput(const sf::RenderWindow& window);

private:
    struct UpgradeCard
    {
        UpgradeType type;
        sf::RectangleShape background;
        sf::RectangleShape glow;
        sf::Text name;
        sf::Text description;
        sf::Text levelText;
        sf::Sprite icon;
        bool hovered;
        float hoverAnim;
        float selectionAnim;
    };

    void createCards(const std::vector<UpgradeType>& upgrades);
    void updateCardAnimation(float dt);
    void drawCard(sf::RenderWindow& window, UpgradeCard& card, int index);
    sf::Texture createGradientTexture(int width, int height, sf::Color color1, sf::Color color2);

    std::vector<UpgradeCard> m_Cards;
    sf::RectangleShape m_Overlay;
    sf::RectangleShape m_TitleBg;
    sf::Text m_Title;
    sf::Text m_Subtitle;
    sf::Text m_Instruction;
    bool m_Active;
    float m_AnimTime;
    float m_SelectionTimer;
    int m_SelectedIndex;

    static constexpr int CARD_WIDTH = 320;
    static constexpr int CARD_HEIGHT = 240;
    static constexpr int CARD_SPACING = 40;
};