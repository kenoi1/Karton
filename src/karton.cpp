// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "karton.h"

#include <libvirt/libvirt.h>

#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include "domain.h"
#include "domainconfig.h"
#include "domaininstaller.h"
#include "domainxmlreader.h"
#include "karton_debug.h"
#include "libvirtmonitor.h"

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
        QString xmlConfigPath = QStringLiteral("%1/libvirt/kde-karton/config/%2_config.xml").arg(dataDir).arg(QString::fromUtf8(name));

        DomainXmlReader *reader = new DomainXmlReader(xmlConfigPath);

        int autoFlag = 0;
        virDomainGetAutostart(domains[i], &autoFlag);
        bool autostart = (autoFlag != 0);

        // TODO: add more fields to xml metadata and parse.
        DomainConfigData data = {.hypervisorType = reader->m_xmlInfo.hypervisorType,
                                 .index = reader->m_xmlInfo.indexId,
                                 .name = QString::fromUtf8(name),
                                 .uuid = Domain::uuidString(domainPtr),
                                 .shortOsId = reader->m_xmlInfo.shortOsId,
                                 .id = reader->m_xmlInfo.osId,
                                 .isActive = isActive,
                                 .state = state,
                                 .maxMemory = maxRam,
                                 .currentMemory = ramUsage,
                                 .vcpus = cpus,
                                 .storage = reader->m_xmlInfo.maxDiskStorage / 1024,
                                 .configPath = xmlConfigPath,
                                 .isoDiskPath = reader->m_xmlInfo.isoDiskPath,
                                 .virtDiskPath = reader->m_xmlInfo.virtualDiskPath,
                                 .autostart = autostart,
                                 .parent = this};

        DomainConfig *config = new DomainConfig(data);
        Domain *domain = new Domain(domainPtr, config, this);
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

    qCInfo(KARTON_DEBUG) << "Successfully undefined domain:" << domain->config()->name();

    if (deleteDisk) {
        if (!QFile::remove(domain->config()->virtualDiskPath())) {
            QString errorMsg = i18nc("%1 is path of the disk file", "Failed to delete disk file: %1", domain->config()->virtualDiskPath());
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

bool Karton::createDomain(const QVariantMap &config)
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QString path = QStringLiteral("%1/libvirt").arg(dataDir);
    QDir dir(path);
    if (!dir.mkpath(QStringLiteral("images"))) // generates path to virtual disk folder if not there
    {
        qCCritical(KARTON_DEBUG) << "Already Exists / Failed: " << path;
    }

    DomainInstaller installer;
    DomainConfigData configData = {.hypervisorType = QString(),
                                   .index = 0,
                                   .name = config.value(QStringLiteral("name")).toString(),
                                   .uuid = QString(),
                                   .shortOsId = config.value(QStringLiteral("shortOsId")).toString(),
                                   .id = QString(),
                                   .isActive = false,
                                   .state = QString(),
                                   .maxMemory = config.value(QStringLiteral("memoryGB")).toInt(),
                                   .currentMemory = config.value(QStringLiteral("memoryGB")).toInt(),
                                   .vcpus = config.value(QStringLiteral("cpus")).toInt(),
                                   .storage = config.value(QStringLiteral("storageGB")).toInt(),
                                   .configPath =
                                       QDir(path).filePath(QStringLiteral("karton-kde/config/%1.xml").arg(config.value(QStringLiteral("name")).toString())),
                                   .isoDiskPath = config.value(QStringLiteral("isoDiskPath")).toString(),
                                   .virtDiskPath = QDir(path).filePath(QStringLiteral("images/%1.qcow2").arg(config.value(QStringLiteral("name")).toString())),
                                   .someFlag = false,
                                   .parent = this};

    auto domainConfig = std::make_unique<DomainConfig>(configData);

    if (!runCommand(QStringLiteral("qemu-img create -f qcow2 %1 %2G").arg(domainConfig->virtualDiskPath()).arg(domainConfig->maxDiskStorage()))) {
        return false;
    }
    if (!installer.setupDomain(m_conn, domainConfig.get())) {
        qCInfo(KARTON_DEBUG) << "Failed to setup domain...";
        return false;
    }
    // TODO: Use storage pool (poolcreate, gen pool xml, parse xml for location)
    refreshDomainList();
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
