#include "ControlsSettings.h"
#include "AssetManager.h"
#include "Utf.h"
#include "Input.h"
#include <fstream>
#include <cstring>

ControlsSettings::ControlsSettings()
    : m_WaitingForInput(false)
    , m_WaitingIndex(-1)
{
    setupUI("CONTROLS SETTINGS");

    // Initialize controls
    m_Controls[0] = { ControlAction::MOVE_UP, "Up", sf::Keyboard::W, sf::Keyboard::W, false };
    m_Controls[1] = { ControlAction::MOVE_DOWN, "Down", sf::Keyboard::S, sf::Keyboard::S, false };
    m_Controls[2] = { ControlAction::MOVE_LEFT, "Left", sf::Keyboard::A, sf::Keyboard::A, false };
    m_Controls[3] = { ControlAction::MOVE_RIGHT, "Right", sf::Keyboard::D, sf::Keyboard::D, false };
    m_Controls[4] = { ControlAction::SHOOT, "Shoot", sf::Keyboard::LControl, sf::Keyboard::LControl, false };
    m_Controls[5] = { ControlAction::DASH, "Dash", sf::Keyboard::Space, sf::Keyboard::Space, false };
    m_Controls[6] = { ControlAction::INTERACT, "Interact", sf::Keyboard::E, sf::Keyboard::E, false };
    m_Controls[7] = { ControlAction::PAUSE, "Pause", sf::Keyboard::Escape, sf::Keyboard::Escape, false };

    loadControls();
    createItems();
}

ControlsSettings::~ControlsSettings()
{
}

void ControlsSettings::onBack(void* data)
{
    ControlsSettings* settings = static_cast<ControlsSettings*>(data);
    if (settings->m_OnClose)
        settings->m_OnClose();
    settings->hide();
}

const char* ControlsSettings::getKeyName(sf::Keyboard::Key key) const
{
    switch (key)
    {
    case sf::Keyboard::W: return "W";
    case sf::Keyboard::A: return "A";
    case sf::Keyboard::S: return "S";
    case sf::Keyboard::D: return "D";
    case sf::Keyboard::Space: return "Space";
    case sf::Keyboard::LControl: return "Ctrl";
    case sf::Keyboard::LShift: return "Shift";
    case sf::Keyboard::E: return "E";
    case sf::Keyboard::Escape: return "Esc";
    case sf::Keyboard::Up: return "Up";
    case sf::Keyboard::Down: return "Down";
    case sf::Keyboard::Left: return "Left";
    case sf::Keyboard::Right: return "Right";
    default: return "?";
    }
}

void ControlsSettings::createItems()
{
    clearItems();

    for (int i = 0; i < 8; ++i)
    {
        const char* keyName = getKeyName(m_Controls[i].currentKey);
        addItem(m_Controls[i].name, keyName, nullptr, nullptr, false, 0.f, 0.f, 0.f, i);
    }

    addItem("Back", "", onBack, this, false, 0.f, 0.f, 0.f, 8);
}

void ControlsSettings::updateDisplayValues()
{
    for (int i = 0; i < 8 && i < m_ItemsCount; ++i)
    {
        const char* keyName = getKeyName(m_Controls[i].currentKey);
        size_t len = strlen(keyName);
        memcpy(m_Items[i].value, keyName, len < 31 ? len : 31);
        m_Items[i].value[len < 31 ? len : 31] = '\0';
    }
}

void ControlsSettings::startWaitingForInput(int index)
{
    m_WaitingForInput = true;
    m_WaitingIndex = index;
    const char* waitingText = "...";
    size_t len = strlen(waitingText);
    memcpy(m_Items[index].value, waitingText, len < 31 ? len : 31);
    m_Items[index].value[len < 31 ? len : 31] = '\0';
}

void ControlsSettings::onItemSelected(int index)
{
    if (m_WaitingForInput) return;

    if (index < 8)
    {
        startWaitingForInput(index);
    }
}

sf::Keyboard::Key ControlsSettings::getKeyForAction(ControlAction action) const
{
    for (int i = 0; i < 8; ++i)
        if (m_Controls[i].action == action)
            return m_Controls[i].currentKey;
    return sf::Keyboard::Unknown;
}

void ControlsSettings::saveControls()
{
    std::ofstream file("controls.txt", std::ios::trunc);
    if (!file.is_open()) return;

    for (int i = 0; i < 8; ++i)
    {
        file << (int)m_Controls[i].action << " " << (int)m_Controls[i].currentKey << "\n";
    }
}

void ControlsSettings::loadControls()
{
    std::ifstream file("controls.txt");
    if (!file.is_open()) return;

    int action, key;
    for (int i = 0; i < 8 && file >> action >> key; ++i)
    {
        m_Controls[i].currentKey = (sf::Keyboard::Key)key;
    }
}

void ControlsSettings::update(float dt, const sf::RenderWindow& window)
{
    if (!m_Active) return;

    m_AnimationTime += dt;
    float alpha = m_AnimationTime * 8.f;
    if (alpha > 1.f) alpha = 1.f;

    sf::Uint8 bgAlpha = static_cast<sf::Uint8>(200 * alpha);
    m_Overlay.setFillColor(sf::Color(0, 0, 0, bgAlpha));

    if (m_WaitingForInput)
    {
        for (int i = 0; i < sf::Keyboard::KeyCount; ++i)
        {
            sf::Keyboard::Key key = static_cast<sf::Keyboard::Key>(i);
            if (key == sf::Keyboard::Unknown) continue;

            if (Input::isKeyJustPressed(key))
            {
                m_Controls[m_WaitingIndex].currentKey = key;
                updateDisplayValues();
                m_WaitingForInput = false;
                m_WaitingIndex = -1;
                saveControls();
                break;
            }
        }

        if (Input::isKeyJustPressed(sf::Keyboard::Escape))
        {
            updateDisplayValues();
            m_WaitingForInput = false;
            m_WaitingIndex = -1;
        }
        return;
    }

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
        if (m_OnClose) m_OnClose();
        hide();
    }
}

void ControlsSettings::draw(sf::RenderWindow& window)
{
    if (!m_Active) return;

    const auto& font = AssetManager::instance().getFont();
    if (!AssetManager::instance().isFontLoaded()) return;

    window.draw(m_Overlay);
    window.draw(m_Panel);
    window.draw(m_Title);

    float startY = WIN_HEIGHT / 2.f - 150.f;
    float y = startY;

    for (int i = 0; i < m_ItemsCount; ++i)
    {
        const SettingItem& item = m_Items[i];

        sf::Text text;
        text.setFont(font);
        text.setString(item.label);
        text.setCharacterSize(22);

        bool isSelected = (i == m_SelectedIndex && !m_WaitingForInput);
        text.setFillColor(isSelected ? sf::Color(255, 220, 100) : sf::Color(200, 200, 200));
        text.setPosition(WIN_WIDTH / 2.f - 180.f, y);
        window.draw(text);

        sf::Text valueText;
        valueText.setFont(font);

        if (m_WaitingForInput && i == m_WaitingIndex)
            valueText.setString("Press any key...");
        else
            valueText.setString(item.value);

        valueText.setCharacterSize(20);
        valueText.setFillColor(m_WaitingForInput && i == m_WaitingIndex ?
            sf::Color(255, 200, 100) : sf::Color(180, 180, 220));
        valueText.setPosition(WIN_WIDTH / 2.f + 130.f, y);
        window.draw(valueText);

        y += 45.f;
    }
}