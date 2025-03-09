// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "libvirtmonitor.h"
LibvirtMonitor::LibvirtMonitor(QObject *parent, virConnectPtr conn)
    : QObject(parent)
    , m_conn(conn)
    , m_callbackId(-1)
{
    if (!m_conn) {
        qDebug() << "No libvirt connection provided to monitor";
        return;
    }

    LibvirtEventLoop::registerQtEventLoop();

    m_callbackId =
        virConnectDomainEventRegisterAny(m_conn, nullptr, VIR_DOMAIN_EVENT_ID_LIFECYCLE, VIR_DOMAIN_EVENT_CALLBACK(domainEventCallback), this, nullptr);

    if (m_callbackId < 0) {
        qDebug() << "Failed to register event callback";
    } else {
        qDebug() << "Successfully registered domain event callback";
    }
}
LibvirtMonitor::~LibvirtMonitor()
{
    if (m_callbackId >= 0 && m_conn) {
        virConnectDomainEventDeregisterAny(m_conn, m_callbackId);
    }
}
int LibvirtMonitor::domainEventCallback(virConnectPtr conn, virDomainPtr dom, int event, int detail, void *opaque)
{
    LibvirtMonitor *monitor = static_cast<LibvirtMonitor *>(opaque);
    const char *name = virDomainGetName(dom);

    Q_EMIT monitor->domainStateChanged(QString::fromUtf8(name), event, detail);

    return 0;
}