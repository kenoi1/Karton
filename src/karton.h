// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>
#pragma once

#include "domain.h"
#include <QObject>
#include <QProcess>
#include <libvirt/libvirt.h>

class LibvirtMonitor;

class Karton : public QObject
{
    Q_OBJECT

public:
    explicit Karton(QObject *parent = nullptr);
    ~Karton();
    QVector<Domain *> domains();
    void refreshDomain(const virDomainPtr domainPtr);
    int searchDomain(virDomainPtr domainPtr);
    void refreshDomainList();

public Q_SLOTS:
    Q_INVOKABLE bool runCommand(const QString &command);
    // bool startDomain(const virDomainPtr domain);
    Q_INVOKABLE bool startDomain(const Domain *domain);
    Q_INVOKABLE bool stopDomain(const Domain *domain);
    Q_INVOKABLE bool viewDomain(const Domain *domain);
    Q_INVOKABLE bool forceStopDomain(const Domain *domain);

Q_SIGNALS:
    void commandFinished(int exitCode, const QString &output);
    void domainsChanged(const virDomainPtr domainPtr, int event, int detail);

private Q_SLOTS:
    void onDomainStateChanged(const virDomainPtr domainPtr, int event, int detail);

private:
    QProcess *m_process;
    virConnectPtr m_conn;
    QVector<Domain *> m_domains;
    LibvirtMonitor *m_monitor;

    bool init();
};
