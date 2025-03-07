// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QObject>
#include <libvirt/libvirt.h>
#include <QString>

class Domain {
    public:
    Domain(const QString& name, const QString& uuid, const bool isActive);

    QString name() const { return m_name; }
    QString uuid() const { return m_uuid; }
    bool isActive() const { return m_isActive; }
    QString statusString() const { return m_isActive ? QStringLiteral("running") : QStringLiteral("stopped"); }

    // void setDomainPtr(virDomainPtr domainPtr) { m_domainPtr = domainPtr; }

    private:
        QString m_name;
        QString m_uuid;
        bool m_isActive;
        // virDomainPtr m_domainPtr = nullptr;

};



