#include "GraphicsSettings.h"
#include "AssetManager.h"
#include "Utf.h"
#include "Input.h"

GraphicsSettings::GraphicsSettings()
    : m_Fullscreen(false)
    , m_VSync(true)
    , m_FPSLimit(60)
    , m_PendingApply(false)
{
    setupUI("GRAPHICS SETTINGS");
    createItems();
}

GraphicsSettings::~GraphicsSettings()
{
}

void GraphicsSettings::onBack(void* data)
{
    GraphicsSettings* settings = static_cast<GraphicsSettings*>(data);
    if (settings->m_OnClose)
        settings->m_OnClose();
    settings->hide();
}

void GraphicsSettings::createItems()
{
    clearItems();

    addItem("Fullscreen", m_Fullscreen ? "ON" : "OFF", nullptr, nullptr, false, 0.f, 0.f, 0.f, 0);
    addItem("VSync", m_VSync ? "ON" : "OFF", nullptr, nullptr, false, 0.f, 0.f, 0.f, 1);

    char buffer[16];
    snprintf(buffer, 16, "%d", m_FPSLimit);
    addItem("FPS Limit", buffer, nullptr, nullptr, true, (float)m_FPSLimit, 30.f, 240.f, 2);

    addItem("Back", "", onBack, this, false, 0.f, 0.f, 0.f, 3);
}

void GraphicsSettings::updateDisplayValues()
{
    if (m_ItemsCount > 0) {
        const char* value = m_Fullscreen ? "ON" : "OFF";
        memcpy(m_Items[0].value, value, strlen(value) + 1);
    }
    if (m_ItemsCount > 1) {
        const char* value = m_VSync ? "ON" : "OFF";
        memcpy(m_Items[1].value, value, strlen(value) + 1);
    }
    if (m_ItemsCount > 2) {
        char buffer[16];
        snprintf(buffer, 16, "%d", m_FPSLimit);
        memcpy(m_Items[2].value, buffer, strlen(buffer) + 1);
    }
}

void GraphicsSettings::toggleFullscreen()
{
    m_Fullscreen = !m_Fullscreen;
    m_PendingApply = true;
    updateDisplayValues();
}

void GraphicsSettings::toggleVSync()
{
    m_VSync = !m_VSync;
    m_PendingApply = true;
    updateDisplayValues();
}

void GraphicsSettings::changeFPSLimit(float delta)
{
    m_FPSLimit += (int)delta;
    if (m_FPSLimit < 30) m_FPSLimit = 30;
    if (m_FPSLimit > 240) m_FPSLimit = 240;
    m_PendingApply = true;
    updateDisplayValues();
}

void GraphicsSettings::onItemSelected(int index)
{
    switch (index)
    {
    case 0: toggleFullscreen(); break;
    case 1: toggleVSync(); break;
    case 2: break;
    }
}

void GraphicsSettings::apply(sf::RenderWindow& window)
{
    if (!m_PendingApply) return;

    if (m_VSync)
        window.setVerticalSyncEnabled(true);
    else
        window.setVerticalSyncEnabled(false);

    window.setFramerateLimit(m_FPSLimit);

    m_PendingApply = false;
}

void GraphicsSettings::update(float dt, const sf::RenderWindow& window)
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

    // Handle FPS slider
    if (m_SelectedIndex == 2 && m_ItemsCount > 2)
    {
        float change = 0.f;
        if (Input::isKeyPressed(sf::Keyboard::Left) || Input::isKeyPressed(sf::Keyboard::A))
            change = -30.f * dt;
        if (Input::isKeyPressed(sf::Keyboard::Right) || Input::isKeyPressed(sf::Keyboard::D))
            change = 30.f * dt;

        if (change != 0.f)
        {
            changeFPSLimit(change);
        }
    }

    if (Input::isKeyJustPressed(sf::Keyboard::Enter) || Input::isKeyJustPressed(sf::Keyboard::Space))
    {
        executeSelected();
    }

    if (Input::isKeyJustPressed(sf::Keyboard::Escape))
    {
        if (m_OnClose) m_OnClose();
        hide();
    }
}

void GraphicsSettings::draw(sf::RenderWindow& window)
{
    if (!m_Active) return;

    const auto& font = AssetManager::instance().getFont();
    if (!AssetManager::instance().isFontLoaded()) return;

    window.draw(m_Overlay);
    window.draw(m_Panel);
    window.draw(m_Title);

    float startY = WIN_HEIGHT / 2.f - 120.f;
    float y = startY;

    for (int i = 0; i < m_ItemsCount; ++i)
    {
        const SettingItem& item = m_Items[i];

        sf::Text text;
        text.setFont(font);
        text.setString(item.label);
        text.setCharacterSize(24);
        text.setFillColor(i == m_SelectedIndex ? sf::Color(255, 220, 100) : sf::Color(200, 200, 200));
        text.setPosition(WIN_WIDTH / 2.f - 200.f, y);
        window.draw(text);

        sf::Text valueText;
        valueText.setFont(font);
        valueText.setString(item.value);
        valueText.setCharacterSize(22);
        valueText.setFillColor(sf::Color(180, 180, 220));
        valueText.setPosition(WIN_WIDTH / 2.f + 120.f, y);
        window.draw(valueText);

        if (item.isSlider)
        {
            sf::RectangleShape sliderBg({ 180.f, 6.f });
            sliderBg.setFillColor(sf::Color(60, 60, 80));
            sliderBg.setPosition(WIN_WIDTH / 2.f + 120.f, y + 30.f);
            window.draw(sliderBg);

            float range = item.sliderMax - item.sliderMin;
            float percent = (range > 0.001f) ? (item.sliderValue - item.sliderMin) / range : 0.f;
            if (percent < 0.f) percent = 0.f;
            if (percent > 1.f) percent = 1.f;

            sf::RectangleShape sliderFill({ 180.f * percent, 6.f });
            sliderFill.setFillColor(sf::Color(100, 200, 100));
            sliderFill.setPosition(WIN_WIDTH / 2.f + 120.f, y + 30.f);
            window.draw(sliderFill);

            sf::CircleShape handle(8.f);
            handle.setFillColor(sf::Color(255, 220, 100));
            handle.setOrigin(8.f, 8.f);
            handle.setPosition(WIN_WIDTH / 2.f + 120.f + 180.f * percent, y + 33.f);
            window.draw(handle);
        }

        y += 55.f;
    }
}