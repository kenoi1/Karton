// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

#ifndef KARTONMACHINE_H
#define KARTONMACHINE_H

#include <QObject>
#include "kartonerror.h"

class QProcess;

class KartonMachine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString machineName READ machineName WRITE setMachineName NOTIFY machineNameChanged)
    Q_PROPERTY(QString machineOs READ machineOs WRITE setMachineOs NOTIFY machineOsChanged)
    Q_PROPERTY(QString cdPath READ cdPath WRITE setCdPath NOTIFY cdPathChanged)
    Q_PROPERTY(quint8 machineCores READ machineCores WRITE setMachineCores NOTIFY machineCoresChanged)
    Q_PROPERTY(quint32 machineRam READ machineRam WRITE setMachineRam NOTIFY machineRamChanged)
    Q_PROPERTY(QString machinePath READ machinePath WRITE setMachinePath NOTIFY machinePathChanged)
    Q_PROPERTY(QString firmware READ firmware WRITE setFirmware NOTIFY firmwareChanged)
    Q_PROPERTY(bool isValid READ isValid WRITE setIsValid NOTIFY isValidChanged)
    Q_PROPERTY(bool vmRunning READ vmRunning NOTIFY vmRunningChanged)
    Q_PROPERTY(KartonError::VmError vmErr READ vmErr NOTIFY vmErrChanged)

public:
    explicit KartonMachine(QString machinePath, QObject *parent = nullptr);
    explicit KartonMachine(KartonMachine &val);
    virtual ~KartonMachine();
    QString machineName();
    void setMachineName(QString val);
    QString machineOs();
    void setMachineOs(QString val);
    QString cdPath();
    void setCdPath(QString val);
    quint32 machineCores();
    void setMachineCores(quint32 val);
    quint32 machineRam();
    void setMachineRam(quint32 val);
    QString machinePath();
    void setMachinePath(QString val);
    QString firmware();
    void setFirmware(QString val);
    bool isValid();
    void setIsValid(bool val);
    bool vmRunning();
    KartonError::VmError vmErr();

    void loadConfig();
    void saveConfig();
    void createDisk(quint32 size);
    void terminate();
    void launchVm(bool fromCd);
    void deleteVm();

Q_SIGNALS:
    void machineNameChanged();
    void machineOsChanged();
    void cdPathChanged();
    void machineCoresChanged();
    void machineRamChanged();
    void machinePathChanged();
    void firmwareChanged();
    void isValidChanged();
    void vmRunningChanged();
    void vmErrChanged();

public Q_SLOTS:
    void vmProcessEnded();

private:
    QString m_machineName;
    QString m_machineOs;
    QString m_cdPath;
    quint32 m_machineCores;
    quint32 m_machineRam;
    QString m_machinePath;
    QString m_firmware;
    bool m_isValid;
    KartonError::VmError m_vmErr;

    QProcess *m_qemuProcess;
    bool m_vmRunning;

    QString m_configName = QStringLiteral("/karton.cfg");
    QString m_diskName = QStringLiteral("/disk.qcow2");

    void createMachine();
    void setVmRunning(bool val);
    void setVmErr(KartonError::VmError val);
};

#endif // KARTONMACHINE_H
