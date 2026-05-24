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
#include "UpgradeSelectionUI.h"
#include "SettingsMenu.h"

// World item structure
struct WorldItem
{
    Item item;
    int roomIndex;
    bool collected;

    WorldItem(const Item& i, int idx)
        : item(i), roomIndex(idx), collected(false) {
    }
};

// Game state enum
enum class GameState
{
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    VICTORY
};

// Main game class
class Game
{
public:
    Game();

    void run();

private:
    static constexpr int WIN_WIDTH = 1280;
    static constexpr int WIN_HEIGHT = 720;
    static constexpr int MAX_FLOORS = 2;

    UpgradeSelectionUI m_UpgradeUI;

    sf::RenderWindow m_Window;
    GameState m_State;

    // Game entities
    Player m_Player;
    Room m_Room;
    Level m_Level;
    HUD m_HUD;

    std::vector<std::unique_ptr<Enemy>> m_Enemies;
    std::vector<Projectile> m_Projectiles;
    std::vector<WorldItem> m_WorldItems;

    // Score
    int m_Score;
    int m_HighScore;

    // Menu
    int m_MenuIndex;
    std::vector<std::string> m_MenuItems;
    sf::Text m_Title;
    sf::Text m_Subtitle;
    std::vector<sf::Text> m_MenuTexts;
    sf::Text m_HighScoreMenu;
    sf::Text m_Prompt;

    // Settings menu
    SettingsMainMenu m_SettingsMainMenu;
    bool m_ShowSettings;

    // State flags
    bool m_NeedRestart;
    float m_TransitionTimer;
    bool m_BossIntroShown;
    bool m_PortalActive;
    int m_PortalRoomIndex;
    sf::Vector2f m_PortalPos;
    sf::CircleShape m_PortalShape;

    // Input state for menu navigation
    bool m_MousePressedPrev;
    bool m_EnterPressedPrev;
    bool m_PausePressedPrev;
    bool m_InteractPressedPrev;

    // Event processing
    void processEvents();
    void update(float dt);
    void render();

    // Update functions for each state
    void updatePlaying(float dt);

    // Drawing functions
    void drawMenu();
    void drawPlaying();
    void drawOverlay(const std::string& title, const std::string& subtitle, sf::Color titleColor);

    // Game initialization
    void newGame();
    void loadFloor(int floorNum);
    void loadCurrentRoom();
    void spawnRoomContent();
    void onRoomCleared();

    // Collision handling
    void resolvePlayerWallCollision(sf::Vector2f desired);
    void checkRoomTransitions();

    // Combat
    void handlePlayerShoot();

    // High score
    void loadHighScore();
    void saveHighScore();

    // Menu helpers
    void setupMenuTexts();
    void refreshMenuHighlight();

    void showUpgradeUI();
};
