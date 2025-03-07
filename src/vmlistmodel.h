#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <libvirt/libvirt.h>
#include <QDate>
#include <QVariant>
#include <QModelIndex>
#include <QHash>
#include <QByteArray>

class VM {
    public :
        VM();
        VM(const QString& firstname, const QString& lastname, const QDate& birthday);  
        
        QString firstname() const {return mFirstname;}
        QString lastname() const {return mLastName;}
        QDate birthday() const {return mBirthday;}
    private:
        QString mFirstname; 
        QString mLastName;
        QDate mBirthday;
};

class VMModel : public QAbstractListModel {
    Q_OBJECT
    public:
        enum Roles {
            Firstnamerole,
            LastNamerole,
            Birthdayrole,

        }
        VMModel(QObject * parent = 0);
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QHash<int, QByteArray> roleNames() const
        // int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role) const;
        void populate();
    private:
        QList<VM> mDatas;

};