// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

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

    Kirigami.CardsListView {
        id: view
        model: VMModel

        delegate: Kirigami.AbstractCard {
            contentItem: Item {
                implicitWidth: delegateLayout.implicitWidth
                implicitHeight: delegateLayout.implicitHeight
                GridLayout {
                    id: delegateLayout
                    anchors {
                        left: parent.left
                        top: parent.top
                        right: parent.right
                    }
                    rowSpacing: Kirigami.Units.largeSpacing
                    columnSpacing: Kirigami.Units.largeSpacing
                    columns: width > Kirigami.Units.gridUnit * 20 ? 4 : 2
                    Kirigami.Icon {
                        source: "applications-graphics"
                        Layout.fillHeight: true
                        Layout.maximumHeight: Kirigami.Units.iconSizes.huge
                        Layout.preferredWidth: height
                    }
                    ColumnLayout {
                        Kirigami.Heading {
                            level: 2
                            text: qsTr("Product ")+ modelData
                        }
                        Kirigami.Separator {
                            Layout.fillWidth: true
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            text: qsTr("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam id risus id augue euismod accumsan.")
                        }
                    }
                    Controls.Button {
                        Layout.alignment: Qt.AlignRight|Qt.AlignVCenter
                        Layout.columnSpan: 2
                        text: qsTr("Install")
                        onClicked: showPassiveNotification("Install for Product " + modelData + " clicked");
                    }
                }
            }
        }
    }

    
}
