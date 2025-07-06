// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.karton

Kirigami.ApplicationWindow {
    id: viewerWindow
    
    property var domain: null
    
    title: domain ? i18n("VM Viewer - %1", domain.config.name) : i18n("VM Viewer")
    
    width: 1008
    height: 680
    
    Controls.Button { // full screen
        anchors.top: parent.top + 0.5
        anchors.right: parent.right
        anchors.margins: 10
        
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
            host: "localhost" // hardcoded TODO
            port: 5900 // hardcoded TODO

            width: 1008
            height: 630
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