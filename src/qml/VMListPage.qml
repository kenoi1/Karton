// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    // ColumnLayout {
    //     anchors.fill:parent
    //     Item {
    //         Layout.fillHeight: true
    //     }
    //     Controls.Label {
    //         // Center label horizontally and vertically within parent object
    //         text: i18n("hi!")
    //         Layout.alignment: Qt.AlignHCenter
    //     }
    //     Controls.Button {
    //         text: "AL VM"
    //         Layout.alignment: Qt.AlignHCenter
    //         onClicked: {
    //             runVM.runVM("tree")
    //             console.log("helo")
    //         }
            
    //     }
    //     Item {
    //         Layout.fillHeight: true
    //     }
    // }


// LIST VER

    // ListView {
    //     id: vmList
        
    //     model: ListModel {
    //         ListElement { name: "Mercury"; surfaceColor: "gray" }
    //         ListElement { name: "Venus"; surfaceColor: "yellow" }
    //         ListElement { name: "Earth"; surfaceColor: "blue" }
    //         ListElement { name: "Mars"; surfaceColor: "orange" }
    //         ListElement { name: "Jupiter"; surfaceColor: "orange" }
    //         ListElement { name: "Saturn"; surfaceColor: "yellow" }
    //         ListElement { name: "Uranus"; surfaceColor: "lightBlue" }
    //         ListElement { name: "Neptune"; surfaceColor: "lightBlue" }
    //     }

    //     delegate: Kirigami.SwipeListItem {
    //         id: vmDelegate

    //         text: name
    //         width: parent.width

    //         onClicked: console.log("clicked:", name)


    //         required property string name

    //         actions: [
    //             Kirigami.Action {
    //                 icon.name: "document-decrypt"
    //                 text: qsTr("Action 1")
    //                 onTriggered: source => {
    //                     showPassiveNotification(qsTr("%1: %2 clicked").arg(listItem.title).arg(text));
    //                 }
    //             },
    //             Kirigami.Action {
    //                 icon.name: "mail-reply-sender"
    //                 text: qsTr("Action 2")
    //                 onTriggered: source => {
    //                     showPassiveNotification(qsTr("%1: %2 clicked").arg(listItem.title).arg(text));
    //                 }
    //             }
    //         ]
        // }

        // ScrollIndicator.vertical:ScrollIndicator { }
    // }
    title: "Karton Virtual Machine Manager"
    actions: [
        Kirigami.Action {
            icon.name: "list-add-symbolic"
            text: "Add"
            onTriggered: source => {
                showPassiveNotification("Add a Virtual Machine!");
                addDomainDialog.open();
            }
        }
    ]
    Kirigami.Dialog {
        id: addDomainDialog
        title: "Add New Virtual Machine"
        modal: true

        standardButtons: Dialog.Ok | Dialog.Cancel
        
        anchors.centerIn: parent
        width: Math.min(root.width - 50, 400)
        
        onAccepted: {
            console.log("VM Name:", nameField.text);
            console.log("VM Type:", vmTypeComboBox.currentText);
            showPassiveNotification("Created VM: " + nameField.text);
        }
            

        ColumnLayout {
            anchors.fill: parent
            spacing: 20
            
            Controls.Label {
                text: "VM Name:"
            }
            
            Kirigami.ActionTextField {
                id: nameField
                Layout.fillWidth: true
                placeholderText: "Enter VM name"
            }
            
            Controls.Label {
                text: "VM Type: "
            }
            
            // Kirigami.OverlayDrawer {
            //     id: vmTypeComboBox
            //     edge: Qt.BottomEdge
            //     modal: false

            //     contentItem: Controls.Label {
            //         text: "Hey"
            //     }
            // }
            
            Controls.Label {
                text: "Memory (MB): "
            }
            
            // SpinBox {
            //     id: memorySpinBox
            //     Layout.fillWidth: true
            //     from: 512
            //     to: 65536
            //     stepSize: 512
            //     value: 2048
            // }
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
                            text: model.domainName
                        }
                        Kirigami.Separator {
                            Layout.fillWidth: true
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "UUID: " + model.uuid
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "State: " + model.state
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "Memory: " + model.maxRam
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "Memory Usage: " + model.ramUsage
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "CPU Cores: " + model.cpus
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: "Disk: " + model.diskPath
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
                            onClicked: {
                                Karton.startDomain(model.domainObject)
                                showPassiveNotification("Starting VM: " + model.domainName + "!");
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: "Stop"
                            onClicked: {
                                Karton.stopDomain(model.domainObject)
                                showPassiveNotification("Stopping VM: " + model.domainName + "!");
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: "Force Stop"
                            onClicked: {
                                Karton.forceStopDomain(model.domainObject)
                                showPassiveNotification("Force-stopping VM: " + model.domainName + "!");
                            }
                        }
                        Controls.Button {
                            Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                            Layout.columnSpan: 1
                            text: "View VM"
                            onClicked: {
                                Karton.viewDomain(model.domainObject)
                                showPassiveNotification("Opening in virt-viewer: " + model.domainName + "!");
                            }
                        }
                    }
                }
            }
        }
    }

    
}
