// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QString>
#include <QObject>
#include <QXmlStreamReader>

class DomainXmlReader : public QObject
{
    Q_OBJECT

public:
    DomainXmlReader(const QString &path);
    ~DomainXmlReader();
    struct XmlInfo
    {
        QString hypervisorType;
        int indexId;
        QString osId;
        QString shortOsId;
        QString isoDiskPath;
        QString virtualDiskPath;
    };
    XmlInfo readConfigFile(const QString &path);
    QString retrieveDiskPath(QXmlStreamReader &xmlReader, QXmlStreamReader::TokenType token);
    XmlInfo xmlInfo;
};