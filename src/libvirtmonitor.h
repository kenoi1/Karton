// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <libvirt/libvirt.h>

#include <QObject>
#include <QString>

class LibvirtMonitor : public QObject
{
    Q_OBJECT

private:
    virConnectPtr m_conn;
    int m_callbackId;

public:
    LibvirtMonitor(QObject *parent = nullptr, virConnectPtr conn = nullptr);
    ~LibvirtMonitor();
    Q_DISABLE_COPY_MOVE(LibvirtMonitor)

Q_SIGNALS:
    void domainStateChanged(const virDomainPtr domainPtr, int event, int detail);

private:
    static int domainEventCallback(virConnectPtr conn, virDomainPtr dom, int event, int detail, void *opaque);
};