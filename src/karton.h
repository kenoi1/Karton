// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>
#pragma once

#include <QObject>
#include <QProcess>
#include <libvirt/libvirt.h>
#include "domain.h"

class Karton : public QObject {
    Q_OBJECT

    public:
        explicit Karton(QObject *parent = nullptr);
        ~Karton();
        QVector<Domain> domains();

    public Q_SLOTS:
        Q_INVOKABLE bool runVM(const QString &command);

    Q_SIGNALS:
        void commandFinished(int exitCode, const QString &output);
        // void domainsChanged();

    private:
        QProcess *m_process;
        virConnectPtr m_conn;
        QVector<Domain> m_domains;

        bool init();
        void refreshDomainList();
};
