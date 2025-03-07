#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <libvirt/libvirt.h>
// #include <QDate>
#include <QVariant>
#include <QModelIndex>
#include <QHash>
#include <QByteArray>

class VM {
    public :
        VM();
        VM(const QString& domainName, const QString& uuid, const bool& isRunning);  
        
        QString domainName() const {return m_domainName;}
        QString uuid() const {return m_uuid;}
        bool isRunning() const {return m_isRunning;}
    private:
        QString m_domainName; 
        QString m_uuid;
        bool m_isRunning;
};

class VMModel : public QAbstractListModel {
    Q_OBJECT
    public:
        enum Roles {
            DomainNameRole,
            UuidRole,
            IsRunningRole
        };
        VMModel(QObject * parent = 0);
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QHash<int, QByteArray> roleNames() const;
        // int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role) const;
        void populate();
    private:
        QList<VM> mDatas;

};