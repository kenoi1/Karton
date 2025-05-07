// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domain.h"

#include <QString>

#include "domainconfig.h"
#include "karton_debug.h"

Domain::Domain(QObject *parent)
    : QObject(parent)
    , m_domainPtr(nullptr)
    , m_config(new DomainConfig(this))
{
    connect(m_config, &DomainConfig::isActiveChanged, this, &Domain::isActiveChanged);
    connect(m_config, &DomainConfig::stateChanged, this, &Domain::stateChanged);
    connect(m_config, &DomainConfig::ramUsageChanged, this, &Domain::ramUsageChanged);
    connect(m_config, &DomainConfig::autostartChanged, this, &Domain::autostartChanged);
}

Domain::Domain(const virDomainPtr domainPtr, DomainConfig *config, QObject *parent)
    : QObject(parent)
    , m_domainPtr(domainPtr)
    , m_config(config)
{
    connect(m_config, &DomainConfig::isActiveChanged, this, &Domain::isActiveChanged);
    connect(m_config, &DomainConfig::stateChanged, this, &Domain::stateChanged);
    connect(m_config, &DomainConfig::ramUsageChanged, this, &Domain::ramUsageChanged);
    connect(m_config, &DomainConfig::autostartChanged, this, &Domain::autostartChanged);
}

Domain::~Domain()
{
    if (m_domainPtr) {
        virDomainFree(m_domainPtr);
    }
}

void Domain::setActive(bool active)
{
    m_config->setActive(active);
    Q_EMIT isActiveChanged(active);
}

void Domain::setState(const QString &state)
{
    m_config->setState(state);
    Q_EMIT stateChanged(state);
}

void Domain::setRamUsage(int ramUsage)
{
    m_config->setRamUsage(ramUsage);
    Q_EMIT ramUsageChanged(ramUsage);
}

void Domain::setAutostart(bool autostart)
{
    m_config->setAutostart(autostart);
    Q_EMIT autostartChanged(autostart);
}

QString Domain::uuidString(virDomainPtr domainPtr)
{
    if (!domainPtr) {
        return {};
    }

    std::array<char, VIR_UUID_STRING_BUFLEN> uuid = {};
    if (virDomainGetUUIDString(domainPtr, uuid.data()) == -1) {
        qCWarning(KARTON_DEBUG) << "Failed to get UUID string for" << domainPtr;
        return {};
    }
    return QString::fromUtf8(uuid.data());
}
