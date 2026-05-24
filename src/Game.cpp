#include "Item.h"
#include "AssetManager.h"
#include "Utf.h"
#include "Input.h"
#include "Upgrade.h"
#include "SettingsManager.h"
#include "SettingsMenu.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <random>
#include <chrono>

static const std::string HIGHSCORE_FILE = "highscore.txt";

namespace {
    inline float lengthSq(const sf::Vector2f& v) {
        return v.x * v.x + v.y * v.y;
    }

    const sf::Color COLOR_OVERLAY_DARK(0, 0, 0, 170);
    const sf::Color COLOR_MENU_SELECT(80, 80, 140);
    const sf::Color COLOR_MENU_NORMAL(40, 40, 60);
}

Game::Game()
    : m_Window(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "RPG Roguelike", sf::Style::Close | sf::Style::Titlebar)
    , m_State(GameState::MENU)
    , m_Score(0)
    , m_HighScore(0)
    , m_MenuIndex(0)
    , m_NeedRestart(false)
    , m_TransitionTimer(0.f)
    , m_BossIntroShown(false)
    , m_PortalActive(false)
    , m_PortalRoomIndex(-1)
    , m_PortalPos(0.f, 0.f)
    , m_MousePressedPrev(false)
    , m_EnterPressedPrev(false)
    , m_PausePressedPrev(false)
    , m_InteractPressedPrev(false)
    , m_ShowSettings(false)
{
    m_Window.setFramerateLimit(60);
    m_Window.setVerticalSyncEnabled(true);

    loadHighScore();

    m_MenuItems = { "New Game", "Settings", "Exit" };
    setupMenuTexts();
    m_ShowSettings = false;
    m_SettingsMainMenu.setOnCloseCallback([this]() {
        m_ShowSettings = false;
        m_State = GameState::MENU;
        refreshMenuHighlight(); });
    m_PortalShape.setRadius(25.f);
    m_PortalShape.setOrigin(25.f, 25.f);
    m_PortalShape.setFillColor(sf::Color(100, 200, 255, 180));
    m_PortalShape.setOutlineColor(sf::Color(255, 255, 255));
    m_PortalShape.setOutlineThickness(2.f);

    // Setup settings menu callbacks
    m_SettingsMainMenu.setOnCloseCallback([this]() {
        m_ShowSettings = false;
        m_State = GameState::MENU;
        });

    // Apply saved graphics settings
    SettingsManager::getInstance().applyGraphics(m_Window);
}

void Game::setupMenuTexts()
{
    if (!AssetManager::instance().isFontLoaded()) return;
    const auto& font = AssetManager::instance().getFont();

    m_Title.setFont(font);
    m_Title.setString("RPG ROGUELIKE");
    m_Title.setCharacterSize(64);
    m_Title.setFillColor(sf::Color(240, 220, 100));
    m_Title.setOutlineColor(sf::Color(80, 50, 10));
    m_Title.setOutlineThickness(3.f);
    auto tb = m_Title.getLocalBounds();
    m_Title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    m_Title.setPosition(WIN_WIDTH / 2.f, 150.f);

    m_Subtitle.setFont(font);
    m_Subtitle.setString("SFML Roguelike Project");
    m_Subtitle.setCharacterSize(22);
    m_Subtitle.setFillColor(sf::Color(180, 180, 180));
    auto sb = m_Subtitle.getLocalBounds();
    m_Subtitle.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
    m_Subtitle.setPosition(WIN_WIDTH / 2.f, 210.f);

    m_MenuTexts.clear();
    m_MenuTexts.reserve(m_MenuItems.size());
    for (size_t i = 0; i < m_MenuItems.size(); ++i)
    {
        sf::Text t;
        t.setFont(font);
        t.setString(m_MenuItems[i]);
        t.setCharacterSize(34);
        auto lb = t.getLocalBounds();
        t.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        t.setPosition(WIN_WIDTH / 2.f, 340.f + i * 60.f);
        m_MenuTexts.push_back(t);
    }

    m_HighScoreMenu.setFont(font);
    m_HighScoreMenu.setCharacterSize(22);
    m_HighScoreMenu.setFillColor(sf::Color(255, 220, 80));
    auto hb = m_HighScoreMenu.getLocalBounds();
    m_HighScoreMenu.setOrigin(hb.left + hb.width / 2.f, hb.top + hb.height / 2.f);
    m_HighScoreMenu.setPosition(WIN_WIDTH / 2.f, 280.f);

    m_Prompt.setFont(font);
    m_Prompt.setCharacterSize(18);
    m_Prompt.setFillColor(sf::Color(200, 200, 200));
    m_Prompt.setString("Controls: WASD/Arrows - move, LMB - shoot, Space/Shift - dash, Esc - pause");
    auto pb = m_Prompt.getLocalBounds();
    m_Prompt.setOrigin(pb.left + pb.width / 2.f, pb.top + pb.height / 2.f);
    m_Prompt.setPosition(WIN_WIDTH / 2.f, 640.f);

    refreshMenuHighlight();
}

void Game::refreshMenuHighlight()
{
    for (size_t i = 0; i < m_MenuTexts.size(); ++i)
    {
        if ((int)i == m_MenuIndex)
        {
            m_MenuTexts[i].setFillColor(sf::Color(255, 220, 100));
            m_MenuTexts[i].setStyle(sf::Text::Bold);
        }
        else
        {
            m_MenuTexts[i].setFillColor(sf::Color(180, 180, 180));
            m_MenuTexts[i].setStyle(sf::Text::Regular);
        }
    }
    m_HighScoreMenu.setString("Best: " + std::to_string(m_HighScore));
    auto hb = m_HighScoreMenu.getLocalBounds();
    m_HighScoreMenu.setOrigin(hb.left + hb.width / 2.f, hb.top + hb.height / 2.f);
    m_HighScoreMenu.setPosition(WIN_WIDTH / 2.f, 280.f);
}

void Game::run()
{
    sf::Clock clock;
    while (m_Window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (dt > 0.033f) dt = 0.033f;

        Input::update(m_Window);

        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents()
{
    sf::Event e;
    while (m_Window.pollEvent(e))
    {
        if (e.type == sf::Event::Closed)
        {
            m_Window.close();
        }

        // Если настройки открыты - не обрабатываем другое нажатие
        if (m_ShowSettings)
        {
            continue;
        }

        if (e.type == sf::Event::KeyPressed)
        {
            if (m_State == GameState::MENU)
            {
                // Обработка навигации по меню
                if (e.key.code == sf::Keyboard::Up || e.key.code == sf::Keyboard::W)
                {
                    m_MenuIndex = (m_MenuIndex + (int)m_MenuItems.size() - 1) % (int)m_MenuItems.size();
                    refreshMenuHighlight();
                }
                else if (e.key.code == sf::Keyboard::Down || e.key.code == sf::Keyboard::S)
                {
                    m_MenuIndex = (m_MenuIndex + 1) % (int)m_MenuItems.size();
                    refreshMenuHighlight();
                }
                else if (e.key.code == sf::Keyboard::Enter || e.key.code == sf::Keyboard::Space)
                {
                    if (m_MenuIndex == 0)
                    {
                        newGame();
                    }
                    else if (m_MenuIndex == 1)
                    {
                        // Открываем настройки
                        m_ShowSettings = true;
                        m_SettingsMainMenu.show();
                    }
                    else if (m_MenuIndex == 2)
                    {
                        m_Window.close();
                    }
                }
                else if (e.key.code == sf::Keyboard::Escape)
                {
                    m_Window.close();
                }
            }
            else if (m_State == GameState::PLAYING)
            {
                if (e.key.code == sf::Keyboard::Escape)
                {
                    m_State = GameState::PAUSED;
                    m_MenuIndex = 0;
                }
            }
            else if (m_State == GameState::PAUSED)
            {
                if (e.key.code == sf::Keyboard::Up || e.key.code == sf::Keyboard::W ||
                    e.key.code == sf::Keyboard::Down || e.key.code == sf::Keyboard::S)
                {
                    m_MenuIndex = (m_MenuIndex + 1) % 2;
                }
                else if (e.key.code == sf::Keyboard::Escape)
                {
                    m_State = GameState::PLAYING;
                }
                else if (e.key.code == sf::Keyboard::Enter || e.key.code == sf::Keyboard::Space)
                {
                    if (m_MenuIndex == 0) m_State = GameState::PLAYING;
                    else { m_State = GameState::MENU; m_MenuIndex = 0; refreshMenuHighlight(); }
                }
            }
            else if (m_State == GameState::GAME_OVER || m_State == GameState::VICTORY)
            {
                if (e.key.code == sf::Keyboard::Enter || e.key.code == sf::Keyboard::Space)
                {
                    newGame();
                }
                else if (e.key.code == sf::Keyboard::Escape)
                {
                    m_State = GameState::MENU;
                    m_MenuIndex = 0;
                    refreshMenuHighlight();
                }
            }
        }
        if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
        {
            if (m_State == GameState::MENU && !m_ShowSettings)
            {
                sf::Vector2f mp(static_cast<float>(e.mouseButton.x),
                    static_cast<float>(e.mouseButton.y));
                for (size_t i = 0; i < m_MenuTexts.size(); ++i)
                {
                    if (m_MenuTexts[i].getGlobalBounds().contains(mp))
                    {
                        m_MenuIndex = (int)i;
                        refreshMenuHighlight();
                        if (i == 0) newGame();
                        else if (i == 1)
                        {
                            m_ShowSettings = true;
                            m_SettingsMainMenu.show();
                        }
                        else if (i == 2) m_Window.close();
                        break;
                    }
                }
            }
        }
    }
}

void Game::update(float dt)
{
    // Если настройки открыты - обновляем их и выходим
    if (m_ShowSettings)
    {
        m_SettingsMainMenu.update(dt, m_Window);
        SettingsManager::getInstance().update(dt, m_Window);
        return;
    }

    switch (m_State)
    {
    case GameState::MENU:      break;
    case GameState::PLAYING:   updatePlaying(dt); break;
    case GameState::PAUSED:    break;
    case GameState::GAME_OVER: break;
    case GameState::VICTORY:   break;
    }
}

void Game::updatePlaying(float dt)
{
    const float dtClamped = std::min(dt, 0.033f);

    m_HUD.tickMessage(dtClamped);
    m_Player.handleInput();
    m_Player.update(dtClamped);
    resolvePlayerWallCollision(m_Player.getDesiredDelta());

    if (Input::isMouseButtonPressed(sf::Mouse::Left)) {
        handlePlayerShoot();
    }

    const int currentRoomIdx = m_Level.getCurrentRoomIndex();

    // Update enemies
    const int COLLISION_ITERATIONS = 2;

    for (int iter = 0; iter < COLLISION_ITERATIONS; ++iter) {
        for (size_t i = 0; i < m_Enemies.size(); ++i) {
            auto& e = m_Enemies[i];
            if (!e->isAlive()) continue;

            e->update(dtClamped, m_Player.getPosition(), m_Projectiles, m_Room);

            float margin = e->getRadius();
            sf::Vector2f pos = e->getPosition();
            pos.x = std::max(margin, std::min(pos.x,
                static_cast<float>(Room::GRID_WIDTH * Room::TILE_SIZE) - margin));
            pos.y = std::max(margin, std::min(pos.y,
                static_cast<float>(Room::GRID_HEIGHT * Room::TILE_SIZE) - margin));
            e->setPosition(pos);

            // Enemy-enemy collisions
            for (size_t j = i + 1; j < m_Enemies.size(); ++j) {
                auto& e2 = m_Enemies[j];
                if (!e2->isAlive()) continue;

                sf::Vector2f delta = e2->getPosition() - e->getPosition();
                float distSq = delta.x * delta.x + delta.y * delta.y;
                float minDist = e->getRadius() + e2->getRadius();
                float minDistSq = minDist * minDist;

                if (distSq < minDistSq && distSq > 0.001f) {
                    float dist = std::sqrt(distSq);
                    sf::Vector2f normal = delta / dist;
                    float overlap = minDist - dist;
                    sf::Vector2f correction = normal * (overlap * 0.5f);
                    e->setPosition(e->getPosition() - correction);
                    e2->setPosition(e2->getPosition() + correction);
                }
            }
        }
    }

    // Update projectiles
    for (auto& p : m_Projectiles) {
        p.update(dtClamped);
    }

    // Player projectiles vs enemies
    for (auto& proj : m_Projectiles) {
        if (proj.isDead()) continue;

        if (proj.isFromPlayer()) {
            for (auto& e : m_Enemies) {
                if (!e->isAlive()) continue;

                if (proj.getBounds().intersects(e->getBounds())) {
                    e->takeDamage(static_cast<int>(proj.getDamage()));
                    e->flashHit();

                    if (!proj.canPierce()) {
                        proj.kill();
                    }

                    if (!e->isAlive()) {
                        m_Score += e->getScoreValue();
                        m_Player.onEnemyKilled(e->getScoreValue());

                        static std::mt19937 rng(static_cast<unsigned>(
                            std::chrono::steady_clock::now().time_since_epoch().count()));
                        if (e->getType() != EnemyType::BOSS && (rng() % 100) < 20) {
                            m_WorldItems.emplace_back(Item(ItemType::COIN, e->getPosition()), currentRoomIdx);
                        }
                    }

                    break;
                }
            }
        }
    }

    // Enemy projectiles vs player
    sf::FloatRect playerBounds = m_Player.getBounds();
    for (auto& proj : m_Projectiles) {
        if (proj.isDead()) continue;

        if (!proj.isFromPlayer()) {
            if (proj.getBounds().intersects(playerBounds)) {
                m_Player.damage(static_cast<int>(proj.getDamage()));
                proj.kill();
                if (!m_Player.isAlive()) break;
            }
        }
    }

    // Projectiles vs walls
    for (auto& proj : m_Projectiles) {
        if (proj.isDead()) continue;

        sf::FloatRect bounds = proj.getBounds();
        if (m_Room.isSolid(bounds.left, bounds.top, false) ||
            m_Room.isSolid(bounds.left + bounds.width, bounds.top, false) ||
            m_Room.isSolid(bounds.left, bounds.top + bounds.height, false) ||
            m_Room.isSolid(bounds.left + bounds.width, bounds.top + bounds.height, false)) {
            proj.kill();
        }
    }

    // Remove dead projectiles
    m_Projectiles.erase(std::remove_if(m_Projectiles.begin(), m_Projectiles.end(),
        [](const Projectile& p) {
            return p.isExpired() || p.isDead()
                || p.getPosition().x < -50.f || p.getPosition().x > Room::GRID_WIDTH * Room::TILE_SIZE + 50.f
                || p.getPosition().y < -50.f || p.getPosition().y > Room::GRID_HEIGHT * Room::TILE_SIZE + 50.f;
        }), m_Projectiles.end());

    // Contact damage from enemies
    if (m_Player.isAlive())
    {
        for (auto& e : m_Enemies)
        {
            if (!e->isAlive() || !e->canContactDamage()) continue;
            if (e->getBounds().intersects(m_Player.getBounds()))
            {
                m_Player.damage(static_cast<int>(e->getDamage()));
                if (!m_Player.isAlive()) break;
            }
        }
    }

    // Pick up items
    for (auto& wi : m_WorldItems)
    {
        if (wi.collected) continue;
        if (wi.roomIndex != currentRoomIdx) continue;

        if (wi.item.getBounds().intersects(m_Player.getBounds()))
        {
            if (m_Level.getCurrentNode().type == RoomType::SHOP && wi.item.getCost() > 0)
            {
                if (Input::isKeyJustPressed(sf::Keyboard::E))
                {
                    if (m_Player.getCoins() >= wi.item.getCost())
                    {
                        m_Player.addCoins(-wi.item.getCost());
                        m_Player.applyItem(wi.item.getType());
                        wi.collected = true;
                        m_HUD.setMessage("Bought: " + Item::getDescription(wi.item.getType()));
                    }
                    else
                    {
                        m_HUD.setMessage("Not enough coins (" +
                            std::to_string(wi.item.getCost()) + ")", 1.5f);
                    }
                }
                else
                {
                    m_HUD.setMessage("Press E to buy for " +
                        std::to_string(wi.item.getCost()) + " coins", 0.1f);
                }
            }
            else
            {
                m_Player.applyItem(wi.item.getType());
                wi.collected = true;
                m_Score += 5;
                if (wi.item.getType() != ItemType::COIN)
                    m_HUD.setMessage("+ " + Item::getDescription(wi.item.getType()));
            }
        }
    }

    // Check room cleared
    if (!m_Level.getCurrentNode().cleared)
    {
        bool anyAlive = false;
        for (auto& e : m_Enemies) if (e->isAlive()) { anyAlive = true; break; }
        if (!anyAlive)
        {
            onRoomCleared();
        }
    }

    checkRoomTransitions();

    // Portal
    m_PortalActive = (m_PortalRoomIndex == m_Level.getCurrentRoomIndex());
    if (m_PortalActive)
    {
        float phase = std::sin(m_TransitionTimer * 6.f) * 0.2f + 0.8f;
        m_PortalShape.setScale(phase, phase);
        m_PortalShape.setPosition(m_PortalPos);

        if (m_Player.getBounds().intersects(m_PortalShape.getGlobalBounds()))
        {
            if (m_Level.getFloorNumber() < MAX_FLOORS)
            {
                loadFloor(m_Level.getFloorNumber() + 1);
                m_PortalActive = false;
                m_PortalRoomIndex = -1;
            }
            else
            {
                if (m_Score > m_HighScore)
                {
                    m_HighScore = m_Score;
                    saveHighScore();
                }
                m_State = GameState::VICTORY;
            }
        }
    }

    // Upgrade system
    if (m_Player.getExpSystem().hasPendingUpgrade() && !m_UpgradeUI.isActive())
    {
        auto choices = m_Player.getExpSystem().getUpgradeChoices(3);
        if (!choices.empty())
        {
            m_UpgradeUI.show(choices);
            // Эффект паузы игры при выборе улучшения
            m_TransitionTimer = 0.f;
        }
    }

    m_UpgradeUI.update(dtClamped);

    if (m_UpgradeUI.isActive())
    {
        UpgradeType selected = m_UpgradeUI.handleInput(m_Window);
        if (!m_UpgradeUI.isActive()) // UI закрылся после выбора
        {
            m_Player.applyUpgrade(selected);
            m_Player.getExpSystem().clearPendingUpgrade();

            // Визуальный фидбек
            m_HUD.setMessage("✦ " + Upgrade::getInfo(selected).name + " ✦", 2.5f);

            // Можно добавить визуальный эффект (вспышка, партиклы)
        }
    }

    // Check player death
    if (!m_Player.isAlive())
    {
        if (m_Score > m_HighScore)
        {
            m_HighScore = m_Score;
            saveHighScore();
        }
        m_State = GameState::GAME_OVER;
    }

    m_TransitionTimer += dtClamped;
    m_HUD.update(m_Player, m_Score, m_HighScore, m_Level);
}

void Game::resolvePlayerWallCollision(sf::Vector2f desired)
{
    sf::Vector2f pos = m_Player.getPosition();

    float newX = pos.x + desired.x;
    sf::FloatRect testX(newX - 18.f, pos.y - 18.f, 36.f, 36.f);
    bool blockX = m_Room.isSolid(testX.left, testX.top, false) ||
        m_Room.isSolid(testX.left + testX.width - 1, testX.top, false) ||
        m_Room.isSolid(testX.left, testX.top + testX.height - 1, false) ||
        m_Room.isSolid(testX.left + testX.width - 1, testX.top + testX.height - 1, false);
    if (!blockX) pos.x = newX;

    float newY = pos.y + desired.y;
    sf::FloatRect testY(pos.x - 18.f, newY - 18.f, 36.f, 36.f);
    bool blockY = m_Room.isSolid(testY.left, testY.top, false) ||
        m_Room.isSolid(testY.left + testY.width - 1, testY.top, false) ||
        m_Room.isSolid(testY.left, testY.top + testY.height - 1, false) ||
        m_Room.isSolid(testY.left + testY.width - 1, testY.top + testY.height - 1, false);
    if (!blockY) pos.y = newY;

    m_Player.setPosition(pos);
}

void Game::checkRoomTransitions()
{
    if (!m_Level.getCurrentNode().cleared) return;

    if (AssetManager::instance().isFontLoaded())
    {
        m_Label.setFont(AssetManager::instance().getFont());
        m_Label.setString(getLabel(type));
        m_Label.setCharacterSize(type == ItemType::COIN ? 10 : 16);
        m_Label.setFillColor(sf::Color::White);
        auto lb = m_Label.getLocalBounds();
        m_Label.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        m_Label.setPosition(m_Position);
    }
}

void Game::handlePlayerShoot()
{
    sf::Vector2f target = Input::getMouseWorldPosition();
    m_Player.tryShoot(target, m_Projectiles);
}

void Game::newGame()
{
    m_Player.reset();
    m_Score = 0;
    m_Projectiles.clear();
    m_Enemies.clear();
    m_WorldItems.clear();
    m_BossIntroShown = false;
    m_PortalActive = false;
    m_PortalRoomIndex = -1;
    m_UpgradeUI.hide();
    m_ShowSettings = false;

    loadFloor(1);
    m_State = GameState::PLAYING;
}

void Game::loadFloor(int floorNum)
{
    m_PortalActive = false;
    m_PortalRoomIndex = -1;
    std::random_device rd;
    unsigned int seed = rd() ^ (static_cast<unsigned>(floorNum) * 0x9E3779B9u);
    m_Level.generate(floorNum, seed);
    loadCurrentRoom();

    sf::Vector2f center = m_Room.getCenter();
    m_Player.setPosition(center);
    m_HUD.setMessage("Floor " + std::to_string(floorNum));
}

void Game::loadCurrentRoom()
{
    m_Projectiles.clear();
    m_Enemies.clear();

    auto doors = m_Level.getCurrentDoors();
    RoomType type = m_Level.getCurrentNode().type;
    int currentRoomIdx = m_Level.getCurrentRoomIndex();

    unsigned int roomSeed = static_cast<unsigned int>(
        currentRoomIdx * 131
        + m_Level.getFloorNumber() * 977
        + m_Level.getCurrentNode().x * 31
        + m_Level.getCurrentNode().y * 17
        + 1);

    m_Room.generate(type, doors, roomSeed);

    // Clean up collected items
    for (auto it = m_WorldItems.begin(); it != m_WorldItems.end(); )
    {
        if (it->roomIndex == currentRoomIdx && it->collected)
        {
            it = m_WorldItems.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (!m_Level.getCurrentNode().cleared)
    {
        bool hasItems = false;
        for (auto& wi : m_WorldItems)
        {
            if (wi.roomIndex == currentRoomIdx && !wi.collected)
            {
                hasItems = true;
                break;
            }
        }

        if (!hasItems)
        {
            spawnRoomContent();
        }
    }

    m_Room.setDoorsLocked(!m_Level.getCurrentNode().cleared);

    if (type == RoomType::BOSS && !m_Level.getCurrentNode().cleared && !m_BossIntroShown)
    {
        m_HUD.setMessage("BOSS! Prepare yourself.", 2.5f);
        m_BossIntroShown = true;
    }
    else if (type == RoomType::SHOP)
    {
        m_HUD.setMessage("Shop: Approach item and press E", 2.5f);
    }
    else if (type == RoomType::ITEM)
    {
        m_HUD.setMessage("Item Room", 1.5f);
    }
}

std::string Item::getDescription(ItemType type)
{
    std::mt19937 rng(static_cast<unsigned>(
        m_Level.getCurrentRoomIndex() * 7919
        + m_Level.getFloorNumber() * 101
        + m_Level.getCurrentNode().x * 17
        + m_Level.getCurrentNode().y * 31));

    RoomType type = m_Level.getCurrentNode().type;
    int floor = m_Level.getFloorNumber();
    int currentRoomIdx = m_Level.getCurrentRoomIndex();

    auto getEnemyRadius = [](EnemyType et) -> float {
        switch (et) {
        case EnemyType::SLIME:     return 20.f;
        case EnemyType::SHOOTER:   return 18.f;
        case EnemyType::BERSERKER: return 24.f;
        case EnemyType::BOSS:      return 42.f;
        default:                   return 16.f;
        }
        };

    if (type == RoomType::NORMAL && !m_Level.getCurrentNode().cleared)
    {
        int baseCount = 2 + (floor - 1);
        int enemyCount = baseCount + (int)(rng() % 2);
        float defaultRadius = getEnemyRadius(EnemyType::BERSERKER);
        auto points = m_Room.getSpawnPoints(enemyCount,
            static_cast<int>(m_Level.getCurrentRoomIndex() + floor * 31),
            defaultRadius);

        for (auto& p : points)
        {
            int roll = (int)(rng() % 100);
            EnemyType et;
            if (roll < 55) et = EnemyType::SLIME;
            else if (roll < 85) et = EnemyType::SHOOTER;
            else et = EnemyType::BERSERKER;
            m_Enemies.push_back(std::make_unique<Enemy>(et, p.x, p.y, floor));
        }
    }
    else if (type == RoomType::BOSS && !m_Level.getCurrentNode().cleared)
    {
        sf::Vector2f c = m_Room.getCenter();
        m_Enemies.push_back(std::make_unique<Enemy>(EnemyType::BOSS, c.x, c.y - 80.f, floor));
    }
    else if (type == RoomType::ITEM && !m_Level.getCurrentNode().cleared)
    {
        auto c = m_Room.getCenter();
        int roll = (int)(rng() % 4);
        ItemType it;
        switch (roll)
        {
        case 0: it = ItemType::HEALTH; break;
        case 1: it = ItemType::DAMAGE_UP; break;
        case 2: it = ItemType::SPEED_UP; break;
        default: it = ItemType::SHIELD; break;
        }
        m_WorldItems.emplace_back(Item(it, c), currentRoomIdx);
    }
    else if (type == RoomType::SHOP && !m_Level.getCurrentNode().cleared)
    {
        float baseX = m_Room.getCenter().x - 140.f;
        float y = m_Room.getCenter().y;
        m_WorldItems.emplace_back(Item(ItemType::HEALTH, sf::Vector2f(baseX, y), 5), currentRoomIdx);
        m_WorldItems.emplace_back(Item(ItemType::DAMAGE_UP, sf::Vector2f(baseX + 140, y), 8), currentRoomIdx);
        m_WorldItems.emplace_back(Item(ItemType::SHIELD, sf::Vector2f(baseX + 280, y), 10), currentRoomIdx);
        m_Level.markCurrentCleared();
    }
    return "";
}

sf::Color Item::getColor(ItemType type)
{
    m_Level.markCurrentCleared();
    m_Room.setDoorsLocked(false);
    m_Score += 50;

    RoomType type = m_Level.getCurrentNode().type;
    int currentRoomIdx = m_Level.getCurrentRoomIndex();

    if (type == RoomType::BOSS)
    {
        m_Score += 100;
        m_Player.getExpSystem().addExperience(200);
        auto c = m_Room.getCenter();

        m_WorldItems.emplace_back(Item(ItemType::HEALTH, c), currentRoomIdx);

        m_PortalRoomIndex = m_Level.getCurrentRoomIndex();
        m_PortalActive = true;
        m_PortalPos = c + sf::Vector2f(0.f, -50.f);
        m_HUD.setMessage("Portal opened! Go to next floor.", 3.f);
    }
    else
    {
        m_HUD.setMessage("Room cleared (+50)", 1.2f);
    }
}

void Game::render()
{
    m_Window.clear(sf::Color(18, 16, 24));

    if (m_State == GameState::MENU && !m_ShowSettings)
    {
        drawMenu();
    }
    else if (!m_ShowSettings)
    {
        drawPlaying();
        if (m_State == GameState::PAUSED)
        {
            drawOverlay("PAUSED", "Enter - resume, Esc - menu",
                sf::Color(230, 230, 230));
        }
        else if (m_State == GameState::GAME_OVER)
        {
            drawOverlay("GAME OVER",
                "Score: " + std::to_string(m_Score) +
                "   Best: " + std::to_string(m_HighScore) +
                "\nEnter - new game, Esc - menu",
                sf::Color(230, 80, 80));
        }
        else if (m_State == GameState::VICTORY)
        {
            drawOverlay("VICTORY!",
                "Score: " + std::to_string(m_Score) +
                "   Best: " + std::to_string(m_HighScore) +
                "\nEnter - new game, Esc - menu",
                sf::Color(120, 240, 140));
        }
    }

    // Рисуем настройки поверх всего
    if (m_ShowSettings)
    {
        m_SettingsMainMenu.draw(m_Window);
        SettingsManager::getInstance().draw(m_Window);
    }

    m_Window.display();
}
void Game::drawMenu()
{
    if (!AssetManager::instance().isFontLoaded())
    {
        sf::RectangleShape bg({ WIN_WIDTH, WIN_HEIGHT });
        bg.setFillColor(sf::Color(20, 20, 30));
        m_Window.draw(bg);
        for (size_t i = 0; i < m_MenuItems.size(); ++i)
        {
            sf::RectangleShape btn({ 300.f, 50.f });
            btn.setOrigin(150.f, 25.f);
            btn.setPosition(WIN_WIDTH / 2.f, 340.f + i * 60.f);
            btn.setFillColor((int)i == m_MenuIndex ? COLOR_MENU_SELECT : COLOR_MENU_NORMAL);
            m_Window.draw(btn);
        }
        return;
    }

    m_Window.draw(m_Title);
    m_Window.draw(m_Subtitle);
    m_Window.draw(m_HighScoreMenu);
    for (auto& t : m_MenuTexts) m_Window.draw(t);
    m_Window.draw(m_Prompt);
}

void Game::drawPlaying()
{
    m_Room.draw(m_Window);

    int currentRoomIdx = m_Level.getCurrentRoomIndex();
    for (auto& wi : m_WorldItems)
    {
        if (wi.roomIndex == currentRoomIdx && !wi.collected)
        {
            wi.item.draw(m_Window);
        }
    }

    for (auto& p : m_Projectiles) p.draw(m_Window);
    for (auto& e : m_Enemies) e->draw(m_Window);
    m_Player.draw(m_Window);
    if (m_PortalActive) m_Window.draw(m_PortalShape);
    m_HUD.draw(m_Window);

    if (m_UpgradeUI.isActive())
        m_UpgradeUI.draw(m_Window);
}

void Game::drawOverlay(const std::string& title, const std::string& subtitle, sf::Color titleColor)
{
    sf::RectangleShape overlay({ (float)WIN_WIDTH, (float)WIN_HEIGHT });
    overlay.setFillColor(COLOR_OVERLAY_DARK);
    m_Window.draw(overlay);

    if (!AssetManager::instance().isFontLoaded()) return;
    const auto& font = AssetManager::instance().getFont();

    sf::Text t;
    t.setFont(font);
    t.setString(title);
    t.setCharacterSize(72);
    t.setFillColor(titleColor);
    t.setOutlineColor(sf::Color::Black);
    t.setOutlineThickness(3.f);
    auto tb = t.getLocalBounds();
    t.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    t.setPosition(WIN_WIDTH / 2.f, 220.f);
    m_Window.draw(t);

    sf::Text s;
    s.setFont(font);
    s.setString(subtitle);
    s.setCharacterSize(22);
    s.setFillColor(sf::Color(230, 230, 230));
    auto sb = s.getLocalBounds();
    s.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
    s.setPosition(WIN_WIDTH / 2.f, 330.f);
    m_Window.draw(s);

    if (m_State == GameState::PAUSED)
    {
        const char* items[] = { "Resume", "Exit to menu" };
        for (int i = 0; i < 2; ++i)
        {
            sf::Text mi;
            mi.setFont(font);
            mi.setString(items[i]);
            mi.setCharacterSize(28);
            mi.setFillColor(i == m_MenuIndex ? sf::Color(255, 220, 100) : sf::Color(200, 200, 200));
            auto lb = mi.getLocalBounds();
            mi.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
            mi.setPosition(WIN_WIDTH / 2.f, 420.f + i * 42.f);
            m_Window.draw(mi);
        }
    }
}

void Game::loadHighScore()
{
    std::ifstream f(HIGHSCORE_FILE);
    if (f.is_open())
    {
        f >> m_HighScore;
        if (!f) m_HighScore = 0;
    }
    else
    {
    case ItemType::HEALTH:    return sf::Color(50, 180, 50);
    case ItemType::DAMAGE_UP: return sf::Color(200, 50, 50);
    case ItemType::SPEED_UP:  return sf::Color(50, 100, 220);
    case ItemType::SHIELD:    return sf::Color(50, 200, 200);
    case ItemType::COIN:      return sf::Color(240, 200, 40);
    }
    return sf::Color::White;
}

std::string Item::getLabel(ItemType type)
{
    switch (type)
    {
    case ItemType::HEALTH:    return "+";
    case ItemType::DAMAGE_UP: return "D";
    case ItemType::SPEED_UP:  return "S";
    case ItemType::SHIELD:    return "O";
    case ItemType::COIN:      return "$";
    }
    return "?";
}

void Game::showUpgradeUI()
{
    auto choices = m_Player.getExpSystem().getUpgradeChoices(3);
    if (!choices.empty())
    {
        m_UpgradeUI.show(choices);

        // Визуальный эффект при открытии
        m_TransitionTimer = 0.f;

        // Создаем партиклы или звук (опционально)
        // Например, можно добавить звук получения уровня
    }
}