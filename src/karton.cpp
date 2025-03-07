// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "karton.h"
#include "domain.h"
#include <QDebug>
#include <libvirt/libvirt.h>
#include <iostream>
#include <QObject>

Karton::Karton(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this)) {

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus) {
            QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
            Q_EMIT commandFinished(exitCode, output);
        });

    init();
}

Karton::~Karton() {
    virConnectClose(m_conn);
    if (m_process->state() == QProcess::Running) {
        m_process->terminate();
        m_process->waitForFinished(1000);
    }
}

bool Karton::init() {
    // Currently set to session, but could also do system for root..
    m_conn = virConnectOpen("qemu:///session");
    if (!m_conn) {
        qDebug() << "Failed to connect to hypervisor";
        return false;
    }

    qDebug() << "Connected to hypervisor";
    refreshDomainList();
    // Print VMs when started
    qDebug() << "Total VMs: " << m_domains.size();
    for (const auto& domain : m_domains) {
        qDebug() << "  VM: " << domain.name()
                    << " (" << domain.statusString() << ")"
                    << " (UUID: " << domain.uuid() << ")";     
    }
    return true;
}

void Karton::refreshDomainList() {
    m_domains.clear();

    virDomainPtr *domains = nullptr;
    int numDomains = virConnectListAllDomains(m_conn, &domains, 0);

    for (int i = 0; i < numDomains; i++) {
        const char* name = virDomainGetName(domains[i]);
        char uuid[VIR_UUID_STRING_BUFLEN];
        virDomainGetUUIDString(domains[i], uuid);
        bool isActive = virDomainIsActive(domains[i]);
        
        // TODO USE POINTER
        m_domains.append(Domain(QString::fromUtf8(name), 
                                QString::fromUtf8(uuid), isActive));

        virDomainFree(domains[i]);
    }
    free(domains);
}

QVector<Domain> Karton::domains() {
    refreshDomainList();
    return m_domains;
}

// For virsh and other CLI
bool Karton::runVM(const QString &command)
{
    qDebug() << "Running VM:" << command;
    m_process->start(command);
    return m_process->waitForStarted();
}