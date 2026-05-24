#include "SettingsMenu.h"
#include "AssetManager.h"
#include "Input.h"

SettingsMainMenu::SettingsMainMenu()
{
    setupUI("SETTINGS");
    createItems();
}

SettingsMainMenu::~SettingsMainMenu()
{
}

void SettingsMainMenu::show()
{
    clearItems();
    createItems();
    m_SelectedIndex = 0;
    m_Active = true;
    m_AnimationTime = 0.f;
    updateDisplayValues();
}

void SettingsMainMenu::onVolume(void* data)
{
    SettingsManager::getInstance().openSettings(SettingsType::VOLUME);
}

void SettingsMainMenu::onControls(void* data)
{
    SettingsManager::getInstance().openSettings(SettingsType::CONTROLS);
}

void SettingsMainMenu::onGraphics(void* data)
{
    SettingsManager::getInstance().openSettings(SettingsType::GRAPHICS);
}

void SettingsMainMenu::onBack(void* data)
{
    SettingsMainMenu* menu = static_cast<SettingsMainMenu*>(data);
    menu->hide();
    if (menu->m_OnClose)
        menu->m_OnClose();
}

void SettingsMainMenu::createItems()
{
    clearItems();
    addItem("Volume", "", onVolume, this, false, 0.f, 0.f, 0.f, 0);
    addItem("Controls", "", onControls, this, false, 0.f, 0.f, 0.f, 1);
    addItem("Graphics", "", onGraphics, this, false, 0.f, 0.f, 0.f, 2);
    addItem("Back", "", onBack, this, false, 0.f, 0.f, 0.f, 3);
}

void SettingsMainMenu::update(float dt, const sf::RenderWindow& window)
{
    if (!m_Active) return;

    m_AnimationTime += dt;
    float alpha = m_AnimationTime * 8.f;
    if (alpha > 1.f) alpha = 1.f;

    sf::Uint8 bgAlpha = static_cast<sf::Uint8>(200 * alpha);
    m_Overlay.setFillColor(sf::Color(0, 0, 0, bgAlpha));

    if (m_RepeatDelay > 0.f)
        m_RepeatDelay -= dt;

    bool upJustPressed = Input::isKeyJustPressed(sf::Keyboard::Up) || Input::isKeyJustPressed(sf::Keyboard::W);
    bool downJustPressed = Input::isKeyJustPressed(sf::Keyboard::Down) || Input::isKeyJustPressed(sf::Keyboard::S);
    bool upHeld = m_RepeatDelay <= 0.f && (Input::isKeyPressed(sf::Keyboard::Up) || Input::isKeyPressed(sf::Keyboard::W));
    bool downHeld = m_RepeatDelay <= 0.f && (Input::isKeyPressed(sf::Keyboard::Down) || Input::isKeyPressed(sf::Keyboard::S));

    if (upJustPressed || upHeld)
    {
        navigateUp();
        m_RepeatDelay = upJustPressed ? REPEAT_DELAY : REPEAT_INTERVAL;
    }
    else if (downJustPressed || downHeld)
    {
        navigateDown();
        m_RepeatDelay = downJustPressed ? REPEAT_DELAY : REPEAT_INTERVAL;
    }

    if (Input::isKeyJustPressed(sf::Keyboard::Enter) || Input::isKeyJustPressed(sf::Keyboard::Space))
    {
        executeSelected();
    }

    if (Input::isKeyJustPressed(sf::Keyboard::Escape))
    {
        hide();
        if (m_OnClose)
            m_OnClose();
    }
}

void SettingsMainMenu::draw(sf::RenderWindow& window)
{
    if (!m_Active) return;

    const auto& font = AssetManager::instance().getFont();
    if (!AssetManager::instance().isFontLoaded()) return;

    window.draw(m_Overlay);
    window.draw(m_Panel);
    window.draw(m_Title);

    float startY = WIN_HEIGHT / 2.f - 100.f;
    float y = startY;

    for (int i = 0; i < m_ItemsCount; ++i)
    {
        const SettingItem& item = m_Items[i];

        sf::Text text;
        text.setFont(font);
        text.setString(item.label);
        text.setCharacterSize(28);

        if (i == m_SelectedIndex)
            text.setFillColor(sf::Color(255, 220, 100));
        else
            text.setFillColor(sf::Color(200, 200, 200));

        auto lb = text.getLocalBounds();
        text.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        text.setPosition(WIN_WIDTH / 2.f, y);

        window.draw(text);
        y += 55.f;
    }
}