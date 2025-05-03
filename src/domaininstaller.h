// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <domainconfig.h>
#include <libvirt/libvirt.h>

#include <QDomDocument>
#include <QDomElement>
#include <QObject>

class DomainInstaller : public QObject {
    Q_OBJECT

   public:
    DomainInstaller();
    ~DomainInstaller();

    virDomainPtr setupDomain(virConnectPtr conn, const DomainConfig *config);
    QString generateXML(virConnectPtr conn, const DomainConfig *config);
    void addDiskDevices(QDomDocument &doc,  // extract to disk obj
                        QDomElement &parent,
                        const QString &diskType,
                        const QString &device,
                        const QString &name,
                        const QString &driverType,
                        const QString &file,
                        const QString &dev,
                        const QString &bus,
                        const bool readOnly);
    void addNetworkInterfaceDevices(QDomDocument &doc,
                                    QDomElement &parent,
                                    const QString &interfaceType,
                                    const QString &macAddress,
                                    const QString &sourceInterfaceType,
                                    const bool hasAddress,
                                    const QString &modelType);
    void addGraphicsDevices(QDomDocument &doc,
                            QDomElement &parent,
                            const QString &graphicsType,
                            const QString &autoport,
                            const QString &listenType);
    void addVideoDevices(QDomDocument &doc,
                         QDomElement &parent,
                         const QString &modelType,
                         const QString &heads,
                         const QString &primary);
    void addInputDevices(QDomDocument &doc,
                         QDomElement &parent,
                         const QString &type,
                         const QString &bus);
    void addConsoleDevices(QDomDocument &doc,
                           QDomElement &parent,
                           const QString &type);
    QString genMac();
    void addElement(QDomDocument &doc, QDomElement &parent, const QString &name, const QString &value);
    void addElementWithAttributes(QDomDocument &doc,
                                  QDomElement &parent,
                                  const QString &name,
                                  const QString &value,
                                  const QMap<QString, QString> &attributes);
};