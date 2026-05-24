#pragma once
#include <SFML/System/String.hpp>
#include <string>
#include <codecvt>
#include <locale>

// Хелперы для явного декодирования UTF-8 строк в sf::String.
// Исправлена проблема с _STL_VERIFY

inline sf::String u8(const std::string& s)
{
    if (s.empty()) return sf::String();

    // Способ 1:直接用 sf::String 的 fromUtf8 (最可靠)
    return sf::String::fromUtf8(s.begin(), s.end());
}

inline sf::String u8(const char* s)
{
    if (!s || *s == '\0') return sf::String();

    // Используем fromUtf8 с указателями
    return sf::String::fromUtf8(s, s + std::strlen(s));
}

// Альтернативная версия для строковых литералов (компилируется в compile-time)
inline sf::String operator"" _u8(const char* str, std::size_t len)
{
    return sf::String::fromUtf8(str, str + len);
}