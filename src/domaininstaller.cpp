// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domaininstaller.h"
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QFile>
#include <QUuid>

DomainInstaller::DomainInstaller()
{
}

DomainInstaller::~DomainInstaller()
{
}

// void DomainInstaller::addElement(QDomDocument &doc, QDomElement &parent, const QString &name, const QString &value)
// {
//     QDomElement element = doc.createElement(name);
//     QDomText textNode = doc.createTextNode(value);
//     element.appendChild(textNode);
//     parent.appendChild(element);
// }

// void DomainInstaller::addElementWithAttribute(QDomDocument &doc,
//                                               QDomElement &parent,
//                                               const QString &name,
//                                               const QString &value,
//                                               const QMap<QString, QString> &attributes)
// {

// }

// use configuration???
void DomainInstaller::configureXML(virConnectPtr conn,
                                   const QString &name,
                                   const QString &osVariant,
                                   const float memoryGB,
                                   const float storageGB,
                                   const QString &diskPath,
                                   const int cpus)
{
    QFile xmlDomain(QStringLiteral("xmlSample.xml"));
    if (!xmlDomain.open(QFile::WriteOnly | QFile::Text)) {
        qCritical() << "xmlDomain opened in another instance or something??";
        return;
    }

    QDomDocument document;
    QDomElement root = document.createElement(QStringLiteral("domain"));
    root.setAttribute(QStringLiteral("type"), QStringLiteral("kvm"));

    virDomainPtr *domains = nullptr;
    int numDomains = virConnectListAllDomains(conn, &domains, 0);
    root.setAttribute(QStringLiteral("id"), QString::number(numDomains + 1));
    document.appendChild(root);

    // name element
    QDomElement nameElement = document.createElement(QStringLiteral("name"));
    QDomText nameString = document.createTextNode(name);
    nameElement.appendChild(nameString);
    root.appendChild(nameElement);

    // UUID element
    QDomElement uuidElement = document.createElement(QStringLiteral("uuid"));
    QString uuidString = QUuid::createUuid().toString();
    uuidString.remove(QLatin1Char('{'));
    uuidString.remove(QLatin1Char('}'));
    QDomText uuidText = document.createTextNode(uuidString);
    uuidElement.appendChild(uuidText);
    root.appendChild(uuidElement);

    // write to file
    QString xmlString = document.toString(4);
    QTextStream xmlContent(&xmlDomain);
    xmlContent << xmlString;
    qDebug().noquote() << "Generated XML:";
    qDebug().noquote() << "Generated XML:";
    qDebug().noquote() << xmlString;
    xmlDomain.close();
}
