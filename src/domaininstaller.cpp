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
#include "karton_debug.h"
// #include <osinfo/osinfo.h>
// #include <osinfo/osinfo_loader.h>

#include <glib.h>

extern "C" // due to undefined references to libosinfo stuff
{
#include <osinfo/osinfo.h>
}

DomainInstaller::DomainInstaller()
{
}

DomainInstaller::~DomainInstaller()
{
}

QString DomainInstaller::getOsIdFromDisk(QString diskPath)
{
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db = osinfo_loader_get_db(loader);
    osinfo_loader_process_default_path(loader, NULL);

    std::string str = diskPath.toStdString();
    const gchar *location = str.c_str();
    // qCInfo(KARTON_DEBUG) << "IDGGG: " << str << ", WAA: " <<  location;

    GError *error = NULL;
    OsinfoMedia *osMedia = osinfo_media_create_from_location(location, NULL, &error);
    if (!osinfo_db_identify_media(db, osMedia)) {
        return QStringLiteral("fail");
    }
    const gchar *idG = osinfo_entity_get_id(OSINFO_ENTITY(osMedia));
    // const gchar *idG = osinfo_media_get_system_id(osMedia);

    // qCInfo(KARTON_DEBUG) << "IDGGG: " << idG;
    if (error) {
        qCCritical(KARTON_DEBUG) << "Id Finder Error: " << error->message;
    }
    QString id = QString::fromUtf8(idG);
    g_object_unref(osMedia);
    g_object_unref(loader);
    return id;
    
}
void DomainInstaller::initLibosinfo()
{
    // qCInfo(KARTON_DEBUG) << "E?HFWIUEHFIUWEHFIUW";
    OsinfoLoader *loader = osinfo_loader_new();
    OsinfoDb *db = osinfo_loader_get_db(loader);
    osinfo_loader_process_default_path(loader, NULL);

    // const gchar *id = "";
    // OsinfoOS *os = osinfo_db_get_os(db, )

    // OsinfoDeviceList *devices = osinfo_db_get_device_list(db);
    // gint len = osinfo_list_get_length(OSINFO_LIST(devices));
    // for (gint i = 0; i < len; i++) {
    //     qCInfo(KARTON_DEBUG) << "OS DEVICE:" << osinfo_get_id(device);
    // }
    // GList *device = osinfo_db_unique_values_for_property_in_device(db, "name");

    // LISTS ALL IDS FOR OPERATING SYSTEMS IN DB
    OsinfoOsList *osList = osinfo_db_get_os_list(db);
    gint len = osinfo_list_get_length(OSINFO_LIST(osList));

    for (gint i = 0; i < len; i++)
    {
        OsinfoOs *os = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(osList), i));
        const gchar *id = osinfo_entity_get_id(OSINFO_ENTITY(os));
        qCInfo(KARTON_DEBUG) << "OS ID:" << id;
    }

    g_object_unref(osList);
    // g_object_unref(devices);
    g_object_unref(loader);
}

void DomainInstaller::addElement(QDomDocument &doc, QDomElement &parent, const QString &name, const QString &value)
{
    QDomElement element = doc.createElement(name);
    if (!value.isEmpty())
    {
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
    if (!value.isEmpty())
    {
        QDomText textNode = doc.createTextNode(value);
        element.appendChild(textNode);
    }

    for (auto i = attributes.cbegin(), end = attributes.cend(); i != end; i++)
    {
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
        qCCritical(KARTON_DEBUG) << "xmlDomain opened in another instance or something??";
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

    // LIBOSINFO
    QDomElement libosinfo = document.createElement(QStringLiteral("libosinfo:libosinfo"));
    metadata.appendChild(libosinfo);

    QMap<QString, QString> idMap;
    QString id = getOsIdFromDisk(config->diskPath());
    idMap[QStringLiteral("id")] = id;
    addElementWithAttributes(document, libosinfo, QStringLiteral("libosinfo:os"), QStringLiteral(""), idMap);
    qCInfo(KARTON_DEBUG) << "OS ID:" << id;

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

    // clock element
    QDomElement clock = document.createElement(QStringLiteral("clock"));
    clock.setAttribute(QStringLiteral("offset"), QStringLiteral("utc"));
    root.appendChild(clock);
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

    // off elements
    addElement(document, root, QStringLiteral("on_poweroff"), QStringLiteral("destroy"));
    addElement(document, root, QStringLiteral("on_reboot"), QStringLiteral("restart"));
    addElement(document, root, QStringLiteral("on_crash"), QStringLiteral("destroy"));

    // pm element
    QDomElement pm = document.createElement(QStringLiteral("pm"));
    root.appendChild(pm);
    QMap<QString, QString> suspend;
    suspend[QStringLiteral("enabled")] = QStringLiteral("no");
    addElementWithAttributes(document, pm, QStringLiteral("suspend-to-mem"), QStringLiteral(""), suspend);
    addElementWithAttributes(document, pm, QStringLiteral("suspend-to-disk"), QStringLiteral(""), suspend);

    // write to file
    QString xmlString = document.toString(4);
    QTextStream xmlContent(&xmlDomain);
    xmlContent << xmlString;
    qDebug().noquote() << "Generated XML:";
    qDebug().noquote() << xmlString;
    xmlDomain.close();
}
