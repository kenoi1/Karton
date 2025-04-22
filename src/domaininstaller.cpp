// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domaininstaller.h"
#include "domainconfig.h"
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QFile>
#include <QMap>
#include <QUuid>

DomainInstaller::DomainInstaller()
{
}

DomainInstaller::~DomainInstaller()
{
}

void DomainInstaller::addElement(QDomDocument &doc, QDomElement &parent, const QString &name, const QString &value)
{
    QDomElement element = doc.createElement(name);
    if (!value.isEmpty()) {
        QDomText textNode = doc.createTextNode(value);
        element.appendChild(textNode);
    }
    parent.appendChild(element);
}

void DomainInstaller::addElementWithAttributes(QDomDocument &doc,
                                              QDomElement &parent,
                                              const QString &name,
                                              const QString &value,
                                              const QMap<QString, QString> &attributes)
{
    QDomElement element = doc.createElement(name);

    if (!value.isEmpty()) {
        QDomText textNode = doc.createTextNode(value);
        element.appendChild(textNode);
    }

    for (auto i = attributes.cbegin(), end = attributes.cend(); i != end; i++) {
        element.setAttribute(i.key(), i.value());
    }
    parent.appendChild(element);
}

// use configuration???
void DomainInstaller::configureXML(virConnectPtr conn,
                                   const DomainConfig *config)
{
    QFile xmlDomain(QStringLiteral("xmlSample.xml"));
    if (!xmlDomain.open(QFile::WriteOnly | QFile::Text))
    {
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
    addElement(document, root, QStringLiteral("name"), config->name());

    // UUID element
    QString uuidString = QUuid::createUuid().toString();
    uuidString.remove(QLatin1Char('{'));
    uuidString.remove(QLatin1Char('}'));
    addElement(document, root, QStringLiteral("uuid"), uuidString);

    // metadata element
    QDomElement metadata = document.createElement(QStringLiteral("metadata"));
    root.appendChild(metadata);
    QDomElement libosinfo = document.createElement(QStringLiteral("libosinfo:libosinfo"));
    metadata.appendChild(libosinfo);

    // memory element
    QMap<QString, QString> mem;
    mem[QStringLiteral("unit")] = QStringLiteral("KiB");
    addElementWithAttributes(document, root, QStringLiteral("memory"), QString::number(config->maxRam() * 1024), mem);
    addElementWithAttributes(document, root, QStringLiteral("currentMemory"), QString::number(config->maxRam() * 1024), mem);

    // vpu element
    QMap<QString, QString> vcpu;
    vcpu[QStringLiteral("placement")] = QStringLiteral("static");
    addElementWithAttributes(document, root, QStringLiteral("vcpu"), QString::number(config->cpus()), vcpu);

    // os element
    QDomElement os = document.createElement(QStringLiteral("os"));
    root.appendChild(os);
    QMap<QString, QString> type;
    type[QStringLiteral("arch")] = QStringLiteral("x86_64");
    type[QStringLiteral("machine")] = QStringLiteral("HELP!!!!");
    addElementWithAttributes(document, os, QStringLiteral("type"), QStringLiteral("hvm"), type);
    QMap<QString, QString> boot1;
    boot1[QStringLiteral("dev")] = QStringLiteral("cdrom");
    QMap<QString, QString> boot2;
    boot2[QStringLiteral("dev")] = QStringLiteral("hd");
    addElementWithAttributes(document, os, QStringLiteral("boot"), QStringLiteral(""), boot1);
    addElementWithAttributes(document, os, QStringLiteral("boot"), QStringLiteral(""), boot2);

    // features element
    QDomElement features = document.createElement(QStringLiteral("features"));
    root.appendChild(features);
    QDomElement acpi = document.createElement(QStringLiteral("acpi"));
    features.appendChild(acpi);
    QDomElement apic = document.createElement(QStringLiteral("apic"));
    features.appendChild(apic);
    QMap<QString, QString> vmport;
    vmport[QStringLiteral("state")] = QStringLiteral("off");
    addElementWithAttributes(document, features, QStringLiteral("vmport"), QStringLiteral(""), vmport);

    // cpu element
    QMap<QString, QString> cpu;
    cpu[QStringLiteral("mode")] = QStringLiteral("host-passthrough");
    cpu[QStringLiteral("check")] = QStringLiteral("none");
    cpu[QStringLiteral("migratable")] = QStringLiteral("on");
    addElementWithAttributes(document, os, QStringLiteral("boot"), QStringLiteral(""), cpu);


    QMap<QString, QString> clock;
    clock[(QStringLiteral("offset"))] = QStringLiteral("utc");
    addElementWithAttributes(document, root, QStringLiteral("clock"), QStringLiteral(""), clock);
    QMap<QString, QString> timer;
    timer[(QStringLiteral("name"))] = QStringLiteral("rtc");
    timer[(QStringLiteral("tickpolicy"))] = QStringLiteral("catchup");
    addElementWithAttributes(document, clock, QStringLiteral("timer"), QStringLiteral(""), timer);
    timer[(QStringLiteral("name"))] = QStringLiteral("pit");
    timer[(QStringLiteral("tickpolicy"))] = QStringLiteral("delay");
    addElementWithAttributes(document, clock, QStringLiteral("timer"), QStringLiteral(""), timer);
    QMap<QString, QString> timer2;
    timer2[(QStringLiteral("name"))] = QStringLiteral("hept");
    timer2[(QStringLiteral("present"))] = QStringLiteral("no");
    addElementWithAttributes(document, clock, QStringLiteral("timer"), QStringLiteral(""), timer2);

    // write to file
    QString xmlString = document.toString(4);
    QTextStream xmlContent(&xmlDomain);
    xmlContent << xmlString;
    qDebug().noquote() << "Generated XML:";
    qDebug().noquote() << xmlString;
    xmlDomain.close();
}
