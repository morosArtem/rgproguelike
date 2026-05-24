#include "VolumeSettings.h"
#include "AssetManager.h"
#include "Utf.h"
#include "Input.h"

VolumeSettings::VolumeSettings()
    : m_MasterVolume(100.f)
    , m_MusicVolume(80.f)
    , m_SfxVolume(80.f)
{
    setupUI("VOLUME SETTINGS");
    createItems();
}

VolumeSettings::~VolumeSettings()
{
}

void VolumeSettings::onBack(void* data)
{
    VolumeSettings* settings = static_cast<VolumeSettings*>(data);
    if (settings->m_OnClose)
        settings->m_OnClose();
    settings->hide();
}

void VolumeSettings::createItems()
{
    clearItems();

    char buffer[16];
    snprintf(buffer, 16, "%.0f%%", m_MasterVolume);
    addItem("Master Volume", buffer, nullptr, nullptr, true, m_MasterVolume, 0.f, 100.f, 0);

    snprintf(buffer, 16, "%.0f%%", m_MusicVolume);
    addItem("Music Volume", buffer, nullptr, nullptr, true, m_MusicVolume, 0.f, 100.f, 1);

    snprintf(buffer, 16, "%.0f%%", m_SfxVolume);
    addItem("SFX Volume", buffer, nullptr, nullptr, true, m_SfxVolume, 0.f, 100.f, 2);

    addItem("Back", "", onBack, this, false, 0.f, 0.f, 0.f, 3);
}

void VolumeSettings::applyVolume()
{
    // Apply volume to audio system here
}

void VolumeSettings::onItemSelected(int index)
{
    if (index < 0 || index >= m_ItemsCount) return;

    SettingItem& item = m_Items[index];
    if (!item.isSlider) return;

    char buffer[16];
    snprintf(buffer, 16, "%.0f%%", item.sliderValue);
    memcpy(item.value, buffer, strlen(buffer) + 1);

    switch (item.type)
    {
    case 0: m_MasterVolume = item.sliderValue; break;
    case 1: m_MusicVolume = item.sliderValue; break;
    case 2: m_SfxVolume = item.sliderValue; break;
    }

    applyVolume();
}

void VolumeSettings::updateDisplayValues()
{
    char buffer[16];

    snprintf(buffer, 16, "%.0f%%", m_MasterVolume);
    if (m_ItemsCount > 0) memcpy(m_Items[0].value, buffer, strlen(buffer) + 1);

    snprintf(buffer, 16, "%.0f%%", m_MusicVolume);
    if (m_ItemsCount > 1) memcpy(m_Items[1].value, buffer, strlen(buffer) + 1);

    snprintf(buffer, 16, "%.0f%%", m_SfxVolume);
    if (m_ItemsCount > 2) memcpy(m_Items[2].value, buffer, strlen(buffer) + 1);
}

void VolumeSettings::update(float dt, const sf::RenderWindow& window)
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

    handleSliderInput(dt);

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

void VolumeSettings::draw(sf::RenderWindow& window)
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