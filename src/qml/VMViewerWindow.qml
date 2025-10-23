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

    onClosing: {
        domainViewer.saveFrameToDomain();
        domainViewer.disconnectFromSpice();
    }

    pageStack.initialPage: Kirigami.Page {
        title: viewerWindow.title
        padding: 0

        actions: [
            Kirigami.Action {
                icon.name: "view-fullscreen"
                onTriggered: {
                    if (viewerWindow.visibility === Window.FullScreen) {
                        viewerWindow.showNormal()
                    } else {
                        viewerWindow.showFullScreen()
                    }
                }
            }
        ]

        DomainViewer {
            id: domainViewer

            property DevicePixelRatioHelper dprHelper: DevicePixelRatioHelper {
                window: domainViewer.Window.window
            }

            // Pre-cancel out scaling, and show VM pixels at 1:1
            width: implicitWidth / dprHelper.devicePixelRatio
            height: implicitHeight / dprHelper.devicePixelRatio

            domain: viewerWindow.domain
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
