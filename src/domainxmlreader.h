// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QObject>
#include <QString>
#include <QXmlStreamReader>

class DomainXmlReader : public QObject
{
    Q_OBJECT

public:
    DomainXmlReader(const QString &path);
    ~DomainXmlReader();
    struct XmlInfo {
        QString hypervisorType;
        int indexId = 0;
        QString osId;
        QString shortOsId;
        QString isoDiskPath;
        QString virtualDiskPath;
        int maxDiskStorage = 0;
    };
    XmlInfo readConfigFile(const QString &path);
    QString retrieveDiskPath(QXmlStreamReader &xmlReader, QXmlStreamReader::TokenType token);
    XmlInfo m_xmlInfo;
};