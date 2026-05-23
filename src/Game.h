#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Item.h"
#include "Room.h"
#include "Level.h"
#include "HUD.h"

// Добавьте где-нибудь в начале файла Game.h, после #include
struct WorldItem
{
    Item item;
    int roomIndex;  // индекс комнаты, где лежит предмет
    bool collected;

    WorldItem(const Item& i, int idx)
        : item(i), roomIndex(idx), collected(false) {
    }
};

// Состояние игры
enum class GameState
{
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    VICTORY
};

// Главный класс — игровой цикл, состояние, координация всех систем
class Game
{
public:
    Game();

    // Главный цикл игры
    void run();

private:
    static constexpr int WIN_WIDTH = 1280;
    static constexpr int WIN_HEIGHT = 720;
    static constexpr int MAX_FLOORS = 2;

    sf::RenderWindow m_Window;
    GameState m_State;

    // Игровые сущности
    Player m_Player;
    Room m_Room;
    Level m_Level;
    HUD m_HUD;

    std::vector<std::unique_ptr<Enemy>> m_Enemies;
    std::vector<Projectile> m_Projectiles;
    std::vector<WorldItem> m_WorldItems;  // вместо std::vector<Item> m_Items

    // Очки
    int m_Score;
    int m_HighScore;

    // Меню
    int m_MenuIndex;
    std::vector<std::string> m_MenuItems;
    sf::Text m_Title;
    sf::Text m_Subtitle;
    std::vector<sf::Text> m_MenuTexts;
    sf::Text m_HighScoreMenu;
    sf::Text m_Prompt;

    // Буфер — отложенный перезапуск/переход
    bool m_NeedRestart;

    // Таймер перехода
    float m_TransitionTimer;

    // Обработка
    void processEvents();
    void update(float dt);
    void render();

    // Состояния
    void updateMenu(float dt);
    void updatePlaying(float dt);
    void updatePaused(float dt);
    void updateGameOver(float dt);
    void updateVictory(float dt);

    void drawMenu();
    void drawPlaying();
    void drawOverlay(const std::string& title, const std::string& subtitle, sf::Color titleColor);

    // Инициализация
    void newGame();
    void loadFloor(int floorNum);
    void loadCurrentRoom();

    void spawnRoomContent();
    void onRoomCleared();

    // Коллизии
    void resolvePlayerWallCollision(sf::Vector2f desired);
    void checkRoomTransitions();

    // Атаки
    void handlePlayerShoot();

    // Высокий счёт
    void loadHighScore();
    void saveHighScore();

    // Меню helpers
    void setupMenuTexts();
    void refreshMenuHighlight();

    // Утилиты
    sf::Vector2f worldMousePos() const;

    // Флаг, нажата ли кнопка мыши (чтобы обработать только один раз для меню)
    bool m_MousePressedPrev;
    bool m_EnterPressedPrev;
    bool m_PausePressedPrev;
    bool m_InteractPressedPrev; // E

    // Вход в босс-комнату: показать сообщение
    bool m_BossIntroShown;

    bool m_PortalActive;
    int m_PortalRoomIndex;
    sf::Vector2f m_PortalPos;
    sf::CircleShape m_PortalShape;
};
