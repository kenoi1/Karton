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
    height: 630
    
    pageStack.initialPage: Kirigami.Page {
        title: viewerWindow.title
        
        DomainViewer {
            anchors.fill: parent
            domain: viewerWindow.domain
            host: "localhost" // hardcoded TODO
            port: 5900 // hardcoded TODO

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