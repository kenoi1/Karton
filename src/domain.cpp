// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domain.h"
#include <QString>

Domain::Domain(QObject *parent)
    : QObject(parent)
    , m_domainPtr(nullptr)
    , m_name(QStringLiteral(""))
    , m_uuid(QStringLiteral(""))
    , m_isActive(false)
    , m_maxRam(0)
    , m_ramUsage(0)
    , m_cpus(0)
    , m_diskPath(QStringLiteral(""))
    , m_autostart(false)
{
}
Domain::Domain(const virDomainPtr domainPtr,
               const QString &name,
               const QString &uuid,
               const bool isActive,
               QString state,
               const int maxRam,
               const int ramUsage,
               const int cpus,
               const QString &diskPath,
               bool autostart,
               QObject *parent)
    : QObject(parent)
    , m_domainPtr(domainPtr)
    , m_name(name)
    , m_uuid(uuid)
    , m_isActive(isActive)
    , m_state(state)
    , m_maxRam(maxRam)
    , m_ramUsage(ramUsage)
    , m_cpus(cpus)
    , m_diskPath(diskPath)
    , m_autostart(autostart)
{
    // qCDebug(KARTON_DEBUG) << "Created domain object:" << m_name << "UUID:" << m_uuid
    //  << "State:" << m_state << "Active:" << m_isActive;
}
Domain::~Domain()
{
    virDomainFree(m_domainPtr);
}

void Domain::setActive(bool active) {
    if (m_isActive != active) {
        m_isActive = active;
        Q_EMIT isActiveChanged(active);
    }
}

void Domain::setState(const QString &state) {
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged(state);
    }
}

void Domain::setRamUsage(int ramUsage) {
    if (m_ramUsage != ramUsage) {
        m_ramUsage = ramUsage;
        Q_EMIT ramUsageChanged(ramUsage);
    }
}

void Domain::setAutostart(bool autostart) {
    if (m_autostart != autostart) {
        m_autostart = autostart;
        Q_EMIT autostartChanged(autostart);
    }
}
