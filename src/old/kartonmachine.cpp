// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

#include <QProcess>
#include <QStringList>
#include <QFile>
#include <QDir>

#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "kartonmachine.h"

/*
 * TODO: Add validity checking - no function should do ANYTHING if m_isValid = false
 */

KartonMachine::KartonMachine(QString machinePath, QObject *parent)
    : QObject{parent}
{
    m_machinePath = machinePath;
    if (!QFile(m_machinePath + m_configName).exists()) {
        if (!QDir(m_machinePath).exists()) {
            createMachine();
        } else {
            qCritical() << "Specified path is not a Karton virtual machine!";
            return;
        }
    } else {
        loadConfig();
    }
    m_vmErr = KartonError::None;
    m_vmRunning = false;
    m_isValid = true;
}

KartonMachine::~KartonMachine()
{
    terminate();
}

QString KartonMachine::machineName() {
    return m_machineName;
}
void KartonMachine::setMachineName(QString val) {
    m_machineName = val;
    machineNameChanged();
}
QString KartonMachine::machineOs() {
    return m_machineOs;
}
void KartonMachine::setMachineOs(QString val) {
    m_machineOs = val;
    machineOsChanged();
}
QString KartonMachine::cdPath() {
    return m_cdPath;
}
void KartonMachine::setCdPath(QString val) {
    m_cdPath = val;
    cdPathChanged();
}
quint32 KartonMachine::machineCores() {
    return m_machineCores;
}
void KartonMachine::setMachineCores(quint32 val) {
    m_machineCores = val;
    machineCoresChanged();
}
quint32 KartonMachine::machineRam() {
    return m_machineRam;
}
void KartonMachine::setMachineRam(quint32 val) {
    m_machineRam = val;
    machineRamChanged();
}
QString KartonMachine::machinePath() {
    return m_machinePath;
}
void KartonMachine::setMachinePath(QString val) {
    m_machinePath = val;
    machinePathChanged();
}
QString KartonMachine::firmware() {
    return m_firmware;
}
void KartonMachine::setFirmware(QString val) {
    m_firmware = val;
    firmwareChanged();
}
bool KartonMachine::isValid() {
    return m_isValid;
}
void KartonMachine::setIsValid(bool val) {
    m_isValid = val;
    isValidChanged();
}
bool KartonMachine::vmRunning() {
    return m_vmRunning;
}
KartonError::VmError KartonMachine::vmErr() {
    return m_vmErr;
}

void KartonMachine::loadConfig() {
    // TODO: Figure out how to implement error checking here, I don't know how KConfig reports
    // errors yet
    QString configPath = m_machinePath + m_configName;
    KSharedConfigPtr vmConfig = KSharedConfig::openConfig(configPath, KConfig::SimpleConfig);
    KConfigGroup rootGroup = vmConfig->group(QString());
    setMachineName(rootGroup.readEntry(QStringLiteral("machineName")));
    setMachineOs(rootGroup.readEntry(QStringLiteral("machineOs")));
    setCdPath(rootGroup.readEntry(QStringLiteral("cdPath")));
    setFirmware(rootGroup.readEntry(QStringLiteral("firmware")));
    QVariant machineCoresTmpV = rootGroup.readEntry(QStringLiteral("machineCores"));
    quint32 machineCoresTmp = machineCoresTmpV.toUInt();
    if (machineCoresTmp < 256 && machineCoresTmp != 0) {
            setMachineCores(machineCoresTmp);
    } else {
        qCritical() << QStringLiteral("Invalid core count specified for VM ") + m_machineName;
        setMachineCores(1);
    }
    QVariant machineRamTmpV = rootGroup.readEntry(QStringLiteral("machineRam"));
    quint32 machineRamTmp = machineRamTmpV.toUInt();
    if (machineRamTmp != 0) {
        setMachineRam(machineRamTmp);
    } else {
        qCritical() << QStringLiteral("Invalid RAM size specified for VM ") + m_machineName;
        setMachineRam(1024);
    }
}

void KartonMachine::saveConfig() {
    // TODO: Figure out how to implement error checking here, I don't know how KConfig reports
    // errors yet
    QString configPath = m_machinePath + m_configName;
    KSharedConfigPtr vmConfig = KSharedConfig::openConfig(configPath, KConfig::SimpleConfig);
    KConfigGroup rootGroup = vmConfig->group(QString());
    rootGroup.writeEntry(QStringLiteral("machineName"), m_machineName);
    rootGroup.writeEntry(QStringLiteral("machineOs"), m_machineOs);
    rootGroup.writeEntry(QStringLiteral("cdPath"), m_cdPath);
    rootGroup.writeEntry(QStringLiteral("machineCores"), m_machineCores);
    rootGroup.writeEntry(QStringLiteral("machineRam"), m_machineRam);
    rootGroup.writeEntry(QStringLiteral("firmware"), m_firmware);
    vmConfig->sync();
}

void KartonMachine::terminate() {
    if (!m_vmRunning) {
        return; // nothing to terminate
    }
    if (m_qemuProcess->state() == QProcess::Running) {
        m_qemuProcess->terminate();
        m_qemuProcess->waitForFinished(5000);
        if (m_qemuProcess->state() == QProcess::Running) {
            m_qemuProcess->kill();
            m_qemuProcess->waitForFinished(10000);
            if (m_qemuProcess->state() == QProcess::Running) {
                // Something has gone severely wrong and QEMU is stuck on a kernel thread.
                // Give up.
                qCritical() << QStringLiteral("Could not terminate VM ") + m_machineName;
                setIsValid(false);
            } else {
                m_qemuProcess->deleteLater();
                setVmRunning(false);
            }
        } else {
            m_qemuProcess->deleteLater();
            setVmRunning(false);
        }
    }
}

void KartonMachine::createDisk(quint32 size) {
    QFile diskFile(m_machinePath + m_diskName);
    if (diskFile.exists()) {
        diskFile.remove();
    }

    QStringList qemuImgMap = {
        // qemu-img create -f qcow2 disk.qcow2 sizeG
        QStringLiteral("create"),
        QStringLiteral("-f"),
        QStringLiteral("qcow2"),
        m_machinePath + m_diskName,
        QString::number(size) + QStringLiteral("M")
    };
    QProcess qemuImgProc;
    qemuImgProc.setProgram(QStringLiteral("/usr/bin/qemu-img"));
    qemuImgProc.setArguments(qemuImgMap);
    qemuImgProc.start();
    qemuImgProc.waitForFinished(-1);
    if (qemuImgProc.exitCode() != 0) {
        setIsValid(false);
    }
}

void KartonMachine::launchVm(bool fromCd) {
    if (m_vmRunning) {
        return; // can't start an already launched VM
    }

    if (!QFile(m_machinePath + m_diskName).exists()) {
        setVmErr(KartonError::LaunchNoDisk);
        return;
    }

    QStringList vmSettingsMap = {
        // Make it fast
        QStringLiteral("-enable-kvm"),
        // Use remotely modern hardware
        QStringLiteral("-machine"), QStringLiteral("q35"),
        // Enable audio
        QStringLiteral("-device"), QStringLiteral("intel-hda"),
        QStringLiteral("-device"), QStringLiteral("hda-duplex"),
        // Use not-quite-so-painful graphics
        QStringLiteral("-vga"), QStringLiteral("qxl"),
        // Enable USB since we don't live in 1995 anymore
        QStringLiteral("-usb"),
        // Make the VM recognizable
        QStringLiteral("-name"), m_machineName,
        // Don't let the user accidentally close it like I keep doing
        QStringLiteral("-display"), QStringLiteral("gtk,window-close=off"),
        // Disk drives are important
        QStringLiteral("-drive"), QStringLiteral("if=virtio,format=qcow2,file=") + m_machinePath + m_diskName,
        // CPU cores matter too
        QStringLiteral("-smp"), QString::number(m_machineCores),
        // As does RAM size
        QStringLiteral("-m"), QString::number(m_machineRam) + QStringLiteral("M"),
        // Pass through all host CPU features to the VM
        QStringLiteral("-cpu"), QStringLiteral("host")
    };

    if (m_firmware == QStringLiteral("UEFI")) {
        // Enable UEFI
        vmSettingsMap << QStringLiteral("-bios") << QStringLiteral("/usr/share/ovmf/OVMF.fd");
    }

    if (m_cdPath != QString()) {
        if (!QFile(m_cdPath).exists()) {
            setVmErr(KartonError::LaunchCdMissing);
            return;
        }

        // An ISO file has been provided, load it
        vmSettingsMap << QStringLiteral("-cdrom") << m_cdPath;
    }

    if (fromCd) {
        if (m_cdPath == QString()) {
            setVmErr(KartonError::LaunchNoCd);
            return;
        }
        // The user wishes to install an OS into the VM, change the boot order
        vmSettingsMap << QStringLiteral("-boot") << QStringLiteral("dc");
    }

    // Finally, launch the VM
    m_qemuProcess = new QProcess();
    m_qemuProcess->setProgram(QStringLiteral("/usr/bin/qemu-system-x86_64"));
    m_qemuProcess->setArguments(vmSettingsMap);
    m_qemuProcess->start();
    m_qemuProcess->waitForStarted();
    if (m_qemuProcess->state() != QProcess::Running) {
        setIsValid(false);
        setVmErr(KartonError::LaunchFailed);
    }
    setVmRunning(true);
    connect(m_qemuProcess, SIGNAL(finished(int)), this, SLOT(vmProcessEnded()));
    setVmErr(KartonError::None);
}

void KartonMachine::deleteVm() {
    if (m_vmRunning) {
        m_vmErr = KartonError::DeleteRunningVm;
        return;
    }

    QDir vmDir(m_machinePath);
    if (!vmDir.removeRecursively()) {
        setVmErr(KartonError::DeleteFailed);
    } else {
        setVmErr(KartonError::None);
    }
    setIsValid(false);
}

void KartonMachine::vmProcessEnded() {
    if (m_qemuProcess->exitCode() != 0) {
        setIsValid(false);
    }
    setVmRunning(false);
    m_qemuProcess->deleteLater();
}

void KartonMachine::createMachine() {
    QDir().mkdir(m_machinePath);

    // Touch the config file so that this is recognizable as a Karton VM
    QFile configFile(m_machinePath + m_configName);
    bool createSucceeded = configFile.open(QFile::WriteOnly);
    if (!createSucceeded) {
        setIsValid(false);
    }
    configFile.close();
}

void KartonMachine::setVmRunning(bool val) {
    m_vmRunning = val;
    vmRunningChanged();
}
void KartonMachine::setVmErr(KartonError::VmError val) {
    m_vmErr = val;
    vmErrChanged();
}
