// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include "domainconfig.h"
#include <libvirt/libvirt.h>

#include <QDomDocument>
#include <QDomElement>
#include <QObject>

class DomainInstaller : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    virDomainPtr setupDomain(virConnectPtr conn, const DomainConfig *config);
    QString generateXML(virConnectPtr conn, const DomainConfig *config);

private:
    struct DiskDeviceConfig {
        QString diskType;
        QString device;
        QString driver;
        QString format;
        QString sourcePath;
        QString target;
        QString bus;
        bool readonly;
    };

    struct NetworkInterfaceConfig {
        QString type;
        QString mac;
        QString source;
        bool linkState;
        QString model;
    };

    struct GraphicsConfig {
        QString type;
        QString autoport;
        QString listen;
    };

    struct SoundConfig {
        QString model;
        QString id;
    };

    struct AudioConfig {
        QString id;
        QString type;
    };

    struct VideoConfig {
        QString model;
        QString heads;
        QString primary;
    };

    struct InputConfig {
        QString type;
        QString bus;
    };

    struct ConsoleConfig {
        QString type;
    };

    void addDiskDevices(QDomDocument &document, QDomElement &parent, const DiskDeviceConfig &config);
    void addNetworkInterfaceDevices(QDomDocument &document, QDomElement &parent, const NetworkInterfaceConfig &config);
    void addGraphicsDevices(QDomDocument &document, QDomElement &parent, const GraphicsConfig &config);
    void addSoundDevices(QDomDocument &document, QDomElement &parent, const SoundConfig &config);
    void addAudioDevices(QDomDocument &document, QDomElement &parent, const AudioConfig &config);
    void addVideoDevices(QDomDocument &document, QDomElement &parent, const VideoConfig &config);
    void addInputDevices(QDomDocument &document, QDomElement &parent, const InputConfig &config);
    void addConsoleDevices(QDomDocument &document, QDomElement &parent, const ConsoleConfig &config);

    QString genMac();
    void addElement(QDomDocument &doc, QDomElement &parent, const QString &name, const QString &value);
    void addElementWithAttributes(QDomDocument &doc, QDomElement &parent, const QString &name, const QString &value, const QMap<QString, QString> &attributes);
};