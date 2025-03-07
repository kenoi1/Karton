// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "vmlistmodel.h"
#include "karton.h"
// VM::VM (const QString& domainName, const QString& uuid, const bool& isRunning) {
//     m_domainName = domainName;
//     m_uuid = uuid;
//     m_isRunning = isRunning;
// }

// VM::VM() {} 

VMModel::VMModel(Karton *parent): QAbstractListModel(parent) {
    m_karton = parent;
}
int VMModel::rowCount(const QModelIndex& parent) const {
      return mDatas.size();
}
// int VMModel::columnCount(const QModelIndex& parent = QModelIndex()) const {
//    return 3;
// }
QVariant VMModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
        return QVariant();
    if (role == DomainNameRole)
        return mDatas[index.row()].name();
    if (role == UuidRole)
        return mDatas[index.row()].uuid();
    if (role == IsRunningRole)
        return mDatas[index.row()].isActive();
    return QVariant();
}
QHash<int, QByteArray> VMModel::roleNames() const {
            return {{DomainNameRole, "domainName"}, {UuidRole, "uuid"}, {IsRunningRole, "isRunning"}};
        }
void VMModel::populate()
{
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