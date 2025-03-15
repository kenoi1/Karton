// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "vmlistmodel.h"
#include "karton.h"
#include <QDebug>

VMModel::VMModel(Karton *parent)
    : QAbstractListModel(parent)
{
    m_karton = parent;
    connect(m_karton, &Karton::domainsChanged, this, &VMModel::onDomainsChanged);
}
int VMModel::rowCount(const QModelIndex &parent) const
{
    return mDatas.size();
}

VMModel::~VMModel()
{
    mDatas.clear();
}
QVariant VMModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mDatas.size())
        return QVariant();

    const Domain *domain = mDatas[index.row()];

    if (role == DomainRole) {
        return QVariant::fromValue(domain);
    }
    return QVariant();
}
QHash<int, QByteArray> VMModel::roleNames() const
{
    return {{DomainRole, "domain"}};
}

void VMModel::onDomainsChanged(const virDomainPtr domainPtr, int event, int detail)
{
    updateDomains(domainPtr);
}

void VMModel::updateDomains(const virDomainPtr domainPtr)
{
    char uuid[VIR_UUID_STRING_BUFLEN];
    virDomainGetUUIDString(domainPtr, uuid);
    QString domainUuid = QString::fromUtf8(uuid);
    qDebug() << "Domain UUID:" << domainUuid;
    
    int modelIndex = -1;
    for (int i = 0; i < mDatas.size(); ++i) {
        if (mDatas[i]->uuid() == domainUuid) {
            modelIndex = i;
            break;
        }
    }
    
    if (modelIndex >= 0) {
        qDebug() << "Found matching domain in model at index" << modelIndex;
        m_karton->refreshDomain(domainPtr);
        
        QModelIndex qModelIndex = createIndex(modelIndex, 0);
        Q_EMIT dataChanged(qModelIndex, qModelIndex);
    } else {
        qDebug() << "Domain not found in model by UUID, reset list";
        beginResetModel();
        mDatas.clear();
        m_karton->refreshDomainList();
        mDatas = m_karton->domains();
        endResetModel();
    }
}