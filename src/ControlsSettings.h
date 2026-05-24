#pragma once
#include "SettingsBase.h"
#include <SFML/Window/Keyboard.hpp>

enum class ControlAction
{
    MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT,
    SHOOT, DASH, INTERACT, PAUSE,
    COUNT
};

struct ControlBinding
{
    ControlAction action;
    char name[20];
    sf::Keyboard::Key defaultKey;
    sf::Keyboard::Key currentKey;
    bool waitingForInput;
};

class ControlsSettings : public SettingsBase
{
public:
    ControlsSettings();
    virtual ~ControlsSettings();

    void update(float dt, const sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    sf::Keyboard::Key getKeyForAction(ControlAction action) const;
    void saveControls();
    void loadControls();

protected:
    void onItemSelected(int index) override;
    void updateDisplayValues() override;

private:
    void createItems();
    void startWaitingForInput(int index);
    const char* getKeyName(sf::Keyboard::Key key) const;

    ControlBinding m_Controls[8];
    bool m_WaitingForInput;
    int m_WaitingIndex;

    static void onBack(void* data);
};