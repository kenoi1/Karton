// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "vmlistmodel.h"
#include "karton.h"

VMModel::VMModel(Karton *parent): QAbstractListModel(parent) {
    m_karton = parent;
    connect(m_karton, &Karton::domainsChanged, this, &VMModel::onDomainsChanged);
}
int VMModel::rowCount(const QModelIndex& parent) const {
      return mDatas.size();
}

QVariant VMModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid() || index.row() >= mDatas.size())
        return QVariant();
        
    const Domain &domain = mDatas[index.row()];
    
    switch (role) {
        case DomainNameRole:
            return domain.name();
        case UuidRole:
            return domain.uuid();
        case IsActiveRole:
            return domain.isActive();
        case StateRole:
            return domain.state();
        case MaxRamRole:
            return domain.maxRam();
        case RamUsageRole:
            return domain.ramUsage();
        case CpusRole:
            return domain.cpus();
        case DiskPathRole:
            return domain.diskPath();
        case AutostartRole:
            return domain.autostart();
        default:
            return QVariant();
    }
}
QHash<int, QByteArray> VMModel::roleNames() const {
    return {
        {DomainNameRole, "domainName"},
        {UuidRole, "uuid"},
        {IsActiveRole, "isActive"},
        {StateRole, "state"},
        {MaxRamRole, "maxRam"},
        {RamUsageRole, "ramUsage"},
        {CpusRole, "cpus"},
        {DiskPathRole, "diskPath"},
        {AutostartRole, "autostart"}
    };
}
void VMModel::onDomainsChanged (const QString &domainName, int event, int detail) {
    updateDomains();
}
void VMModel::updateDomains() {
        beginResetModel();
        mDatas.clear();
        mDatas = m_karton->domains();

        /*
        Testing Sample Data
        */
        // mDatas.clear();
        // mDatas.append(VM(QStringLiteral("Fedora"), QStringLiteral("3hr9823u8f924u8"), true));
        // mDatas.append(VM(QStringLiteral("Mint"), QStringLiteral("u9f898u498f2"), false));
        // mDatas.append(VM(QStringLiteral("Ubuntu Tux :)"), QStringLiteral("4u98fu4398"), true));
        endResetModel();
}