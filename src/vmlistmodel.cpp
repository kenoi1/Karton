// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "vmlistmodel.h"
#include "karton.h"
#include "karton_debug.h"

VMModel::VMModel(Karton *parent)
    : QAbstractListModel(parent)
{
    m_karton = parent;
    connect(m_karton, &Karton::domainsChanged, this, &VMModel::onDomainsChanged);
}
int VMModel::rowCount(const QModelIndex &parent) const
{
    return m_datas.size();
}

VMModel::~VMModel()
{
    m_datas.clear();
}
QVariant VMModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_datas.size())
        return QVariant();

    const Domain *domain = m_datas[index.row()];

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
    if(domainPtr) {
        updateDomains(domainPtr);
    }
}
void VMModel::refreshAllDomains() {
    qCDebug(KARTON_DEBUG) << "Refreshing all domains.";
    beginResetModel();
    m_datas.clear();
    m_karton->refreshDomainList();
    m_datas = m_karton->domains();
    endResetModel();
}

void VMModel::updateDomains(const virDomainPtr domainPtr)
{
    char uuid[VIR_UUID_STRING_BUFLEN] = {0};
    virDomainGetUUIDString(domainPtr, uuid);
    QString domainUuid = QString::fromUtf8(uuid);
    qCDebug(KARTON_DEBUG) << "Domain UUID:" << domainUuid;
    
    bool domainExists = false;
    virDomainInfo info;
    int ret = virDomainGetInfo(domainPtr, &info);
    domainExists = (ret == 0);
    
    if (!domainExists) {
        refreshAllDomains();
        return;
    }

    int modelIndex = -1;
    for (int i = 0; i < m_datas.size(); ++i) {
        if (m_datas[i]->uuid() == domainUuid) {
            modelIndex = i;
            break;
        }
    }
    
    if (modelIndex >= 0) {
        qCDebug(KARTON_DEBUG) << "Found matching domain in model at index" << modelIndex;
        m_karton->refreshDomain(domainPtr);
        
        QModelIndex qModelIndex = createIndex(modelIndex, 0);
        Q_EMIT dataChanged(qModelIndex, qModelIndex);
    } else {
        refreshAllDomains();
    }
}