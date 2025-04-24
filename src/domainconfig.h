// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QString>
#include <QObject>

class DomainConfig : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString uuid READ uuid CONSTANT)
    Q_PROPERTY(QString osVariant READ osVariant CONSTANT)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(int maxRam READ maxRam CONSTANT)
    Q_PROPERTY(int ramUsage READ ramUsage NOTIFY ramUsageChanged)
    Q_PROPERTY(int cpus READ cpus CONSTANT)
    Q_PROPERTY(int maxDiskStorage READ maxDiskStorage CONSTANT)
    Q_PROPERTY(QString isoDiskPath READ isoDiskPath CONSTANT)
    Q_PROPERTY(QString virtualDiskPath READ virtualDiskPath CONSTANT)
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
                    const QString &osVariant,
                    bool isActive,
                    QString state,
                    int maxRam,
                    int ramUsage,
                    int cpus,
                    int maxDiskStorage,
                    const QString &isoDiskPath,
                    const QString &virtualDiskPath,
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
    [[nodiscard]] QString osVariant() const
    {
        return m_osVariant;
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
    [[nodiscard]] int maxDiskStorage() const
    {
        return m_maxDiskStorage;
    }
    [[nodiscard]] QString isoDiskPath() const
    {
        return m_isoDiskPath;
    }
    [[nodiscard]] QString virtualDiskPath() const
    {
        return m_virtualDiskPath;
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
    QString m_osVariant;
    bool m_isActive;
    QString m_state;
    int m_maxRam;
    int m_ramUsage;
    int m_cpus;
    int m_maxDiskStorage;
    QString m_isoDiskPath;
    QString m_virtualDiskPath;
    bool m_autostart;
};