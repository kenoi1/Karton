import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.Page {
    ColumnLayout {
        anchors.fill:parent
        Item {
            Layout.fillHeight: true
        }
        Controls.Label {
            // Center label horizontally and vertically within parent object
            text: i18n("hi!")
            Layout.alignment: Qt.AlignHCenter
        }
        Controls.Button {
            text: "AL VM"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                runVM.runVM("tree")
                console.log("helo")
            }
            
        }
        Item {
            Layout.fillHeight: true
        }
    }
}