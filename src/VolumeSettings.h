#pragma once
#include "SettingsBase.h"

class VolumeSettings : public SettingsBase
{
public:
    VolumeSettings();
    virtual ~VolumeSettings();

    void update(float dt, const sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

protected:
    void onItemSelected(int index) override;
    void updateDisplayValues() override;

private:
    void createItems();
    void applyVolume();

    float m_MasterVolume;
    float m_MusicVolume;
    float m_SfxVolume;

    static void onBack(void* data);
};