// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "libvirtmonitor.h"

#include "karton_debug.h"
#include "libvirteventloop.h"

LibvirtMonitor::LibvirtMonitor(QObject *parent, virConnectPtr conn)
    : QObject(parent)
    , m_conn(conn)
    , m_callbackId(-1)
{
    if (!m_conn) {
        qCCritical(KARTON_DEBUG) << "No libvirt connection provided to monitor";
        return;
    }

    auto virtEventLoop = new LibvirtEventLoop{this}; // TODO store

    // LibvirtEventLoop::registerQtEventLoop();
    connect(virtEventLoop, &LibvirtEventLoop::result, this, [this](bool result) {
        if (!result) {
            qCCritical(KARTON_DEBUG) << "LibvirtEventLoop register failed";
            return;
        }
        m_callbackId =
            virConnectDomainEventRegisterAny(m_conn, nullptr, VIR_DOMAIN_EVENT_ID_LIFECYCLE, VIR_DOMAIN_EVENT_CALLBACK(domainEventCallback), this, nullptr);

        // For reference...
        // EVENT_ID_LIFECYCLE is listening to all events under:
        // https://libvirt.org/html/libvirt-libvirt-domain.html#virDomainEventType
        // Also a list of other callbacks:
        // https://libvirt.org/html/libvirt-libvirt-domain.html#virDomainEventID

        if (m_callbackId < 0) {
            qCCritical(KARTON_DEBUG) << "Failed to register event callback";
        } else {
            qCInfo(KARTON_DEBUG) << "Successfully registered domain event callback";
        }
    });
    virtEventLoop->run();
}

LibvirtMonitor::~LibvirtMonitor()
{
    if (m_callbackId >= 0 && m_conn) {
        virConnectDomainEventDeregisterAny(m_conn, m_callbackId);
    }
}

int LibvirtMonitor::domainEventCallback(virConnectPtr conn, virDomainPtr dom, int event, int detail, void *opaque)
{
    qCInfo(KARTON_DEBUG) << "event callback!";
    auto monitor = static_cast<LibvirtMonitor *>(opaque);
    // const char *name = virDomainGetName(dom);

    Q_EMIT monitor->domainStateChanged(dom, event, detail);

    return 0;
}
