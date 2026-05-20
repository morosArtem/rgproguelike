#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class Input
{
public:
    static void update(const sf::RenderWindow& window);

    static bool isKeyPressed(sf::Keyboard::Key key);
    static bool isKeyJustPressed(sf::Keyboard::Key key);

    static bool isMouseButtonPressed(sf::Mouse::Button button);
    static bool isMouseButtonJustPressed(sf::Mouse::Button button);
    static sf::Vector2i getMousePosition();
    static sf::Vector2f getMouseWorldPosition();

    static sf::Vector2f getMovementDirection();

private:
    static void updateKeys();
    static void updateMouse(const sf::RenderWindow& window);

    static bool m_CurrentKeys[sf::Keyboard::KeyCount];
    static bool m_PreviousKeys[sf::Keyboard::KeyCount];
    static bool m_CurrentMouse[sf::Mouse::ButtonCount];
    static bool m_PreviousMouse[sf::Mouse::ButtonCount];
    static sf::Vector2i m_MousePos;
    static sf::Vector2f m_WorldMousePos;
    static const sf::RenderWindow* m_Window;
};
