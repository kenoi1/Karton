// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domaininstaller.h"

#include <glib.h>

#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QFile>
#include <QMap>
#include <QStandardPaths>
#include <QString>
#include <QUuid>

#include "domainconfig.h"
#include "karton_debug.h"
#include "osinfoconfig.h"

virDomainPtr DomainInstaller::setupDomain(virConnectPtr conn, const DomainConfig *config)
{
    QString xmlString = generateXML(conn, config);
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QString path = QStringLiteral("%1/libvirt/kde-karton").arg(dataDir);
    QDir dir(path);
    if (!dir.mkpath(QStringLiteral("config"))) {
        qCCritical(KARTON_DEBUG) << "Already Exists / Failed: " << path;
    }
    QFile domainXML(QStringLiteral("%1/config/%2_config.xml").arg(path).arg(config->name()));
    if (!domainXML.open(QFile::WriteOnly | QFile::Text)) {
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

QString DomainInstaller::generateXML(virConnectPtr conn, const DomainConfig *config)
{
    OsinfoConfig osinfo;
    // const QString osId = osinfo.getOsIdFromDisk(config->isoDiskPath());f
    const QString osId = osinfo.getOsIdFromShortId(config->shortOsId());
    const QString osArchitecture = osinfo.getOsArchitecture(osId);
    if (osArchitecture.isEmpty()) {
        qCCritical(KARTON_DEBUG) << "Warning no specified architecture!";
        return QString();
    }

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

    // metadata->Karton
    QDomElement karton = document.createElement(QStringLiteral("karton:kde-karton"));
    karton.setAttribute(QStringLiteral("xmlns:karton"), QStringLiteral("https://invent.kde.org/sitter/karton"));
    metadata.appendChild(karton);
    QMap<QString, QString> kartonData;
    kartonData[QStringLiteral("maxDiskStorage")] = QString::number(config->maxDiskStorage() * 1024);
    addElementWithAttributes(document, karton, QStringLiteral("karton:data"), QString(), kartonData);

    // metadata->LIBOSINFO
    QDomElement libosinfo = document.createElement(QStringLiteral("libosinfo:libosinfo"));
    libosinfo.setAttribute(QStringLiteral("xmlns:libosinfo"), QStringLiteral("http://libosinfo.org/xmlns/libvirt/domain/1.0"));
    metadata.appendChild(libosinfo);
    QMap<QString, QString> libosinfoId;
    libosinfoId[QStringLiteral("id")] = osId;
    libosinfoId[QStringLiteral("short-id")] = config->shortOsId();
    addElementWithAttributes(document, libosinfo, QStringLiteral("libosinfo:os"), QString(), libosinfoId);
    // qCInfo(KARTON_DEBUG) << "OS ID:" << id;

    // memory element
    QMap<QString, QString> mem;
    mem[QStringLiteral("unit")] = QStringLiteral("MiB");
    addElementWithAttributes(document, root, QStringLiteral("memory"), QString::number(config->maxRam() * 1024), mem); // GB->MB
    addElementWithAttributes(document, root, QStringLiteral("currentMemory"), QString::number(config->maxRam() * 1024), mem);

    // vpu element
    QMap<QString, QString> vcpu;
    vcpu[QStringLiteral("placement")] = QStringLiteral("static"); // static || auto
    addElementWithAttributes(document, root, QStringLiteral("vcpu"), QString::number(config->cpus()), vcpu);

    // os element
    QDomElement os = document.createElement(QStringLiteral("os"));
    root.appendChild(os);
    QMap<QString, QString> type;
    type[QStringLiteral("arch")] = osArchitecture;
    type[QStringLiteral("machine")] = QStringLiteral("q35"); // parameterize using libos?
    // QEMU machine types see: https://people.redhat.com/~cohuck/2022/01/05/qemu-machine-types.html
    addElementWithAttributes(document, os, QStringLiteral("type"), QStringLiteral("hvm"), type);
    QMap<QString, QString> boot1;
    boot1[QStringLiteral("dev")] = QStringLiteral("cdrom");
    QMap<QString, QString> boot2;
    boot2[QStringLiteral("dev")] = QStringLiteral("hd");
    addElementWithAttributes(document, os, QStringLiteral("boot"), QString(), boot1);
    addElementWithAttributes(document, os, QStringLiteral("boot"), QString(), boot2);

    // features element
    QDomElement features = document.createElement(QStringLiteral("features"));
    root.appendChild(features);
    QDomElement acpi = document.createElement(QStringLiteral("acpi"));
    features.appendChild(acpi);
    QDomElement apic = document.createElement(QStringLiteral("apic"));
    features.appendChild(apic);

    // QMap<QString, QString> vmport; // probably not necessary (was in virt-install)
    // vmport[QStringLiteral("state")] = QStringLiteral("off");
    // addElementWithAttributes(document, features, QStringLiteral("vmport"), QString(), vmport);

    // cpu element
    QMap<QString, QString> cpu;
    cpu[QStringLiteral("mode")] = QStringLiteral("host-passthrough");
    // cpu[QStringLiteral("check")] = QStringLiteral("none");
    // cpu[QStringLiteral("migratable")] = QStringLiteral("on");
    addElementWithAttributes(document, root, QStringLiteral("cpu"), QString(), cpu);

    // clock element
    QDomElement clock = document.createElement(QStringLiteral("clock"));
    clock.setAttribute(QStringLiteral("offset"), QStringLiteral("utc"));
    root.appendChild(clock);
    // QMap<QString, QString> timer; // some nodes i saw made by virt-install, maybe use in future?
    // timer[(QStringLiteral("name"))] = QStringLiteral("rtc");
    // timer[(QStringLiteral("tickpolicy"))] = QStringLiteral("catchup");
    // addElementWithAttributes(document, clock, QStringLiteral("timer"), QString(), timer);
    // timer[(QStringLiteral("name"))] = QStringLiteral("pit");
    // timer[(QStringLiteral("tickpolicy"))] = QStringLiteral("delay");
    // addElementWithAttributes(document, clock, QStringLiteral("timer"), QString(), timer);
    // QMap<QString, QString> timer2;
    // timer2[(QStringLiteral("name"))] = QStringLiteral("hept");
    // timer2[(QStringLiteral("present"))] = QStringLiteral("no");
    // addElementWithAttributes(document, clock, QStringLiteral("timer"), QString(), timer2);

    // // off elements
    // addElement(document, root, QStringLiteral("on_poweroff"), QStringLiteral("destroy"));
    // addElement(document, root, QStringLiteral("on_reboot"), QStringLiteral("restart"));
    // addElement(document, root, QStringLiteral("on_crash"), QStringLiteral("destroy"));

    // // pm element
    // QDomElement pm = document.createElement(QStringLiteral("pm"));
    // root.appendChild(pm);
    // QMap<QString, QString> suspend;
    // suspend[QStringLiteral("enabled")] = QStringLiteral("no");
    // addElementWithAttributes(document, pm, QStringLiteral("suspend-to-mem"), QString(), suspend);
    // addElementWithAttributes(document, pm, QStringLiteral("suspend-to-disk"), QString(), suspend);

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
    addNetworkInterfaceDevices(document, devices, QStringLiteral("user"), genMac(), QString(), true, QStringLiteral("virtio"));

    // devices->graphics element
    addGraphicsDevices(document, devices, QStringLiteral("spice"), QStringLiteral("yes"), QStringLiteral("address"));

    // devices->video element
    addVideoDevices(document, devices, QStringLiteral("virtio"), QStringLiteral("1"), QStringLiteral("yes"));

    addInputDevices(document, devices, QStringLiteral("tablet"), QStringLiteral("usb"));
    addInputDevices(document, devices, QStringLiteral("keyboard"), QStringLiteral("usb"));
    addConsoleDevices(document, devices, QStringLiteral("pty"));

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
    addElementWithAttributes(doc, disk, QStringLiteral("driver"), QString(), driver);

    QMap<QString, QString> source;
    source[QStringLiteral("file")] = file;
    addElementWithAttributes(doc, disk, QStringLiteral("source"), QString(), source);

    QMap<QString, QString> target;
    target[QStringLiteral("dev")] = dev;
    target[QStringLiteral("bus")] = bus;
    addElementWithAttributes(doc, disk, QStringLiteral("target"), QString(), target);

    if (readOnly) {
        addElement(doc, disk, QStringLiteral("readonly"), QString());
    }
}

void DomainInstaller::addNetworkInterfaceDevices(QDomDocument &doc,
                                                 QDomElement &parent,
                                                 const QString &interfaceType,
                                                 const QString &macAddress,
                                                 const QString &sourceInterfaceType,
                                                 const bool hasAddress,
                                                 const QString &modelType)
{
    QDomElement interface = doc.createElement(QStringLiteral("interface"));
    parent.appendChild(interface);
    interface.setAttribute(QStringLiteral("type"), interfaceType);

    if (!macAddress.isEmpty()) {
        QMap<QString, QString> mac;
        mac[QStringLiteral("address")] = macAddress;
        addElementWithAttributes(doc, interface, QStringLiteral("mac"), QString(), mac);
    }

    if (!sourceInterfaceType.isEmpty()) {
        QMap<QString, QString> source;
        source[interfaceType] = sourceInterfaceType;
        addElementWithAttributes(doc, interface, QStringLiteral("source"), QString(), source);
    }

    QMap<QString, QString> model;
    model[QStringLiteral("type")] = modelType;
    addElementWithAttributes(doc, interface, QStringLiteral("model"), QString(), model);
    if (hasAddress) {
        QMap<QString, QString> address;
        address[QStringLiteral("type")] = QStringLiteral("pci");
        address[QStringLiteral("domain")] = QStringLiteral("0x0000");
        address[QStringLiteral("bus")] = QStringLiteral("0x01");
        address[QStringLiteral("slot")] = QStringLiteral("0x00");
        addElementWithAttributes(doc, interface, QStringLiteral("address"), QString(), address);
    }
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
    addElementWithAttributes(doc, graphics, QStringLiteral("listen"), QString(), listen);
}

void DomainInstaller::addVideoDevices(QDomDocument &doc, QDomElement &parent, const QString &modelType, const QString &heads, const QString &primary)
{
    QDomElement video = doc.createElement(QStringLiteral("video"));
    parent.appendChild(video);
    QMap<QString, QString> model;
    model[QStringLiteral("type")] = modelType;
    model[QStringLiteral("heads")] = heads;
    model[QStringLiteral("primary")] = primary;
    addElementWithAttributes(doc, video, QStringLiteral("model"), QString(), model);
}

void DomainInstaller::addInputDevices(QDomDocument &doc, QDomElement &parent, const QString &type, const QString &bus)
{
    QDomElement input = doc.createElement(QStringLiteral("input"));
    parent.appendChild(input);
    input.setAttribute(QStringLiteral("type"), type);
    input.setAttribute(QStringLiteral("bus"), bus);
}

void DomainInstaller::addConsoleDevices(QDomDocument &doc, QDomElement &parent, const QString &type)
{
    QDomElement console = doc.createElement(QStringLiteral("console"));
    parent.appendChild(console);
    console.setAttribute(QStringLiteral("type"), type);
}

// Temporarily: generate a random mac address (in unicast)...
// eventually generate a network domain.
QString DomainInstaller::genMac()
{
    int i, tp;
    srand(time(NULL) + getpid());
    QString s;

    tp = rand() % 256; // first significant bit as unicast
    tp &= 0xFE;
    s += QString::asprintf("%s%X:", tp < 16 ? "0" : "", tp);

    for (i = 1; i < 6; i++) {
        tp = rand() % 256;
        s += QString::asprintf("%s%X%s", tp < 16 ? "0" : "", tp, i < 5 ? ":" : "");
    }
    return s.toLower();
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

    for (const auto &[key, value] : attributes.asKeyValueRange()) {
        element.setAttribute(key, value);
    }

    parent.appendChild(element);
}
