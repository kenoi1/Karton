// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <libvirt/libvirt.h>
#include <QVariant>
#include <QModelIndex>
#include <QHash>
#include <QByteArray>
#include "domain.h"
#include "karton.h"

class VMModel : public QAbstractListModel {
    Q_OBJECT
    public:
        enum Roles {
            DomainNameRole,
            UuidRole,
            IsActiveRole,
            StateRole,
            MaxRamRole,
            RamUsageRole,
            CpusRole,
            DiskPathRole,
            AutostartRole,
            DomainObjectRole
    };
        VMModel(Karton * parent = nullptr);
        ~VMModel();

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QHash<int, QByteArray> roleNames() const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        void updateDomains();
    private Q_SLOTS:
        void onDomainsChanged (const QString &domainName, int event, int detail);
    private:
        QList<Domain*> mDatas;
        Karton *m_karton;
};