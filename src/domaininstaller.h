// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <domainconfig.h>
#include <libvirt/libvirt.h>

class DomainInstaller : public QObject
{
    Q_OBJECT

public:
    DomainInstaller();
    ~DomainInstaller();

    void configureXML(virConnectPtr conn, const DomainConfig *config);
    void addElement(QDomDocument &doc, QDomElement &parent, const QString &name, const QString &value);
    void addElementWithAttributes(QDomDocument &doc,
                                              QDomElement &parent,
                                              const QString &name,
                                              const QString &value,
                                              const QMap<QString, QString> &attributes);
};