#include "HUD.h"
#include "AssetManager.h"
#include "Utf.h"

HUD::HUD()
    : m_MessageTimer(0.f)
    , m_FontReady(AssetManager::instance().isFontLoaded())
{
    m_HpBarBg.setSize({ 240.f, 22.f });
    m_HpBarBg.setFillColor(sf::Color(30, 30, 30, 200));
    m_HpBarBg.setOutlineColor(sf::Color::White);
    m_HpBarBg.setOutlineThickness(2.f);
    m_HpBarBg.setPosition(16.f, 16.f);

    m_HpBarFill.setSize({ 240.f, 22.f });
    m_HpBarFill.setFillColor(sf::Color(200, 60, 60));
    m_HpBarFill.setPosition(16.f, 16.f);

    if (m_FontReady)
    {
        const auto& font = AssetManager::instance().getFont();
        m_HpText.setFont(font);
        m_HpText.setCharacterSize(14);
        m_HpText.setFillColor(sf::Color::White);
        m_HpText.setPosition(22.f, 18.f);

        m_ScoreText.setFont(font);
        m_ScoreText.setCharacterSize(18);
        m_ScoreText.setFillColor(sf::Color(255, 230, 100));
        m_ScoreText.setPosition(16.f, 48.f);

        m_HighScoreText.setFont(font);
        m_HighScoreText.setCharacterSize(14);
        m_HighScoreText.setFillColor(sf::Color(200, 200, 200));
        m_HighScoreText.setPosition(16.f, 72.f);

        m_FloorText.setFont(font);
        m_FloorText.setCharacterSize(18);
        m_FloorText.setFillColor(sf::Color::White);
        m_FloorText.setPosition(1100.f, 16.f);

        m_CoinText.setFont(font);
        m_CoinText.setCharacterSize(16);
        m_CoinText.setFillColor(sf::Color(255, 220, 80));
        m_CoinText.setPosition(16.f, 94.f);

        m_Message.setFont(font);
        m_Message.setCharacterSize(22);
        m_Message.setFillColor(sf::Color::White);
    }
}

void HUD::update(const Player& player, int score, int highScore, const Level& level)
{
    float frac = static_cast<float>(player.getHP()) / static_cast<float>(player.getMaxHP());
    if (frac < 0.f) frac = 0.f;
    m_HpBarFill.setSize({ 240.f * frac, 22.f });

    if (m_FontReady)
    {
        // Используем std::to_string, но можно добавить буфер для ещё большей скорости
        m_HpText.setString("HP: " + std::to_string(player.getHP()) + " / " + std::to_string(player.getMaxHP()));
        m_ScoreText.setString("Score: " + std::to_string(score));
        m_HighScoreText.setString("Best: " + std::to_string(highScore));
        m_FloorText.setString("Floor " + std::to_string(level.getFloorNumber()));
        m_CoinText.setString("Coins: " + std::to_string(player.getCoins()));
    }

    // Иконки предметов
    const auto& items = player.getItems();
    m_ItemIcons.clear();
    m_ItemLabels.clear();
    m_ItemIcons.reserve(items.size());
    m_ItemLabels.reserve(items.size());

    float x = 16.f;
    float y = 120.f;
    size_t idx = 0;

    for (auto type : items)
    {
        sf::RectangleShape icon({ 26.f, 26.f });
        icon.setPosition(x + idx * 32.f, y);
        icon.setFillColor(Item::getColor(type));
        icon.setOutlineColor(sf::Color::White);
        icon.setOutlineThickness(1.f);
        m_ItemIcons.push_back(std::move(icon));

        if (m_FontReady)
        {
            sf::Text t;
            t.setFont(AssetManager::instance().getFont());
            t.setString(Item::getLabel(type));
            t.setCharacterSize(14);
            t.setFillColor(sf::Color::White);
            auto lb = t.getLocalBounds();
            t.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
            t.setPosition(x + idx * 32.f + 13.f, y + 13.f);
            m_ItemLabels.push_back(std::move(t));
        }
        idx++;
    }

    // Миникарта
    const auto& nodes = level.getNodes();
    if (!nodes.empty())
    {
        int minX = 999, minY = 999, maxX = -999, maxY = -999;
        for (const auto& n : nodes)
        {
            if (n.x < minX) minX = n.x;
            if (n.y < minY) minY = n.y;
            if (n.x > maxX) maxX = n.x;
            if (n.y > maxY) maxY = n.y;
        }

        const float cell = 18.f;
        const float pad = 3.f;
        float baseX = 1280.f - (maxX - minX + 1) * (cell + pad) - 16.f;
        float baseY = 56.f;

        m_MinimapCells.clear();
        m_MinimapCells.reserve(nodes.size());

        int curIdx = level.getCurrentRoomIndex();

        for (size_t i = 0; i < nodes.size(); ++i)
        {
            const auto& n = nodes[i];
            sf::RectangleShape r({ cell, cell });
            r.setPosition(baseX + (n.x - minX) * (cell + pad),
                baseY + (n.y - minY) * (cell + pad));

            // Быстрый выбор цвета через switch
            sf::Color c;
            switch (n.type)
            {
            case RoomType::START:  c = sf::Color(100, 200, 100); break;
            case RoomType::ITEM:   c = sf::Color(255, 215, 80);  break;
            case RoomType::SHOP:   c = sf::Color(180, 120, 220); break;
            case RoomType::BOSS:   c = sf::Color(220, 60, 60);   break;
            default:               c = sf::Color(120, 120, 120); break;
            }

            if (!n.visited)
                c = sf::Color(40, 40, 40, 180);

            r.setFillColor(c);
            r.setOutlineColor((int)i == curIdx ? sf::Color::White : sf::Color(20, 20, 20));
            r.setOutlineThickness((int)i == curIdx ? 2.f : 1.f);
            m_MinimapCells.push_back(std::move(r));
        }
    }
}

void HUD::draw(sf::RenderWindow& window)
{
    window.draw(m_HpBarBg);
    window.draw(m_HpBarFill);

    for (const auto& icon : m_ItemIcons) window.draw(icon);
    for (const auto& cell : m_MinimapCells) window.draw(cell);

    if (m_FontReady)
    {
        window.draw(m_HpText);
        window.draw(m_ScoreText);
        window.draw(m_HighScoreText);
        window.draw(m_FloorText);
        window.draw(m_CoinText);
        for (const auto& t : m_ItemLabels) window.draw(t);
        if (m_MessageTimer > 0.f) window.draw(m_Message);
    }
}

void HUD::setMessage(const std::string& msg, float duration)
{
    if (!m_FontReady) return;
    m_Message.setString(u8(msg));
    auto lb = m_Message.getLocalBounds();
    m_Message.setOrigin(lb.left + lb.width / 2.f, 0.f);
    m_Message.setPosition(640.f, 640.f);
    m_MessageTimer = duration;
}

void HUD::tickMessage(float dt)
{
    if (m_MessageTimer > 0.f) m_MessageTimer -= dt;
}