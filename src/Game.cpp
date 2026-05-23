#include "Item.h"
#include "AssetManager.h"
#include "Utf.h"
#include "Input.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <random>
#include <chrono>

static const std::string HIGHSCORE_FILE = "highscore.txt";

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
{
    m_Window.setFramerateLimit(60);
    m_Window.setVerticalSyncEnabled(true);

    loadHighScore();

    m_MenuItems = { "Новая игра", "Выход" };
    setupMenuTexts();

    // Настройка портала
    m_PortalShape.setRadius(25.f);
    m_PortalShape.setOrigin(25.f, 25.f);
    m_PortalShape.setFillColor(sf::Color(100, 200, 255, 180));
    m_PortalShape.setOutlineColor(sf::Color(255, 255, 255));
    m_PortalShape.setOutlineThickness(2.f);
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
    m_Subtitle.setString(u8("Курсовой проект на SFML"));
    m_Subtitle.setCharacterSize(22);
    m_Subtitle.setFillColor(sf::Color(180, 180, 180));
    auto sb = m_Subtitle.getLocalBounds();
    m_Subtitle.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
    m_Subtitle.setPosition(WIN_WIDTH / 2.f, 210.f);

    m_MenuTexts.clear();
    for (size_t i = 0; i < m_MenuItems.size(); ++i)
    {
        sf::Text t;
        t.setFont(font);
        t.setString(u8(m_MenuItems[i]));
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
    m_Prompt.setString(u8("Управление: WASD/стрелки — движение, ЛКМ — стрелять, Space/Shift — рывок, Esc — пауза"));
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
    m_HighScoreMenu.setString(u8("Рекорд: " + std::to_string(m_HighScore)));
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
        if (dt > 0.1f) dt = 0.1f;

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
        if (e.type == sf::Event::KeyPressed)
        {
            if (m_State == GameState::MENU)
            {
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
                    if (m_MenuIndex == 0) newGame();
                    else if (m_MenuIndex == 1) m_Window.close();
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
                if (e.key.code == sf::Keyboard::Up || e.key.code == sf::Keyboard::W)
                {
                    m_MenuIndex = (m_MenuIndex + 1) % 2;
                }
                else if (e.key.code == sf::Keyboard::Down || e.key.code == sf::Keyboard::S)
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
            if (m_State == GameState::MENU)
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
                        else if (i == 1) m_Window.close();
                    }
                }
            }
        }
    }
}

void Game::update(float dt)
{
    switch (m_State)
    {
    case GameState::MENU:      updateMenu(dt); break;
    case GameState::PLAYING:   updatePlaying(dt); break;
    case GameState::PAUSED:    updatePaused(dt); break;
    case GameState::GAME_OVER: updateGameOver(dt); break;
    case GameState::VICTORY:   updateVictory(dt); break;
    }
}

void Game::updateMenu(float /*dt*/) { /* events only */ }
void Game::updatePaused(float /*dt*/) { /* events only */ }
void Game::updateGameOver(float /*dt*/) { /* events only */ }
void Game::updateVictory(float /*dt*/) { /* events only */ }

void Game::updatePlaying(float dt)
{
    m_HUD.tickMessage(dt);

    m_Player.handleInput();
    m_Player.update(dt);

    resolvePlayerWallCollision(m_Player.getDesiredDelta());

    if (Input::isMouseButtonPressed(sf::Mouse::Left))
    {
        handlePlayerShoot();
    }

    int currentRoomIdx = m_Level.getCurrentRoomIndex();  // ← ДОБАВЬТЕ ЭТУ СТРОЧКУ

    // Обновление врагов
    for (auto& e : m_Enemies)
    {
        if (!e->isAlive()) continue;
        e->update(dt, m_Player.getPosition(), m_Projectiles, m_Room);
        sf::Vector2f p = e->getPosition();
        float margin = e->getRadius();
        p.x = std::max(margin, std::min(p.x, static_cast<float>(Room::GRID_WIDTH * Room::TILE_SIZE) - margin));
        p.y = std::max(margin, std::min(p.y, static_cast<float>(Room::GRID_HEIGHT * Room::TILE_SIZE) - margin));
        e->setPosition(p);
    }

    // Коллизии между врагами
    const int COLLISION_ITERATIONS = 3;
    for (int iter = 0; iter < COLLISION_ITERATIONS; ++iter) {
        for (size_t i = 0; i < m_Enemies.size(); ++i) {
            if (!m_Enemies[i]->isAlive()) continue;
            sf::Vector2f pos1 = m_Enemies[i]->getPosition();
            float r1 = m_Enemies[i]->getRadius();

            for (size_t j = i + 1; j < m_Enemies.size(); ++j) {
                if (!m_Enemies[j]->isAlive()) continue;
                sf::Vector2f pos2 = m_Enemies[j]->getPosition();
                float r2 = m_Enemies[j]->getRadius();

                sf::Vector2f delta = pos2 - pos1;
                float distSq = delta.x * delta.x + delta.y * delta.y;
                float minDist = r1 + r2;

                if (distSq < minDist * minDist && distSq > 0.001f) {
                    float dist = std::sqrt(distSq);
                    sf::Vector2f normal = delta / dist;
                    float overlap = minDist - dist;
                    sf::Vector2f correction = normal * (overlap * 0.5f);
                    m_Enemies[i]->setPosition(pos1 - correction);
                    m_Enemies[j]->setPosition(pos2 + correction);
                }
            }
        }
    }

    // Снаряды
    for (auto& p : m_Projectiles) p.update(dt);

    sf::FloatRect pb = m_Player.getBounds();
    for (auto& proj : m_Projectiles)
    {
        if (proj.isDead()) continue;

        if (proj.isFromPlayer())
        {
            for (auto& e : m_Enemies)
            {
                if (!e->isAlive()) continue;
                if (proj.getBounds().intersects(e->getBounds()))
                {
                    e->takeDamage(static_cast<int>(proj.getDamage()));
                    e->flashHit();
                    proj.kill();
                    if (!e->isAlive())
                    {
                        m_Score += e->getScoreValue();
                        std::mt19937 rng(static_cast<unsigned>(
                            std::chrono::steady_clock::now().time_since_epoch().count()));
                        int roll = rng() % 100;
                        if (e->getType() != EnemyType::BOSS && roll < 20)
                        {
                            m_WorldItems.emplace_back(Item(ItemType::COIN, e->getPosition()), currentRoomIdx);
                        }
                        break;
                    }
                }
            }
        }
        else
        {
            if (proj.getBounds().intersects(pb))
            {
                m_Player.damage(static_cast<int>(proj.getDamage()));
                proj.kill();
                if (!m_Player.isAlive()) break;
            }
        }
        // Стены
        sf::FloatRect bounds = proj.getBounds();
        if (m_Room.isSolid(bounds.left, bounds.top, false) ||
            m_Room.isSolid(bounds.left + bounds.width, bounds.top, false) ||
            m_Room.isSolid(bounds.left, bounds.top + bounds.height, false) ||
            m_Room.isSolid(bounds.left + bounds.width, bounds.top + bounds.height, false)) {
            proj.kill();
        }
    }
    m_Projectiles.erase(std::remove_if(m_Projectiles.begin(), m_Projectiles.end(),
        [](const Projectile& p) {
            return p.isExpired() || p.isDead()
                || p.getPosition().x < -50.f || p.getPosition().x > Room::GRID_WIDTH * Room::TILE_SIZE + 50.f
                || p.getPosition().y < -50.f || p.getPosition().y > Room::GRID_HEIGHT * Room::TILE_SIZE + 50.f;
        }), m_Projectiles.end());

    // Контактный урон
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

    // Предметы
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
                        m_HUD.setMessage("Куплено: " + Item::getDescription(wi.item.getType()));
                    }
                    else
                    {
                        m_HUD.setMessage("Не хватает монет (" +
                            std::to_string(wi.item.getCost()) + ")", 1.5f);
                    }
                }
                else
                {
                    m_HUD.setMessage("E — купить за " +
                        std::to_string(wi.item.getCost()) + " монет", 0.1f);
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
    // Очистка комнаты
    if (!m_Level.getCurrentNode().cleared)
    {
        bool anyAlive = false;
        for (auto& e : m_Enemies) if (e->isAlive()) { anyAlive = true; break; }
        if (!anyAlive)
        {
            onRoomCleared();
        }
    }

    // Переход между комнатами
    checkRoomTransitions();

    // *** Портал: активен только в той комнате, где был открыт ***
    if (m_PortalRoomIndex == m_Level.getCurrentRoomIndex())
    {
        m_PortalActive = true;
    }
    else
    {
        m_PortalActive = false;
    }

    if (m_PortalActive)
    {
        // Анимация пульсации
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

    // Смерть игрока
    if (!m_Player.isAlive())
    {
        if (m_Score > m_HighScore)
        {
            m_HighScore = m_Score;
            saveHighScore();
        }
        m_State = GameState::GAME_OVER;
    }

    m_TransitionTimer += dt;
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
    m_WorldItems.clear();  // ← ВМЕСТО m_Items.clear()
    m_BossIntroShown = false;
    m_PortalActive = false;
    m_PortalRoomIndex = -1;

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
    m_HUD.setMessage("Этаж " + std::to_string(floorNum));
}

void Game::loadCurrentRoom()
{
    m_Projectiles.clear();
    m_Enemies.clear();

    auto doors = m_Level.getCurrentDoors();
    RoomType type = m_Level.getCurrentNode().type;
    int currentRoomIdx = m_Level.getCurrentRoomIndex();

    // Генерируем комнату
    unsigned int roomSeed = static_cast<unsigned int>(
        currentRoomIdx * 131
        + m_Level.getFloorNumber() * 977
        + m_Level.getCurrentNode().x * 31
        + m_Level.getCurrentNode().y * 17
        + 1);

    m_Room.generate(type, doors, roomSeed);

    // Очищаем временные предметы (которые на полу в текущей комнате)
    // НО НЕ УДАЛЯЕМ ИЗ m_WorldItems!
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

    // Спавним новые предметы только если комната не очищена
    if (!m_Level.getCurrentNode().cleared)
    {
        // Проверяем, есть ли уже предметы в этой комнате
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

    // Блокируем двери
    m_Room.setDoorsLocked(!m_Level.getCurrentNode().cleared);

    // Сообщения
    if (type == RoomType::BOSS && !m_Level.getCurrentNode().cleared && !m_BossIntroShown)
    {
        m_HUD.setMessage("БОСС! Подготовьтесь.", 2.5f);
        m_BossIntroShown = true;
    }
    else if (type == RoomType::SHOP)
    {
        m_HUD.setMessage("Магазин: подойдите к предмету и нажмите E", 2.5f);
    }
    else if (type == RoomType::ITEM)
    {
        m_HUD.setMessage("Комната с предметом", 1.5f);
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

    // Вспомогательная лямбда для получения радиуса врага
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
        float bossRadius = getEnemyRadius(EnemyType::BOSS);
        sf::FloatRect bossBox(c.x - bossRadius, c.y - 80.f - bossRadius,
            bossRadius * 2.f, bossRadius * 2.f);
        if (m_Room.isRectSolid(bossBox))
        {
            c.y -= bossRadius;
        }
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
        auto c = m_Room.getCenter();

        // ИСПРАВЛЕНО: используем m_WorldItems вместо m_Items
        m_WorldItems.emplace_back(Item(ItemType::HEALTH, c), currentRoomIdx);

        // Активация портала в этой комнате
        m_PortalRoomIndex = m_Level.getCurrentRoomIndex();
        m_PortalActive = true;
        m_PortalPos = c + sf::Vector2f(0.f, -50.f);
        m_HUD.setMessage("Портал открыт! Идите к нему, чтобы перейти на следующий этаж.", 3.f);
    }
    else
    {
        m_HUD.setMessage("Комната пройдена (+50)", 1.2f);
    }
}

void Game::render()
{
    m_Window.clear(sf::Color(18, 16, 24));

    if (m_State == GameState::MENU)
    {
        drawMenu();
    }
    else
    {
        drawPlaying();
        if (m_State == GameState::PAUSED)
        {
            drawOverlay("ПАУЗА", "Enter — продолжить, Esc — в меню",
                sf::Color(230, 230, 230));
        }
        else if (m_State == GameState::GAME_OVER)
        {
            drawOverlay("GAME OVER",
                "Счёт: " + std::to_string(m_Score) +
                "   Рекорд: " + std::to_string(m_HighScore) +
                "\nEnter — начать заново, Esc — в меню",
                sf::Color(230, 80, 80));
        }
        else if (m_State == GameState::VICTORY)
        {
            drawOverlay("ПОБЕДА!",
                "Счёт: " + std::to_string(m_Score) +
                "   Рекорд: " + std::to_string(m_HighScore) +
                "\nEnter — заново, Esc — в меню",
                sf::Color(120, 240, 140));
        }
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
            btn.setFillColor((int)i == m_MenuIndex
                ? sf::Color(80, 80, 140)
                : sf::Color(40, 40, 60));
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

    // Рисуем только предметы из текущей комнаты
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
}

void Game::drawOverlay(const std::string& title, const std::string& subtitle, sf::Color titleColor)
{
    sf::RectangleShape overlay({ (float)WIN_WIDTH, (float)WIN_HEIGHT });
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    m_Window.draw(overlay);

    if (!AssetManager::instance().isFontLoaded()) return;
    const auto& font = AssetManager::instance().getFont();

    sf::Text t;
    t.setFont(font);
    t.setString(u8(title));
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
    s.setString(u8(subtitle));
    s.setCharacterSize(22);
    s.setFillColor(sf::Color(230, 230, 230));
    auto sb = s.getLocalBounds();
    s.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
    s.setPosition(WIN_WIDTH / 2.f, 330.f);
    m_Window.draw(s);

    if (m_State == GameState::PAUSED)
    {
        const char* items[] = { "Продолжить", "Выйти в меню" };
        for (int i = 0; i < 2; ++i)
        {
            sf::Text mi;
            mi.setFont(font);
            mi.setString(u8(items[i]));
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
