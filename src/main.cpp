// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QIcon>
#include <QUrl>
#include <QQuickStyle>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KIconTheme>
#include "karton.h"
#include "vmlistmodel.h"
#include <libvirt/libvirt.h>
#include <iostream>
#include "karton_debug.h"

int main(int argc, char *argv[])
{
    KIconTheme::initTheme();
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("karton");
    QApplication::setOrganizationName(QStringLiteral("KDE"));
    QApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QApplication::setApplicationName(QStringLiteral("Karton"));
    QApplication::setDesktopFileName(QStringLiteral("org.kde.karton"));
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.karton")));
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    QQmlApplicationEngine engine;

    qCDebug(KARTON_DEBUG) << "Hello! Starting application...";
    Karton karton;
    VMModel *model = new VMModel(&karton);
    model->refreshAllDomains();
    
    engine.rootContext()->setContextProperty(QStringLiteral("Karton"), &karton);
    engine.rootContext()->setContextProperty(QStringLiteral("VMModel"), model);


    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.loadFromModule("org.kde.karton", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
