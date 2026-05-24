#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <cstring>

class SettingsBase
{
public:
    SettingsBase();
    virtual ~SettingsBase();

    virtual void update(float dt, const sf::RenderWindow& window) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    virtual void show();
    virtual void hide();
    virtual bool isActive() const { return m_Active; }

    void setOnCloseCallback(std::function<void()> callback) { m_OnClose = callback; }

protected:
    struct SettingItem
    {
        char label[32];
        char value[32];
        void (*action)(void*);
        void* actionData;
        bool isSlider;
        float sliderValue;
        float sliderMin;
        float sliderMax;
        int type;
    };

    void addItem(const char* label, const char* value, void (*action)(void*), void* data,
        bool isSlider, float val, float minv, float maxv, int type);
    void clearItems();
    void navigateUp();
    void navigateDown();
    void executeSelected();
    void handleSliderInput(float dt);

    virtual void onItemSelected(int index);
    virtual void updateDisplayValues();
    virtual void setupUI(const char* title);

    SettingItem* m_Items;
    int m_ItemsCount;
    int m_ItemsCapacity;
    int m_SelectedIndex;
    bool m_Active;
    float m_AnimationTime;
    float m_RepeatDelay;

    sf::Text m_Title;
    sf::RectangleShape m_Panel;
    sf::RectangleShape m_Overlay;

    std::function<void()> m_OnClose;

    static constexpr int WIN_WIDTH = 1280;
    static constexpr int WIN_HEIGHT = 720;
    static constexpr float REPEAT_DELAY = 0.2f;
    static constexpr float REPEAT_INTERVAL = 0.05f;
};