#include "SettingsBase.h"
#include "AssetManager.h"
#include "Utf.h"
#include "Input.h"

SettingsBase::SettingsBase()
    : m_Items(nullptr)
    , m_ItemsCount(0)
    , m_ItemsCapacity(0)
    , m_SelectedIndex(0)
    , m_Active(false)
    , m_AnimationTime(0.f)
    , m_RepeatDelay(0.f)
{
}

SettingsBase::~SettingsBase()
{
    clearItems();
}

void SettingsBase::setupUI(const char* title)
{
    // Проверяем загружен ли шрифт
    if (!AssetManager::instance().isFontLoaded())
    {
        // Создаем визуальные элементы без текста
        m_Panel.setSize({ 550.f, 450.f });
        m_Panel.setFillColor(sf::Color(25, 25, 40, 240));
        m_Panel.setOutlineColor(sf::Color(100, 100, 140));
        m_Panel.setOutlineThickness(2.f);
        m_Panel.setOrigin(275.f, 225.f);
        m_Panel.setPosition(WIN_WIDTH / 2.f, WIN_HEIGHT / 2.f);

        m_Overlay.setSize({ static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT) });
        m_Overlay.setFillColor(sf::Color(0, 0, 0, 200));
        return;
    }

    const auto& font = AssetManager::instance().getFont();
    m_Title.setFont(font);
    m_Title.setString(title);
    m_Title.setCharacterSize(40);
    m_Title.setFillColor(sf::Color(255, 220, 100));
    m_Title.setStyle(sf::Text::Bold);
    auto tb = m_Title.getLocalBounds();
    m_Title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    m_Title.setPosition(WIN_WIDTH / 2.f, 70.f);

    m_Panel.setSize({ 550.f, 450.f });
    m_Panel.setFillColor(sf::Color(25, 25, 40, 240));
    m_Panel.setOutlineColor(sf::Color(100, 100, 140));
    m_Panel.setOutlineThickness(2.f);
    m_Panel.setOrigin(275.f, 225.f);
    m_Panel.setPosition(WIN_WIDTH / 2.f, WIN_HEIGHT / 2.f);

    m_Overlay.setSize({ static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT) });
    m_Overlay.setFillColor(sf::Color(0, 0, 0, 200));
}

void SettingsBase::addItem(const char* label, const char* value, void (*action)(void*), void* data,
    bool isSlider, float val, float minv, float maxv, int type)
{
    if (m_ItemsCount >= m_ItemsCapacity)
    {
        int newCapacity = m_ItemsCapacity == 0 ? 16 : m_ItemsCapacity * 2;
        SettingItem* newItems = new SettingItem[newCapacity];

        for (int i = 0; i < m_ItemsCount; ++i)
            newItems[i] = m_Items[i];

        delete[] m_Items;
        m_Items = newItems;
        m_ItemsCapacity = newCapacity;
    }

    SettingItem& item = m_Items[m_ItemsCount];

    size_t labelLen = strlen(label);
    size_t copyLen = labelLen < 31 ? labelLen : 31;
    memcpy(item.label, label, copyLen);
    item.label[copyLen] = '\0';

    size_t valueLen = strlen(value);
    copyLen = valueLen < 31 ? valueLen : 31;
    memcpy(item.value, value, copyLen);
    item.value[copyLen] = '\0';

    item.action = action;
    item.actionData = data;
    item.isSlider = isSlider;
    item.sliderValue = val;
    item.sliderMin = minv;
    item.sliderMax = maxv;
    item.type = type;

    m_ItemsCount++;
}

void SettingsBase::clearItems()
{
    delete[] m_Items;
    m_Items = nullptr;
    m_ItemsCount = 0;
    m_ItemsCapacity = 0;
}

void SettingsBase::navigateUp()
{
    m_SelectedIndex--;
    if (m_SelectedIndex < 0)
        m_SelectedIndex = m_ItemsCount - 1;
}

void SettingsBase::navigateDown()
{
    m_SelectedIndex++;
    if (m_SelectedIndex >= m_ItemsCount)
        m_SelectedIndex = 0;
}

void SettingsBase::executeSelected()
{
    if (m_SelectedIndex >= 0 && m_SelectedIndex < m_ItemsCount)
    {
        SettingItem& item = m_Items[m_SelectedIndex];
        if (item.action)
            item.action(item.actionData);
    }
}

void SettingsBase::handleSliderInput(float dt)
{
    if (m_SelectedIndex < 0 || m_SelectedIndex >= m_ItemsCount) return;

    SettingItem& item = m_Items[m_SelectedIndex];
    if (!item.isSlider) return;

    float change = 0.f;

    if (Input::isKeyPressed(sf::Keyboard::Left) || Input::isKeyPressed(sf::Keyboard::A))
        change = -100.f * dt;
    if (Input::isKeyPressed(sf::Keyboard::Right) || Input::isKeyPressed(sf::Keyboard::D))
        change = 100.f * dt;

    if (change != 0.f)
    {
        float newValue = item.sliderValue + change;
        if (newValue < item.sliderMin) newValue = item.sliderMin;
        if (newValue > item.sliderMax) newValue = item.sliderMax;

        if (newValue != item.sliderValue)
        {
            item.sliderValue = newValue;
            onItemSelected(m_SelectedIndex);
        }
    }
}

void SettingsBase::onItemSelected(int index)
{
    // Override in child classes
}

void SettingsBase::updateDisplayValues()
{
    // Override in child classes
}

void SettingsBase::show()
{
    m_Active = true;
    m_AnimationTime = 0.f;
    m_SelectedIndex = 0;
    updateDisplayValues();
}

void SettingsBase::hide()
{
    m_Active = false;
}