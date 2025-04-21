// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domainconfig.h"
#include <QString>

DomainConfig::DomainConfig(QObject *parent)
    : QObject(parent), m_name(QStringLiteral("")), m_uuid(QStringLiteral("")), m_isActive(false), m_maxRam(0), m_ramUsage(0), m_cpus(0), m_diskPath(QStringLiteral("")), m_autostart(false)
{
}

DomainConfig::DomainConfig(const QString &name,
                           const QString &uuid,
                           bool isActive,
                           QString state,
                           int maxRam,
                           int ramUsage,
                           int cpus,
                           const QString &diskPath,
                           bool autostart,
                           QObject *parent)
    : QObject(parent), m_name(name), m_uuid(uuid), m_isActive(isActive), m_state(state), m_maxRam(maxRam), m_ramUsage(ramUsage), m_cpus(cpus), m_diskPath(diskPath), m_autostart(autostart)
{
}

void DomainConfig::setActive(bool active)
{
    if (m_isActive != active)
    {
        m_isActive = active;
        Q_EMIT isActiveChanged(active);
    }
}

void DomainConfig::setState(const QString &state)
{
    if (m_state != state)
    {
        m_state = state;
        Q_EMIT stateChanged(state);
    }
}

void DomainConfig::setRamUsage(int ramUsage)
{
    if (m_ramUsage != ramUsage)
    {
        m_ramUsage = ramUsage;
        Q_EMIT ramUsageChanged(ramUsage);
    }
}

void DomainConfig::setAutostart(bool autostart)
{
    if (m_autostart != autostart)
    {
        m_autostart = autostart;
        Q_EMIT autostartChanged(autostart);
    }
}

// QVariantMap DomainConfig::serialize() const
// {
//     QVariantMap map;

// }
