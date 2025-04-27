// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "karton_debug.h"
#include "domainxmlreader.h"
#include <QFile>
#include <QXmlStreamReader>

DomainXmlReader::DomainXmlReader(const QString &path)
{
    xmlInfo = readConfigFile(path);
}

DomainXmlReader::~DomainXmlReader()
{
}

DomainXmlReader::XmlInfo DomainXmlReader::readConfigFile(const QString &path)
{
    QString isoDiskPath = QString();
    QString virtualDiskPath = QString();
    QString hypervisorType = QString();
    int indexId = 0;
    QString osId = QString();
    QString shortOsId = QString();
    int maxDiskStorage = 0;

    QFile xmlFile(path);
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCInfo(KARTON_DEBUG) << "Could not open reader!!";
        return {};
    }
    QXmlStreamReader xmlReader(&xmlFile);

    while (!xmlReader.atEnd() && !xmlReader.hasError())
    {
        QXmlStreamReader::TokenType token = xmlReader.readNext();
        if (token == QXmlStreamReader::StartElement)
        {
            // qCInfo(KARTON_DEBUG) << xmlReader.name(); // prints each element
            if (xmlReader.name() == QStringLiteral("domain"))
            {
                indexId = xmlReader.attributes().value("id").toInt();
                hypervisorType = xmlReader.attributes().value("type").toString();
            }
            if (xmlReader.name() == QStringLiteral("os") && xmlReader.attributes().hasAttribute(QStringLiteral("id")))
            {
                osId = xmlReader.attributes().value("id").toString();
                shortOsId = xmlReader.attributes().value("short-id").toString();
            }
            if (xmlReader.name() == QStringLiteral("data") && xmlReader.attributes().hasAttribute(QStringLiteral("maxDiskStorage")))
            {
                maxDiskStorage = xmlReader.attributes().value("maxDiskStorage").toInt();
            }
            if (xmlReader.name() == QStringLiteral("disk") && xmlReader.attributes().hasAttribute(QStringLiteral("device")))
            {
                if (xmlReader.attributes().value(QStringLiteral("device")) == QStringLiteral("disk"))
                {
                    virtualDiskPath = retrieveDiskPath(xmlReader, token);
                }
                if (xmlReader.attributes().value(QStringLiteral("device")) == QStringLiteral("cdrom"))
                {
                    isoDiskPath = retrieveDiskPath(xmlReader, token);
                }
            }
        }
    }

    if (xmlReader.hasError())
    {
        qCWarning(KARTON_DEBUG) << "XML parsing error:" << xmlReader.errorString();
    }
    xmlFile.close();

    // qCInfo(KARTON_DEBUG) << "ISO:" << isoDiskPath; // check if populates correctly
    // qCInfo(KARTON_DEBUG) << "Disk:" << virtualDiskPath;
    // qCInfo(KARTON_DEBUG) << "type:" << hypervisorType;
    // qCInfo(KARTON_DEBUG) << "id index:" << indexId;
    // qCInfo(KARTON_DEBUG) << "id:" << osId;
    // qCInfo(KARTON_DEBUG) << "short:" << maxDiskStorage;
    
    return {
        hypervisorType,
        indexId,
        osId,
        shortOsId,
        isoDiskPath,
        virtualDiskPath,
        maxDiskStorage};
}

QString DomainXmlReader::retrieveDiskPath(QXmlStreamReader &xmlReader, QXmlStreamReader::TokenType token)
{
    while (!xmlReader.atEnd())
    {
        token = xmlReader.readNext();
        if (token == QXmlStreamReader::StartElement && xmlReader.name() == QStringLiteral("source"))
        {
            return xmlReader.attributes().value("file").toString();
        }
    }
    qCCritical(KARTON_DEBUG) << "did not find path.";
    return QString();
}
