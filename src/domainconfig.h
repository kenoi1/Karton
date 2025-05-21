// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QObject>
#include <QString>

class DomainConfig : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString hypervisorType READ hypervisorType CONSTANT)
    Q_PROPERTY(int indexId READ indexId CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString uuid READ uuid CONSTANT)
    Q_PROPERTY(QString shortOsId READ shortOsId CONSTANT)
    Q_PROPERTY(QString osId READ osId CONSTANT)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(int maxRam READ maxRam CONSTANT)
    Q_PROPERTY(int ramUsage READ ramUsage NOTIFY ramUsageChanged)
    Q_PROPERTY(int cpus READ cpus CONSTANT)
    Q_PROPERTY(int maxDiskStorage READ maxDiskStorage CONSTANT)
    Q_PROPERTY(QString xmlConfigPath READ xmlConfigPath CONSTANT)
    Q_PROPERTY(QString isoDiskPath READ isoDiskPath CONSTANT)
    Q_PROPERTY(QString virtualDiskPath READ virtualDiskPath CONSTANT)
    Q_PROPERTY(bool autostart READ autostart NOTIFY autostartChanged)

Q_SIGNALS:
    void isActiveChanged(bool active);
    void stateChanged(const QString &state);
    void ramUsageChanged(int ramUsage);
    void autostartChanged(bool autostart);

public:
    struct DomainConfigData {
        QString hypervisorType;
        int indexId;
        QString name;
        QString uuid;
        QString shortOsId;
        QString osId;
        bool isActive;
        QString state;
        int maxRam;
        int ramUsage;
        int cpus;
        int maxDiskStorage;
        QString xmlConfigPath;
        QString isoDiskPath;
        QString virtualDiskPath;
        bool autostart;
        QObject *parent = nullptr;
    };
    explicit DomainConfig(QObject *parent = nullptr);
    explicit DomainConfig(const DomainConfigData &data);
    explicit DomainConfig(const QString &hypervisorType,
                          int indexId,
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
                          QObject *parent = nullptr);

    // getters
    [[nodiscard]] QString hypervisorType() const
    {
        return m_hypervisorType;
    }
    [[nodiscard]] int indexId() const
    {
        return m_indexId;
    }
    [[nodiscard]] QString name() const
    {
        return m_name;
    }
    [[nodiscard]] QString uuid() const
    {
        return m_uuid;
    }
    [[nodiscard]] QString shortOsId() const
    {
        return m_shortOsId;
    }
    [[nodiscard]] QString osId() const
    {
        return m_osId;
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
    [[nodiscard]] QString xmlConfigPath() const
    {
        return m_xmlConfigPath;
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
    QString m_hypervisorType;
    int m_indexId;
    QString m_name;
    QString m_uuid;
    QString m_shortOsId;
    QString m_osId;
    bool m_isActive;
    QString m_state;
    int m_maxRam;
    int m_ramUsage;
    int m_cpus;
    int m_maxDiskStorage;
    QString m_xmlConfigPath;
    QString m_isoDiskPath;
    QString m_virtualDiskPath;
    bool m_autostart;
};