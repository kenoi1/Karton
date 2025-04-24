// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domaininstaller.h"
#include "domainconfig.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QFile>
#include <QMap>
#include <QUuid>
#include <QString>
#include "karton_debug.h"

#include "osinfoconfig.h"
#include <glib.h>

// extern "C" // due to undefined references to libosinfo stuff
// {
// #include <osinfo/osinfo.h>
// }

DomainInstaller::DomainInstaller()
{
}

DomainInstaller::~DomainInstaller()
{
}

// OsinfoDb *DomainInstaller::initOsDb() {
//     OsinfoLoader *loader = osinfo_loader_new();
//     OsinfoDb *db = osinfo_loader_get_db(loader);
//     osinfo_loader_process_default_path(loader, NULL);
//     return db;
// }

// const gchar *DomainInstaller::getOsIdFromDisk(QString diskPath, OsinfoDb *db)
// {
//     std::string str = diskPath.toStdString();
//     const gchar *location = str.c_str();
//     // qCInfo(KARTON_DEBUG) << "IDGGG: " << str << ", WAA: " <<  location;

//     GError *error = NULL;
//     OsinfoMedia *osMedia = osinfo_media_create_from_location(location, NULL, &error);
//     if (!osinfo_db_identify_media(db, osMedia)) {
//         return "fail";
//     }
//     const gchar *idG = osinfo_entity_get_id(OSINFO_ENTITY(osMedia));
//     // const gchar *idG = osinfo_media_get_system_id(osMedia);

//     // qCInfo(KARTON_DEBUG) << "IDGGG: " << idG;
//     if (error) {
//         qCCritical(KARTON_DEBUG) << "Id Finder Error: " << error->message;
//     }
//     // QString id = QString::fromUtf8(idG);
//     g_object_unref(osMedia);
//     // g_object_unref(loader);
//     return idG;

// }
virDomainPtr DomainInstaller::setupDomain(virConnectPtr conn,
                                  const DomainConfig *config)
{
    QString xmlString = generateXML(conn, config);
    QFile domainXML(QStringLiteral("/home/dereklin/Downloads/%1_config.xml").arg(config->name()));
    if (!domainXML.open(QFile::WriteOnly | QFile::Text))
    {
        qCCritical(KARTON_DEBUG) << "qfile opened in another instance or something??";
        return NULL;
    }
    QTextStream xmlContent(&domainXML);
    xmlContent << xmlString;
    domainXML.close();
    
    virDomainPtr dom = virDomainDefineXML(conn, xmlString.toStdString().c_str());
    // create?
    return dom;
}
QString DomainInstaller::generateXML(virConnectPtr conn,
                                  const DomainConfig *config)
{
    OsinfoConfig osinfo;
    const QString os_id = osinfo.getOsIdFromDisk(config->isoDiskPath());
    const QString os_arch = osinfo.getOsArchitecture(os_id);

    QDomDocument document;
    QDomElement root = document.createElement(QStringLiteral("domain"));
    root.setAttribute(QStringLiteral("type"), QStringLiteral("kvm")); // parameterize libos

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

    // // LIBOSINFO
    // QDomElement libosinfo = document.createElement(QStringLiteral("libosinfo:libosinfo"));
    // metadata.appendChild(libosinfo);
    // QMap<QString, QString> idMap;
    // idMap[QStringLiteral("id")] = os_id;
    // addElementWithAttributes(document, libosinfo, QStringLiteral("libosinfo:os"), QStringLiteral(""), idMap);
    // // qCInfo(KARTON_DEBUG) << "OS ID:" << id;

    // memory element
    QMap<QString, QString> mem;
    mem[QStringLiteral("unit")] = QStringLiteral("GiB"); // change to gb?
    addElementWithAttributes(document, root, QStringLiteral("memory"), QString::number(config->maxRam()), mem);
    addElementWithAttributes(document, root, QStringLiteral("currentMemory"), QString::number(config->maxRam()), mem);

    // vpu element
    QMap<QString, QString> vcpu;
    vcpu[QStringLiteral("placement")] = QStringLiteral("static"); // static || auto
    addElementWithAttributes(document, root, QStringLiteral("vcpu"), QString::number(config->cpus()), vcpu);

    // os element
    QDomElement os = document.createElement(QStringLiteral("os"));
    root.appendChild(os);
    QMap<QString, QString> type;
    type[QStringLiteral("arch")] = os_arch;                  // parameterize
    type[QStringLiteral("machine")] = QStringLiteral("q35"); // parameterize using libos?
    // QEMU machine types see: https://people.redhat.com/~cohuck/2022/01/05/qemu-machine-types.html
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

    // QMap<QString, QString> vmport; // probably not necessary (was in virt-install)
    // vmport[QStringLiteral("state")] = QStringLiteral("off");
    // addElementWithAttributes(document, features, QStringLiteral("vmport"), QStringLiteral(""), vmport);

    // cpu element
    QMap<QString, QString> cpu;
    cpu[QStringLiteral("mode")] = QStringLiteral("host-passthrough");
    // cpu[QStringLiteral("check")] = QStringLiteral("none");
    // cpu[QStringLiteral("migratable")] = QStringLiteral("on");
    addElementWithAttributes(document, root, QStringLiteral("cpu"), QStringLiteral(""), cpu);

    // clock element
    QDomElement clock = document.createElement(QStringLiteral("clock"));
    clock.setAttribute(QStringLiteral("offset"), QStringLiteral("utc"));
    root.appendChild(clock);
    // QMap<QString, QString> timer; // some nodes i saw made by virt-install, maybe use in future?
    // timer[(QStringLiteral("name"))] = QStringLiteral("rtc");
    // timer[(QStringLiteral("tickpolicy"))] = QStringLiteral("catchup");
    // addElementWithAttributes(document, clock, QStringLiteral("timer"), QStringLiteral(""), timer);
    // timer[(QStringLiteral("name"))] = QStringLiteral("pit");
    // timer[(QStringLiteral("tickpolicy"))] = QStringLiteral("delay");
    // addElementWithAttributes(document, clock, QStringLiteral("timer"), QStringLiteral(""), timer);
    // QMap<QString, QString> timer2;
    // timer2[(QStringLiteral("name"))] = QStringLiteral("hept");
    // timer2[(QStringLiteral("present"))] = QStringLiteral("no");
    // addElementWithAttributes(document, clock, QStringLiteral("timer"), QStringLiteral(""), timer2);

    // // off elements
    // addElement(document, root, QStringLiteral("on_poweroff"), QStringLiteral("destroy"));
    // addElement(document, root, QStringLiteral("on_reboot"), QStringLiteral("restart"));
    // addElement(document, root, QStringLiteral("on_crash"), QStringLiteral("destroy"));

    // // pm element
    // QDomElement pm = document.createElement(QStringLiteral("pm"));
    // root.appendChild(pm);
    // QMap<QString, QString> suspend;
    // suspend[QStringLiteral("enabled")] = QStringLiteral("no");
    // addElementWithAttributes(document, pm, QStringLiteral("suspend-to-mem"), QStringLiteral(""), suspend);
    // addElementWithAttributes(document, pm, QStringLiteral("suspend-to-disk"), QStringLiteral(""), suspend);

    // devices element
    QDomElement devices = document.createElement(QStringLiteral("devices"));
    root.appendChild(devices);

    // devices->disk element
    addDiskDevices(document,
                   devices,
                   QStringLiteral("file"),
                   QStringLiteral("disk"),
                   QStringLiteral("qemu"),
                   QStringLiteral("qcow2"),
                   config->virtualDiskPath(),
                   QStringLiteral("vda"),
                   QStringLiteral("virtio"),
                   false);
    addDiskDevices(document,
                   devices,
                   QStringLiteral("file"),
                   QStringLiteral("cdrom"),
                   QStringLiteral("qemu"),
                   QStringLiteral("raw"),
                   config->isoDiskPath(),
                   QStringLiteral("sda"),
                   QStringLiteral("sata"),
                   true);

    // devices->network interfaces element
    // Userspace connection https://libvirt.org/formatdomain.html#id44
    addNetworkInterfaceDevices(document,
                               devices,
                               QStringLiteral("network"),
                               QStringLiteral("default"),
                               QStringLiteral("virtio"));

    // devices->graphics element
    addGraphicsDevices(document,
                       devices,
                       QStringLiteral("spice"),
                       QStringLiteral("yes"),
                       QStringLiteral("address"));

    // devices->video element
    addVideoDevices(document,
                    devices,
                    QStringLiteral("virtio"),
                    QStringLiteral("1"),
                    QStringLiteral("yes"));

    addInputDevices(document,
                    devices,
                    QStringLiteral("tablet"),
                    QStringLiteral("usb"));
    addInputDevices(document,
                    devices,
                    QStringLiteral("keyboard"),
                    QStringLiteral("usb"));
    addConsoleDevices(document,
                      devices,
                      QStringLiteral("pty"));

    // write to file
    QString xmlString = document.toString(4);
    
    qCInfo(KARTON_DEBUG).noquote() << "Generated XML:";
    qCInfo(KARTON_DEBUG).noquote() << xmlString;
    return xmlString;
}

void DomainInstaller::addDiskDevices(QDomDocument &doc, // extract to disk obj
                                     QDomElement &parent,
                                     const QString &diskType,
                                     const QString &device,
                                     const QString &name,
                                     const QString &driverType,
                                     const QString &file,
                                     const QString &dev,
                                     const QString &bus,
                                     const bool readOnly)
{

    QDomElement disk = doc.createElement(QStringLiteral("disk"));
    parent.appendChild(disk);
    disk.setAttribute(QStringLiteral("type"), diskType);
    disk.setAttribute(QStringLiteral("device"), device);

    QMap<QString, QString> driver;
    driver[QStringLiteral("name")] = name;
    driver[QStringLiteral("type")] = driverType;
    addElementWithAttributes(doc, disk, QStringLiteral("driver"), QStringLiteral(""), driver);

    QMap<QString, QString> source;
    source[QStringLiteral("file")] = file;
    addElementWithAttributes(doc, disk, QStringLiteral("source"), QStringLiteral(""), source);

    QMap<QString, QString> target;
    target[QStringLiteral("dev")] = dev;
    target[QStringLiteral("bus")] = bus;
    addElementWithAttributes(doc, disk, QStringLiteral("target"), QStringLiteral(""), target);

    if (readOnly)
    {
        addElement(doc, disk, QStringLiteral("readonly"), QStringLiteral(""));
    }
}

void DomainInstaller::addNetworkInterfaceDevices(QDomDocument &doc,
                                                 QDomElement &parent,
                                                 const QString &interfaceType,
                                                 const QString &network,
                                                 const QString &modelType)
{
    QDomElement interface = doc.createElement(QStringLiteral("interface"));
    parent.appendChild(interface);
    interface.setAttribute(QStringLiteral("type"), interfaceType);

    QMap<QString, QString> source;
    source[QStringLiteral("network")] = network;
    addElementWithAttributes(doc, interface, QStringLiteral("source"), QStringLiteral(""), source);

    QMap<QString, QString> model;
    model[QStringLiteral("type")] = modelType;
    addElementWithAttributes(doc, interface, QStringLiteral("model"), QStringLiteral(""), model);
}

void DomainInstaller::addGraphicsDevices(QDomDocument &doc,
                                         QDomElement &parent,
                                         const QString &graphicsType,
                                         const QString &autoport,
                                         const QString &listenType)
{
    QDomElement graphics = doc.createElement(QStringLiteral("graphics"));
    parent.appendChild(graphics);
    graphics.setAttribute(QStringLiteral("type"), graphicsType);
    graphics.setAttribute(QStringLiteral("autoport"), autoport);

    QMap<QString, QString> listen;
    listen[QStringLiteral("type")] = listenType;
    addElementWithAttributes(doc, graphics, QStringLiteral("listen"), QStringLiteral(""), listen);
}

void DomainInstaller::addVideoDevices(QDomDocument &doc,
                                      QDomElement &parent,
                                      const QString &modelType,
                                      const QString &heads,
                                      const QString &primary)
{
    QDomElement video = doc.createElement(QStringLiteral("video"));
    parent.appendChild(video);
    QMap<QString, QString> model;
    model[QStringLiteral("type")] = modelType;
    model[QStringLiteral("heads")] = heads;
    model[QStringLiteral("primary")] = primary;
    addElementWithAttributes(doc, video, QStringLiteral("model"), QStringLiteral(""), model);
}

void DomainInstaller::addInputDevices(QDomDocument &doc,
                                      QDomElement &parent,
                                      const QString &type,
                                      const QString &bus)
{
    QDomElement input = doc.createElement(QStringLiteral("input"));
    parent.appendChild(input);
    input.setAttribute(QStringLiteral("type"), type);
    input.setAttribute(QStringLiteral("bus"), bus);
}

void DomainInstaller::addConsoleDevices(QDomDocument &doc,
                                        QDomElement &parent,
                                        const QString &type)
{
    QDomElement console = doc.createElement(QStringLiteral("console"));
    parent.appendChild(console);
    console.setAttribute(QStringLiteral("type"), type);
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

// void DomainInstaller::initLibosinfo()
// {
//     // qCInfo(KARTON_DEBUG) << "E?HFWIUEHFIUWEHFIUW";
//     OsinfoLoader *loader = osinfo_loader_new();
//     OsinfoDb *db = osinfo_loader_get_db(loader);
//     osinfo_loader_process_default_path(loader, NULL);

//     // const gchar *os_id = "";
//     // OsinfoOS *os = osinfo_db_get_os(db, )

//     // OsinfoDeviceList *devices = osinfo_db_get_device_list(db);
//     // gint len = osinfo_list_get_length(OSINFO_LIST(devices));
//     // for (gint i = 0; i < len; i++) {
//     //     qCInfo(KARTON_DEBUG) << "OS DEVICE:" << osinfo_get_id(device);
//     // }
//     // GList *device = osinfo_db_unique_values_for_property_in_device(db, "name");

//     // LISTS ALL IDS FOR OPERATING SYSTEMS IN DB
//     OsinfoOsList *osList = osinfo_db_get_os_list(db);
//     gint len = osinfo_list_get_length(OSINFO_LIST(osList));

//     for (gint i = 0; i < len; i++)
//     {
//         OsinfoOs *os = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(osList), i));
//         const gchar *id = osinfo_entity_get_id(OSINFO_ENTITY(os));
//         qCInfo(KARTON_DEBUG) << "OS ID:" << id;
//     }

//     g_object_unref(osList);
//     // g_object_unref(devices);
//     g_object_unref(loader);
// }