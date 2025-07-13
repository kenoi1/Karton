// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>
#pragma once

#include <libvirt/libvirt.h>

#include <QJSEngine>
#include <QObject>
#include <QProcess>
#include <QQmlEngine>
#include <QXmlStreamReader>

#include "commandrunner.h"
#include "domain.h"
#include "domainviewer.h"
#include <qqmlintegration.h>

class LibvirtMonitor;

struct _virDomain; // need this for compile issues
Q_DECLARE_OPAQUE_POINTER(_virDomain *)
Q_DECLARE_METATYPE(_virDomain *)

class Karton : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(Domain *currentDomain READ currentDomain NOTIFY currentDomainChanged) // TODO extract

public:
    explicit Karton(QObject *parent = nullptr);
    ~Karton();
    Q_DISABLE_COPY_MOVE(Karton)
    static Karton *create(QQmlEngine *qmlEngine, QJSEngine *);
    static Karton *self();

    QVector<Domain *> domains();
    void refreshDomain(const virDomainPtr domainPtr);
    int searchDomain(virDomainPtr domainPtr);
    void refreshDomainList();
    QString getVirtualDiskPath(const QString &domainName);
    QString getXmlConfigPath(const QString &domainName);

    void cleanupDomainViewer();
    void setCurrentDomain(Domain *domain);
    Domain *currentDomain();

Q_SIGNALS:
    void currentDomainChanged();
    void commandFinished(int exitCode, const QString &output);

public Q_SLOTS:
    Q_INVOKABLE bool startDomain(const Domain *domain);
    Q_INVOKABLE bool stopDomain(const Domain *domain);
    Q_INVOKABLE bool viewDomain(const Domain *domain);
    Q_INVOKABLE bool forceStopDomain(const Domain *domain);
    Q_INVOKABLE bool createDomain(const QVariantMap &config);
    Q_INVOKABLE bool deleteDomain(const Domain *domain, const bool deleteDisk);

Q_SIGNALS:
    void domainsChanged(const virDomainPtr domainPtr, int event, int detail);
    void errorOccurred(const QString &errorMessage);

private Q_SLOTS:
    void onDomainStateChanged(const virDomainPtr domainPtr, int event, int detail);

private:
    virConnectPtr m_conn;
    CommandRunner *m_commandRunner;
    QVector<Domain *> m_domains;
    LibvirtMonitor *m_monitor;

    DomainViewer *m_domainViewer = nullptr;
    Domain *m_currentDomain = nullptr;

    bool init();
};
