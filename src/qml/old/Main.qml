// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as Delegates
import backendengine 1.0
import kartonerror 1.0

Kirigami.ApplicationWindow {
    id: root
    title: "Karton Virtual Machine Manager"

    width: Kirigami.Units.gridUnit * 40
    height: Kirigami.Units.gridUnit * 30
    minimumWidth: Kirigami.Units.gridUnit * 40
    minimumHeight: Kirigami.Units.gridUnit * 30
    pageStack.defaultColumnWidth: Kirigami.Units.gridUnit * 10


    BackendEngine {
        id: iface

        onSelectedIsoFileChanged: {
            cdField.text = selectedIsoFile;
        }
    }

    ListModel {
        id: vmListModel
    }

    Component {
        id: vmListDelegate
        Kirigami.AbstractCard {
            contentItem: Item {
                implicitWidth: vmDashboardPage.implicitWidth
                implicitHeight: vmItemLayout.implicitHeight

                GridLayout {
                    id: vmItemLayout
                    Layout.fillWidth: true
                    columns: 2
                    Kirigami.Icon {
                        Layout.margins: Kirigami.Units.gridUnit * 0.5
                        source: 'karton-' + model.os.toLowerCase()
                    }

                    Kirigami.Heading {
                        text: model.name
                    }

                    Item {
                        // TODO: intentionally blank, figure out how to not need this
                    }

                    GridLayout {
                        columns: 2
                        Layout.fillWidth: true

                        Kirigami.Heading {
                            Layout.alignment: Qt.AlignRight
                            Layout.rightMargin: Kirigami.Units.gridUnit * 0.5
                            text: 'CPUs:'
                            level: 2
                        }
                        Kirigami.Heading {
                            Layout.fillWidth: true
                            Layout.preferredWidth: root.width - Kirigami.Units.gridUnit * 12
                            elide: Text.ElideRight
                            text: model.cpus
                            level: 2
                        }
                        Kirigami.Heading {
                            Layout.alignment: Qt.AlignRight
                            Layout.rightMargin: Kirigami.Units.gridUnit * 0.5
                            text: 'RAM:'
                            level: 2
                        }
                        Kirigami.Heading {
                            Layout.fillWidth: true
                            Layout.preferredWidth: root.width - Kirigami.Units.gridUnit * 12
                            elide: Text.ElideRight
                            text: model.ram + ' MiB'
                            level: 2
                        }
                        Kirigami.Heading {
                            Layout.alignment: Qt.AlignRight
                            Layout.rightMargin: Kirigami.Units.gridUnit * 0.5
                            text: 'Location:'
                            level: 2
                        }
                        Kirigami.Heading {
                            Layout.fillWidth: true
                            Layout.preferredWidth: root.width - Kirigami.Units.gridUnit * 12
                            elide: Text.ElideRight
                            text: model.machinePath
                            level: 2
                        }
                        Kirigami.Heading {
                            Layout.alignment: Qt.AlignRight
                            Layout.rightMargin: Kirigami.Units.gridUnit * 0.5
                            text: 'CD location:'
                            level: 2
                        }
                        Kirigami.Heading {
                            Layout.fillWidth: true
                            Layout.preferredWidth: root.width - Kirigami.Units.gridUnit * 12
                            elide: Text.ElideRight
                            text: model.cdPath
                            level: 2
                        }
                        RowLayout {
                            Layout.columnSpan: 2
                            Layout.topMargin: Kirigami.Units.gridUnit * 0.5
                            Layout.bottomMargin: Kirigami.Units.gridUnit * 0.5

                            Controls.Button {
                                Layout.rightMargin: Kirigami.Units.gridUnit * 0.5
                                Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                                text: 'Launch';
                                icon.name: 'media-playback-start'
                                onClicked: {
                                    launchVm(model.vmIdx, false)
                                }
                            }
                            Controls.Button {
                                Layout.rightMargin: Kirigami.Units.gridUnit * 0.5
                                Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                                text: 'Boot from CD';
                                icon.name: 'media-optical'
                                onClicked: {
                                    launchVm(model.vmIdx, true)
                                }
                            }
                            Controls.Button {
                                Layout.rightMargin: Kirigami.Units.gridUnit * 0.5
                                Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                                text: 'Delete'
                                icon.name: 'edit-delete'
                                onClicked: {
                                    deleteVm(model.vmIdx);
                                }
                            }
                            Controls.Button {
                                Layout.rightMargin: Kirigami.Units.gridUnit * 0.5
                                Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                                text: 'Configure'
                                icon.name: 'settings-configure'
                                onClicked: {
                                    configVmDialog.vmIdx = model.vmIdx;
                                    configVmDialog.setConfigValues(model.firmware, model.os, model.cpus, model.ram, 0, model.name, model.cdPath);
                                    configVmDialog.open();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Kirigami.ScrollablePage {
        id: vmDashboardPage
        title: 'VM Dashboard'

        actions: [
            Kirigami.Action {
                id: createAction
                icon.name: 'list-add'
                text: 'Create'
                onTriggered: {
                    createVmDialog.open();
                }
            }
        ]

        Kirigami.Dialog {
            id: errorDialog
            width: Kirigami.Units.gridUnit * 20
            height: Kirigami.Units.gridUnit * 10
            title: 'Error'

            Item {
                anchors.fill: parent
                RowLayout {
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        topMargin: Kirigami.Units.gridUnit * 0.5
                        leftMargin: Kirigami.Units.gridUnit * 0.5
                        rightMargin: Kirigami.Units.gridUnit * 0.5
                    }

                    Kirigami.Icon {
                        source: 'dialog-warning'
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignTop
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.gridUnit * 0.5
                        id: errorDialogLabel
                        wrapMode: Text.WordWrap
                    }
                }

                Controls.Button {
                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                        rightMargin: Kirigami.Units.gridUnit * 0.5
                        bottomMargin: Kirigami.Units.gridUnit * 0.5
                    }

                    text: 'OK'
                    onClicked: {
                        errorDialog.close();
                    }
                }
            }
        }

        VmConfigDialog {
            id: createVmDialog
            title: 'Create New VM'

            onSaveClicked: {
                createVm(firmware, os, cpus, ram, diskSize, name, cdLocation);
            }
        }

        VmConfigDialog {
            id: configVmDialog
            title: 'Configure VM'
            showDiskSize: false

            onSaveClicked: {
                configVm(vmIdx, firmware, os, cpus, ram, name, cdLocation);
            }
        }

        Kirigami.CardsListView {
            id: vmList
            model: vmListModel
            delegate: vmListDelegate
        }
    }

    function loadVmList(fullReload) {
        if (fullReload) {
            iface.loadVmList();
        }

        vmListModel.clear();

        for(let i = 0; i < iface.getMachineCount(); i++) {
            let machineData = iface.getMachineData(i);

            vmListModel.append(
                {
                    name: machineData[0],
                    os: machineData[1],
                    ram: machineData[2],
                    cpus: machineData[3],
                    machinePath: machineData[4],
                    cdPath: machineData[5] === '' ? 'None' : machineData[5],
                    firmware: machineData[6],
                    vmIdx: i
                }
            );
        }
    }

    function createVm(firmware, os, cpus, ram, diskSize, name, cdLoc) {
        if (name === '') {
            errorDialogLabel.text = 'Cannot create a VM with no name!';
            errorDialog.open();
            return;
        }

        if (iface.createVm(firmware, os, cpus, ram, diskSize, name, cdLoc)) {
            errorDialogLabel.text = '';
            loadVmList(false);
        } else {
            errorDialogLabel.text = 'VM creation failed! Check the file permissions on the VM repo at ' + iface.vmRepoPath + '.';
            errorDialog.open();
        }
    }

    function configVm(vmIdx, firmware, os, cpus, ram, name, cdLoc) {
        if (name === '') {
            errorDialogLabel.text = 'Cannot set an empty VM name!';
            errorDialog.open();
            return;
        }

        if (iface.configVm(vmIdx, firmware, os, cpus, ram, name, cdLoc)) {
            errorDialogLabel.text = '';
            loadVmList(false);
        } else {
            errorDialogLabel.text = 'VM configuration failed! Check the file permissions on the VM repo at ' + iface.vmRepoPath + '.';
            errorDialog.open();
        }
    }

    function launchVm(idx, withCd) {
        iface.launchVm(idx, withCd);
        switch (iface.vmErr(idx)) {
        case KartonError.None:
            errorDialogLabel.text = '';
            return;
        case KartonError.LaunchFailed:
            errorDialogLabel.text = 'The VM process failed to launch!';
            break;
        case KartonError.LaunchNoDisk:
            errorDialogLabel.text = 'The VM has no disk image!';
            break;
        case KartonError.LaunchNoCd:
            errorDialogLabel.text = 'The VM has no CD specified, cannot boot from CD!';
            break;
        case KartonError.LaunchCdMissing:
            errorDialogLabel.text = 'The specified CD for this VM does not exist!'
        }
        errorDialog.open();
    }

    function deleteVm(idx) {
        iface.deleteVm(idx);
        switch(iface.vmErr(idx)) {
        case KartonError.None:
            errorDialogLabel.text = '';
            loadVmList(true);
            return;
        case KartonError.DeleteFailed:
            errorDialogLabel.text = 'The VM delete operation failed!';
            break;
        case KartonError.DeleteRunningVm:
            errorDialogLabel.text = 'Cannot delete a running VM!';
            break;
        }
        errorDialog.open();
    }

    Component.onCompleted: {
        loadVmList(true);
        pageStack.push(vmDashboardPage);
    }
}
