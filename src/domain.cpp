// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domain.h"
#include <QDebug>
#include <QString>


Domain::Domain(const QString& name, 
               const QString& uuid, 
               const bool isActive,
               QString state,
               const int maxRam,
               const int ramUsage,
               const int cpus,
               const QString& diskPath,
               bool autostart)
    : m_name(name)
    , m_uuid(uuid)
    , m_isActive(isActive)
    , m_state(state)
    , m_maxRam(maxRam)
    , m_ramUsage(ramUsage)
    , m_cpus(cpus)
    , m_diskPath(diskPath)
    , m_autostart(autostart) 
{
    qDebug() << "Created domain object:" << m_name << "UUID:" << m_uuid 
             << "State:" << m_state << "Active:" << m_isActive;
}

