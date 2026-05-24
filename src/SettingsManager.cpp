#include "SettingsManager.h"

SettingsManager::SettingsManager()
    : m_CurrentSettings(nullptr)
    , m_CurrentType(SettingsType::NONE)
{
    setupCallbacks();
}

SettingsManager::~SettingsManager()
{
}

SettingsManager& SettingsManager::getInstance()
{
    static SettingsManager instance;
    return instance;
}

void SettingsManager::setupCallbacks()
{
    m_VolumeSettings.setOnCloseCallback([this]() {
        m_CurrentSettings = nullptr;
        m_CurrentType = SettingsType::NONE;
        });

    m_ControlsSettings.setOnCloseCallback([this]() {
        m_CurrentSettings = nullptr;
        m_CurrentType = SettingsType::NONE;
        });

    m_GraphicsSettings.setOnCloseCallback([this]() {
        m_CurrentSettings = nullptr;
        m_CurrentType = SettingsType::NONE;
        });
}

void SettingsManager::openSettings(SettingsType type)
{
    closeSettings();

    switch (type)
    {
    case SettingsType::VOLUME:
        m_CurrentSettings = &m_VolumeSettings;
        break;
    case SettingsType::CONTROLS:
        m_CurrentSettings = &m_ControlsSettings;
        break;
    case SettingsType::GRAPHICS:
        m_CurrentSettings = &m_GraphicsSettings;
        break;
    default:
        m_CurrentSettings = nullptr;
        break;
    }

    if (m_CurrentSettings)
    {
        m_CurrentType = type;
        m_CurrentSettings->show();
    }
}

void SettingsManager::closeSettings()
{
    if (m_CurrentSettings)
    {
        m_CurrentSettings->hide();
        m_CurrentSettings = nullptr;
    }
    m_CurrentType = SettingsType::NONE;
}

bool SettingsManager::isActive() const
{
    return m_CurrentSettings != nullptr && m_CurrentSettings->isActive();
}

void SettingsManager::applyGraphics(sf::RenderWindow& window)
{
    m_GraphicsSettings.apply(window);
}

sf::Keyboard::Key SettingsManager::getKeyForAction(ControlAction action) const
{
    return m_ControlsSettings.getKeyForAction(action);
}

void SettingsManager::update(float dt, const sf::RenderWindow& window)
{
    if (m_CurrentSettings)
    {
        m_CurrentSettings->update(dt, window);
    }
}

void SettingsManager::draw(sf::RenderWindow& window)
{
    if (m_CurrentSettings)
    {
        m_CurrentSettings->draw(window);
    }
}