// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domainconfig.h"
#include <QString>

DomainConfig::DomainConfig(QObject *parent)
    : QObject(parent),
      m_hypervisorType(QStringLiteral("")),
      m_indexId(0),
      m_name(QStringLiteral("")),
      m_uuid(QStringLiteral("")),
      m_shortOsId(QStringLiteral("")),
      m_osId(QStringLiteral("")),
      m_isActive(false),
      m_state(QStringLiteral("")),
      m_maxRam(0),
      m_ramUsage(0),
      m_maxDiskStorage(0),
      m_xmlConfigPath(QStringLiteral("")),
      m_isoDiskPath(QStringLiteral("")),
      m_virtualDiskPath(QStringLiteral("")),
      m_autostart(false)
{
}

DomainConfig::DomainConfig(const QString &hypervisorType,
                           const int indexId,
                           const QString &name,
                           const QString &uuid,
                           const QString &shortOsId,
                           const QString &osId,
                           bool isActive,
                           QString state,
                           int maxRam,
                           int ramUsage,
                           int cpus,
                           int maxDiskStorage,
                           const QString &xmlConfigPath,
                           const QString &isoDiskPath,
                           const QString &virtualDiskPath,
                           bool autostart,
                           QObject *parent)
    : QObject(parent),
      m_hypervisorType(hypervisorType),
      m_indexId(indexId),
      m_name(name),
      m_uuid(uuid),
      m_shortOsId(shortOsId),
      m_osId(osId),
      m_isActive(isActive),
      m_state(state),
      m_maxRam(maxRam),
      m_ramUsage(ramUsage),
      m_cpus(cpus),
      m_maxDiskStorage(maxDiskStorage),
      m_xmlConfigPath(xmlConfigPath),
      m_isoDiskPath(isoDiskPath),
      m_virtualDiskPath(virtualDiskPath),
      m_autostart(autostart)
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
