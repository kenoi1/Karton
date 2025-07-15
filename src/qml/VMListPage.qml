// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs as Dialogs
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kirigamiaddons.components 1.0 as Components
import org.kde.karton


Kirigami.ScrollablePage {
    title: i18nc("noun, title of listpage","Karton Virtual Machine Manager")
    Component.onCompleted: {
        Karton.errorOccurred.connect(function(errorMessage) {
            showPassiveNotification(errorMessage, "long");
        });
    }
    actions: [
        Kirigami.Action {
            icon.name: "list-add-symbolic"
            text: i18nc("verb, to add a new virtual machine","Add")
            onTriggered: source => {
                addDomainDialog.open();
            }
        }
    ]

    function getShortOsId(path) {
        return OsinfoConfig.getShortIdFromId(OsinfoConfig.getOsIdFromDisk(path));
    }
  
    Kirigami.Dialog {
        id: addDomainDialog
        title: i18n("Add New Virtual Machine")
        padding: Kirigami.Units.largeSpacing
        modal: true

        customFooterActions: [
                Kirigami.Action {
                    text: i18nc("verb, creation button for a new VM", "Create")
                    icon.name: "dialog-ok"
                    onTriggered: {
                        const domainConfig = {
                            name: nameField.text.trim(),
                            shortOsId: osField.text.trim(),
                            isoDiskPath: diskImageField.text,
                            memoryGB: memorySpinBox.value,
                            storageGB: storageSpinBox.value,
                            cpus: cpuSpinBox.value
                        };
                        Karton.createDomain(domainConfig);
                        showPassiveNotification(i18nc("%1 is the name of the virtual machine", "Created VM: %1", nameField.text));
                        addDomainDialog.close();
                    }
                }
        ]
        
        preferredWidth: root.width - Kirigami.Units.gridUnit * 10
        preferredHeight: root.height - Kirigami.Units.gridUnit * 10
        onAccepted: {
            console.log("VM Name:", nameField.text);
            console.log("VM Type:", vmTypeComboBox.currentText);
            showPassiveNotification(i18nc("%1 is the name of the virtual machine", "Created VM: %1", nameField.text));
        }
            

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            FormCard.FormCard {
                Layout.fillWidth: true

                FormCard.FormTextFieldDelegate {
                    id: nameField
                    label: i18nc("@label:textbox", "VM Name:")
                    placeholderText: i18n("Enter VM Name")
                    Layout.fillWidth: true
                    validator: RegularExpressionValidator {
                        regularExpression: /^[^\s]+$/ 
                    }
                }
                
                Dialogs.FileDialog {
                    id: fileDialog
                    title: i18nc("@label:filedialog", "Choose a disk image")
                    nameFilters: ["Disk images (*.qcow2 *.raw *.img *.iso *.vdi *.vmdk)"]
                    onAccepted: {
                        diskImageField.text = fileDialog.selectedFile.toString().replace("file://", "");
                        let shortOsId = getShortOsId(diskImageField.text);
                        if (shortOsId === "") {
                            osField.placeholderText = i18n("Could not identify the OS. Please enter an OS Variant.");
                            osField.text = "";
                        } else {
                            osField.text = shortOsId;
                            osField.placeholderText = i18n( "Enter an OS Variant");
                        }
                        // TODO: some kind of error with passing file path to libosinfo call. possibly rework exposing osinfoconfig
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
                            placeholderText: i18n("Select a disk image")
                            readOnly: true
                        }
                        
                        Controls.Button {
                            text: i18nc("verb, look for a file in explorer", "Browse")
                            onClicked: {
                                fileDialog.open();

                            }
                        }
                    }
                }
                FormCard.FormTextFieldDelegate {
                    id: osField
                    label: i18nc("@label:textbox", "OS Variant:")
                    placeholderText: i18n( "Enter an OS Variant")
                    Layout.fillWidth: true
                }
            }

            FormCard.FormCard {
                Layout.bottomMargin: Kirigami.Units.largeSpacing
                FormCard.FormSpinBoxDelegate {
                    id: memorySpinBox
                    label: i18nc("@label:spinbox, RAM", "Memory (GB)")
                    value: 4
                    from: 1
                    to: 64

                    Layout.fillWidth: true
                }

                FormCard.FormDelegateSeparator {}

                FormCard.FormSpinBoxDelegate {
                    id: storageSpinBox
                    label: i18nc("@label:spinbox", "Disk Storage (GB)")
                    from: 1
                    to: 2048
                    value: 4


                    Layout.fillWidth: true
                }

                FormCard.FormDelegateSeparator {}

                FormCard.FormSpinBoxDelegate {
                    id: cpuSpinBox
                    label: i18nc("@label:spinbox, number of cpus", "CPUs")
                    from: 1
                    to: 16
                    value: 2


                    Layout.fillWidth: true
                }
            }
        }
        
    }

    Component {
        id: vmViewerWindowComponent
        VMViewerWindow {}
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
                            text: domain.config.name
                        }
                        Kirigami.Separator {
                            Layout.fillWidth: true
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "UUID: " + domain.config.uuid
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "State: " + domain.config.state
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "Memory (GB): " + domain.config.maxRam
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "Memory Usage (GB): " + domain.config.ramUsage
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "CPU Cores: " + domain.config.cpus
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "Virtual Disk: " + domain.config.virtualDiskPath
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "ISO Disk: " + domain.config.isoDiskPath
                        }

                    }
                    ColumnLayout{
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: i18nc("verb, start a VM", "Start")
                            icon.name: "media-playback-start"
                            onClicked: {
                                Karton.startDomain(domain)
                                showPassiveNotification(i18nc("%1 is the name of the virtual machine", "Starting VM: %1!", domain.config.name));
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: i18nc("verb, stop a VM", "Stop")
                            icon.name: "system-shutdown"
                            onClicked: {
                                Karton.stopDomain(domain)
                                showPassiveNotification(i18nc("%1 is the name of the virtual machine", "Stopping VM: %1!", domain.config.name));
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: i18nc("verb, stop a VM immediately", "Force Stop")
                            // icon.name: "process-stop"
                            onClicked: {
                                Karton.forceStopDomain(domain)
                                showPassiveNotification(i18nc("%1 is the name of the virtual machine", "Force-stopping VM: %1!", domain.config.name));
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: i18nc("verb, open viewer for VM", "View VM")
                            icon.name: "computer-laptop-symbolic"
                            onClicked: {
                                let component = Qt.createComponent("VMViewerWindow.qml")
                                if (component.status === Component.Ready) {
                                    let window = component.createObject(null, {domain: domain})
                                    if (window) {
                                        window.show()
                                        showPassiveNotification(i18nc("%1 is the name of the virtual machine", "Opening viewer for: %1", domain.config.name))
                                    }
                                }
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: i18nc("verb, delete a VM", "Delete")
                            icon.name: "delete"
                            onClicked: {
                                if (domain.config.state === "running") {
                                    showPassiveNotification(i18nc("%1 is the name of the virtual machine", "Error: %1 is still running!", domain.config.name));
                                    return;
                                }
                                deleteConfirmationDialog.domain = domain;
                                deleteConfirmationDialog.open();
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

            text: i18nc("@title, greet user to Karton", "Welcome to Karton!")
            explanation: i18n("Create a new virtual machine to proceed.")
            icon.name: "computer-symbolic"
        }
    }

    Kirigami.Dialog {
        id: deleteConfirmationDialog
        title: i18nc("Confirm deleting %1 (virtual machine name)", "Delete '%1'?", 
            deleteConfirmationDialog.domain ? deleteConfirmationDialog.domain.config.name : "")

        padding: Kirigami.Units.largeSpacing
        preferredWidth: root.width - Kirigami.Units.gridUnit * 30
        preferredHeight: root.height - Kirigami.Units.gridUnit * 30
        
        showCloseButton: false
        standardButtons: Kirigami.Dialog.NoButton
        flatFooterButtons: false

        property var domain: null

        Controls.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: i18nc("%1 is the virtual machine name",
`You are about to remove the virtual machine, '%1'. 
Would you like to remove the disk image as well? 
This action cannot be undone.`, 
            deleteConfirmationDialog.domain.config.name)
        }

        customFooterActions: [
            Kirigami.Action {
                text: i18nc("verb, close confirmation dialog", "Cancel")
                icon.name: "dialog-cancel"
                onTriggered: {
                    deleteConfirmationDialog.domain = null;
                    deleteConfirmationDialog.close();
                }
            },
            Kirigami.Action {
                text: i18nc("action, delete VM but keep disk image", "Keep File")
                icon.name: "edit-delete-remove"
                
                onTriggered: {
                    if (deleteConfirmationDialog.domain) {
                        Karton.deleteDomain(deleteConfirmationDialog.domain, false);
                        showPassiveNotification(i18nc("%1 is the virtual machine name", "Undefining %1!", deleteConfirmationDialog.domain.config.name));
                        deleteConfirmationDialog.domain = null;
                        deleteConfirmationDialog.close();
                    }
                }
            },
            Kirigami.Action {
                text: i18nc("action, delete VM and disk image", "Delete Disk")
                icon.name: "delete"
                
                onTriggered: {
                    if (deleteConfirmationDialog.domain) {
                        Karton.deleteDomain(deleteConfirmationDialog.domain, true);
                        showPassiveNotification(i18nc("%1 is the virtual machine name", "Undefining %1!", deleteConfirmationDialog.domain.config.name));
                        deleteConfirmationDialog.domain = null;
                        deleteConfirmationDialog.close();
                    }
                }
            }
        ]
    }
}
