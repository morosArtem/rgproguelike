#include "Input.h"
#include <cmath>

// Статические члены
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
    for (int i = 0; i < sf::Keyboard::KeyCount; ++i)
        m_PreviousKeys[i] = m_CurrentKeys[i];

    for (int i = 0; i < sf::Keyboard::KeyCount; ++i)
        m_CurrentKeys[i] = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(i));
}

void Input::updateMouse(const sf::RenderWindow& window)
{
    for (int i = 0; i < sf::Mouse::ButtonCount; ++i)
        m_PreviousMouse[i] = m_CurrentMouse[i];

    for (int i = 0; i < sf::Mouse::ButtonCount; ++i)
        m_CurrentMouse[i] = sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(i));

    m_MousePos = sf::Mouse::getPosition(window);
    m_WorldMousePos = sf::Vector2f(static_cast<float>(m_MousePos.x), static_cast<float>(m_MousePos.y));
}

bool Input::isKeyPressed(sf::Keyboard::Key key)
{
    return m_CurrentKeys[static_cast<int>(key)];
}

bool Input::isKeyJustPressed(sf::Keyboard::Key key)
{
    return m_CurrentKeys[static_cast<int>(key)] && !m_PreviousKeys[static_cast<int>(key)];
}

bool Input::isMouseButtonPressed(sf::Mouse::Button button)
{
    return m_CurrentMouse[static_cast<int>(button)];
}

bool Input::isMouseButtonJustPressed(sf::Mouse::Button button)
{
    return m_CurrentMouse[static_cast<int>(button)] && !m_PreviousMouse[static_cast<int>(button)];
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
    if (isKeyPressed(sf::Keyboard::W) || isKeyPressed(sf::Keyboard::Up))    dir.y -= 1.f;
    if (isKeyPressed(sf::Keyboard::S) || isKeyPressed(sf::Keyboard::Down))  dir.y += 1.f;
    if (isKeyPressed(sf::Keyboard::A) || isKeyPressed(sf::Keyboard::Left))  dir.x -= 1.f;
    if (isKeyPressed(sf::Keyboard::D) || isKeyPressed(sf::Keyboard::Right)) dir.x += 1.f;

    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f) dir /= len;
    return dir;
}
