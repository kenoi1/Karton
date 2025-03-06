// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import backendengine 1.0

Kirigami.Dialog {
    property string os: osComboBox.currentText
    property int cpus: cpuSpinBox.value
    property int ram: ramSpinBox.value
    property int diskSize: diskSpinBox.value
    property string name: nameField.text
    property string cdLocation: cdField.text
    property string firmware: firmwareComboBox.currentText
    property bool showDiskSize: true
    property int vmIdx: 0

    signal saveClicked()
    signal cancelClicked()

    width: Kirigami.Units.gridUnit * 30
    height: Kirigami.Units.gridUnit * 25

    BackendEngine {
        id: iface

        onSelectedIsoFileChanged: {
            cdField.text = selectedIsoFile;
        }
    }

    ListModel {
        id: osListModel
    }

    ListModel {
        id: firmwareListModel
    }

    Item {
        anchors.fill: parent
        GridLayout {
            anchors {
                left: parent.left
                leftMargin: Kirigami.Units.gridUnit
                right: parent.right
                rightMargin: Kirigami.Units.gridUnit
                top: parent.top
                topMargin: Kirigami.Units.gridUnit
            }

            columns: 2

            Kirigami.Heading {
                Layout.alignment: Qt.AlignRight
                text: 'OS:'
                level: 2
            }

            Controls.ComboBox {
                id: osComboBox
                Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                Layout.bottomMargin: Kirigami.Units.gridUnit * 0.5
                model: osListModel
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignRight
                text: 'CPUs:'
                level: 2
            }

            Controls.SpinBox {
                id: cpuSpinBox
                Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                Layout.bottomMargin: Kirigami.Units.gridUnit * 0.5
                editable: true
                from: 1
                to: 64
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignRight
                text: 'RAM (MiB):'
                level: 2
            }

            Controls.SpinBox {
                id: ramSpinBox
                Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                Layout.bottomMargin: Kirigami.Units.gridUnit * 0.5
                editable: true
                from: 1
                to: 262144
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignRight
                text: 'Disk size (GiB):'
                visible: showDiskSize
                level: 2
            }

            Controls.SpinBox {
                id: diskSpinBox
                Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                Layout.bottomMargin: Kirigami.Units.gridUnit * 0.5
                editable: true
                visible: showDiskSize
                from: 1
                to: 32768
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignRight
                text: 'Firmware:'
                level: 2
            }

            Controls.ComboBox {
                id: firmwareComboBox
                Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                Layout.bottomMargin: Kirigami.Units.gridUnit * 0.5
                model: firmwareListModel
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignRight
                text: 'Name:'
                level: 2
            }

            Controls.TextField {
                id: nameField
                Layout.bottomMargin: Kirigami.Units.gridUnit * 0.5
                Layout.fillWidth: true
            }

            Kirigami.Heading {
                text: 'CD location:'
                Layout.alignment: Qt.AlignRight
                level: 2
            }

            RowLayout {
                Controls.TextField {
                    id: cdField
                    Layout.fillWidth: true
                }

                Controls.Button {
                    Layout.leftMargin: Kirigami.Units.gridUnit * 0.5
                    text: 'Browse'
                    icon.name: 'document-open-symbolic'
                    onClicked: {
                        iface.selectIsoFile();
                    }
                }
            }
        }

        RowLayout {
            // anchors {
            //     right: parent.right
            //     rightMargin: Kirigami.Units.gridUnit
            //     bottom: parent.bottom
            //     bottomMargin: Kirigami.Units.gridUnit
            // }

            Controls.Button {
                id: saveButton
                text: 'Save'
                icon.name: 'document-save-symbolic'
                onClicked: {
                    saveClicked();
                    closeDialog();
                }
            }

            Controls.Button {
                id: cancelButton
                Layout.leftMargin: Kirigami.Units.gridUnit * 0.5
                text: 'Cancel'
                icon.name: 'cancel'
                onClicked: {
                    closeDialog();
                    cancelClicked();
                }
            }
        }
    }

    function setConfigValues(firmware, os, cpus, ram, diskSize, name, cdLocation) {
        let i = 0;
        for (i = 0;i < firmwareListModel.count;i++) {
            if (firmwareListModel.get(i).text === firmware) {
                firmwareComboBox.currentIndex = i;
                break;
            }
        }
        for (i = 0;i < osListModel.count;i++) {
            if (osListModel.get(i).text === os) {
                osComboBox.currentIndex = i;
                break;
            }
        }
        cpuSpinBox.value = cpus;
        ramSpinBox.value = ram;
        diskSpinBox.value = diskSize;
        nameField.text = name;
        cdField.text = cdLocation;
    }

    function closeDialog() {
        cpuSpinBox.value = 0;
        ramSpinBox.value = 0;
        diskSpinBox.value = 0;
        osComboBox.currentIndex = 0;
        firmwareComboBox.currentIndex = 0;
        nameField.text = '';
        cdField.text = '';
        this.close();
    }

    Component.onCompleted: {
        let i = 0;
        for (i = 0;i < iface.osList.length;i++) {
            osListModel.append({
                text: iface.osList[i]
            });
        }
        for (i = 0;i < iface.firmwareList.length;i++) {
            firmwareListModel.append({
                text: iface.firmwareList[i]
            });
        }

        osComboBox.currentIndex = 0;
        firmwareComboBox.currentIndex = 0;
    }
}
