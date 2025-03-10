// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>
#pragma once

#include <QObject>
#include <QProcess>
#include <libvirt/libvirt.h>
#include "domain.h"

class LibvirtMonitor;

class Karton : public QObject {
    Q_OBJECT

    public:
        explicit Karton(QObject *parent = nullptr);
        ~Karton();
        QVector<Domain> domains();

    public Q_SLOTS:
        Q_INVOKABLE bool runCommand(const QString &command);
        bool startDomain(const virDomainPtr domain);
        Q_INVOKABLE bool startDomain(const QString &uuid);
        Q_INVOKABLE bool stopDomain(const QString &uuid);
        Q_INVOKABLE bool viewDomain(const QString &domainName);
        Q_INVOKABLE bool forceStopDomain(const QString &uuid);

    Q_SIGNALS:
        void commandFinished(int exitCode, const QString &output);
        void domainsChanged(const QString &domainName, int event, int detail);

    private Q_SLOTS:
        void onDomainStateChanged(const QString &domainName, int event, int detail);
    
    private:
        QProcess *m_process;
        virConnectPtr m_conn;
        QVector<Domain> m_domains;
        LibvirtMonitor *m_monitor;

        bool init();
        void refreshDomainList();
};
