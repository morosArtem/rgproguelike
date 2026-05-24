#include "UpgradeSelectionUI.h"
#include "AssetManager.h"
#include "Utf.h"
#include <cmath>

UpgradeSelectionUI::UpgradeSelectionUI()
    : m_Active(false)
    , m_AnimTime(0.f)
    , m_SelectionTimer(0.f)
    , m_SelectedIndex(0)
{
    const auto& font = AssetManager::instance().getFont();
    if (AssetManager::instance().isFontLoaded())
    {
        // Заголовок с фоном
        m_Title.setFont(font);
        m_Title.setString(u8("ВЫБЕРИТЕ УЛУЧШЕНИЕ"));
        m_Title.setCharacterSize(48);
        m_Title.setFillColor(sf::Color(255, 220, 100));
        m_Title.setStyle(sf::Text::Bold);
        auto tb = m_Title.getLocalBounds();
        m_Title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        m_Title.setPosition(640.f, 100.f);

        // Фон для заголовка
        m_TitleBg.setSize({ tb.width + 80.f, 70.f });
        m_TitleBg.setFillColor(sf::Color(0, 0, 0, 180));
        m_TitleBg.setOutlineColor(sf::Color(255, 220, 100));
        m_TitleBg.setOutlineThickness(2.f);
        m_TitleBg.setOrigin((tb.width + 80.f) / 2.f, 35.f);
        m_TitleBg.setPosition(640.f, 100.f);

        m_Subtitle.setFont(font);
        m_Subtitle.setString(u8("НОВЫЙ УРОВЕНЬ ДОСТИГНУТ!"));
        m_Subtitle.setCharacterSize(22);
        m_Subtitle.setFillColor(sf::Color(200, 200, 100));
        m_Subtitle.setStyle(sf::Text::Bold);
        auto sb = m_Subtitle.getLocalBounds();
        m_Subtitle.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
        m_Subtitle.setPosition(640.f, 165.f);

        // Инструкция внизу
        m_Instruction.setFont(font);
        m_Instruction.setString(u8("Нажмите 1, 2 или 3 для выбора"));
        m_Instruction.setCharacterSize(20);
        m_Instruction.setFillColor(sf::Color(180, 180, 180));
        auto ib = m_Instruction.getLocalBounds();
        m_Instruction.setOrigin(ib.left + ib.width / 2.f, ib.top + ib.height / 2.f);
        m_Instruction.setPosition(640.f, 650.f);
    }

    m_Overlay.setSize({ 1280.f, 720.f });
    m_Overlay.setFillColor(sf::Color(0, 0, 0, 0));
}

void UpgradeSelectionUI::show(const std::vector<UpgradeType>& upgrades)
{
    m_Active = true;
    m_AnimTime = 0.f;
    m_SelectionTimer = 0.f;
    m_SelectedIndex = 0;
    createCards(upgrades);
}

void UpgradeSelectionUI::hide()
{
    m_Active = false;
    m_Cards.clear();
}

void UpgradeSelectionUI::createCards(const std::vector<UpgradeType>& upgrades)
{
    m_Cards.clear();

    if (upgrades.empty()) return;

    const auto& font = AssetManager::instance().getFont();
    if (!AssetManager::instance().isFontLoaded()) return;

    int totalWidth = CARD_WIDTH * static_cast<int>(upgrades.size()) + CARD_SPACING * (static_cast<int>(upgrades.size()) - 1);
    int startX = (1280 - totalWidth) / 2;
    int y = 280;

    for (size_t i = 0; i < upgrades.size(); ++i)
    {
        UpgradeCard card;
        card.type = upgrades[i];
        card.hovered = false;
        card.hoverAnim = 0.f;
        card.selectionAnim = 0.f;

        const auto& info = Upgrade::getInfo(upgrades[i]);

        // Основной фон карточки
        card.background.setSize({ static_cast<float>(CARD_WIDTH), static_cast<float>(CARD_HEIGHT) });
        card.background.setPosition(static_cast<float>(startX + i * (CARD_WIDTH + CARD_SPACING)), static_cast<float>(y));
        card.background.setFillColor(sf::Color(25, 25, 40, 230));
        card.background.setOutlineColor(info.color);
        card.background.setOutlineThickness(3.f);

        // Свечение для выделенной карточки
        card.glow.setSize({ static_cast<float>(CARD_WIDTH) + 8, static_cast<float>(CARD_HEIGHT) + 8 });
        card.glow.setPosition(card.background.getPosition().x - 4, static_cast<float>(y) - 4);
        card.glow.setFillColor(sf::Color::Transparent);
        card.glow.setOutlineThickness(3.f);

        // Название улучшения
        card.name.setFont(font);
        card.name.setString(u8(info.name));
        card.name.setCharacterSize(26);
        card.name.setFillColor(info.color);
        card.name.setStyle(sf::Text::Bold);
        auto nb = card.name.getLocalBounds();
        card.name.setOrigin(nb.left + nb.width / 2.f, nb.top + nb.height / 2.f);
        card.name.setPosition(card.background.getPosition().x + CARD_WIDTH / 2.f, static_cast<float>(y) + 55);

        // Описание
        card.description.setFont(font);
        card.description.setString(u8(info.description));
        card.description.setCharacterSize(16);
        card.description.setFillColor(sf::Color(220, 220, 220));
        card.description.setStyle(sf::Text::Regular);
        auto db = card.description.getLocalBounds();
        card.description.setOrigin(db.left + db.width / 2.f, db.top + db.height / 2.f);
        card.description.setPosition(card.background.getPosition().x + CARD_WIDTH / 2.f, static_cast<float>(y) + 120);

        // Номер уровня/выбора
        card.levelText.setFont(font);
        card.levelText.setString("[" + std::to_string(i + 1) + "]");
        card.levelText.setCharacterSize(20);
        card.levelText.setFillColor(sf::Color(255, 255, 255, 150));
        auto lb = card.levelText.getLocalBounds();
        card.levelText.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        card.levelText.setPosition(card.background.getPosition().x + CARD_WIDTH / 2.f, static_cast<float>(y) + 190);

        m_Cards.push_back(std::move(card));
    }
}

void UpgradeSelectionUI::update(float dt)
{
    if (m_Active)
    {
        // Анимация появления
        m_AnimTime += dt;
        float alpha = std::min(1.f, m_AnimTime * 4.f);
        m_Overlay.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(200 * alpha)));

        // Анимация выделения
        if (m_SelectionTimer > 0.f)
        {
            m_SelectionTimer -= dt;
        }

        updateCardAnimation(dt);
    }
}

void UpgradeSelectionUI::updateCardAnimation(float dt)
{
    for (auto& card : m_Cards)
    {
        // Анимация наведения
        if (card.hovered)
        {
            card.hoverAnim += dt * 8.f;
            if (card.hoverAnim > 1.f) card.hoverAnim = 1.f;
        }
        else
        {
            card.hoverAnim -= dt * 8.f;
            if (card.hoverAnim < 0.f) card.hoverAnim = 0.f;
        }

        // Анимация выделения выбранной карточки
        float scale = 0.9f + 0.1f * std::min(1.f, m_AnimTime * 10.f);
        float hoverScale = 1.f + card.hoverAnim * 0.05f;
        float finalScale = scale * hoverScale;

        card.background.setScale(finalScale, finalScale);
        card.name.setScale(finalScale, finalScale);
        card.description.setScale(finalScale, finalScale);
        card.levelText.setScale(finalScale, finalScale);

        // Свечение для наведения
        if (card.hoverAnim > 0.1f)
        {
            card.glow.setOutlineColor(sf::Color(255, 220, 100, static_cast<sf::Uint8>(150 * card.hoverAnim)));
        }
    }
}

void UpgradeSelectionUI::draw(sf::RenderWindow& window)
{
    if (!m_Active) return;

    window.draw(m_Overlay);

    // Анимация появления элементов
    float titleAlpha = std::min(1.f, m_AnimTime * 5.f);
    float titleScale = 0.7f + 0.3f * titleAlpha;

    m_TitleBg.setScale(titleScale, titleScale);
    m_TitleBg.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(180 * titleAlpha)));
    window.draw(m_TitleBg);

    m_Title.setScale(titleScale, titleScale);
    m_Title.setFillColor(sf::Color(255, 220, 100, static_cast<sf::Uint8>(255 * titleAlpha)));
    window.draw(m_Title);

    m_Subtitle.setScale(titleScale, titleScale);
    m_Subtitle.setFillColor(sf::Color(200, 200, 100, static_cast<sf::Uint8>(255 * titleAlpha)));
    window.draw(m_Subtitle);

    // Рисуем карточки
    for (size_t i = 0; i < m_Cards.size(); ++i)
    {
        drawCard(window, m_Cards[i], static_cast<int>(i));
    }

    float instructionAlpha = std::min(1.f, std::max(0.f, (m_AnimTime - 0.5f) * 3.f));
    m_Instruction.setFillColor(sf::Color(180, 180, 180, static_cast<sf::Uint8>(255 * instructionAlpha)));
    window.draw(m_Instruction);
}

void UpgradeSelectionUI::drawCard(sf::RenderWindow& window, UpgradeCard& card, int index)
{
    // Рисуем свечение
    if (card.hoverAnim > 0.1f)
    {
        window.draw(card.glow);
    }

    // Рисуем фон
    window.draw(card.background);

    // Рисуем иконку (символ)
    sf::CircleShape iconBg(card.hoverAnim > 0.5f ? 30.f : 28.f);
    iconBg.setFillColor(card.name.getFillColor());
    iconBg.setOutlineColor(sf::Color::White);
    iconBg.setOutlineThickness(2.f);
    iconBg.setOrigin(28.f, 28.f);
    iconBg.setPosition(card.background.getPosition().x + CARD_WIDTH / 2.f,
        card.background.getPosition().y + 40);

    float iconScale = 1.f + card.hoverAnim * 0.1f;
    iconBg.setScale(iconScale, iconScale);
    window.draw(iconBg);

    // Рисуем символ внутри иконки
    sf::Text iconSymbol;
    iconSymbol.setFont(AssetManager::instance().getFont());
    iconSymbol.setCharacterSize(28);
    iconSymbol.setFillColor(sf::Color::White);
    iconSymbol.setStyle(sf::Text::Bold);

    // Выбираем символ в зависимости от типа улучшения
    switch (card.type)
    {
    case UpgradeType::HEALTH_BOOST:    iconSymbol.setString("?"); break;
    case UpgradeType::DAMAGE_BOOST:    iconSymbol.setString("?"); break;
    case UpgradeType::SPEED_BOOST:     iconSymbol.setString("?"); break;
    case UpgradeType::FIRE_RATE_BOOST: iconSymbol.setString("?"); break;
    case UpgradeType::REGENERATION:    iconSymbol.setString("?"); break;
    case UpgradeType::PIERCING_SHOTS:  iconSymbol.setString("?"); break;
    case UpgradeType::EXPLOSIVE_SHOTS: iconSymbol.setString("??"); break;
    case UpgradeType::LIFESTEAL:       iconSymbol.setString("??"); break;
    case UpgradeType::DOUBLE_SHOT:     iconSymbol.setString("??"); break;
    case UpgradeType::SHIELD_CHARGE:   iconSymbol.setString("??"); break;
    case UpgradeType::DASH_RESET:      iconSymbol.setString("?"); break;
    case UpgradeType::GREED:           iconSymbol.setString("??"); break;
    case UpgradeType::CRITICAL_HIT:    iconSymbol.setString("?"); break;
    default:                           iconSymbol.setString("?"); break;
    }

    auto isb = iconSymbol.getLocalBounds();
    iconSymbol.setOrigin(isb.left + isb.width / 2.f, isb.top + isb.height / 2.f);
    iconSymbol.setPosition(card.background.getPosition().x + CARD_WIDTH / 2.f,
        card.background.getPosition().y + 40);
    iconSymbol.setScale(iconScale, iconScale);
    window.draw(iconSymbol);

    // Рисуем название
    window.draw(card.name);

    // Рисуем описание
    window.draw(card.description);

    // Рисуем номер
    window.draw(card.levelText);

    // Рисуем кнопку "Выбрать" при наведении
    if (card.hoverAnim > 0.3f)
    {
        sf::RectangleShape selectBtn;
        selectBtn.setSize({ 100.f, 30.f });
        selectBtn.setFillColor(sf::Color(card.name.getFillColor().r,
            card.name.getFillColor().g,
            card.name.getFillColor().b,
            static_cast<sf::Uint8>(200 * card.hoverAnim)));
        selectBtn.setOutlineColor(sf::Color::White);
        selectBtn.setOutlineThickness(1.f);
        selectBtn.setOrigin(50.f, 15.f);
        selectBtn.setPosition(card.background.getPosition().x + CARD_WIDTH / 2.f,
            card.background.getPosition().y + CARD_HEIGHT - 35);
        selectBtn.setScale(1.f + card.hoverAnim * 0.1f, 1.f + card.hoverAnim * 0.1f);
        window.draw(selectBtn);

        sf::Text selectText;
        selectText.setFont(AssetManager::instance().getFont());
        selectText.setString(u8("ВЫБРАТЬ"));
        selectText.setCharacterSize(14);
        selectText.setFillColor(sf::Color::White);
        auto stb = selectText.getLocalBounds();
        selectText.setOrigin(stb.left + stb.width / 2.f, stb.top + stb.height / 2.f);
        selectText.setPosition(card.background.getPosition().x + CARD_WIDTH / 2.f,
            card.background.getPosition().y + CARD_HEIGHT - 35);
        selectText.setScale(1.f + card.hoverAnim * 0.1f, 1.f + card.hoverAnim * 0.1f);
        window.draw(selectText);
    }
}

UpgradeType UpgradeSelectionUI::handleInput(const sf::RenderWindow& window)
{
    if (!m_Active || m_Cards.empty()) return UpgradeType::HEALTH_BOOST;

    // Получаем позицию мыши
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    // Обновляем состояние наведения
    bool anyHovered = false;
    for (size_t i = 0; i < m_Cards.size(); ++i)
    {
        auto& card = m_Cards[i];
        if (card.background.getGlobalBounds().contains(mousePosF))
        {
            card.hovered = true;
            anyHovered = true;
            m_SelectedIndex = static_cast<int>(i);
        }
        else
        {
            card.hovered = false;
        }
    }

    // Анимация при выборе
    if (m_SelectionTimer > 0.f)
    {
        return UpgradeType::HEALTH_BOOST; // Возвращаем заглушку, ожидаем завершения анимации
    }

    // Проверка нажатий цифр
    for (size_t i = 0; i < m_Cards.size() && i < 3; ++i)
    {
        bool isPressed = false;

        switch (i)
        {
        case 0: isPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1); break;
        case 1: isPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2); break;
        case 2: isPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Num3); break;
        default: break;
        }

        if (isPressed)
        {
            UpgradeType selected = m_Cards[i].type;
            m_SelectionTimer = 0.2f; // Небольшая задержка перед закрытием
            hide();
            return selected;
        }
    }

    // Проверка клика мышью
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        static bool wasPressed = false;
        if (!wasPressed)
        {
            for (size_t i = 0; i < m_Cards.size(); ++i)
            {
                if (m_Cards[i].hovered)
                {
                    UpgradeType selected = m_Cards[i].type;
                    m_SelectionTimer = 0.2f;
                    hide();
                    return selected;
                }
            }
        }
        wasPressed = true;
    }

    return UpgradeType::HEALTH_BOOST; // Возвращаем заглушку
}