import QtQuick 2.0
import RenderSurface 1.0

Item {

    width: 1200
    height: 800


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
