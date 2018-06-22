import QtQuick 2.0
import RenderSurface 1.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import QtQuick.Dialogs 1.3

Item {

    width: 1200
    height: 800

    FileDialog {
        id: modelChooserDialog
        title: "Please choose a file"
        folder: shortcuts.home
        onAccepted: {
            topMenu.modelPathText = modelChooserDialog.fileUrls[0] //fileDialog.fileUrls
            Qt.quit()
        }
        onRejected: {
            Qt.quit()
        }
       // Component.onCompleted: visible = true
        visible: false
    }


    Rectangle {
        id: topMenu
        color: Qt.rgba(1, 1, 1, 0.7)
        radius: 0
        border.width: 1
        border.color: "black"
        x: 0
        y: 0
        height: 60
        width: parent.width
        property alias modelPathText: modelNameText.text

        RowLayout {
            x: 5
            y: 5
            TextField {
                Layout.preferredWidth: 400
                id: modelNameText
                placeholderText: qsTr("Enter filename...")
            }

            Button {
                text: "Browse for file"
                onClicked: {
                    modelChooserDialog.visible = true
                }
            }

            Button {
                text: "Load"
                onClicked: {
                    _modelerApp.loadModel(modelNameText.text);
                }
            }
        }
    }

    Rectangle {
        id: leftMenu
        color: Qt.rgba(1, 1, 1, 0.7)
        radius: 0
        border.width: 1
        border.color: "black"
        x: 0
        anchors.top: topMenu.bottom
        height: parent.height - topMenu.height
        width:250
    }

    RenderSurface {
        objectName: "render_surface"
        SequentialAnimation on t {
            NumberAnimation { to: 1; duration: 2500; easing.type: Easing.InQuad }
            NumberAnimation { to: 0; duration: 2500; easing.type: Easing.OutQuad }
            loops: Animation.Infinite
            running: true
        }

        anchors.left: leftMenu.right
        anchors.top: topMenu.bottom
        width: parent.width - leftMenu.width
        height: parent.height - topMenu.height

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons

           // onClicked: { console.log("Bar"); }
        }
        /*MouseArea {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 100
            onClicked: {
                console.log("Foo");
                mouse.accepted = true
            }
        }*/
    }


    /*Text {
        id: label
        color: "black"
        wrapMode: Text.WordWrap
        text: "The background here is a squircle rendered with raw OpenGL using the 'beforeRender()' signal in QQuickWindow. This text label and its border is rendered using QML"
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 20
    }*/
}
