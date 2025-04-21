// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QString>
#include <QObject>

class DomainConfig : public QObject
{
    Q_OBJECT

    // Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    // Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
    // Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)
    // Q_PROPERTY(QString state READ state WRITE setState NOTIFY stateChanged)
    // Q_PROPERTY(int maxRam READ maxRam WRITE setMaxRam NOTIFY maxRamChanged)
    // Q_PROPERTY(int ramUsage READ ramUsage WRITE setRamUsage NOTIFY ramUsageChanged)
    // Q_PROPERTY(int cpus READ cpus WRITE setCpus NOTIFY cpusChanged)
    // Q_PROPERTY(QString diskPath READ diskPath WRITE setDiskPath NOTIFY diskPathChanged)
    // Q_PROPERTY(bool autostart READ autostart WRITE setAutostart NOTIFY autostartChanged)

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
    // void nameChanged(const QString &name);
    // void uuidChanged(const QString &uuid);
    void isActiveChanged(bool active);
    void stateChanged(const QString &state);
    // void maxRamChanged(int maxRam);
    void ramUsageChanged(int ramUsage);
    // void cpusChanged(int cpus);
    // void diskPathChanged(const QString &diskPath);
    void autostartChanged(bool autostart);

public:
    explicit DomainConfig(QObject *parent = nullptr);
    explicit DomainConfig(const QString &name,
                    const QString &uuid,
                    bool isActive,
                    QString state,
                    int maxRam,
                    int ramUsage,
                    int cpus,
                    const QString &diskPath,
                    bool autostart,
                    QObject *parent = nullptr);

    // getters
    [[nodiscard]] QString name() const
    {
        return m_name;
    }
    [[nodiscard]] QString uuid() const
    {
        return m_uuid;
    }
    [[nodiscard]] bool isActive() const
    {
        return m_isActive;
    }
    [[nodiscard]] QString state() const
    {
        return m_state;
    }
    [[nodiscard]] int maxRam() const
    {
        return m_maxRam;
    }
    [[nodiscard]] int ramUsage() const
    {
        return m_ramUsage;
    }
    [[nodiscard]] int cpus() const
    {
        return m_cpus;
    }
    [[nodiscard]] QString diskPath() const
    {
        return m_diskPath;
    }
    [[nodiscard]] bool autostart() const
    {
        return m_autostart;
    }

    void setActive(bool active);
    void setState(const QString &state);
    void setRamUsage(int ramUsage);
    void setAutostart(bool autostart);

private:
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