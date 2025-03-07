#include "vmlistmodel.h"

VM::VM (const QString& firstname, const QString& lastname, const QDate& birthday) {
    mFirstname = firstname;
    mLastName = lastname;
    mBirthday = birthday;
}
VM::VM() { } 
VMModel::VMModel(QObject *parent): QAbstractListModel(parent) {
    
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
   if ( role == Qt::DisplayRole)
   {
       if ( index.column() == 0)
           return mDatas[index.row()].firstname();
       if ( index.column() == 1)
           return mDatas[index.row()].lastname();
      if ( index.column() == 2)
           return mDatas[index.row()].birthday();
   }
   return QVariant();
}
void VMModel::populate()
{
        beginResetModel();
        mDatas.clear();
        mDatas.append(VM(QStringLiteral("Charles"), QStringLiteral("Charles"), QDate(1812,22,23)));
        mDatas.append(VM(QStringLiteral("Charles"), QStringLiteral("Charles"), QDate(1976,22,12)));
        mDatas.append(VM(QStringLiteral("Charles"), QStringLiteral("Charles"), QDate(1951,21,31)));
        endResetModel();
}