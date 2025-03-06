// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QTextStream>
#include <QList>
#include <QDir>
#include <QDebug>
#include <QtEnvironmentVariables>
#include <QQuickStyle>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "backendengine.h"
#include "kartonerror.h"
#include "kartonmachine.h"
#include "configlists.h"

QList<KartonMachine *> *BackendEngine::m_machineList = new QList<KartonMachine *>();
QString BackendEngine::m_vmRepoPath = QDir::homePath() + QStringLiteral("/KartonVMs");
QStringList BackendEngine::m_osList = QStringList();
QStringList BackendEngine::m_firmwareList = QStringList();
QString BackendEngine::m_selectedIsoFile = QString();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    BackendEngine eng;
    ConfigLists cfg;
    eng.setOsList(cfg.osList());
    eng.setFirmwareList(cfg.firmwareList());
    qmlRegisterType<BackendEngine>("backendengine", 1, 0, "BackendEngine");
    qmlRegisterType<KartonError>("kartonerror", 1, 0, "KartonError");

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.karton")));

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/karton/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
