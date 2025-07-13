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
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QString>
#include <QUuid>
#include <net/ethernet.h>

#include "domainconfig.h"
#include "karton_debug.h"
#include "osinfoconfig.h"

virDomainPtr DomainInstaller::setupDomain(virConnectPtr conn, const DomainConfig *config)
{
    QString xmlString = generateXML(conn, config);
    virDomainPtr dom = virDomainDefineXML(conn, xmlString.toStdString().c_str());
    // this function generates a new XML file: ~/.config/libvirt/qemu/[name].xml

    return dom;
}

QString DomainInstaller::generateXML(virConnectPtr conn, const DomainConfig *config)
{
    OsinfoConfig osinfo;
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
                   {// virtual disk
                    .diskType = QStringLiteral("file"),
                    .device = QStringLiteral("disk"),
                    .driver = QStringLiteral("qemu"),
                    .format = QStringLiteral("qcow2"),
                    .sourcePath = config->virtualDiskPath(),
                    .target = QStringLiteral("vda"),
                    .bus = QStringLiteral("virtio"),
                    .readonly = false});

    addDiskDevices(document,
                   devices,
                   {// cdrom
                    .diskType = QStringLiteral("file"),
                    .device = QStringLiteral("cdrom"),
                    .driver = QStringLiteral("qemu"),
                    .format = QStringLiteral("raw"),
                    .sourcePath = config->isoDiskPath(),
                    .target = QStringLiteral("sda"),
                    .bus = QStringLiteral("sata"),
                    .readonly = true});

    // devices->network interfaces element
    // Userspace connection https://libvirt.org/formatdomain.html#id44
    addNetworkInterfaceDevices(document,
                               devices,
                               {.type = QStringLiteral("user"), .mac = genMac(), .source = QString(), .linkState = true, .model = QStringLiteral("virtio")});

    // devices->graphics element
    addGraphicsDevices(document, devices, {.type = QStringLiteral("spice"), .autoport = QStringLiteral("yes"), .listen = QStringLiteral("address")});

    // devices->sound element
    addSoundDevices(document, devices, {.model = QStringLiteral("ich9"), .id = QStringLiteral("1")});

    // devices->audio element
    addAudioDevices(document, devices, {.id = QStringLiteral("1"), .type = QStringLiteral("spice")});

    // devices->video element
    addVideoDevices(document, devices, {.model = QStringLiteral("virtio"), .heads = QStringLiteral("1"), .primary = QStringLiteral("yes")});

    addInputDevices(document, devices, {.type = QStringLiteral("tablet"), .bus = QStringLiteral("usb")});

    addInputDevices(document, devices, {.type = QStringLiteral("keyboard"), .bus = QStringLiteral("usb")});

    addConsoleDevices(document, devices, {.type = QStringLiteral("pty")});

    // write to file
    QString xmlString = document.toString(4);

    qCInfo(KARTON_DEBUG).noquote() << "Generated XML:";
    qCInfo(KARTON_DEBUG).noquote() << xmlString;
    return xmlString;
}

void DomainInstaller::addDiskDevices(QDomDocument &document, QDomElement &parent, const DiskDeviceConfig &config)
{
    QDomElement disk = document.createElement(QStringLiteral("disk"));
    parent.appendChild(disk);
    disk.setAttribute(QStringLiteral("type"), config.diskType);
    disk.setAttribute(QStringLiteral("device"), config.device);

    QMap<QString, QString> driver;
    driver[QStringLiteral("name")] = config.driver;
    driver[QStringLiteral("type")] = config.format;
    addElementWithAttributes(document, disk, QStringLiteral("driver"), QString(), driver);

    QMap<QString, QString> source;
    source[QStringLiteral("file")] = config.sourcePath;
    addElementWithAttributes(document, disk, QStringLiteral("source"), QString(), source);

    QMap<QString, QString> target;
    target[QStringLiteral("dev")] = config.target;
    target[QStringLiteral("bus")] = config.bus;
    addElementWithAttributes(document, disk, QStringLiteral("target"), QString(), target);

    if (config.readonly) {
        addElement(document, disk, QStringLiteral("readonly"), QString());
    }
}

void DomainInstaller::addNetworkInterfaceDevices(QDomDocument &document, QDomElement &parent, const NetworkInterfaceConfig &config)
{
    QDomElement interface = document.createElement(QStringLiteral("interface"));
    parent.appendChild(interface);
    interface.setAttribute(QStringLiteral("type"), config.type);

    if (!config.mac.isEmpty()) {
        QMap<QString, QString> mac;
        mac[QStringLiteral("address")] = config.mac;
        addElementWithAttributes(document, interface, QStringLiteral("mac"), QString(), mac);
    }

    if (!config.source.isEmpty()) {
        QMap<QString, QString> source;
        source[config.type] = config.source;
        addElementWithAttributes(document, interface, QStringLiteral("source"), QString(), source);
    }

    QMap<QString, QString> model;
    model[QStringLiteral("type")] = config.model;
    addElementWithAttributes(document, interface, QStringLiteral("model"), QString(), model);

    if (config.linkState) {
        QMap<QString, QString> address;
        address[QStringLiteral("type")] = QStringLiteral("pci");
        address[QStringLiteral("domain")] = QStringLiteral("0x0000");
        address[QStringLiteral("bus")] = QStringLiteral("0x01");
        address[QStringLiteral("slot")] = QStringLiteral("0x00");
        addElementWithAttributes(document, interface, QStringLiteral("address"), QString(), address);
    }
}

void DomainInstaller::addGraphicsDevices(QDomDocument &document, QDomElement &parent, const GraphicsConfig &config)
{
    QDomElement graphics = document.createElement(QStringLiteral("graphics"));
    parent.appendChild(graphics);
    graphics.setAttribute(QStringLiteral("type"), config.type);
    graphics.setAttribute(QStringLiteral("autoport"), config.autoport);

    QMap<QString, QString> listen;
    listen[QStringLiteral("type")] = config.listen;
    addElementWithAttributes(document, graphics, QStringLiteral("listen"), QString(), listen);
}

void DomainInstaller::addSoundDevices(QDomDocument &document, QDomElement &parent, const SoundConfig &config)
{
    QDomElement sound = document.createElement(QStringLiteral("sound"));
    parent.appendChild(sound);
    sound.setAttribute(QStringLiteral("model"), config.model);

    QDomElement audio = document.createElement(QStringLiteral("audio"));
    sound.appendChild(audio);
    audio.setAttribute(QStringLiteral("id"), config.id);
}

void DomainInstaller::addAudioDevices(QDomDocument &document, QDomElement &parent, const AudioConfig &config)
{
    QDomElement audio = document.createElement(QStringLiteral("audio"));
    parent.appendChild(audio);
    audio.setAttribute(QStringLiteral("id"), config.id);
    audio.setAttribute(QStringLiteral("type"), config.type);
}

void DomainInstaller::addVideoDevices(QDomDocument &document, QDomElement &parent, const VideoConfig &config)
{
    QDomElement video = document.createElement(QStringLiteral("video"));
    parent.appendChild(video);
    QMap<QString, QString> model;
    model[QStringLiteral("type")] = config.model;
    model[QStringLiteral("heads")] = config.heads;
    model[QStringLiteral("primary")] = config.primary;
    addElementWithAttributes(document, video, QStringLiteral("model"), QString(), model);
}

void DomainInstaller::addInputDevices(QDomDocument &document, QDomElement &parent, const InputConfig &config)
{
    QDomElement input = document.createElement(QStringLiteral("input"));
    parent.appendChild(input);
    input.setAttribute(QStringLiteral("type"), config.type);
    input.setAttribute(QStringLiteral("bus"), config.bus);
}

void DomainInstaller::addConsoleDevices(QDomDocument &document, QDomElement &parent, const ConsoleConfig &config)
{
    QDomElement console = document.createElement(QStringLiteral("console"));
    parent.appendChild(console);
    console.setAttribute(QStringLiteral("type"), config.type);
}

// Temporarily: generate a random mac address (in unicast)...
// eventually generate a network domain.
QString DomainInstaller::genMac()
{
    QByteArray data(ETH_ALEN, Qt::Uninitialized);
    QRandomGenerator::global()->generate(data.begin(), data.end());
    constexpr auto CLEAR_MULTICAST_BIT = static_cast<char>(0xFE);
    data[0] &= CLEAR_MULTICAST_BIT;
    constexpr auto SET_LOCAL_ASSIGNMENT_BIT = static_cast<char>(0x02);
    data[0] |= SET_LOCAL_ASSIGNMENT_BIT;
    return QString::fromLatin1(data.toHex(':')).toLower();
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
