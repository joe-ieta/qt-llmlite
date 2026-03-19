#pragma once

#include <QCoreApplication>
#include <QLocale>
#include <QSettings>
#include <QString>
#include <QStringView>

namespace qtllm::toolsinside {

inline QString toolsInsideUiLanguage()
{
    QString language;
    if (QCoreApplication::instance()) {
        language = QCoreApplication::instance()->property("qtllm.uiLanguage").toString().trimmed().toLower();
    }

    if (language.isEmpty()) {
        QSettings settings(QStringLiteral("qtllm"), QStringLiteral("tools_inside"));
        language = settings.value(QStringLiteral("display/languageMode")).toString().trimmed().toLower();
    }

    if (language.startsWith(QStringLiteral("zh"))) {
        return QStringLiteral("zh");
    }
    if (language.startsWith(QStringLiteral("en"))) {
        return QStringLiteral("en");
    }

    switch (QLocale::system().language()) {
    case QLocale::Chinese:
        return QStringLiteral("zh");
    default:
        return QStringLiteral("en");
    }
}

inline QString ti18n(QStringView english, QStringView chinese)
{
    return toolsInsideUiLanguage() == QStringLiteral("zh") ? chinese.toString() : english.toString();
}

} // namespace qtllm::toolsinside
