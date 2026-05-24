#pragma once
#include "VolumeSettings.h"
#include "ControlsSettings.h"
#include "GraphicsSettings.h"

enum class SettingsType
{
    VOLUME,
    CONTROLS,
    GRAPHICS,
    NONE
};

class SettingsManager
{
public:
    static SettingsManager& getInstance();

    void update(float dt, const sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    void openSettings(SettingsType type);
    void closeSettings();
    bool isActive() const;

    void applyGraphics(sf::RenderWindow& window);

    sf::Keyboard::Key getKeyForAction(ControlAction action) const;

private:
    SettingsManager();
    ~SettingsManager();

    VolumeSettings m_VolumeSettings;
    ControlsSettings m_ControlsSettings;
    GraphicsSettings m_GraphicsSettings;

    SettingsBase* m_CurrentSettings;
    SettingsType m_CurrentType;

    void setupCallbacks();
};