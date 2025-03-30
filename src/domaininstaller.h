// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QObject>
#include <libvirt/libvirt.h>

class DomainInstaller : public QObject
{
    Q_OBJECT

public:
    DomainInstaller();
    ~DomainInstaller();

    void configureXML(virConnectPtr conn, const QString &name, const QString &osVariant, 
                     const float memoryGB, const float storageGB, 
                     const QString &diskPath, const int cpus);

};
