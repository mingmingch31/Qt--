//1. 搬运材料与建造大楼
import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts
import QtCharts
import LarkWorkstation 3.0
//import：搬运装修材料。前两个搬来了基础形状和按钮，最后个把我们在 C++ 里写的厂长（DeviceSimulator）搬了进来。

//QML 是一种用来画界面的标记语言，它长得像 JSON，没有 C++ 那么多指针，核心思想是“画格子”和“连线”。

//ApplicationWindow：大楼的框架。
//id：身份证号。给这个大楼起个名叫 window，以后别的代码喊 window.width 就能拿到它的宽度。
ApplicationWindow {
    id: window
    width: 1280
    height: 720
    visible: true
    title: qsTr("工业设备监控与报警系统 - Lark Workstation (多线程双核驱动架构)")

    //property：声明属性。大白话是“大楼（ApplicationWindow）我要给你加个私人物品”。
    property bool lightMode: Application.styleHints.colorScheme === Qt.ColorSchemeLight
    color: lightMode ? "#f0f0f0" : "#2d2d2d"

    LogModel { id: myLogModel }
    DeviceSimulator { id: deviceSimulator } //在大厅里，正式摆上一张名为 deviceSimulator 的桌子。
    //通过这个 id，前台就能随时使唤后台的厂长。

    property int timeSeconds: 0

    // 【关键新增】用于控制右下角红框显示“实时”还是“历史”的状态机变量
    //string：规定这个物品的数据类型是“字符串（一长串文本）”。
    //historyTextData：这是我们给它起的专属名字。
    //: ""：初始化赋值。大白话是“软件刚启动时，这块板子上什么字都还没写，是空的”。
    property bool isShowingHistory: false
    property string historyTextData: ""

    //Connections：前台桌子上的特制无线电对讲机。
    //target: deviceSimulator：把对讲机频道死死锁定在厂长身上，只听他一个人的指挥。
    Connections {
        target: deviceSimulator

        //onDataUpdated：当对讲机里传来厂长按下的 dataUpdated 信号时，立刻触发这个函数。
        //tempSeries.append(...)：在坐标系里画一个点。X 轴是时间，Y 轴是厂长黑板上的温度。
        function onDataUpdated() {
            timeSeconds++;
            tempSeries.append(timeSeconds, deviceSimulator.temperature);

            if (tempSeries.count > 65) {
                //防爆内存逻辑： 图表里的点如果一直增加，电脑会卡死。
                //at(0)：拿到最左边（最老）的那个点的对象。
                //remove(x, y)：拿着橡皮擦，精确对准那个坐标，把它擦掉。始终保持屏幕上只有 60 多个点。
                // 【绝杀修复】QML 中没有 remove(index) 函数！直接调用会导致 JS 引擎崩溃进而导致左侧页面全黑。
                // 正确做法：先获取最老那个点的坐标(x,y)，再根据坐标精准将其从画布上抹去！
                var oldPoint = tempSeries.at(0);
                tempSeries.remove(oldPoint.x, oldPoint.y);
            }

            //动画幻术逻辑： 如果运行时间超过了尺子（X轴）的最大刻度，
            //就把尺子的最小值和最大值同时向右挪动！在人眼看来，红线就是在向左滑动！
            if (timeSeconds > axisX.max) {
                axisX.min = timeSeconds - 60;
                axisX.max = timeSeconds;
            }
        }

        function onNewSystemLog(msg) {
            myLogModel.appendLog(msg);
        }

        function onHistoryLoaded(historyText) {
            // 【关键修复】不要直接去赋值破坏控件绑定，而是保存在状态变量里，让控件自己去选
            historyTextData = historyText;
            isShowingHistory = true; // 切换到历史模式
        }
    }

    //ColumnLayout：列布局。大白话：“把大厅按照从上往下的顺序，分成一排排的格子。”
    //anchors.fill: parent：锚点。大白话：“像八爪鱼一样，
    //把自己的上下左右死死吸附在大楼（父组件）的墙壁上，随着大楼变大变小而伸缩。”
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // 第一块：顶部控制栏
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: lightMode ? "#ffffff" : "#3a3a3a"
            radius: 8
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20

                Text {
                    text: "Lark Workstation (SQLite + QThread 硬件轮询引擎就绪)"
                    font.pixelSize: 24
                    font.bold: true
                    color: lightMode ? "#333333" : "#eeeeee"
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: deviceSimulator.status === "已暂停" ? "启动设备监控" : "暂停设备监控"
                    font.bold: true
                    onClicked: deviceSimulator.toggleSimulation()
                }

                Button {
                    text: "数据库故障溯源"
                    onClicked: {
                        deviceSimulator.loadHistoryFromDatabase()
                        myLogModel.appendLog("执行数据检索指令：从 SQLite 拉取历史记录...")
                    }
                }

                Button {
                    text: "清空当前报警"
                    onClicked: {
                        deviceSimulator.clearAlarmLog()
                        isShowingHistory = false // 清空时顺便切回实时模式
                    }
                }
            }
        }

        // 第二块：大分屏
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 15

            // ========= 左侧区域 =========
            ColumnLayout {
                Layout.fillWidth: true
                // 【防卫性代码】给左边强制规定一个最小宽度，防止被右侧挤没
                Layout.minimumWidth: 600
                Layout.fillHeight: true
                spacing: 15

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 100
                    color: lightMode ? "#ffffff" : "#3a3a3a"
                    radius: 8
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 25
                        spacing: 40
                        Text {
                            text: "状态: " + deviceSimulator.status
                            font.pixelSize: 22
                            font.bold: true
                            color: deviceSimulator.status === "运行中" ? "#00aa00" : (deviceSimulator.status === "已暂停" ? "#ffaa00" : "#ff0000")
                        }
                        Text {
                            text: "当前温度: " + Number(deviceSimulator.temperature).toFixed(1) + " ℃"
                            font.pixelSize: 22
                            color: lightMode ? "#333333" : "#eeeeee"
                        }
                        Text {
                            text: "核心电压: " + Number(deviceSimulator.voltage).toFixed(1) + " V"
                            font.pixelSize: 22
                            color: lightMode ? "#333333" : "#eeeeee"
                        }
                        Item { Layout.fillWidth: true }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: lightMode ? "#ffffff" : "#3a3a3a"
                    radius: 8

                    ChartView {
                        id: chart
                        anchors.fill: parent
                        title: "设备温度实时监控曲线 (多线程无阻塞)"
                        theme: lightMode ? ChartView.ChartThemeLight : ChartView.ChartThemeDark
                        antialiasing: true
                        legend.visible: false

                        ValueAxis {
                            id: axisX
                            min: 0
                            max: 60
                            titleText: "运行时间 (秒)"
                        }
                        ValueAxis {
                            id: axisY
                            min: 15
                            max: 45
                            titleText: "温度 (℃)"
                        }

                        LineSeries {
                            id: tempSeries
                            name: "实时温度"
                            axisX: axisX
                            axisY: axisY
                            width: 3
                            color: "#ff5500"
                        }
                    }
                }
            }

            // ========= 右侧区域 =========
            ColumnLayout {
                Layout.preferredWidth: 420
                Layout.fillHeight: true
                spacing: 15

                // 系统日志
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: lightMode ? "#ffffff" : "#3a3a3a"
                    radius: 8
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            color: "transparent"
                            Text {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.leftMargin: 15
                                text: "系统运行日志"
                                font.bold: true
                                font.pixelSize: 16
                                color: lightMode ? "#333333" : "#dddddd"
                            }
                        }
                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.margins: 10
                            clip: true
                            TextArea {
                                id: logArea
                                readOnly: true
                                text: myLogModel.logText
                                wrapMode: TextEdit.Wrap
                                font.family: "Consolas"
                                font.pixelSize: 13
                                color: lightMode ? "#444444" : "#cccccc"
                                background: Rectangle { color: "transparent" }
                                onTextChanged: logArea.cursorPosition = logArea.text.length
                            }
                        }
                    }
                }

                // 报警信息面板
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 250
                    color: lightMode ? "#fff5f5" : "#4a2a2a"
                    border.color: "#ff5555"
                    border.width: 1
                    radius: 8
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        // 【绝杀修复2】灵活的头部工具栏
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            color: "transparent"
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 15
                                anchors.rightMargin: 15

                                // 动态改变标题，告诉你现在到底看的是什么
                                Text {
                                    text: isShowingHistory ? "历史报警记录 (数据库离线)" : "实时报警记录 (内存监听)"
                                    font.bold: true
                                    font.pixelSize: 16
                                    color: "#ff5555"
                                }

                                Item { Layout.fillWidth: true }

                                // 只有在历史模式下，才显示“返回”按钮。一点击，立刻切回实时模式！
                                Button {
                                    text: "返回实时"
                                    visible: isShowingHistory
                                    onClicked: isShowingHistory = false
                                }
                            }
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.margins: 10
                            clip: true
                            TextArea {
                                id: alarmArea
                                readOnly: true
                                // 【核心逻辑】三元运算符：如果是历史模式，就显示历史字符串；否则，老老实实绑定 C++ 的实时字符串！
                                //text: ...：这里不是死板的赋值，而是绑定了一根“魔法皮筋”。
                                //? : （三元运算符）：这是一个选择开关。
                                //逻辑走向重点： QML 引擎会死死盯着 isShowingHistory 这个开关。
                                //如果你没点故障溯源，它是 false，皮筋就死死绑在deviceSimulator.alarmLog 上，
                                //只要厂长改了日志，这里立马刷新（实时模式）。
                                //一旦你点了故障溯源按钮，把开关改成了 true，皮筋瞬间断开，重新绑到
                                //historyTextData（历史文本）上（历史模式）。这种设计保证了界面的极度灵活
                                //性，绝对不会卡死！
                                text: isShowingHistory ? historyTextData : deviceSimulator.alarmLog
                                wrapMode: TextEdit.Wrap
                                font.family: "Consolas"
                                font.pixelSize: 13
                                color: "#ff3333"
                                background: Rectangle { color: "transparent" }
                                onTextChanged: alarmArea.cursorPosition = alarmArea.text.length
                            }
                        }
                    }
                }
            }
        }
    }
}