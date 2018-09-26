import QtQuick 2.0
import RenderSurface 1.0
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2


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
        height: 38
        width: parent.width
        property alias modelPathText: modelNameText.text

        RowLayout {
            x: 5
            y: 5
            TextField {
                Layout.preferredWidth: 400
                id: modelNameText
                text: "file:///home/mark/Development/GTE/resources/models/toonlevel/mushroom/MushRoom_01.fbx"
                placeholderText: qsTr("Enter filename...")
            }

            Button {
                text: "Browse for file"
                onClicked: {
                    modelChooserDialog.visible = true
                }
            }

            Rectangle{
               height: navigation.height
               width: 15
            }

            Label {
                text: "Scale: "
            }

            TextField {
                Layout.preferredWidth: 40
                id: modelScaleText
                text: "0.05"
            }

            Rectangle{
               height: navigation.height
               width: 15
            }

            Label {
                text: "Smoothing limit: "
            }

            TextField {
                Layout.preferredWidth: 40
                id: modelSmoothingThresholdText
                text: "80"
            }

            Rectangle{
               height: navigation.height
               width: 15
            }

            CheckBox {
               id: zUpCheckbox
               text: qsTr("Z-up")
               checked: true
            }

            Rectangle{
               height: navigation.height
               width: 15
            }

            Button {
                text: "Load"
                onClicked: {
                    _modelerApp.loadModel(modelNameText.text, modelScaleText.text, modelSmoothingThresholdText.text, zUpCheckbox.checked);
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

        TreeView {
            anchors.fill: parent
            model: theModel
            alternatingRowColors: false
            style: TreeViewStyle {
                alternateBackgroundColor: 'white'
                backgroundColor: 'white'
                branchDelegate: Rectangle {
                   width: 15; height: 15
                   color: "#00FFFF00"
                   Image {
                       visible: styleData.column === 0 && styleData.hasChildren
                       anchors.fill: parent
                       anchors.verticalCenterOffset: 2
                       source: "images/arrow.png"
                       transform: Rotation {
                           origin.x: width / 2
                           origin.y: height / 2
                           angle: styleData.isExpanded ? 0 : -90
                       }
                   }

               }
            }


            rowDelegate: Rectangle {
                color: ( styleData.selected ) ? "#FF99CCFF" : "white"
            }


            itemDelegate: Rectangle {
                color: ( styleData.selected ) ? "#FF99CCFF" : "white"
                height: 20
                Text {
                    color: ( styleData.selected ) ? "black" : "black"
                    anchors.verticalCenter: parent.verticalCenter
                    text: styleData.value === undefined ? "" : styleData.value // The branches don't have a description_role so styleData.value will be undefined
                }
             }

             TableViewColumn {
                role: "name_role"
                title: "Name"
             }
        }
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
