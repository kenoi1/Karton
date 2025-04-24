// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "karton.h"
#include "domain.h"
#include "domainconfig.h"
#include "libvirtmonitor.h"

#include "karton_debug.h"
#include <KLocalizedString>
#include <QObject>
#include <QFile>
#include <QStandardPaths>

#include <libvirt/libvirt.h>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QUuid>


Karton::Karton(QObject *parent)
    : QObject(parent)
    , m_conn(nullptr)
    , m_monitor(nullptr)
{
    init();
}

Karton::~Karton()
{
    if (m_conn) {
        virConnectClose(m_conn);
        m_conn = nullptr;
    }
}

void Karton::onDomainStateChanged(virDomainPtr domainPtr, int event, int detail)
{
    const char *domainName = virDomainGetName(domainPtr);
    qCInfo(KARTON_DEBUG) << "Domain state changed:" << domainName << "Event:" << event << "Detail:" << detail;
    // Reference for event/details: https://libvirt.org/html/libvirt-libvirt-domain.html#virDomainEventType

    Q_EMIT domainsChanged(domainPtr, event, detail);
}

bool Karton::init()
{
    // Register event loop. Note: must be done before connection to hypervisor.
    virEventRegisterDefaultImpl();

    // Currently set to session, but could also do system for root..
    m_conn = virConnectOpen("qemu:///session");
    if (!m_conn) {
        qCCritical(KARTON_DEBUG) << "Failed to connect to hypervisor";
        return false;
    }

    qCInfo(KARTON_DEBUG) << "Connected to hypervisor";

    m_monitor = new LibvirtMonitor(this, m_conn);
    connect(m_monitor, &LibvirtMonitor::domainStateChanged, this, &Karton::onDomainStateChanged);

    refreshDomainList();

    return true;
}

// searchDomain(domain) returns index position of the domain in m_domains
int Karton::searchDomain(const virDomainPtr domainPtr)
{
    QString searchUuid = Domain::uuidString(domainPtr);

    for (int i = 0; i < m_domains.size(); i++) {
        if (searchUuid == m_domains[i]->config()->uuid()) {
            return i;
        }
    }
    return -1;
}

// get string from state
static QString domainStateString(unsigned int state)
{
    switch (state) {
    case VIR_DOMAIN_NOSTATE:
        return i18n("no state");
    case VIR_DOMAIN_RUNNING:
        return i18n("running");
    case VIR_DOMAIN_BLOCKED:
        return i18n("blocked");
    case VIR_DOMAIN_PAUSED:
        return i18n("paused");
    case VIR_DOMAIN_SHUTDOWN:
        return i18n("shutting down");
    case VIR_DOMAIN_SHUTOFF:
        return i18n("shutoff");
    case VIR_DOMAIN_CRASHED:
        return i18n("crashed");
    case VIR_DOMAIN_PMSUSPENDED:
        return i18n("suspended");
    default:
        return i18n("unknown");
    }
}

// refresh a singular domain, used to update list
void Karton::refreshDomain(const virDomainPtr domainPtr)
{
    int index = searchDomain(domainPtr);
    if (index == -1) {
        qCWarning(KARTON_DEBUG) << "Domain not found in list.";
        return;
    }

    Domain *domain = m_domains[index];

    bool isActive = virDomainIsActive(domainPtr);

    virDomainInfo domInfo;
    virDomainGetInfo(domainPtr, &domInfo);
    QString state = domainStateString(domInfo.state);

    int ramUsage = domInfo.memory / (1024 * 1024);

    int autoFlag = 0;
    virDomainGetAutostart(domainPtr, &autoFlag);
    bool autostart = (autoFlag != 0);

    // updates only mutable fields
    domain->setActive(isActive);
    domain->setState(state);
    domain->setRamUsage(ramUsage);
    domain->setAutostart(autostart);
}

// Refresh the complete list of domains
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
        bool isActive = virDomainIsActive(domains[i]);

        virDomainInfo domInfo;
        virDomainGetInfo(domains[i], &domInfo);
        QString state = domainStateString(domInfo.state);

        int maxRam = domInfo.maxMem / (1024 * 1024); // convert to MB
        int ramUsage = domInfo.memory / (1024 * 1024);
        int cpus = domInfo.nrVirtCpu;
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        QString virtualDiskPath = QStringLiteral("%1/libvirt/images/%2.qcow2").arg(dataDir).arg(QString::fromUtf8(name));
        // WIP better to use .xml parsing instead of hardcode
        
        int autoFlag = 0;
        virDomainGetAutostart(domains[i], &autoFlag);
        bool autostart = (autoFlag != 0);
        

        // TODO READ EVERYTHING FROM XML?
        DomainConfig *config = new DomainConfig(QString::fromUtf8(name),
                                    Domain::uuidString(domainPtr),
                                    QString::fromUtf8("WIP"), // osvariant
                                    isActive,
                                    state,
                                    maxRam,
                                    ramUsage,
                                    cpus,
                                    0, // disk storage
                                    QStringLiteral("WIP ISO DISK PATH"),
                                    virtualDiskPath,
                                    autostart,
                                    this);
        Domain *domain = new Domain(domainPtr,
                                    config,
                                    this);
        m_domains.emplace_back(domain);
    }
    free(domains);
}

QVector<Domain *> Karton::domains()
{
    return m_domains;
}

bool Karton::startDomain(const Domain *domain)
{
    virDomainPtr domainPtr = domain->domainPtr();
    int result = virDomainCreate(domainPtr);

    if (result < 0) {
        QString errorMsg = QStringLiteral("Failed to start domain: %1").arg(domain->config()->name());
        qCWarning(KARTON_DEBUG) << errorMsg;
        Q_EMIT errorOccurred(errorMsg);
        return false;
    }
    qCInfo(KARTON_DEBUG) << "Successfully started domain:" << domain->config()->name();
    return true;
}

bool Karton::stopDomain(const Domain *domain)
{
    virDomainPtr domainPtr = domain->domainPtr();
    virDomainInfo info;
    virDomainGetInfo(domainPtr, &info);

    if (info.state == VIR_DOMAIN_RUNNING || info.state == VIR_DOMAIN_PAUSED) {
        int result = virDomainShutdown(domainPtr);
        if (result < 0) {
            QString errorMsg = QStringLiteral("Failed to stop domain: %1").arg(domain->config()->name());
            qCWarning(KARTON_DEBUG) << errorMsg;
            Q_EMIT errorOccurred(errorMsg);
            return false;
        }
    }

    qCInfo(KARTON_DEBUG) << "Successfully stopped domain:" << domain->config()->name();
    return true;
}

bool Karton::forceStopDomain(const Domain *domain)
{
    virDomainPtr domainPtr = domain->domainPtr();
    int result = virDomainDestroy(domainPtr);

    if (result < 0) {
        QString errorMsg = QStringLiteral("Failed to force-stop domain: %1").arg(domain->config()->name());
        qCWarning(KARTON_DEBUG) << errorMsg;
        Q_EMIT errorOccurred(errorMsg);
        return false;
    }
    qCInfo(KARTON_DEBUG) << "Successfully force-stopped domain:" << domain->config()->name();
    return true;
}

bool Karton::deleteDomain(const Domain *domain, const bool deleteDisk)
{
    virDomainPtr domainPtr = domain->domainPtr();
    int result = virDomainUndefine(domainPtr);

    if (result < 0) {
        QString errorMsg = QStringLiteral("Failed to undefine domain: %1").arg(domain->config()->name());
        qCWarning(KARTON_DEBUG) << errorMsg;
        Q_EMIT errorOccurred(errorMsg);
        return false;
    }
    
    if (deleteDisk) {
        if (!QFile::remove(domain->config()->isoDiskPath())) {
            QString errorMsg = QStringLiteral("Failed to delete disk file: %1").arg(domain->config()->isoDiskPath());
            qCWarning(KARTON_DEBUG) << errorMsg;
            Q_EMIT errorOccurred(errorMsg);
            return false;
        }
        qCInfo(KARTON_DEBUG) << "Successfully deleted disk image of " << domain->config()->name();
    }

    qCInfo(KARTON_DEBUG) << "Successfully undefined domain:" << domain->config()->name();
    
    if (deleteDisk) {
        if (!QFile::remove(domain->config()->isoDiskPath())) {
            QString errorMsg = i18nc("%1 is path of the disk file", "Failed to delete disk file: %1", domain->config()->isoDiskPath());
            qCWarning(KARTON_DEBUG) << errorMsg;
            Q_EMIT errorOccurred(errorMsg);
            return false;
        }
        qCInfo(KARTON_DEBUG) << "Successfully deleted disk image of " << domain->config()->name();
    }

    qCInfo(KARTON_DEBUG) << "Successfully undefined domain:" << domain->config()->name();
    return true;
}

bool Karton::viewDomain(const Domain *domain)
{
    return runCommand(QStringLiteral("virt-viewer --attach ") + domain->config()->name());
}

bool Karton::createDomain(const QString &name,
                                const QString &osVariant, 
                                const float memoryGB, 
                                const float storageGB, 
                                const QString &isoDiskPath,
                                const int cpus)
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    DomainInstaller installer;
    const DomainConfig *config = new DomainConfig (
                                name,
                                QString::fromUtf8("WIP"), // uuid
                                osVariant,
                                false,
                                QString::fromUtf8("WIP"), // state
                                memoryGB,
                                memoryGB, // current usage
                                cpus,
                                storageGB, // max
                                isoDiskPath,
                                QStringLiteral("%1/libvirt/images/%2.qcow2").arg(dataDir).arg(name), 
                                // virt disk path 
                                false,
                                this);
    installer.configureXML(m_conn, config);
    return true;
}

// Use for virsh, virt-viewer, virt-install and other CLI
bool Karton::runCommand(const QString &command)
{
    qCDebug(KARTON_DEBUG) << "Running Command:" << command;
    auto process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, process](int exitCode, QProcess::ExitStatus) {
        QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
        Q_EMIT commandFinished(exitCode, output);
    });
    process->startCommand(command);

    return process->waitForStarted();
}
