#include "Input.h"
#include "SettingsManager.h"
#include <cmath>

bool Input::m_CurrentKeys[sf::Keyboard::KeyCount];
bool Input::m_PreviousKeys[sf::Keyboard::KeyCount];
bool Input::m_CurrentMouse[sf::Mouse::ButtonCount];
bool Input::m_PreviousMouse[sf::Mouse::ButtonCount];
sf::Vector2i Input::m_MousePos;
sf::Vector2f Input::m_WorldMousePos;
const sf::RenderWindow* Input::m_Window = nullptr;

void Input::update(const sf::RenderWindow& window)
{
    m_Window = &window;
    updateKeys();
    updateMouse(window);
}

void Input::updateKeys()
{
    // Copy current to previous
    for (int i = 0; i < sf::Keyboard::KeyCount; ++i)
        m_PreviousKeys[i] = m_CurrentKeys[i];

    // Update current states
    for (int i = 0; i < sf::Keyboard::KeyCount; ++i)
        m_CurrentKeys[i] = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(i));
}

void Input::updateMouse(const sf::RenderWindow& window)
{
    // Copy current to previous
    for (int i = 0; i < sf::Mouse::ButtonCount; ++i)
        m_PreviousMouse[i] = m_CurrentMouse[i];

    // Update current states
    for (int i = 0; i < sf::Mouse::ButtonCount; ++i)
        m_CurrentMouse[i] = sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(i));

    m_MousePos = sf::Mouse::getPosition(window);
    m_WorldMousePos = sf::Vector2f(static_cast<float>(m_MousePos.x), static_cast<float>(m_MousePos.y));
}

bool Input::isKeyPressed(sf::Keyboard::Key key)
{
    if (key == sf::Keyboard::Unknown) return false;
    return m_CurrentKeys[static_cast<int>(key)];
}

bool Input::isKeyJustPressed(sf::Keyboard::Key key)
{
    if (key == sf::Keyboard::Unknown) return false;
    int idx = static_cast<int>(key);
    return m_CurrentKeys[idx] && !m_PreviousKeys[idx];
}

bool Input::isMouseButtonPressed(sf::Mouse::Button button)
{
    return m_CurrentMouse[static_cast<int>(button)];
}

bool Input::isMouseButtonJustPressed(sf::Mouse::Button button)
{
    int idx = static_cast<int>(button);
    return m_CurrentMouse[idx] && !m_PreviousMouse[idx];
}

sf::Vector2i Input::getMousePosition()
{
    return m_MousePos;
}

sf::Vector2f Input::getMouseWorldPosition()
{
    return m_WorldMousePos;
}

sf::Vector2f Input::getMovementDirection()
{
    sf::Vector2f dir(0.f, 0.f);

    // Get keys from settings manager (for rebindable controls)
    auto& settings = SettingsManager::getInstance();

    if (isKeyPressed(settings.getKeyForAction(ControlAction::MOVE_UP)))    dir.y -= 1.f;
    if (isKeyPressed(settings.getKeyForAction(ControlAction::MOVE_DOWN)))  dir.y += 1.f;
    if (isKeyPressed(settings.getKeyForAction(ControlAction::MOVE_LEFT)))  dir.x -= 1.f;
    if (isKeyPressed(settings.getKeyForAction(ControlAction::MOVE_RIGHT))) dir.x += 1.f;

    if (dir.x != 0.f || dir.y != 0.f)
    {
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        dir.x /= len;
        dir.y /= len;
    }
    return dir;
}

bool Input::isDashPressed()
{
    auto& settings = SettingsManager::getInstance();
    return isKeyPressed(settings.getKeyForAction(ControlAction::DASH));
}

bool Input::isInteractPressed()
{
    auto& settings = SettingsManager::getInstance();
    return isKeyPressed(settings.getKeyForAction(ControlAction::INTERACT));
}

bool Input::isInteractJustPressed()
{
    auto& settings = SettingsManager::getInstance();
    return isKeyJustPressed(settings.getKeyForAction(ControlAction::INTERACT));
}

bool Input::isPausePressed()
{
    auto& settings = SettingsManager::getInstance();
    return isKeyJustPressed(settings.getKeyForAction(ControlAction::PAUSE));
}

bool Input::isShootPressed()
{
    return isMouseButtonPressed(sf::Mouse::Left);
}