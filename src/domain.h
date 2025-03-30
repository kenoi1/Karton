// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QString>
#include <libvirt/libvirt.h>

class DomainConfig;

class Domain : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString uuid READ uuid CONSTANT)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(int maxRam READ maxRam CONSTANT)
    Q_PROPERTY(int ramUsage READ ramUsage NOTIFY ramUsageChanged)
    Q_PROPERTY(int cpus READ cpus CONSTANT)
    Q_PROPERTY(QString diskPath READ diskPath CONSTANT)
    Q_PROPERTY(bool autostart READ autostart NOTIFY autostartChanged)

Q_SIGNALS:
    void isActiveChanged(const bool active);
    void stateChanged(const QString &state);
    void ramUsageChanged(const int usage);
    void autostartChanged(const bool autostart);

public:
    explicit Domain(QObject *parent = nullptr);
    Domain(virDomainPtr domainPtr,
           DomainConfig *config,
           QObject *parent = nullptr);
    ~Domain();

    // getters
    [[nodiscard]] virDomainPtr domainPtr() const
    {
        return m_domainPtr;
    }

    [[nodiscard]] DomainConfig *config() const
    {
        return m_config;
    }
    

private:
    virDomainPtr m_domainPtr;
    DomainConfig *m_config;
};
