// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QObject>
#include <QString>
#include <QDebug>
#include <libvirt/libvirt.h>
#include "libvirteventloop.h"

class LibvirtEventLoop;

class LibvirtMonitor : public QObject {
    Q_OBJECT
    
private:
    virConnectPtr m_conn;
    int m_callbackId;
    
public:
    LibvirtMonitor(QObject *parent = nullptr, virConnectPtr conn = nullptr)
        : QObject(parent)
        , m_conn(conn)
        , m_callbackId(-1) {
        
        if (!m_conn) {
            qDebug() << "No libvirt connection provided to monitor";
            return;
        }
        
        // Register the event implementation
        LibvirtEventLoop::registerQtEventLoop();
        
   
        m_callbackId = virConnectDomainEventRegisterAny(
            m_conn,
            nullptr, 
            VIR_DOMAIN_EVENT_ID_LIFECYCLE,
            VIR_DOMAIN_EVENT_CALLBACK(domainEventCallback),
            this, 
            nullptr  
        );
        
        if (m_callbackId < 0) {
            qDebug() << "Failed to register event callback";
        } else {
            qDebug() << "Successfully registered domain event callback";
        }
    }
    
    ~LibvirtMonitor() {
        if (m_callbackId >= 0 && m_conn) {
            virConnectDomainEventDeregisterAny(m_conn, m_callbackId);
        }
    }
    
Q_SIGNALS:
    void domainStateChanged(const QString &domainName, int event, int detail);
    
private:
    static int domainEventCallback(virConnectPtr conn, virDomainPtr dom,
                                  int event, int detail, void *opaque) {
        LibvirtMonitor *monitor = static_cast<LibvirtMonitor*>(opaque);
        const char *name = virDomainGetName(dom);
        
        Q_EMIT monitor->domainStateChanged(QString::fromUtf8(name), event, detail);
        
        return 0;
    }
};