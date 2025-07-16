// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.karton

Kirigami.ApplicationWindow {
    id: viewerWindow
    required property Domain domain
    
    title: domain ? i18nc("%1 is the name of the virtual machine", "VM Viewer - %1", domain.config.name) : i18n("VM Viewer")
    
    width: Kirigami.Units.gridUnit * 53
    height: Kirigami.Units.gridUnit * 36
    
    Controls.Button { // full screen
        anchors.top: parent.top - 0.5
        anchors.right: parent.right
        anchors.margins: Kirigami.Units.gridUnit * 1
        
        icon.name: "view-fullscreen"
        onClicked: {
            if (viewerWindow.visibility === Window.FullScreen) {
                viewerWindow.showNormal()
            } else {
                viewerWindow.showFullScreen()
            }
        }
    }

    pageStack.initialPage: Kirigami.Page {
        title: viewerWindow.title
        padding: 0 

        DomainViewer {
            anchors.centerIn: parent
            domain: viewerWindow.domain

            width: Kirigami.Units.gridUnit * 56.55
            height: Kirigami.Units.gridUnit * 36 // TODO: expose m_imageStuff to create window from viewer size.
            scale: Math.min(parent.width / width, parent.height / height)

            focus: true
            activeFocusOnTab: true
            onActiveFocusChanged: {
                console.log("DomainViewer focus changed to:", activeFocus)
            }
            onFocusChanged: {
                console.log("DomainViewer focus property changed to:", focus)
            }
            MouseArea {
                anchors.fill: parent
                onPressed: {
                    console.log("MouseArea click. giving focus to domainviewer")
                    parent.forceActiveFocus()
                    mouse.accepted = false
                }
            }
        }
    }
}