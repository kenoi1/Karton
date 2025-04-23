#pragma once

#include <QString>
#include <QObject>

extern "C" // due to undefined references to libosinfo stuff
{
#include <osinfo/osinfo.h>
}


class OsinfoConfig : public QObject
{
    Q_OBJECT

public:
    OsinfoConfig();
    ~OsinfoConfig();

    QString getOsIdFromDisk(const QString &diskPath);
    QString getOsArchitecture(const QString &osId);

private:
    bool initOsDb();
    
    OsinfoLoader *m_loader;
    OsinfoDb *m_db;    
};
