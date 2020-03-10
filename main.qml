import QtQuick 2.7
import QtQuick.Window 2.2
import QmlOpenglItem 1.0
Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")
    QmlOpenglItem{
        anchors.fill:parent
    }
}
