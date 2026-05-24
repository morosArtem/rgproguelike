#include "UpgradeViewer.h"
#include "AssetManager.h"
#include "Utf.h"
#include "Input.h"
#include <cmath>

UpgradeViewer::UpgradeViewer()
    : m_Active(false)
    , m_AnimTime(0.f)
    , m_ScrollOffset(0.f)
    , m_TargetScrollOffset(0.f)
    , m_MaxScrollOffset(0.f)
    , m_MousePressed(false)
    , m_DraggingScroll(false)
{
    const auto& font = AssetManager::instance().getFont();
    if (AssetManager::instance().isFontLoaded())
    {
        m_Title.setFont(font);
        m_Title.setString(u8("ПОЛУЧЕННЫЕ УЛУЧШЕНИЯ"));
        m_Title.setCharacterSize(44);
        m_Title.setFillColor(sf::Color(255, 220, 100));
        m_Title.setStyle(sf::Text::Bold);
        auto tb = m_Title.getLocalBounds();
        m_Title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        m_Title.setPosition(640.f, 60.f);

        m_Subtitle.setFont(font);
        m_Subtitle.setString(u8("Все улучшения, которые вы получили во время игры"));
        m_Subtitle.setCharacterSize(18);
        m_Subtitle.setFillColor(sf::Color(180, 180, 180));
        auto sb = m_Subtitle.getLocalBounds();
        m_Subtitle.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
        m_Subtitle.setPosition(640.f, 105.f);

        m_EmptyText.setFont(font);
        m_EmptyText.setString(u8("У вас пока нет улучшений\n\nПроходите уровни и побеждайте врагов,\nчтобы получать новые способности!"));
        m_EmptyText.setCharacterSize(22);
        m_EmptyText.setFillColor(sf::Color(150, 150, 150));
        m_EmptyText.setStyle(sf::Text::Regular);
        auto eb = m_EmptyText.getLocalBounds();
        m_EmptyText.setOrigin(eb.left + eb.width / 2.f, eb.top + eb.height / 2.f);
        m_EmptyText.setPosition(640.f, 380.f);

        m_Instruction.setFont(font);
        m_Instruction.setString(u8"ESC - закрыть | Стрелки / W/S - прокрутка");
        m_Instruction.setCharacterSize(16);
        m_Instruction.setFillColor(sf::Color(140, 140, 140));
        auto ib = m_Instruction.getLocalBounds();
        m_Instruction.setOrigin(ib.left + ib.width / 2.f, ib.top + ib.height / 2.f);
        m_Instruction.setPosition(640.f, 670.f);
    }

    m_Panel.setSize({ 900.f, 500.f });
    m_Panel.setFillColor(sf::Color(20, 20, 35, 240));
    m_Panel.setOutlineColor(sf::Color(100, 100, 140));
    m_Panel.setOutlineThickness(2.f);
    m_Panel.setOrigin(450.f, 250.f);
    m_Panel.setPosition(640.f, 380.f);

    m_Overlay.setSize({ 1280.f, 720.f });
    m_Overlay.setFillColor(sf::Color(0, 0, 0, 0));

    m_ScrollBar.setSize({ 8.f, 460.f });
    m_ScrollBar.setFillColor(sf::Color(40, 40, 60));
    m_ScrollBar.setPosition(1100.f, 150.f);

    m_ScrollThumb.setSize({ 8.f, 60.f });
    m_ScrollThumb.setFillColor(sf::Color(200, 180, 100));
    m_ScrollThumb.setPosition(1100.f, 150.f);
}

void UpgradeViewer::show(const std::vector<UpgradeType>& acquiredUpgrades)
{
    m_AcquiredUpgrades = acquiredUpgrades;
    m_Active = true;
    m_AnimTime = 0.f;
    m_ScrollOffset = 0.f;
    m_TargetScrollOffset = 0.f;

    // Вычисляем максимальную прокрутку
    int totalEntries = static_cast<int>(m_AcquiredUpgrades.size());
    if (totalEntries > ENTRIES_PER_PAGE)
    {
        m_MaxScrollOffset = static_cast<float>((totalEntries - ENTRIES_PER_PAGE) * ENTRY_HEIGHT);
    }
    else
    {
        m_MaxScrollOffset = 0.f;
    }

    createEntries();
}

void UpgradeViewer::hide()
{
    m_Active = false;
    m_Entries.clear();
    if (m_OnClose) m_OnClose();
}

void UpgradeViewer::createEntries()
{
    m_Entries.clear();

    const auto& font = AssetManager::instance().getFont();
    if (!AssetManager::instance().isFontLoaded()) return;

    int panelX = static_cast<int>(m_Panel.getPosition().x - m_Panel.getSize().x / 2 + 20);
    int startY = static_cast<int>(m_Panel.getPosition().y - m_Panel.getSize().y / 2 + 20);

    for (size_t i = 0; i < m_AcquiredUpgrades.size(); ++i)
    {
        UpgradeEntry entry;
        entry.type = m_AcquiredUpgrades[i];
        entry.animTime = 0.f;

        const auto& info = Upgrade::getInfo(entry.type);

        // Фон записи
        entry.background.setSize({ 840.f, static_cast<float>(ENTRY_HEIGHT - 10) });
        entry.background.setPosition(static_cast<float>(panelX),
            static_cast<float>(startY + static_cast<int>(i) * ENTRY_HEIGHT));
        entry.background.setFillColor(sf::Color(30, 30, 45, 200));
        entry.background.setOutlineColor(info.color);
        entry.background.setOutlineThickness(1.f);

        // Иконка
        entry.iconBg.setRadius(25.f);
        entry.iconBg.setFillColor(info.color);
        entry.iconBg.setOutlineColor(sf::Color::White);
        entry.iconBg.setOutlineThickness(2.f);
        entry.iconBg.setOrigin(25.f, 25.f);
        entry.iconBg.setPosition(static_cast<float>(panelX + 40),
            static_cast<float>(startY + static_cast<int>(i) * ENTRY_HEIGHT + 35));

        // Символ иконки
        entry.iconSymbol.setFont(font);
        entry.iconSymbol.setCharacterSize(24);
        entry.iconSymbol.setFillColor(sf::Color::White);
        entry.iconSymbol.setStyle(sf::Text::Bold);

        switch (entry.type)
        {
        case UpgradeType::HEALTH_BOOST:    entry.iconSymbol.setString("?"); break;
        case UpgradeType::DAMAGE_BOOST:    entry.iconSymbol.setString("?"); break;
        case UpgradeType::SPEED_BOOST:     entry.iconSymbol.setString("?"); break;
        case UpgradeType::FIRE_RATE_BOOST: entry.iconSymbol.setString("?"); break;
        case UpgradeType::REGENERATION:    entry.iconSymbol.setString("?"); break;
        case UpgradeType::PIERCING_SHOTS:  entry.iconSymbol.setString("?"); break;
        case UpgradeType::EXPLOSIVE_SHOTS: entry.iconSymbol.setString("??"); break;
        case UpgradeType::LIFESTEAL:       entry.iconSymbol.setString("??"); break;
        case UpgradeType::DOUBLE_SHOT:     entry.iconSymbol.setString("??"); break;
        case UpgradeType::SHIELD_CHARGE:   entry.iconSymbol.setString("??"); break;
        case UpgradeType::DASH_RESET:      entry.iconSymbol.setString("?"); break;
        case UpgradeType::GREED:           entry.iconSymbol.setString("??"); break;
        case UpgradeType::CRITICAL_HIT:    entry.iconSymbol.setString("?"); break;
        default:                           entry.iconSymbol.setString("?"); break;
        }

        auto isb = entry.iconSymbol.getLocalBounds();
        entry.iconSymbol.setOrigin(isb.left + isb.width / 2.f, isb.top + isb.height / 2.f);
        entry.iconSymbol.setPosition(static_cast<float>(panelX + 40),
            static_cast<float>(startY + static_cast<int>(i) * ENTRY_HEIGHT + 35));

        // Название
        entry.name.setFont(font);
        entry.name.setString(u8(info.name));
        entry.name.setCharacterSize(22);
        entry.name.setFillColor(info.color);
        entry.name.setStyle(sf::Text::Bold);
        entry.name.setPosition(static_cast<float>(panelX + 85),
            static_cast<float>(startY + static_cast<int>(i) * ENTRY_HEIGHT + 22));

        // Описание
        entry.description.setFont(font);
        entry.description.setString(u8(info.description));
        entry.description.setCharacterSize(16);
        entry.description.setFillColor(sf::Color(200, 200, 200));
        entry.description.setPosition(static_cast<float>(panelX + 85),
            static_cast<float>(startY + static_cast<int>(i) * ENTRY_HEIGHT + 50));

        // Номер
        entry.levelText.setFont(font);
        entry.levelText.setString("#" + std::to_string(i + 1));
        entry.levelText.setCharacterSize(14);
        entry.levelText.setFillColor(sf::Color(150, 150, 150));
        entry.levelText.setPosition(static_cast<float>(panelX + 800),
            static_cast<float>(startY + static_cast<int>(i) * ENTRY_HEIGHT + 35));

        m_Entries.push_back(std::move(entry));
    }
}

void UpgradeViewer::update(float dt)
{
    if (!m_Active) return;

    m_AnimTime += dt;
    float alpha = std::min(1.f, m_AnimTime * 5.f);
    m_Overlay.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(200 * alpha)));

    // Обновляем анимацию записей
    for (auto& entry : m_Entries)
    {
        entry.animTime += dt;
    }

    // Прокрутка клавишами
    float scrollSpeed = 300.f * dt;
    if (Input::isKeyPressed(sf::Keyboard::Up) || Input::isKeyPressed(sf::Keyboard::W))
    {
        m_TargetScrollOffset -= scrollSpeed;
    }
    if (Input::isKeyPressed(sf::Keyboard::Down) || Input::isKeyPressed(sf::Keyboard::S))
    {
        m_TargetScrollOffset += scrollSpeed;
    }

    // Прокрутка колесиком мыши
    // (для простоты используем клавиши, можно добавить колесо позже)

    // Ограничиваем прокрутку
    if (m_TargetScrollOffset < 0.f) m_TargetScrollOffset = 0.f;
    if (m_TargetScrollOffset > m_MaxScrollOffset) m_TargetScrollOffset = m_MaxScrollOffset;

    // Плавная прокрутка
    m_ScrollOffset += (m_TargetScrollOffset - m_ScrollOffset) * dt * 10.f;

    // Обновляем позиции записей с учетом прокрутки
    int panelX = static_cast<int>(m_Panel.getPosition().x - m_Panel.getSize().x / 2 + 20);
    int startY = static_cast<int>(m_Panel.getPosition().y - m_Panel.getSize().y / 2 + 20 - m_ScrollOffset);

    for (size_t i = 0; i < m_Entries.size(); ++i)
    {
        float yPos = static_cast<float>(startY + static_cast<int>(i) * ENTRY_HEIGHT);
        m_Entries[i].background.setPosition(static_cast<float>(panelX), yPos);
        m_Entries[i].iconBg.setPosition(static_cast<float>(panelX + 40), yPos + 35);
        m_Entries[i].iconSymbol.setPosition(static_cast<float>(panelX + 40), yPos + 35);
        m_Entries[i].name.setPosition(static_cast<float>(panelX + 85), yPos + 22);
        m_Entries[i].description.setPosition(static_cast<float>(panelX + 85), yPos + 50);
        m_Entries[i].levelText.setPosition(static_cast<float>(panelX + 800), yPos + 35);
    }

    // Обновляем скроллбар
    if (m_MaxScrollOffset > 0.f)
    {
        float thumbHeight = 460.f * (static_cast<float>(ENTRIES_PER_PAGE) /
            static_cast<float>(m_AcquiredUpgrades.size()));
        if (thumbHeight < 40.f) thumbHeight = 40.f;
        m_ScrollThumb.setSize({ 8.f, thumbHeight });

        float thumbPos = 150.f + (m_ScrollOffset / m_MaxScrollOffset) * (460.f - thumbHeight);
        m_ScrollThumb.setPosition(1100.f, thumbPos);
    }

    // Закрытие по ESC
    if (Input::isKeyJustPressed(sf::Keyboard::Escape))
    {
        hide();
    }
}

void UpgradeViewer::draw(sf::RenderWindow& window)
{
    if (!m_Active) return;

    window.draw(m_Overlay);
    window.draw(m_Panel);
    window.draw(m_Title);
    window.draw(m_Subtitle);
    window.draw(m_Instruction);

    if (m_Entries.empty())
    {
        window.draw(m_EmptyText);
    }
    else
    {
        // Рисуем только видимые записи
        int startIndex = static_cast<int>(m_ScrollOffset / ENTRY_HEIGHT);
        if (startIndex < 0) startIndex = 0;

        int endIndex = startIndex + ENTRIES_PER_PAGE + 1;
        if (endIndex > static_cast<int>(m_Entries.size())) endIndex = static_cast<int>(m_Entries.size());

        for (int i = startIndex; i < endIndex; ++i)
        {
            drawEntry(window, m_Entries[i], i);
        }

        // Рисуем скроллбар только если нужно
        if (m_MaxScrollOffset > 0.f)
        {
            window.draw(m_ScrollBar);
            window.draw(m_ScrollThumb);
        }
    }
}

void UpgradeViewer::drawEntry(sf::RenderWindow& window, UpgradeEntry& entry, int index)
{
    // Пропускаем записи вне видимой области
    float yPos = entry.background.getPosition().y;
    float panelTop = m_Panel.getPosition().y - m_Panel.getSize().y / 2;
    float panelBottom = m_Panel.getPosition().y + m_Panel.getSize().y / 2;

    if (yPos + ENTRY_HEIGHT < panelTop || yPos > panelBottom) return;

    // Анимация появления
    float appearAnim = std::min(1.f, entry.animTime * 3.f);
    float scale = 0.95f + appearAnim * 0.05f;

    entry.background.setScale(scale, scale);
    entry.iconBg.setScale(scale, scale);
    entry.iconSymbol.setScale(scale, scale);
    entry.name.setScale(scale, scale);
    entry.description.setScale(scale, scale);
    entry.levelText.setScale(scale, scale);

    window.draw(entry.background);
    window.draw(entry.iconBg);
    window.draw(entry.iconSymbol);
    window.draw(entry.name);
    window.draw(entry.description);
    window.draw(entry.levelText);
}