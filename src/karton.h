// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>
#pragma once

#include <libvirt/libvirt.h>

#include <QObject>
#include <QProcess>
#include <QXmlStreamReader>

#include "domain.h"
#include "domainconfig.h"
#include "domaininstaller.h"
#include "domainviewer.h"

class LibvirtMonitor;

class Karton : public QObject {
    Q_OBJECT
    Q_PROPERTY(Domain *currentDomain READ currentDomain NOTIFY currentDomainChanged)

   public:
    explicit Karton(QObject *parent = nullptr);
    ~Karton();
    Q_DISABLE_COPY_MOVE(Karton)
    QVector<Domain *> domains();
    void refreshDomain(const virDomainPtr domainPtr);
    int searchDomain(virDomainPtr domainPtr);
    void refreshDomainList();

    void cleanupDomainViewer();

    void setCurrentDomain(Domain *domain) {
        if (m_currentDomain != domain) {
            m_currentDomain = domain;
            Q_EMIT currentDomainChanged();
        }
    }
    Domain *currentDomain() {
        if (!m_currentDomain)
            qWarning() << "Warning: currentDomain is null!";
        return m_currentDomain;
    }

   Q_SIGNALS:
    void currentDomainChanged();

   public Q_SLOTS:
    Q_INVOKABLE bool runCommand(const QString &command);
    Q_INVOKABLE bool startDomain(const Domain *domain);
    Q_INVOKABLE bool stopDomain(const Domain *domain);
    Q_INVOKABLE bool viewDomain(const Domain *domain);
    Q_INVOKABLE bool forceStopDomain(const Domain *domain);
    Q_INVOKABLE bool createDomain(const QString &name,
                                  const QString &shortOsId,
                                  const float memoryGB,
                                  const float storageGB,
                                  const QString &isoDiskPath,
                                  const int cpus);
    Q_INVOKABLE bool deleteDomain(const Domain *domain, const bool deleteDisk);

   Q_SIGNALS:
    void commandFinished(int exitCode, const QString &output);
    void domainsChanged(const virDomainPtr domainPtr, int event, int detail);
    void errorOccurred(const QString &errorMessage);

   private Q_SLOTS:
    void onDomainStateChanged(const virDomainPtr domainPtr, int event, int detail);

   private:
    virConnectPtr m_conn;
    QVector<Domain *> m_domains;
    LibvirtMonitor *m_monitor;

    DomainViewer *m_domainViewer = nullptr;
    Domain *m_currentDomain = nullptr;

    bool init();
};
