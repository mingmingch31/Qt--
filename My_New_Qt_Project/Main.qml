import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: "惠州工控 - 第一战役"

    // 在屏幕正中间画一个按钮
    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 20//两个按钮之间的间距

        Button{
            text: "点击我！存入一条故障记录"
            width: 250
            height: 60
            font.pixelSize: 18

            // 点击事件：一旦被按下，就执行大括号里的逻辑
            onClicked: {
                // 直接呼叫刚才在 main.cpp 里定好的暗号 "myDb"
                // 并调用图纸上标了 Q_INVOKABLE 的那个对外服务窗口！
                myDb.saveRecord("危险！1号机械臂温度过高：85℃")
            }
        }

        Button{
            text: "2.调取所有历史档案"
            width:250
            height: 60
            font.pixelSize: 18

            onClicked: {
                historyList.model=myDb.getAllRecords()
            }

        }
    }

    Rectangle{
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width:320
        color:"#2b2b2b"//工业风深灰底色

        ListView{
            id:historyList
            anchors.fill: parent
            anchors.margins: 15
            clip:true
            spacing: 5

            delegate: Text{
                text: "."+modelData
                color:"#00FF00"
                font.pixelSize:15

            }


        }


    }




}