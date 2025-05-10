// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <domainconfig.h>
#include <libvirt/libvirt.h>

#include <QObject>
#include <QString>

class Domain : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(int ramUsage READ ramUsage NOTIFY ramUsageChanged)
    Q_PROPERTY(bool autostart READ autostart NOTIFY autostartChanged)
    Q_PROPERTY(DomainConfig *config READ config CONSTANT)

Q_SIGNALS:
    void isActiveChanged(const bool active);
    void stateChanged(const QString &state);
    void ramUsageChanged(const int usage);
    void autostartChanged(const bool autostart);
    void configChanged();

public:
    Domain(QObject *parent = nullptr);
    Domain(const virDomainPtr domainPtr, DomainConfig *config, QObject *parent = nullptr);
    Q_DISABLE_COPY_MOVE(Domain);
    ~Domain() override;

    // getters
    [[nodiscard]] virDomainPtr domainPtr() const
    {
        return m_domainPtr;
    }

    [[nodiscard]] DomainConfig *config() const
    {
        return m_config;
    }

    [[nodiscard]] bool isActive() const
    {
        return m_config->isActive();
    }
    [[nodiscard]] int ramUsage() const
    {
        return m_config->ramUsage();
    }
    [[nodiscard]] QString state() const
    {
        return m_config->state();
    }
    [[nodiscard]] bool autostart() const
    {
        return m_config->autostart();
    }

    void setActive(bool active);
    void setState(const QString &state);
    void setRamUsage(int ramUsage);
    void setAutostart(bool autostart);
    static QString uuidString(virDomainPtr domainPtr);

private:
    virDomainPtr m_domainPtr;
    DomainConfig *m_config;
};
