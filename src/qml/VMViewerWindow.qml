// VMViewerWindow.qml
import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.karton

Kirigami.ApplicationWindow {
    id: viewerWindow
    
    property var domain: null
    
    title: domain ? i18n("VM Viewer - %1", domain.config.name) : i18n("VM Viewer")
    
    width: 800
    height: 600
    
    pageStack.initialPage: Kirigami.Page {
        title: viewerWindow.title
        
        DomainViewer {
            anchors.fill: parent
            domain: viewerWindow.domain
            host: "localhost"
            port: 5900
        }
    }
}