// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

#ifndef BACKENDENGINE_H
#define BACKENDENGINE_H

#include <QObject>
#include <QList>
#include <QtQml/qqml.h>
#include "kartonmachine.h"

class BackendEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString vmRepoPath READ vmRepoPath CONSTANT)
    Q_PROPERTY(QStringList osList READ osList CONSTANT)
    Q_PROPERTY(QStringList firmwareList READ firmwareList CONSTANT)
    Q_PROPERTY(QString selectedIsoFile READ selectedIsoFile NOTIFY selectedIsoFileChanged)
    QML_ELEMENT
public:
    explicit BackendEngine(QObject *parent = nullptr);

    // Internal
    QList<KartonMachine *> *machineList();
    void setMachineList(QList<KartonMachine *> *val);
    QString vmRepoPath();

    // Global data
    QStringList osList();
    QStringList firmwareList();
    void setOsList(QStringList val); // Intentionally not exposed to QML
    void setFirmwareList(QStringList val); // Intentionally not exposed to QML

    // Machine info gathering
    Q_INVOKABLE void loadVmList();
    Q_INVOKABLE quint32 getMachineCount();
    Q_INVOKABLE QStringList getMachineData(quint32 idx);

    // VM management
    Q_INVOKABLE bool createVm(QString firmware, QString os, quint32 cpus, quint32 ram, quint32 diskSize, QString name, QString cdLoc);
    Q_INVOKABLE bool configVm(quint32 idx, QString firmware, QString os, quint32 cpus, quint32 ram, QString name, QString cdLoc);
    Q_INVOKABLE void launchVm(quint32 idx, bool fromCd);
    Q_INVOKABLE void deleteVm(quint32 idx);

    // VM creation
    QString selectedIsoFile();
    Q_INVOKABLE void selectIsoFile();

    // Error reporting
    Q_INVOKABLE KartonError::VmError vmErr(quint32 idx);

Q_SIGNALS:
    void selectedIsoFileChanged();

private:
    static QList<KartonMachine *> *m_machineList;
    static QString m_vmRepoPath;
    static QStringList m_osList;
    static QStringList m_firmwareList;
    static QString m_selectedIsoFile;
};

#endif // BACKENDENGINE_H
