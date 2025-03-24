// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs as Dialogs
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kirigamiaddons.components 1.0 as Components

Kirigami.ScrollablePage {
    title: "Karton Virtual Machine Manager"
    actions: [
        Kirigami.Action {
            icon.name: "list-add-symbolic"
            text: "Add"
            onTriggered: source => {
                addDomainDialog.open();
            }
        }
    ]
    function createDomainWrapper(config) {
    console.log("Creating VM with configuration:", JSON.stringify(config));
    Karton.createDomain(config.name, 
                        config.osVariant,
                        config.memoryGB, 
                        config.storageGB, 
                        config.diskImage,
                        config.cpu
                        );
    }
    Kirigami.Dialog {
        id: addDomainDialog
        title: "Add New Virtual Machine"
        padding: Kirigami.Units.largeSpacing
        modal: true

        customFooterActions: [
                Kirigami.Action {
                    text: i18n("Create")
                    icon.name: "dialog-ok"
                    onTriggered: {
                        const domainConfig = {
                            name: nameField.text.trim(),
                            osVariant: osField.text.trim(),
                            diskImage: diskImageField.text,
                            memoryGB: memorySpinBox.value,
                            storageGB: storageSpinBox.value,
                            cpu: cpuSpinBox.value
                        };
                        createDomainWrapper(domainConfig);
                        showPassiveNotification("Created VM: " + nameField.text);
                        addDomainDialog.close();
                    }
                }
        ]
        
        preferredWidth: root.width - Kirigami.Units.gridUnit * 10
        preferredHeight: root.height - Kirigami.Units.gridUnit * 10
        onAccepted: {
            console.log("VM Name:", nameField.text);
            console.log("VM Type:", vmTypeComboBox.currentText);
            showPassiveNotification("Created VM: " + nameField.text);
        }
            

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            FormCard.FormCard {
                Layout.fillWidth: true

                FormCard.FormTextFieldDelegate {
                    id: nameField
                    label: i18nc("@label:textbox Enter VM Name", "VM Name:")
                    placeholderText: "Enter VM Name"
                    Layout.fillWidth: true
                }
                FormCard.FormTextFieldDelegate {
                    id: osField
                    label: i18nc("@label:textbox OS Variant", "OS Variant:")
                    placeholderText: "Enter an OS Variant"
                    Layout.fillWidth: true
                }
                
                Dialogs.FileDialog {
                    id: fileDialog
                    title: "Choose a disk image"
                    nameFilters: ["Disk images (*.qcow2 *.raw *.img *.iso *.vdi *.vmdk)"]
                      onAccepted: {
                    diskImageField.text = fileDialog.selectedFile.toString().replace("file://", "")
                    }
                }
                FormCard.FormDelegateSeparator {}
                FormCard.AbstractFormDelegate {
                    background: null
                    contentItem: RowLayout {
                        Layout.fillWidth: true
                        
                        Controls.TextField {
                            id: diskImageField
                            Layout.fillWidth: true
                            placeholderText: "Select a disk image"
                            readOnly: true
                        }
                        
                        Controls.Button {
                            text: "Browse"
                            onClicked: {
                                fileDialog.open();

                            }
                        }
                    }
                }
            }

            FormCard.FormCard {
                Layout.bottomMargin: Kirigami.Units.largeSpacing
                FormCard.FormSpinBoxDelegate {
                    id: memorySpinBox
                    label: "Memory (GB)"
                    value: 4
                    from: 1
                    to: 64

                    Layout.fillWidth: true
                }

                FormCard.FormDelegateSeparator {}

                FormCard.FormSpinBoxDelegate {
                    id: storageSpinBox
                    label: "Disk Storage (GB)"
                    from: 1
                    to: 2048
                    value: 4


                    Layout.fillWidth: true
                }

                FormCard.FormDelegateSeparator {}

                FormCard.FormSpinBoxDelegate {
                    id: cpuSpinBox
                    label: "CPUs"
                    from: 1
                    to: 16
                    value: 2


                    Layout.fillWidth: true
                }
            }
        }
        
    }
    Kirigami.CardsListView {
        id: view
        model: VMModel

        delegate: Kirigami.AbstractCard {
            contentItem: Item {
                implicitWidth: delegateLayout.implicitWidth
                implicitHeight: delegateLayout.implicitHeight
                RowLayout {
                    id: delegateLayout
                    anchors {
                        left: parent.left
                        top: parent.top
                        right: parent.right
                    }
                    // rowSpacing: Kirigami.Units.largeSpacing
                    // columnSpacing: Kirigami.Units.largeSpacing
                    // columns: width > Kirigami.Units.gridUnit * 20 ? 4 : 2
                    Kirigami.Icon {
                        source: "computer-symbolic" 
                        // TODO: Add OS Icon -> eventually, have screencap of VM window
                        Layout.fillHeight: true
                        Layout.maximumHeight: Kirigami.Units.iconSizes.huge
                        Layout.preferredWidth: height
                    }
                    ColumnLayout {
                        Kirigami.Heading {
                            level: 2
                            text: domain.name
                        }
                        Kirigami.Separator {
                            Layout.fillWidth: true
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "UUID: " + domain.uuid
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "State: " + domain.state
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "Memory (GB): " + domain.maxRam
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "Memory Usage (GB): " + domain.ramUsage
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "CPU Cores: " + domain.cpus
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "Disk: " + domain.diskPath
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "Autostart: " + (model.autostart ? "Enabled" : "Disabled")
                        }

                    }
                    ColumnLayout{
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: "Start"
                            icon.name: "media-playback-start"
                            onClicked: {
                                Karton.startDomain(domain)
                                showPassiveNotification("Starting VM: " + domain.name + "!");
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: "Stop"
                            icon.name: "media-playback-pause"
                            onClicked: {
                                Karton.stopDomain(domain)
                                showPassiveNotification("Stopping VM: " + domain.name + "!");
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: "Force Stop"
                            // icon.name: "process-stop"
                            onClicked: {
                                Karton.forceStopDomain(domain)
                                showPassiveNotification("Force-stopping VM: " + domain.name + "!");
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: "View VM"
                            icon.name: "computer-laptop-symbolic"
                            onClicked: {
                                Karton.viewDomain(domain)
                                showPassiveNotification("Opening in virt-viewer: " + domain.name + "!");
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: "Delete"
                            icon.name: "delete"
                            onClicked: {
                                Karton.undefineDomain(domain)
                                showPassiveNotification("Deleting " + domain.name + "!");
                            }
                        }
                    }
                }
            }
        }
        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)

            visible: view.count === 0

            text: "Welcome to Karton!"
            explanation: "Create a new virtual machine to proceed."
            icon.name: "computer-symbolic"
        }
    }

    
}
