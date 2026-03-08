import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    width: 1180
    height: 760
    minimumWidth: 760
    minimumHeight: 560
    visible: true
    title: "Astreus - Package Downloader"
    color: "#282828"

    FontLoader {
        id: outfitFont
        source: "qrc:/qt/qml/Astreus/resources/fonts/Outfit-Variable.ttf"
    }

    readonly property string fontFamily: outfitFont.status === FontLoader.Ready ? outfitFont.name : "Sans Serif"

    readonly property color bg0: "#282828"
    readonly property color bg1: "#3c3836"
    readonly property color bg2: "#504945"
    readonly property color fg0: "#fbf1c7"
    readonly property color fg2: "#d5c4a1"
    readonly property color yellow: "#fabd2f"
    readonly property color green: "#b8bb26"
    readonly property color red: "#fb4934"
    readonly property color blue: "#83a598"
    readonly property color aqua: "#8ec07c"
    readonly property bool compactMode: width < 1100

    FileDialog {
        id: packageDialog
        title: "Choose a package file"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Packages (*.deb *.rpm *.rhel)", "All files (*)"]
        onAccepted: installerService.packageFile = selectedFile
    }

    component GruvButton: Button {
        id: btn
        font.family: root.fontFamily
        font.pixelSize: 14
        padding: 12

        contentItem: Text {
            text: btn.text
            font: btn.font
            color: btn.enabled ? root.fg0 : root.fg2
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            opacity: btn.enabled ? 1.0 : 0.55
        }

        background: Rectangle {
            radius: 10
            color: btn.enabled ? (btn.down ? root.bg2 : root.bg1) : root.bg1
            border.width: 1
            border.color: btn.enabled ? root.blue : root.bg2
        }
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: 18
        rowSpacing: 18
        columnSpacing: 18
        columns: root.compactMode ? 1 : 2

        Rectangle {
            Layout.fillWidth: root.compactMode
            Layout.preferredWidth: root.compactMode ? -1 : 340
            Layout.fillHeight: true
            color: root.bg1
            radius: 14
            border.color: root.bg2
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 14

                Label {
                    text: "Astreus"
                    font.family: root.fontFamily
                    font.pixelSize: 30
                    font.bold: true
                    color: root.yellow
                }

                Label {
                    text: "Package Downloader"
                    font.family: root.fontFamily
                    font.pixelSize: 16
                    color: root.fg2
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: root.bg2
                }

                Label {
                    text: "Distribution"
                    font.family: root.fontFamily
                    font.pixelSize: 13
                    color: root.fg2
                }

                Label {
                    Layout.fillWidth: true
                    text: installerService.distro
                    wrapMode: Text.Wrap
                    font.family: root.fontFamily
                    font.pixelSize: 16
                    color: root.fg0
                }

                Label {
                    text: "Package"
                    font.family: root.fontFamily
                    font.pixelSize: 13
                    color: root.fg2
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 72
                    radius: 10
                    color: root.bg0
                    border.color: root.bg2

                    Label {
                        anchors.fill: parent
                        anchors.margins: 10
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideMiddle
                        text: installerService.packageFile.length > 0
                              ? installerService.packageFile
                              : "No package selected"
                        font.family: root.fontFamily
                        font.pixelSize: 13
                        color: installerService.packageFile.length > 0 ? root.fg0 : root.fg2
                    }
                }

                GruvButton {
                    Layout.fillWidth: true
                    text: "Browse Package"
                    onClicked: packageDialog.open()
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: root.bg2
                }

                Label {
                    text: "Package Info"
                    font.family: root.fontFamily
                    font.pixelSize: 16
                    color: root.aqua
                }

                Label {
                    text: "Type: " + installerService.packageType
                    font.family: root.fontFamily
                    font.pixelSize: 13
                    color: root.fg0
                }

                Label {
                    text: "Name: " + (installerService.packageName.length > 0 ? installerService.packageName : "Unknown")
                    font.family: root.fontFamily
                    font.pixelSize: 13
                    color: root.fg0
                }

                Label {
                    text: installerService.packageInstalled ? "Installed: Yes" : "Installed: No"
                    font.family: root.fontFamily
                    font.pixelSize: 13
                    color: installerService.packageInstalled ? root.green : root.red
                }

                Label {
                    text: installerService.busy ? "Status: Running" : "Status: Idle"
                    font.family: root.fontFamily
                    font.pixelSize: 13
                    color: installerService.busy ? root.blue : root.fg2
                }

                Item { Layout.fillHeight: true }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: root.bg1
            radius: 14
            border.color: root.bg2
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Label {
                    text: "Actions"
                    font.family: root.fontFamily
                    font.pixelSize: 22
                    font.bold: true
                    color: root.yellow
                }

                GridLayout {
                    id: actionsGrid
                    Layout.fillWidth: true
                    columns: root.compactMode ? 2 : 6
                    rowSpacing: 10
                    columnSpacing: 10

                    GruvButton {
                        text: "Install"
                        Layout.fillWidth: true
                        enabled: installerService.validPackage && !installerService.busy
                        onClicked: installerService.installPackage()
                    }

                    GruvButton {
                        text: "Download Deps"
                        Layout.fillWidth: true
                        enabled: installerService.validPackage && !installerService.busy
                        onClicked: installerService.downloadDependencies()
                    }

                    GruvButton {
                        text: "Check Dependencies"
                        Layout.fillWidth: true
                        enabled: installerService.packageInstalled && !installerService.busy
                        onClicked: installerService.checkDependencies()
                    }

                    GruvButton {
                        text: "Uninstall"
                        Layout.fillWidth: true
                        enabled: installerService.packageInstalled && !installerService.busy
                        onClicked: installerService.uninstallPackage()
                    }

                    GruvButton {
                        text: "Cancel"
                        enabled: installerService.busy
                        onClicked: installerService.cancelRunningTask()
                    }

                    GruvButton {
                        text: "Clear Log"
                        onClicked: installerService.clearLog()
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: root.bg2
                }

                Label {
                    text: "Command Plan"
                    font.family: root.fontFamily
                    font.pixelSize: 18
                    color: root.aqua
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 84
                    color: root.bg0
                    radius: 10
                    border.color: root.bg2

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 6

                        Label {
                            Layout.fillWidth: true
                            text: "Install: " + installerService.installCommandPreview
                            wrapMode: Text.WrapAnywhere
                            font.family: root.fontFamily
                            font.pixelSize: 12
                            color: root.fg0
                        }

                        Label {
                            Layout.fillWidth: true
                            text: "Download: " + installerService.dependencyCommandPreview
                            wrapMode: Text.WrapAnywhere
                            font.family: root.fontFamily
                            font.pixelSize: 12
                            color: root.fg2
                        }
                    }
                }

                SplitView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    orientation: root.compactMode ? Qt.Vertical : Qt.Horizontal

                    Rectangle {
                        SplitView.fillWidth: true
                        SplitView.minimumWidth: root.compactMode ? 0 : 420
                        SplitView.minimumHeight: root.compactMode ? 220 : 0
                        color: root.bg0
                        radius: 10
                        border.color: root.bg2

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 8

                            Label {
                                text: "Execution Log"
                                font.family: root.fontFamily
                                font.pixelSize: 17
                                color: root.yellow
                            }

                            ScrollView {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                clip: true

                                TextArea {
                                    readOnly: true
                                    wrapMode: TextEdit.Wrap
                                    text: installerService.outputLog
                                    font.family: "monospace"
                                    font.pixelSize: 12
                                    color: root.fg0
                                    selectionColor: root.blue
                                    selectedTextColor: root.bg0
                                    background: Rectangle { color: root.bg0 }
                                }
                            }
                        }
                    }

                    Rectangle {
                        SplitView.preferredWidth: root.compactMode ? -1 : 340
                        SplitView.minimumWidth: root.compactMode ? 0 : 280
                        SplitView.minimumHeight: root.compactMode ? 180 : 0
                        color: root.bg0
                        radius: 10
                        border.color: root.bg2

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 8

                            Label {
                                text: "Documentation"
                                font.family: root.fontFamily
                                font.pixelSize: 17
                                color: root.yellow
                            }

                            ScrollView {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                clip: true

                                TextArea {
                                    readOnly: true
                                    wrapMode: TextEdit.Wrap
                                    font.family: root.fontFamily
                                    font.pixelSize: 13
                                    color: root.fg0
                                    background: Rectangle { color: root.bg0 }
                                    text:
                                        "Workflow\n" +
                                        "1. Browse and choose .deb/.rpm/.rhel\n" +
                                        "2. Confirm package info and command plan\n" +
                                        "3. Install or download dependencies\n" +
                                        "4. After install, use Check Dependencies\n" +
                                        "5. Use Uninstall if rollback is needed\n\n" +
                                        "Notes\n" +
                                        "- No manual command typing required.\n" +
                                        "- Root actions use pkexec.\n" +
                                        "- Dependency check uses installed package metadata."
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
