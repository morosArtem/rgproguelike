#pragma once
#include "SettingsBase.h"
#include "SettingsManager.h"

class SettingsMainMenu : public SettingsBase
{
public:
    SettingsMainMenu();
    virtual ~SettingsMainMenu();

    void show() override;
    void update(float dt, const sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

private:
    void createItems();

    static void onVolume(void* data);
    static void onControls(void* data);
    static void onGraphics(void* data);
    static void onBack(void* data);
};