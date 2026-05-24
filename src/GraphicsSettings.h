#pragma once
#include "SettingsBase.h"

class GraphicsSettings : public SettingsBase
{
public:
    GraphicsSettings();
    virtual ~GraphicsSettings();

    void update(float dt, const sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    bool isFullscreen() const { return m_Fullscreen; }
    bool isVSyncEnabled() const { return m_VSync; }
    int getFPSLimit() const { return m_FPSLimit; }

    void apply(sf::RenderWindow& window);

protected:
    void onItemSelected(int index) override;
    void updateDisplayValues() override;

private:
    void createItems();
    void toggleFullscreen();
    void toggleVSync();
    void changeFPSLimit(float delta);

    bool m_Fullscreen;
    bool m_VSync;
    int m_FPSLimit;
    bool m_PendingApply;

    static void onBack(void* data);
};