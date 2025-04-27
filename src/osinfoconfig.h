// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QString>
#include <QObject>

extern "C" // due to undefined references to libosinfo content
{
#include <osinfo/osinfo.h>
}


class OsinfoConfig : public QObject
{
    Q_OBJECT

public:
    OsinfoConfig();
    ~OsinfoConfig();
    QString getOsIdFromShortId(const QString &short_id);
    Q_INVOKABLE QString getShortIdFromId(const QString &id);
    Q_INVOKABLE QString getOsIdFromDisk(const QString &isoDiskPath);
    QString getOsArchitecture(const QString &osId);

private:
    bool initOsDb();
    
    OsinfoLoader *m_loader;
    OsinfoDb *m_db;    
};
