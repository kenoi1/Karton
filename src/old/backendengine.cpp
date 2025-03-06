// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

#include "backendengine.h"

#include <QDir>
#include <QFileDialog>

BackendEngine::BackendEngine(QObject *parent)
    : QObject{parent}
{

}

QList<KartonMachine *> *BackendEngine::machineList() {
    return m_machineList;
}
void BackendEngine::setMachineList(QList<KartonMachine *> *val) {
    m_machineList = val;
}
QString BackendEngine::vmRepoPath() {
    return m_vmRepoPath;
}
QStringList BackendEngine::osList() {
    return m_osList;
}
QStringList BackendEngine::firmwareList() {
    return m_firmwareList;
}
void BackendEngine::setOsList(QStringList val) {
    m_osList = val;
}
void BackendEngine::setFirmwareList(QStringList val) {
    m_firmwareList = val;
}

void BackendEngine::loadVmList() {
    for (int i = 0;i < m_machineList->count();i++) {
        m_machineList->at(i)->deleteLater();
    }
    m_machineList->clear();

    QDir vmRepo(m_vmRepoPath);
    QStringList vmRepoDirs = vmRepo.entryList(QDir::Dirs);
    for(QString vmDirStr : std::as_const(vmRepoDirs)) {
        if (vmDirStr == QStringLiteral("..") || vmDirStr == QStringLiteral(".")) {
            continue;
        }
        QDir vmDir(m_vmRepoPath + QStringLiteral("/") + vmDirStr);
        KartonMachine *vm = new KartonMachine(vmDir.absolutePath());
        if (vm->isValid()) {
            m_machineList->append(vm);
        } else {
            delete vm;
        }
    }
}

quint32 BackendEngine::getMachineCount() {
    return m_machineList->count();
}

QStringList BackendEngine::getMachineData(quint32 idx) {
    KartonMachine *selectedMachine;
    if (idx < m_machineList->count()) {
        selectedMachine = m_machineList->at(idx);
    } else {
        return QStringList();
    }

    return QStringList({
        selectedMachine->machineName(),
        selectedMachine->machineOs(),
        QString::number(selectedMachine->machineRam()),
        QString::number(selectedMachine->machineCores()),
        selectedMachine->machinePath(),
        selectedMachine->cdPath(),
        selectedMachine->firmware()
    });
}

bool BackendEngine::createVm(QString firmware, QString os, quint32 cpus, quint32 ram, quint32 diskSize, QString name, QString cdLoc) {
    if (firmware == QString() || os == QString() || cpus == 0 || ram == 0 || diskSize == 0 || name == QString()) {
        return false;
    }

    KartonMachine *newVm = new KartonMachine(m_vmRepoPath + QStringLiteral("/") + name);
    if (!newVm->isValid()) {
        delete newVm;
        return false;
    }

    newVm->setMachineCores(cpus);
    newVm->setMachineRam(ram);
    newVm->setMachineName(name);
    newVm->setCdPath(cdLoc);
    newVm->setMachineOs(os);
    newVm->createDisk(diskSize);
    newVm->setFirmware(firmware);
    newVm->saveConfig();

    m_machineList->append(newVm);
    return true;
}

bool BackendEngine::configVm(quint32 idx, QString firmware, QString os, quint32 cpus, quint32 ram, QString name, QString cdLoc) {
    if (idx >= m_machineList->count() || os == QString() || cpus == 0 || ram == 0 || name == QString()) {
        return false;
    }

    KartonMachine *targetVm = m_machineList->at(idx);
    if (!targetVm->isValid()) {
        return false;
    }

    targetVm->setMachineCores(cpus);
    targetVm->setMachineRam(ram);
    targetVm->setMachineName(name);
    targetVm->setCdPath(cdLoc);
    targetVm->setMachineOs(os);
    targetVm->setFirmware(firmware);
    targetVm->saveConfig();

    return true;
}

void BackendEngine::launchVm(quint32 idx, bool fromCd) {
    m_machineList->at(idx)->launchVm(fromCd);
}

void BackendEngine::deleteVm(quint32 idx) {
    m_machineList->at(idx)->deleteVm();
}

QString BackendEngine::selectedIsoFile() {
    return m_selectedIsoFile;
}

void BackendEngine::selectIsoFile() {
    m_selectedIsoFile = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Select ISO File"));
    selectedIsoFileChanged();
}

KartonError::VmError BackendEngine::vmErr(quint32 idx) {
    return m_machineList->at(idx)->vmErr();
}
