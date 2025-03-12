// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "karton.h"
#include "domain.h"
#include "libvirtmonitor.h"

#include <KLocalizedString>
#include <QDebug>
#include <QObject>
#include <iostream>
#include <libvirt/libvirt.h>

Karton::Karton(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_conn(nullptr)
    , m_monitor(nullptr)
{
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode, QProcess::ExitStatus) {
        QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
        Q_EMIT commandFinished(exitCode, output);
    });

    init();
}

Karton::~Karton()
{
    if (m_conn) {
        virConnectClose(m_conn);
        m_conn = nullptr;
    }

    if (m_process->state() == QProcess::Running) {
        m_process->terminate();
        m_process->waitForFinished(1000);
    }
}

void Karton::onDomainStateChanged(const QString &domainName, int event, int detail)
{
    qDebug() << "Domain state changed:" << domainName << "Event:" << event << "Detail:" << detail;

    Q_EMIT domainsChanged(domainName, event, detail);
}

bool Karton::init()
{
    // Currently set to session, but could also do system for root..
    m_conn = virConnectOpen("qemu:///session");
    if (!m_conn) {
        qDebug() << "Failed to connect to hypervisor";
        return false;
    }

    qDebug() << "Connected to hypervisor";

    m_monitor = new LibvirtMonitor(this, m_conn);
    connect(m_monitor, &LibvirtMonitor::domainStateChanged, this, &Karton::onDomainStateChanged);

    refreshDomainList();

    // Print VMs when started
    // qDebug() << "Total VMs: " << m_domains.size();
    // for (const auto& domain : m_domains) {
    //     qDebug() << "VM:" << domain.name()
    //          << "\n    UUID:" << domain.uuid()
    //          << "\n    Active:" << (domain.isActive() ? "Yes" : "No")
    //          << "\n    State:" << domain.state()
    //          << "\n    RAM:" << domain.maxRam() << "MB"
    //          << "\n    RAM Usage:" << domain.ramUsage() << "MB"
    //          << "\n    CPUs:" << domain.cpus()
    //          << "\n    Disk Path:" << domain.diskPath()
    //          << "\n    Autostart:" << (domain.autostart() ? "Yes" : "No");
    // }
    return true;
}

void Karton::refreshDomainList()
{
    m_domains.clear();

    virDomainPtr *domains = nullptr;
    int numDomains = virConnectListAllDomains(m_conn, &domains, 0);
    m_domains.reserve(numDomains);

    for (int i = 0; i < numDomains; i++) {
        // getting all information from libvirt
        virDomainPtr domainPtr = domains[i];
        const char *name = virDomainGetName(domains[i]);
        char uuid[VIR_UUID_STRING_BUFLEN];
        virDomainGetUUIDString(domains[i], uuid);
        bool isActive = virDomainIsActive(domains[i]);

        virDomainInfo domInfo;
        virDomainGetInfo(domains[i], &domInfo);
        QString state;
        switch (domInfo.state) {
        case VIR_DOMAIN_NOSTATE:
            state = i18n("no state");
            break;
        case VIR_DOMAIN_RUNNING:
            state = i18n("running");
            break;
        case VIR_DOMAIN_BLOCKED:
            state = i18n("blocked");
            break;
        case VIR_DOMAIN_PAUSED:
            state = i18n("paused");
            break;
        case VIR_DOMAIN_SHUTDOWN:
            state = i18n("shutting down");
            break;
        case VIR_DOMAIN_SHUTOFF:
            state = i18n("shutoff");
            break;
        case VIR_DOMAIN_CRASHED:
            state = i18n("crashed");
            break;
        case VIR_DOMAIN_PMSUSPENDED:
            state = i18n("suspended");
            break;
        default:
            state = i18n("unknown");
            break;
        }

        int maxRam = domInfo.maxMem / 1024; // convert to MB
        int ramUsage = domInfo.memory / 1024;
        int cpus = domInfo.nrVirtCpu;
        QString diskPath = QStringLiteral(" "); // TODO

        int autoFlag = 0;
        virDomainGetAutostart(domains[i], &autoFlag);
        bool autostart = (autoFlag != 0);

        Domain *domain = new Domain(domainPtr,
                                    QString::fromUtf8(name),
                                    QString::fromUtf8(uuid),
                                    isActive,
                                    state,
                                    maxRam,
                                    ramUsage,
                                    cpus,
                                    diskPath, // TODO: implement retrieving path
                                    autostart,
                                    this);
        m_domains.emplace_back(domain);
        // virDomainFree(domains[i]);
    }
    free(domains);
}

QVector<Domain *> Karton::domains()
{
    refreshDomainList();
    return m_domains;
}

bool Karton::startDomain(const Domain *domain)
{
    virDomainPtr domainPtr = domain->domainPtr();
    int result = virDomainCreate(domainPtr);

    if (result < 0) {
        qDebug() << "Failed to start domain:" << domain->name();
        return false;
    }
    qDebug() << "Successfully started domain:" << domain->name();
    return true;
}

bool Karton::stopDomain(const Domain *domain)
{
    virDomainPtr domainPtr = domain->domainPtr();
    virDomainInfo info;
    if (info.state == VIR_DOMAIN_RUNNING || info.state == VIR_DOMAIN_PAUSED) {
        int result = virDomainShutdown(domainPtr);
        if (result < 0) {
            qDebug() << "Failed to stop domain:" << domain->name();
            return false;
        }
    }

    qDebug() << "Successfully stopped domain:"<< domain->name();
    virDomainFree(domainPtr);
    return true;
}
bool Karton::forceStopDomain(const Domain *domain)
{
    virDomainPtr domainPtr = domain->domainPtr();
    int result = virDomainDestroy(domainPtr);

    if (result < 0) {
        qDebug() << "Failed to force-stop domain:" << domain->name();
        return false;
    }
    qDebug() << "Successfully force-stopped domain:" << domain->name();
    return true;
}
bool Karton::viewDomain(const Domain *domain)
{
    return runCommand(QStringLiteral("virt-viewer") + domain->name());
}

// Use for virsh, virt-viewer, virt-install and other CLI
bool Karton::runCommand(const QString &command)
{
    qDebug() << "Running Command:" << command;
    m_process->startCommand(command);
    return m_process->waitForStarted();
}
