// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QObject>
#include <libvirt/libvirt.h>
#include <QString>

class Domain {
    public:
    Domain(const virDomainPtr domainPtr,
            const QString& name,
            const QString& uuid,
            const bool isActive,
            QString state,
            const int maxRam,
            const int ramUsage,
            const int cpus,
            const QString& diskPath,
            bool autostart);

    // getters
    virDomainPtr domainPtr() const { return m_domainPtr; }
    QString name() const { return m_name; }
    QString uuid() const { return m_uuid; }
    bool isActive() const { return m_isActive; }
    QString state() const {return m_state; }
    int maxRam() const { return m_maxRam; }
    int ramUsage() const { return m_ramUsage; }
    int cpus() const { return m_cpus; }
    QString diskPath() const { return m_diskPath; }
    bool autostart() const { return m_autostart; }

    private:
        virDomainPtr m_domainPtr;
        QString m_name;
        QString m_uuid;
        bool m_isActive;
        QString m_state;
        int m_maxRam;
        int m_ramUsage;
        int m_cpus;
        QString m_diskPath;
        bool m_autostart;
};



