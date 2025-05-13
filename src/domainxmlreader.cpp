// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domainxmlreader.h"

#include <QFile>
#include <QXmlStreamReader>

#include "karton_debug.h"

DomainXmlReader::DomainXmlReader(const QString &path)
    : m_xmlInfo(readConfigFile(path))
{
}

DomainXmlReader::~DomainXmlReader()
{
}

DomainXmlReader::XmlInfo DomainXmlReader::readConfigFile(const QString &path)
{
    XmlInfo info;

    QFile xmlFile(path);
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCInfo(KARTON_DEBUG) << "Could not open reader!!";
        return {};
    }
    QXmlStreamReader xmlReader(&xmlFile);

    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        QXmlStreamReader::TokenType token = xmlReader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            // qCInfo(KARTON_DEBUG) << xmlReader.name(); // prints each element
            if (xmlReader.name() == QStringLiteral("domain")) {
                info.indexId = xmlReader.attributes().value("id").toInt();
                info.hypervisorType = xmlReader.attributes().value("type").toString();
            }
            if (xmlReader.name() == QStringLiteral("os") && xmlReader.attributes().hasAttribute(QStringLiteral("id"))) {
                info.osId = xmlReader.attributes().value("id").toString();
                info.shortOsId = xmlReader.attributes().value("short-id").toString();
            }
            if (xmlReader.name() == QStringLiteral("data") && xmlReader.attributes().hasAttribute(QStringLiteral("maxDiskStorage"))) {
                info.maxDiskStorage = xmlReader.attributes().value("maxDiskStorage").toInt();
            }
            if (xmlReader.name() == QStringLiteral("disk") && xmlReader.attributes().hasAttribute(QStringLiteral("device"))) {
                if (xmlReader.attributes().value(QStringLiteral("device")) == QStringLiteral("disk")) {
                    info.virtualDiskPath = retrieveDiskPath(xmlReader, token);
                }
                if (xmlReader.attributes().value(QStringLiteral("device")) == QStringLiteral("cdrom")) {
                    info.isoDiskPath = retrieveDiskPath(xmlReader, token);
                }
            }
        }
    }

    if (xmlReader.hasError()) {
        qCWarning(KARTON_DEBUG) << "XML parsing error:" << xmlReader.errorString();
        return {};
    }
    xmlFile.close();

    return info;
}

QString DomainXmlReader::retrieveDiskPath(QXmlStreamReader &xmlReader, QXmlStreamReader::TokenType token)
{
    while (!xmlReader.atEnd()) {
        token = xmlReader.readNext();
        if (token == QXmlStreamReader::StartElement && xmlReader.name() == QStringLiteral("source")) {
            return xmlReader.attributes().value("file").toString();
        }
    }
    qCCritical(KARTON_DEBUG) << "did not find path.";
    return QString();
}
